// Galaxy Frontier — AGFEnemyAIController.cpp (Phase 02)

#include "AI/AGFEnemyAIController.h"

#include "AI/AGFEnemyCharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/UGFHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "TimerManager.h"

const FName AGFEnemyAIController::BB_TargetActor = TEXT("TargetActor");
const FName AGFEnemyAIController::BB_TargetLocation = TEXT("TargetLocation");
const FName AGFEnemyAIController::BB_HomeLocation = TEXT("HomeLocation");
const FName AGFEnemyAIController::BB_State = TEXT("State");

AGFEnemyAIController::AGFEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = false;

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1500.0f;
	SightConfig->LoseSightRadius = 1800.0f;
	SightConfig->PeripheralVisionAngleDegrees = 90.0f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
	PerceptionComponent->ConfigureSense(*SightConfig);
	PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AGFEnemyAIController::OnPerceptionUpdated);
}

void AGFEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	if (APawn* Pawn = GetPawn())
	{
		HomeLocation = Pawn->GetActorLocation();
	}
}

void AGFEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	AGFEnemyCharacter* Enemy = Cast<AGFEnemyCharacter>(InPawn);
	if (!Enemy)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFEnemyAIController possessed a non-enemy pawn %s — AI disabled."),
			InPawn ? *InPawn->GetName() : TEXT("<null>"));
		return;
	}

	// Sync sight ranges with the enemy's editable values.
	if (SightConfig)
	{
		SightConfig->SightRadius = Enemy->DetectionRange;
		SightConfig->LoseSightRadius = Enemy->DetectionRange * 1.2f;
		PerceptionComponent->ConfigureSense(*SightConfig);
	}

	// Behavior Tree first: if the asset exists, it owns decisions.
	// UseBlackboard + RunBehaviorTree are the standard pair; if the
	// BT has no Blackboard asset, RunBehaviorTree fails and we fall
	// back to the timer state machine below — never silently inert.
	bool bBTStarted = false;
	if (BehaviorTreeAsset)
	{
		if (UseBlackboard(BehaviorTreeAsset->BlackboardAsset, Blackboard))
		{
			bBTStarted = RunBehaviorTree(BehaviorTreeAsset);
			if (bBTStarted && Blackboard)
			{
				Blackboard->SetValueAsVector(BB_HomeLocation, HomeLocation);
				Blackboard->SetValueAsEnum(BB_State, static_cast<uint8>(EGFEnemyState::Patrol));
			}
		}
		if (!bBTStarted)
		{
			UE_LOG(LogTemp, Warning, TEXT("[GF] AGFEnemyAIController: Behavior Tree failed to start — using fallback state machine."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFEnemyAIController: no BehaviorTreeAsset assigned — using fallback state machine (see Docs/AI.md)."));
	}

	// Fallback timer runs only when the BT did not start. It is
	// cheap (0.25 s) and event-driven perception still feeds it.
	if (!bBTStarted)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				AIUpdateTimerHandle, this, &AGFEnemyAIController::OnAIUpdateTimer, 0.25f, true);
		}
		Enemy->SetState(EGFEnemyState::Patrol);
	}
}

void AGFEnemyAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AIUpdateTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void AGFEnemyAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	AGFEnemyCharacter* Enemy = GetEnemyCharacter();
	if (!Enemy || !Actor)
	{
		return;
	}
	// Only the player is a valid target here. Other sensed actors
	// (props, other enemies) are ignored — no friendly fire.
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (Actor != PlayerPawn)
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		bHasSeenPlayer = true;
		LastKnownPlayerLocation = Actor->GetActorLocation();
		Enemy->SetState(EGFEnemyState::Chase);
		SetBlackboardState(static_cast<int32>(EGFEnemyState::Chase));
		if (Blackboard)
		{
			Blackboard->SetValueAsObject(BB_TargetActor, Actor);
			Blackboard->SetValueAsVector(BB_TargetLocation, LastKnownPlayerLocation);
		}
	}
	else
	{
		// Lost sight: search the last known location, do not teleport
		// or instantly forget — see UpdateSearch().
		Enemy->SetState(EGFEnemyState::Search);
		SetBlackboardState(static_cast<int32>(EGFEnemyState::Search));
		SearchTimeRemaining = 6.0f;
		if (Blackboard)
		{
			Blackboard->SetValueAsObject(BB_TargetActor, nullptr);
			Blackboard->SetValueAsVector(BB_TargetLocation, LastKnownPlayerLocation);
		}
	}
}

void AGFEnemyAIController::OnAIUpdateTimer()
{
	AGFEnemyCharacter* Enemy = GetEnemyCharacter();
	if (!Enemy)
	{
		return;
	}
	if (Enemy->HealthComponent && Enemy->HealthComponent->IsDead())
	{
		return; // Dead enemies take no further AI decisions.
	}
	// Retreat has priority over chase/patrol.
	if (Enemy->HealthComponent
		&& Enemy->HealthComponent->GetHealthFraction() <= Enemy->RetreatHealthFraction
		&& Enemy->RetreatHealthFraction > 0.0f)
	{
		Enemy->SetState(EGFEnemyState::Retreat);
		UpdateRetreat();
		return;
	}

	switch (Enemy->CurrentState)
	{
	case EGFEnemyState::Patrol:
	case EGFEnemyState::Idle:
		UpdatePatrol();
		break;
	case EGFEnemyState::Chase:
	case EGFEnemyState::DetectPlayer:
		UpdateChase();
		break;
	case EGFEnemyState::Search:
		UpdateSearch();
		break;
	case EGFEnemyState::Retreat:
		UpdateRetreat();
		break;
	default:
		break;
	}
}

void AGFEnemyAIController::UpdatePatrol()
{
	AGFEnemyCharacter* Enemy = GetEnemyCharacter();
	if (!Enemy)
	{
		return;
	}
	if (!bHasPatrolTarget)
	{
		if (!PickPatrolPoint(PatrolTarget))
		{
			return; // No reachable point: stay put, try again next tick.
		}
		bHasPatrolTarget = true;
	}
	const EPathFollowingRequestResult::Type Result = MoveToLocation(PatrolTarget, 50.0f, true, true, true, nullptr, true);
	if (Result == EPathFollowingRequestResult::AlreadyAtGoal
		|| FVector::Dist(Enemy->GetActorLocation(), PatrolTarget) < 100.0f)
	{
		bHasPatrolTarget = false;
	}
}

void AGFEnemyAIController::UpdateChase()
{
	AActor* Player = GetPlayerActor();
	AGFEnemyCharacter* Enemy = GetEnemyCharacter();
	if (!Player || !Enemy)
	{
		return;
	}
	const float Distance = FVector::Dist(Enemy->GetActorLocation(), Player->GetActorLocation());
	if (Distance <= Enemy->AttackRange)
	{
		Enemy->SetState(EGFEnemyState::Attack);
		SetBlackboardState(static_cast<int32>(EGFEnemyState::Attack));
		Enemy->Attack(Player);
		StopMovement();
	}
	else
	{
		MoveToActor(Player, Enemy->AttackRange * 0.5f, true, true, true, nullptr, true);
	}
}

void AGFEnemyAIController::UpdateSearch()
{
	AGFEnemyCharacter* Enemy = GetEnemyCharacter();
	if (!Enemy)
	{
		return;
	}
	SearchTimeRemaining -= 0.25f;
	if (SearchTimeRemaining <= 0.0f)
	{
		// Give up searching: return to patrol, do not keep hunting
		// a player who is long gone.
		bHasSeenPlayer = false;
		Enemy->SetState(EGFEnemyState::Patrol);
		SetBlackboardState(static_cast<int32>(EGFEnemyState::Patrol));
		return;
	}
	MoveToLocation(LastKnownPlayerLocation, 50.0f, true, true, true, nullptr, true);
}

void AGFEnemyAIController::UpdateRetreat()
{
	AGFEnemyCharacter* Enemy = GetEnemyCharacter();
	if (!Enemy)
	{
		return;
	}
	// Move away from the player, towards home. If already near home
	// and still low on health, hold position — retreating off the
	// map is worse than holding.
	AActor* Player = GetPlayerActor();
	FVector AwayDirection = FVector::ZeroVector;
	if (Player)
	{
		AwayDirection = (Enemy->GetActorLocation() - Player->GetActorLocation()).GetSafeNormal();
	}
	const FVector RetreatTarget = Enemy->GetActorLocation() + AwayDirection * Enemy->PatrolRadius * 0.5f;
	MoveToLocation(RetreatTarget, 50.0f, true, true, true, nullptr, true);
}

bool AGFEnemyAIController::PickPatrolPoint(FVector& OutPoint) const
{
	AGFEnemyCharacter* Enemy = GetEnemyCharacter();
	if (!Enemy)
	{
		return false;
	}
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());
	if (!NavSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFEnemyAIController::PickPatrolPoint — no NavigationSystem (no NavMesh in this level yet)."));
		return false;
	}
	FNavLocation NavLocation;
	if (NavSystem->GetRandomReachablePointInRadius(Enemy->GetActorLocation(), Enemy->PatrolRadius, NavLocation))
	{
		OutPoint = NavLocation.Location;
		return true;
	}
	return false;
}

void AGFEnemyAIController::SetBlackboardState(int32 StateValue)
{
	if (Blackboard)
	{
		Blackboard->SetValueAsEnum(BB_State, static_cast<uint8>(StateValue));
	}
}

AGFEnemyCharacter* AGFEnemyAIController::GetEnemyCharacter() const
{
	return Cast<AGFEnemyCharacter>(GetPawn());
}

AActor* AGFEnemyAIController::GetPlayerActor() const
{
	return UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

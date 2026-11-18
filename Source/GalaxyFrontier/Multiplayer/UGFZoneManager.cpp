// Galaxy Frontier — UGFZoneManager.cpp (Phase 06)

#include "Multiplayer/UGFZoneManager.h"

#include "Components/UGFHealthComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"
#include "Multiplayer/AGFGameMode.h"
#include "Multiplayer/AGFGameState.h"
#include "Multiplayer/AGFPlayerState.h"
#include "TimerManager.h"

UGFZoneManager::UGFZoneManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.25f;

	// The design table (Docs/BATTLE-ROYALE.md). Phase 0 is the
	// starting zone: full radius, no damage, 30 s wait.
	Phases = {
		FGFZonePhase{ 6000.0f, 30.0f, 0.0f, 0.0f },
		FGFZonePhase{ 4000.0f, 20.0f, 60.0f, 1.0f },
		FGFZonePhase{ 2500.0f, 15.0f, 45.0f, 2.0f },
		FGFZonePhase{ 1500.0f, 15.0f, 40.0f, 5.0f },
		FGFZonePhase{ 800.0f, 10.0f, 35.0f, 10.0f },
		FGFZonePhase{ 300.0f, 10.0f, 30.0f, 15.0f },
		FGFZonePhase{ 100.0f, 5.0f, 25.0f, 20.0f },
		// Final phase: radius 0, 25 DPS. The 60 s wait is a
		// backstop — at 25 DPS nobody survives long, so the
		// match ends by elimination well before it expires.
		FGFZonePhase{ 0.0f, 60.0f, 20.0f, 25.0f },
	};
	CurrentRadius = Phases.Num() > 0 ? Phases[0].RadiusCm : 6000.0f;
}

void UGFZoneManager::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
}

void UGFZoneManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DamageTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void UGFZoneManager::StartZone()
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || bZoneActive || Phases.Num() == 0)
	{
		return;
	}
	bZoneActive = true;
	CurrentPhaseIndex = 0;
	CurrentRadius = Phases[0].RadiusCm;
	PhaseTimeRemaining = Phases[0].WaitSeconds;
	bIsShrinking = false;
	ShrinkElapsed = 0.0f;
	PublishZoneToGameState();
	SetComponentTickEnabled(true);

	// Damage tick: 1 s, server only. The timer is the only
	// per-second work; the phase machine runs on the component
	// Tick (0.25 s interval, no per-frame cost).
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DamageTimerHandle, this, &UGFZoneManager::ApplyZoneDamage, 1.0f, true);
	}
}

void UGFZoneManager::StopZone()
{
	bZoneActive = false;
	SetComponentTickEnabled(false);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DamageTimerHandle);
	}
}

float UGFZoneManager::GetCurrentRadius() const
{
	return CurrentRadius;
}

float UGFZoneManager::GetCurrentDamagePerSecond() const
{
	if (CurrentPhaseIndex < 0 || CurrentPhaseIndex >= Phases.Num())
	{
		return 0.0f;
	}
	return Phases[CurrentPhaseIndex].DamagePerSecond;
}

void UGFZoneManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!bZoneActive || !GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	AdvancePhase(DeltaTime);
}

void UGFZoneManager::AdvancePhase(float DeltaTime)
{
	if (CurrentPhaseIndex < 0 || CurrentPhaseIndex >= Phases.Num())
	{
		return;
	}
	if (!bIsShrinking)
	{
		PhaseTimeRemaining -= DeltaTime;
		if (PhaseTimeRemaining > 0.0f)
		{
			return;
		}
		// Wait over: start shrinking to the next phase's radius,
		// or finish if this is the last phase.
		if (CurrentPhaseIndex + 1 >= Phases.Num())
		{
			StopZone();
			return;
		}
		const FGFZonePhase& NextPhase = Phases[CurrentPhaseIndex + 1];
		ShrinkFromRadius = CurrentRadius;
		ShrinkElapsed = 0.0f;
		bIsShrinking = true;
		// The next phase's shrink time drives the interpolation;
		// a 0 s shrink jumps straight to the new radius.
		if (NextPhase.ShrinkSeconds <= 0.0f)
		{
			CurrentRadius = NextPhase.RadiusCm;
			CurrentPhaseIndex += 1;
			PhaseTimeRemaining = Phases[CurrentPhaseIndex].WaitSeconds;
			bIsShrinking = false;
			PublishZoneToGameState();
		}
		return;
	}
	// Shrinking: linear interpolation from the old radius to the
	// next phase's radius over its ShrinkSeconds.
	const FGFZonePhase& NextPhase = Phases[CurrentPhaseIndex + 1];
	ShrinkElapsed += DeltaTime;
	const float Alpha = NextPhase.ShrinkSeconds > 0.0f
		? FMath::Clamp(ShrinkElapsed / NextPhase.ShrinkSeconds, 0.0f, 1.0f)
		: 1.0f;
	CurrentRadius = FMath::Lerp(ShrinkFromRadius, NextPhase.RadiusCm, Alpha);
	PublishZoneToGameState();
	if (Alpha >= 1.0f)
	{
		CurrentPhaseIndex += 1;
		PhaseTimeRemaining = Phases[CurrentPhaseIndex].WaitSeconds;
		bIsShrinking = false;
	}
}

void UGFZoneManager::ApplyZoneDamage()
{
	if (!bZoneActive || !GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	const float DPS = GetCurrentDamagePerSecond();
	if (DPS <= 0.0f)
	{
		return;
	}
	AGFGameState* GS = Cast<AGFGameState>(GetWorld() ? GetWorld()->GetGameState() : nullptr);
	if (!GS)
	{
		return;
	}
	AGFGameMode* GM = Cast<AGFGameMode>(GetWorld() ? GetWorld()->GetAuthGameMode() : nullptr);
	for (APlayerState* PS : GS->PlayerArray)
	{
		AGFPlayerState* GFPS = Cast<AGFPlayerState>(PS);
		if (!GFPS || !GFPS->bIsAlive)
		{
			continue;
		}
		APawn* Pawn = GFPS->GetPawn();
		if (!Pawn)
		{
			continue;
		}
		// 2D distance: the zone is a circle on the ground plane.
		const FVector ToPlayer = Pawn->GetActorLocation() - ZoneCenter;
		const float Distance2D = FVector(ToPlayer.X, ToPlayer.Y, 0.0f).Size();
		if (Distance2D <= CurrentRadius)
		{
			continue;
		}
		if (UGFHealthComponent* Health = Pawn->FindComponentByClass<UGFHealthComponent>())
		{
			if (Health->IsDead())
			{
				continue;
			}
			// Zone damage ignores shield first? No — it uses the
			// normal shield-first rule (ApplyDamage). The zone
			// has no causer actor.
			Health->ApplyDamage(DPS, nullptr);
			if (Health->IsDead() && GM)
			{
				// No killer credit for the zone.
				GM->NotifyElimination(GFPS, nullptr);
			}
		}
	}
}

void UGFZoneManager::PublishZoneToGameState() const
{
	if (AGFGameState* GS = Cast<AGFGameState>(GetWorld() ? GetWorld()->GetGameState() : nullptr))
	{
		GS->SetZone(ZoneCenter, CurrentRadius);
	}
}

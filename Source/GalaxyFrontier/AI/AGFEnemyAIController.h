// Galaxy Frontier — AGFEnemyAIController.h (Phase 02)
// Decisions for AGFEnemyCharacter. Perception (sight) feeds a
// Blackboard; a Behavior Tree asset (created in the Editor, see
// Docs/AI.md) runs when assigned. If no BT asset is assigned, a
// timer-driven fallback state machine runs the same states, so the
// enemy is never inert because of a missing asset. No Tick: the
// fallback updates on a 0.25 s timer, perception is event-driven.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AGFEnemyAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UBlackboardComponent;
class UBehaviorTree;
class AGFEnemyCharacter;

UCLASS()
class GALAXYFRONTIER_API AGFEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AGFEnemyAIController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|AI")
	TObjectPtr<UAIPerceptionComponent> PerceptionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|AI")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	// Blackboard keys (must match Docs/AI.md / the BT asset).
	static const FName BB_TargetActor;
	static const FName BB_TargetLocation;
	static const FName BB_HomeLocation;
	static const FName BB_State;

	// Last seen player location, for Investigate / Search.
	UPROPERTY(BlueprintReadOnly, Category = "GF|AI")
	FVector LastKnownPlayerLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "GF|AI")
	bool bHasSeenPlayer = false;

protected:
	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	// Fallback state machine (0.25 s timer, no Tick).
	void OnAIUpdateTimer();
	void UpdatePatrol();
	void UpdateChase();
	void UpdateSearch();
	void UpdateRetreat();
	bool PickPatrolPoint(FVector& OutPoint) const;
	void SetBlackboardState(int32 StateValue);

	AGFEnemyCharacter* GetEnemyCharacter() const;
	AActor* GetPlayerActor() const;

	FTimerHandle AIUpdateTimerHandle;
	FVector HomeLocation = FVector::ZeroVector;
	FVector PatrolTarget = FVector::ZeroVector;
	bool bHasPatrolTarget = false;
	float SearchTimeRemaining = 0.0f;
};

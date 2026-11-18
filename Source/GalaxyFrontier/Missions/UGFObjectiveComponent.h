// Galaxy Frontier — UGFObjectiveComponent.h (Phase 03)
// Attach to an actor that represents one mission stage (a trigger
// volume, a terminal, the Power Core, the Extraction Zone). When its
// condition is met in game, call CompleteObjective() — it forwards
// to UGFMissionSubsystem, which enforces stage order. This component
// never completes a stage by itself on BeginPlay.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UGFObjectiveComponent.generated.h"

UCLASS(ClassGroup = (GF), meta = (BlueprintSpawnableComponent))
class GALAXYFRONTIER_API UGFObjectiveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGFObjectiveComponent();

	// Must match an ObjectiveId in the mission data asset.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Mission")
	FName ObjectiveId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Mission")
	FText ObjectiveTitle = FText::GetEmpty();

	UPROPERTY(BlueprintReadOnly, Category = "GF|Mission")
	bool bIsCompleted = false;

	// Attempts completion via the mission subsystem. Returns false
	// if this is not the current stage (out-of-order), if there is
	// no subsystem, or if already completed.
	UFUNCTION(BlueprintCallable, Category = "GF|Mission")
	bool CompleteObjective();

	UFUNCTION(BlueprintCallable, Category = "GF|Mission")
	void ResetObjective();
};

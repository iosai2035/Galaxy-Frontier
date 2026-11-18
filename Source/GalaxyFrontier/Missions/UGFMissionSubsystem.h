// Galaxy Frontier — UGFMissionSubsystem.h (Phase 03)
// GameInstance subsystem: one active mission, stages in strict
// order. Out-of-order completion is rejected with a warning, never
// silently accepted. Progress is saved to slot "GF_MissionSave"
// (UGFMissionSaveGame) — the full player save is Phase 05.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UGFMissionSubsystem.generated.h"

class UGFMissionDataAsset;
class UGFObjectiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGFMissionObjectiveDelegate, FName, ObjectiveId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGFMissionDelegate, FName, MissionId);

UCLASS()
class GALAXYFRONTIER_API UGFMissionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "GF|Mission")
	TObjectPtr<UGFMissionDataAsset> CurrentMission;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Mission")
	int32 CurrentStageIndex = -1;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Mission")
	bool bMissionActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Mission")
	bool bMissionCompleted = false;

	UPROPERTY(BlueprintAssignable, Category = "GF|Mission")
	FGFMissionObjectiveDelegate OnObjectiveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "GF|Mission")
	FGFMissionDelegate OnMissionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "GF|Mission")
	FGFMissionDelegate OnMissionFailed;

	UFUNCTION(BlueprintCallable, Category = "GF|Mission")
	bool StartMission(UGFMissionDataAsset* MissionData);

	// Completes the CURRENT stage only if ObjectiveId matches it.
	// Returns false (and changes nothing) for a wrong id, a
	// completed/failed mission, or no active mission.
	UFUNCTION(BlueprintCallable, Category = "GF|Mission")
	bool CompleteObjective(FName ObjectiveId);

	UFUNCTION(BlueprintCallable, Category = "GF|Mission")
	void FailMission();

	UFUNCTION(BlueprintCallable, Category = "GF|Mission")
	void CompleteMission();

	UFUNCTION(BlueprintPure, Category = "GF|Mission")
	FName GetCurrentObjectiveId() const;

	UFUNCTION(BlueprintPure, Category = "GF|Mission")
	FText GetCurrentObjectiveTitle() const;

	UFUNCTION(BlueprintCallable, Category = "GF|Mission")
	bool SaveMissionProgress();

	UFUNCTION(BlueprintCallable, Category = "GF|Mission")
	bool LoadMissionProgress();

	static const FString SaveSlotName;

protected:
	void AdvanceStage();
};

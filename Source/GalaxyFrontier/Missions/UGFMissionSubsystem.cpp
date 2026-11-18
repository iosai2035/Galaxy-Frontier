// Galaxy Frontier — UGFMissionSubsystem.cpp (Phase 03)

#include "Missions/UGFMissionSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "Missions/UGFMissionDataAsset.h"
#include "Missions/UGFMissionSaveGame.h"

const FString UGFMissionSubsystem::SaveSlotName = TEXT("GF_MissionSave");

bool UGFMissionSubsystem::StartMission(UGFMissionDataAsset* MissionData)
{
	if (!MissionData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFMissionSubsystem::StartMission — null MissionData."));
		return false;
	}
	if (MissionData->Objectives.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFMissionSubsystem::StartMission — mission %s has no objectives; refusing to start."),
			*MissionData->MissionId.ToString());
		return false;
	}
	if (bMissionActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFMissionSubsystem::StartMission — mission %s already active; finish or fail it first."),
			*CurrentMission->MissionId.ToString());
		return false;
	}
	CurrentMission = MissionData;
	CurrentStageIndex = 0;
	bMissionActive = true;
	bMissionCompleted = false;
	return true;
}

bool UGFMissionSubsystem::CompleteObjective(FName ObjectiveId)
{
	if (!bMissionActive || !CurrentMission || bMissionCompleted)
	{
		return false;
	}
	if (CurrentStageIndex < 0 || CurrentStageIndex >= CurrentMission->Objectives.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFMissionSubsystem::CompleteObjective — stage index %d out of range."), CurrentStageIndex);
		return false;
	}
	const FName ExpectedId = CurrentMission->Objectives[CurrentStageIndex].ObjectiveId;
	if (ObjectiveId != ExpectedId)
	{
		// Out-of-order completion is a bug in the caller, not a
		// state to silently accept. Log it and change nothing.
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFMissionSubsystem::CompleteObjective — got %s, expected %s (stage %d). Rejected."),
			*ObjectiveId.ToString(), *ExpectedId.ToString(), CurrentStageIndex);
		return false;
	}
	OnObjectiveCompleted.Broadcast(ObjectiveId);
	AdvanceStage();
	return true;
}

void UGFMissionSubsystem::AdvanceStage()
{
	CurrentStageIndex++;
	if (CurrentStageIndex >= CurrentMission->Objectives.Num())
	{
		CompleteMission();
	}
}

void UGFMissionSubsystem::FailMission()
{
	if (!bMissionActive || bMissionCompleted)
	{
		return;
	}
	const FName MissionId = CurrentMission ? CurrentMission->MissionId : NAME_None;
	bMissionActive = false;
	CurrentStageIndex = -1;
	OnMissionFailed.Broadcast(MissionId);
}

void UGFMissionSubsystem::CompleteMission()
{
	if (!CurrentMission)
	{
		return;
	}
	bMissionCompleted = true;
	bMissionActive = false;
	OnMissionCompleted.Broadcast(CurrentMission->MissionId);
	SaveMissionProgress();
}

FName UGFMissionSubsystem::GetCurrentObjectiveId() const
{
	if (!bMissionActive || !CurrentMission
		|| CurrentStageIndex < 0 || CurrentStageIndex >= CurrentMission->Objectives.Num())
	{
		return NAME_None;
	}
	return CurrentMission->Objectives[CurrentStageIndex].ObjectiveId;
}

FText UGFMissionSubsystem::GetCurrentObjectiveTitle() const
{
	if (!bMissionActive || !CurrentMission
		|| CurrentStageIndex < 0 || CurrentStageIndex >= CurrentMission->Objectives.Num())
	{
		return FText::GetEmpty();
	}
	return CurrentMission->Objectives[CurrentStageIndex].Title;
}

bool UGFMissionSubsystem::SaveMissionProgress()
{
	if (!CurrentMission)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFMissionSubsystem::SaveMissionProgress — no mission to save."));
		return false;
	}
	UGFMissionSaveGame* Save = Cast<UGFMissionSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UGFMissionSaveGame::StaticClass()));
	if (!Save)
	{
		return false;
	}
	Save->SavedMissionId = CurrentMission->MissionId;
	Save->SavedStageIndex = CurrentStageIndex;
	Save->bMissionCompleted = bMissionCompleted;
	for (int32 i = 0; i < CurrentStageIndex && i < CurrentMission->Objectives.Num(); ++i)
	{
		Save->CompletedObjectiveIds.Add(CurrentMission->Objectives[i].ObjectiveId);
	}
	return UGameplayStatics::SaveGameToSlot(Save, SaveSlotName, 0);
}

bool UGFMissionSubsystem::LoadMissionProgress()
{
	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0);
	UGFMissionSaveGame* Save = Cast<UGFMissionSaveGame>(Loaded);
	if (!Save || Save->SavedMissionId.IsNone())
	{
		return false;
	}
	// Loading restores the stage index only if the same mission is
	// currently loaded; the mission asset itself is assigned in the
	// Editor / by the level, not by the save file.
	if (!CurrentMission || CurrentMission->MissionId != Save->SavedMissionId)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFMissionSubsystem::LoadMissionProgress — save is for mission %s, current mission is %s. Not applied."),
			*Save->SavedMissionId.ToString(),
			CurrentMission ? *CurrentMission->MissionId.ToString() : TEXT("<none>"));
		return false;
	}
	CurrentStageIndex = FMath::Clamp(Save->SavedStageIndex, 0, CurrentMission->Objectives.Num());
	bMissionCompleted = Save->bMissionCompleted;
	bMissionActive = !bMissionCompleted;
	return true;
}

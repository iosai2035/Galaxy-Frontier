// Galaxy Frontier — UGFMissionSaveGame.h (Phase 03)
// Minimal save object for mission progress only. Full player saving
// (level, health, inventory, customisation, settings) is UGFSaveGame
// in Phase 05 — this class is deliberately small so mission progress
// can be saved now without faking the full save system.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "UGFMissionSaveGame.generated.h"

UCLASS()
class GALAXYFRONTIER_API UGFMissionSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "GF|Save")
	FName SavedMissionId = NAME_None;

	UPROPERTY(BlueprintReadWrite, Category = "GF|Save")
	int32 SavedStageIndex = -1;

	UPROPERTY(BlueprintReadWrite, Category = "GF|Save")
	TArray<FName> CompletedObjectiveIds;

	UPROPERTY(BlueprintReadWrite, Category = "GF|Save")
	bool bMissionCompleted = false;
};

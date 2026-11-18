// Galaxy Frontier — UGFMissionDataAsset.h (Phase 03)
// Data-only mission definition. Logic lives in UGFMissionSubsystem /
// UGFObjectiveComponent; this asset holds the stage list, so a new
// mission is data, not a code change. The first mission is
// "The Lost Power Core" (9 stages) — see Docs/MISSIONS.md.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UGFMissionDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FGFObjectiveData
{
	GENERATED_BODY()

	// Stable id, e.g. "ReachOutpost". Used by CompleteObjective() —
	// out-of-order ids are rejected (Docs/RELIABILITY.md).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Mission")
	FName ObjectiveId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Mission")
	FText Title = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Mission")
	FText Description = FText::GetEmpty();

	// World location the HUD arrow / objective marker points at.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Mission")
	FVector ObjectiveLocation = FVector::ZeroVector;

	// True if completing this stage requires fighting an enemy
	// (stage 6 of The Lost Power Core). Used by the HUD, not by logic.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Mission")
	bool bIsCombatObjective = false;
};

UCLASS(BlueprintType)
class GALAXYFRONTIER_API UGFMissionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Mission")
	FName MissionId = TEXT("TheLostPowerCore");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Mission")
	FText MissionName = FText::FromString(TEXT("The Lost Power Core"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Mission")
	FText MissionDescription = FText::GetEmpty();

	// Stages in order. The subsystem enforces this order strictly.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Mission")
	TArray<FGFObjectiveData> Objectives;

	// Reward granted on completion (display name only here; the
	// inventory item itself is Phase 05).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Mission")
	FText CompletionRewardText = FText::FromString(TEXT("Power Core recovered — Outpost restored."));
};

// Galaxy Frontier — UGFProgressionComponent.h (Phase 05)
// Player level + experience, on the character. The curve is
// documented in Docs/EXPANSION.md and is the same for every
// player: XP to go from level L to L+1 is 100 * L (L1->2: 100,
// L2->3: 200, L3->4: 300...). Levels are cosmetic progression
// gates (planet recommendations) — they grant no combat stats,
// so a higher-level player is never stronger per-shot than a
// new one. Mission rewards are the only XP source in Phase 05
// (enemy XP is Phase 06 design, not implemented here).

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UGFProgressionComponent.generated.h"

class UGFMissionDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGFLevelUpDelegate, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGFXPChangedDelegate, int32, CurrentXP, int32, XPToNextLevel);

UCLASS(ClassGroup = (GF), meta = (BlueprintSpawnableComponent))
class GALAXYFRONTIER_API UGFProgressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGFProgressionComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Highest level in Phase 05. XP stops accumulating at the cap
	// (AddXP becomes a no-op) so saved XP can never overflow the
	// curve.
	static const int32 MaxLevel = 20;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Progression")
	int32 PlayerLevel = 1;

	// XP earned towards the next level (0 <= CurrentXP < XPToNext).
	UPROPERTY(BlueprintReadOnly, Category = "GF|Progression")
	int32 CurrentXP = 0;

	UPROPERTY(BlueprintAssignable, Category = "GF|Progression")
	FGFLevelUpDelegate OnLevelUp;

	UPROPERTY(BlueprintAssignable, Category = "GF|Progression")
	FGFXPChangedDelegate OnXPChanged;

	// XP needed to go from Level to Level+1. Pure function of the
	// level, so the save only stores level + current XP.
	UFUNCTION(BlueprintPure, Category = "GF|Progression")
	static int32 GetXPToNextLevel(int32 Level);

	// Total XP needed to reach Level from level 1 (for the HUD
	// progress bar): 50 * Level * (Level - 1).
	UFUNCTION(BlueprintPure, Category = "GF|Progression")
	static int32 GetTotalXPForLevel(int32 Level);

	// Adds XP, levelling up as many times as the amount allows.
	// Returns the XP actually added (0 at the level cap or for
	// invalid input). Fires OnLevelUp once per level gained.
	UFUNCTION(BlueprintCallable, Category = "GF|Progression")
	int32 AddXP(int32 Amount);

	// Grants a mission's completion reward (XP + one reward item)
	// exactly once per mission id. Returns false when the reward
	// was already granted or the mission has no reward.
	UFUNCTION(BlueprintCallable, Category = "GF|Progression")
	bool GrantMissionReward(UGFMissionDataAsset* MissionData);

	// Restores level + XP from a save (UGFPlayerSaveManager).
	// Clamps to 1..MaxLevel and 0..XPToNext-1; invalid input is
	// rejected and changes nothing.
	UFUNCTION(BlueprintCallable, Category = "GF|Progression")
	bool SetProgress(int32 NewLevel, int32 NewXP);

	UFUNCTION(BlueprintPure, Category = "GF|Progression")
	float GetLevelProgressFraction() const;

protected:
	// Bound to the mission subsystem's OnMissionCompleted in
	// BeginPlay. Grants the completed mission's reward once.
	UFUNCTION()
	void HandleMissionCompleted(FName MissionId);

	// Missions whose reward was already granted (prevents double
	// rewards if the subsystem ever broadcasts twice).
	TSet<FName> GrantedMissionRewards;
};

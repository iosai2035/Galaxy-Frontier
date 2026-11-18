// Galaxy Frontier — UGFSaveGame.h (Phase 05)
// The full player save. One slot ("GF_PlayerSave") holds
// everything the game needs to resume: level / XP, completed
// missions, inventory, equipment, customisation profile, active
// planet, and a reference copy of the language code.
//
// What is NOT stored here: passwords, secrets, tokens, account
// data — nothing of the kind exists in the game, and this save
// never gains a field for it. Settings (graphics, camera,
// volumes) stay in their own slot, "GF_SettingsSave"
// (UGFSettingsManager, Phase 04); the language code here is a
// reference copy only — the settings slot is authoritative.
// Mission-only progress stays in its own slot too
// ("GF_MissionSave", Phase 03); the completed-mission list here
// is the source for planet unlocks.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Inventory/UGFItemDataAsset.h"
#include "UGFSaveGame.generated.h"

UCLASS()
class GALAXYFRONTIER_API UGFSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// Save format version. Bump when a field changes meaning;
	// UGFPlayerSaveManager migrates older versions (see
	// Docs/EXPANSION.md). Current version: 1.
	static const int32 CurrentSaveVersion = 1;

	UPROPERTY(BlueprintReadWrite, Category = "GF|Save")
	int32 SaveVersion = CurrentSaveVersion;

	// --- Progression ---
	UPROPERTY(BlueprintReadWrite, Category = "GF|Save")
	int32 PlayerLevel = 1;

	UPROPERTY(BlueprintReadWrite, Category = "GF|Save")
	int32 PlayerXP = 0;

	// Mission ids completed at least once. Drives planet unlocks
	// (UGFPlanetDataAsset::IsUnlocked) and mission replay gating.
	UPROPERTY(BlueprintReadWrite, Category = "GF|Save")
	TArray<FName> CompletedMissionIds;

	// --- Inventory / equipment ---
	UPROPERTY(BlueprintReadWrite, Category = "GF|Save")
	TArray<FGFInventoryEntry> InventoryEntries;

	UPROPERTY(BlueprintReadWrite, Category = "GF|Save")
	FName EquippedPrimaryWeaponId = NAME_None;

	UPROPERTY(BlueprintReadWrite, Category = "GF|Save")
	FName EquippedSecondaryWeaponId = NAME_None;

	UPROPERTY(BlueprintReadWrite, Category = "GF|Save")
	FName EquippedShieldGeneratorId = NAME_None;

	// --- Customisation / world ---
	UPROPERTY(BlueprintReadWrite, Category = "GF|Save")
	FName CustomizationProfileId = NAME_None;

	UPROPERTY(BlueprintReadWrite, Category = "GF|Save")
	FName ActivePlanetId = TEXT("ZERON");

	// Reference copy of the language code (authoritative value
	// lives in GF_SettingsSave). Kept so a save file alone still
	// says which language the player used.
	UPROPERTY(BlueprintReadWrite, Category = "GF|Save")
	FString SavedLanguageCode = TEXT("ar");

	// Clamps every field into range (level 1..20, XP 0..next-1,
	// no negative quantities, no duplicate mission ids). Called
	// by the save manager after every load. Returns false when
	// the save is unusable at all (version newer than this build).
	UFUNCTION(BlueprintCallable, Category = "GF|Save")
	bool Validate();
};

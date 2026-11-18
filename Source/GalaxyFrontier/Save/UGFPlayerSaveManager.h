// Galaxy Frontier — UGFPlayerSaveManager.h (Phase 05)
// GameInstance subsystem: save / load / delete the player save
// (slot "GF_PlayerSave", UGFSaveGame). It gathers state from the
// player character's components and applies a loaded save back
// to them — it never invents state.
//
// Versioning: saves carry SaveVersion. A save older than
// CurrentSaveVersion is migrated (see MigrateSave); a save newer
// than this build is refused by UGFSaveGame::Validate().
// The Phase 03 mission-only slot ("GF_MissionSave") is imported
// once during migration so players with a Phase 03/04 save keep
// their completed mission.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UGFPlayerSaveManager.generated.h"

class AGFCharacter;
class UGFSaveGame;

UCLASS()
class GALAXYFRONTIER_API UGFPlayerSaveManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static const FString SaveSlotName;

	// Writes the current player state to the slot. Returns false
	// when there is no player character to save.
	UFUNCTION(BlueprintCallable, Category = "GF|Save")
	bool SaveGame();

	// Loads the slot and applies it to the player character.
	// Returns false when there is no save, the save is invalid,
	// or there is no player character yet (call again after
	// spawn — the caller owns the retry).
	UFUNCTION(BlueprintCallable, Category = "GF|Save")
	bool LoadGame();

	// Deletes the slot. Returns false when there was no save.
	UFUNCTION(BlueprintCallable, Category = "GF|Save")
	bool DeleteSave();

	UFUNCTION(BlueprintPure, Category = "GF|Save")
	bool HasSaveGame() const;

	// Builds a save object from the character's live state
	// (progression, inventory, equipment, customisation, planet).
	// Returns null when the character is null.
	UFUNCTION(BlueprintCallable, Category = "GF|Save")
	UGFSaveGame* CollectPlayerState(AGFCharacter* Character) const;

	// Applies a validated save object to the character. Returns
	// false when the save or the character is null.
	UFUNCTION(BlueprintCallable, Category = "GF|Save")
	bool ApplyPlayerState(UGFSaveGame* SaveData, AGFCharacter* Character);

	// Migrates an older save in place. Version 0 (no version
	// field — pre-Phase 05 saves) imports the completed mission
	// from the Phase 03 slot "GF_MissionSave" when present.
	UFUNCTION(BlueprintCallable, Category = "GF|Save")
	bool MigrateSave(UGFSaveGame* SaveData);

protected:
	AGFCharacter* GetPlayerCharacter() const;
};

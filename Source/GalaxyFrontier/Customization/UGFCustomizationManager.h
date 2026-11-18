// Galaxy Frontier — UGFCustomizationManager.h (Phase 05)
// GameInstance subsystem: owns the active customisation profile
// and applies it to the player character. Profiles are Data
// Assets created in the Editor (AvailableProfiles); the manager
// never invents a profile. The active profile id is what
// UGFSaveGame stores, so a save only ever references a profile
// that exists in this list.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UGFCustomizationManager.generated.h"

class AGFCharacter;
class UGFCharacterCustomizationData;

UCLASS()
class GALAXYFRONTIER_API UGFCustomizationManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// All profiles the game knows about (Editor-assigned). The
	// default profile is the first valid entry.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Customization")
	TArray<TObjectPtr<UGFCharacterCustomizationData>> AvailableProfiles;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Customization")
	TObjectPtr<UGFCharacterCustomizationData> ActiveProfile = nullptr;

	// Sets the active profile by asset. Returns false for a null
	// asset, a failed validation, or a profile outside
	// AvailableProfiles. On success the profile is applied to
	// the player character immediately.
	UFUNCTION(BlueprintCallable, Category = "GF|Customization")
	bool SetActiveProfile(UGFCharacterCustomizationData* ProfileData);

	// Sets the active profile by id (used when a save is loaded).
	// Returns false when no profile with that id exists.
	UFUNCTION(BlueprintCallable, Category = "GF|Customization")
	bool SetActiveProfileById(FName ProfileId);

	// Applies ActiveProfile to the player character now. Returns
	// false when there is no active profile or no player pawn.
	UFUNCTION(BlueprintCallable, Category = "GF|Customization")
	bool ApplyActiveProfile();

	UFUNCTION(BlueprintPure, Category = "GF|Customization")
	UGFCharacterCustomizationData* FindProfileById(FName ProfileId) const;

	UFUNCTION(BlueprintPure, Category = "GF|Customization")
	FName GetActiveProfileId() const;

protected:
	AGFCharacter* GetPlayerCharacter() const;
};

// Galaxy Frontier — UGFCharacterCustomizationData.h (Phase 05)
// One saved customisation profile, as a Data Asset created in the
// Editor (e.g. DA_Profile_Scout). A profile is a complete look:
// body, hairstyle, armour set, helmet, accessories, colours.
//
// COSMETIC ONLY: a profile changes nothing about combat, health,
// speed or damage. There are no stat bonuses, no rarity tiers and
// no paid advantages attached to any look (see Docs/EXPANSION.md).
// All characters are original designs — no real-person likeness,
// no copied character (Docs/CHARACTERS.md). Hairstyles / armour
// sets / helmets are FName ids of meshes authored in the Editor;
// this asset never contains a mesh.

#pragma once

#include "CoreMinimal.h"
#include "Characters/AGFCharacter.h"
#include "Engine/DataAsset.h"
#include "UGFCharacterCustomizationData.generated.h"

class AGFCharacter;

UCLASS(BlueprintType)
class GALAXYFRONTIER_API UGFCharacterCustomizationData : public UDataAsset
{
	GENERATED_BODY()

public:
	// Stable profile id, e.g. "Scout". Stored in UGFSaveGame —
	// never rename an id a released save may contain.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Customization")
	FName ProfileId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Customization")
	FText ProfileName = FText::GetEmpty();

	// Body, colours and material slots. Applied directly by
	// ApplyToCharacter() through AGFCharacter::ApplyCustomization().
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Customization")
	FGFCharacterCustomization Customization;

	// Hairstyle mesh id (e.g. "Buzz", "Short", "Tied", "Mohawk").
	// The mesh itself is authored in the Editor; NAME_None means
	// the character's default hair. A missing mesh is logged by
	// the Editor-side setup, never faked here.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Customization")
	FName HairstyleId = NAME_None;

	// Armour set id (e.g. "Scout", "Vanguard", "Ranger"). Meshes
	// authored in the Editor; NAME_None = default armour.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Customization")
	FName ArmourSetId = NAME_None;

	// Helmet id. NAME_None = no helmet (bare head).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Customization")
	FName HelmetId = NAME_None;

	// Accessory ids (e.g. "Visor", "Backpack", "Scarf"). Each id
	// maps to an Editor-authored attachable mesh; unknown ids are
	// ignored by the Editor-side setup with a warning.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Customization")
	TArray<FName> AccessoryIds;

	// Applies the colours / body type to the character now. Mesh
	// swaps (hair / armour / helmet / accessories) are Editor-side:
	// the character's Animation Blueprint reads HairstyleId /
	// ArmourSetId / HelmetId / AccessoryIds and swaps the assigned
	// meshes (see Docs/CHARACTERS.md). Returns false for a null
	// character.
	UFUNCTION(BlueprintCallable, Category = "GF|Customization")
	bool ApplyToCharacter(AGFCharacter* Character) const;

	// Checks the profile's own consistency (id, slot indices,
	// finite colours, no duplicate accessory ids).
	UFUNCTION(BlueprintCallable, Category = "GF|Customization")
	bool Validate() const;
};

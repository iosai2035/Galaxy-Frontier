// Galaxy Frontier — UGFCharacterCustomizationData.cpp (Phase 05)
// Colour application only. Mesh swaps are Editor-side by design.

#include "Customization/UGFCharacterCustomizationData.h"

#include "Characters/AGFCharacter.h"

bool UGFCharacterCustomizationData::ApplyToCharacter(AGFCharacter* Character) const
{
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFCharacterCustomizationData::ApplyToCharacter — null character."));
		return false;
	}
	// Colours + body type go through the character's own path
	// (dynamic material instances per slot). The mesh ids are
	// read by the Animation Blueprint in the Editor — see the
	// header comment; nothing here pretends to swap a mesh.
	Character->ApplyCustomization(Customization);
	return true;
}

bool UGFCharacterCustomizationData::Validate() const
{
	if (ProfileId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFCharacterCustomizationData::Validate — ProfileId is None."));
		return false;
	}
	if (Customization.SkinMaterialSlot < 0 || Customization.ArmourMaterialSlot < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFCharacterCustomizationData::Validate — profile %s has a negative material slot."),
			*ProfileId.ToString());
		return false;
	}
	auto IsFiniteColour = [](const FLinearColor& Colour) -> bool
	{
		return FMath::IsFinite(Colour.R) && FMath::IsFinite(Colour.G)
			&& FMath::IsFinite(Colour.B) && FMath::IsFinite(Colour.A);
	};
	if (!IsFiniteColour(Customization.SkinTone) || !IsFiniteColour(Customization.HairColour)
		|| !IsFiniteColour(Customization.ArmourTint) || !IsFiniteColour(Customization.AccentColour))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFCharacterCustomizationData::Validate — profile %s has a non-finite colour."),
			*ProfileId.ToString());
		return false;
	}
	// Duplicate accessory ids would attach the same mesh twice.
	TSet<FName> Seen;
	for (const FName& AccessoryId : AccessoryIds)
	{
		if (AccessoryId.IsNone() || Seen.Contains(AccessoryId))
		{
			UE_LOG(LogTemp, Warning, TEXT("[GF] UGFCharacterCustomizationData::Validate — profile %s has an empty or duplicate accessory id."),
				*ProfileId.ToString());
			return false;
		}
		Seen.Add(AccessoryId);
	}
	return true;
}

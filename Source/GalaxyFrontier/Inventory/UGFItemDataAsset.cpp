// Galaxy Frontier — UGFItemDataAsset.cpp (Phase 05)
// Validation only — the asset holds data, no gameplay logic.

#include "Inventory/UGFItemDataAsset.h"

#include "Weapons/UGFWeaponDataAsset.h"

bool UGFItemDataAsset::Validate() const
{
	if (ItemId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFItemDataAsset::Validate — ItemId is None; every item needs a stable id."));
		return false;
	}
	if (MaxStackSize < 1 || MaxStackSize > 999)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFItemDataAsset::Validate — item %s has MaxStackSize %d (must be 1-999)."),
			*ItemId.ToString(), MaxStackSize);
		return false;
	}
	if (!FMath::IsFinite(EffectAmount) || EffectAmount < 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFItemDataAsset::Validate — item %s has an invalid EffectAmount %f."),
			*ItemId.ToString(), EffectAmount);
		return false;
	}
	if (!FMath::IsFinite(ShieldBonus) || ShieldBonus < 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFItemDataAsset::Validate — item %s has an invalid ShieldBonus %f."),
			*ItemId.ToString(), ShieldBonus);
		return false;
	}
	// A consumable must do something, and a use effect must belong
	// to a consumable — otherwise the UI would offer a dead button.
	if (bIsConsumable && UseEffect == EGFItemUseEffect::None)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFItemDataAsset::Validate — consumable item %s has UseEffect None."),
			*ItemId.ToString());
		return false;
	}
	if (!bIsConsumable && UseEffect != EGFItemUseEffect::None)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFItemDataAsset::Validate — non-consumable item %s has a UseEffect; it can never fire."),
			*ItemId.ToString());
		return false;
	}
	if (ItemType == EGFItemType::Weapon && !WeaponData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFItemDataAsset::Validate — weapon item %s has no WeaponData."),
			*ItemId.ToString());
		return false;
	}
	// Mission items and weapons never stack; a stack size above 1
	// on them is a data error, not a feature.
	if ((ItemType == EGFItemType::Weapon || ItemType == EGFItemType::MissionItem) && MaxStackSize != 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFItemDataAsset::Validate — item %s must have MaxStackSize 1, not %d."),
			*ItemId.ToString(), MaxStackSize);
		return false;
	}
	if (ItemType == EGFItemType::ShieldGenerator && ShieldBonus <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFItemDataAsset::Validate — shield generator %s has ShieldBonus 0; equipping it would do nothing."),
			*ItemId.ToString());
		return false;
	}
	return true;
}

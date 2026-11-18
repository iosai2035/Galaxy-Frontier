// Galaxy Frontier — UGFEquipmentComponent.h (Phase 05)
// Three equipment slots: primary weapon, secondary weapon, shield
// generator. Equipment is data-driven (UGFItemDataAsset) and the
// player must own the item to equip it — equipping never creates
// items. Cosmetic-only rule: no equipped item grants a stat bonus
// except the shield generator's documented MaxShield bonus; there
// are no rarity-based bonuses and no paid stats anywhere.
//
// Weapon equipping works through UGFCombatComponent: the equipped
// weapon must already be in the combat loadout (its class listed
// in WeaponClasses in the Editor). Equipping a weapon item whose
// class is not in the loadout returns false with a warning — the
// fix is an Editor setup step, not a silent spawn (see
// Docs/EXPANSION.md).

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UGFEquipmentComponent.generated.h"

class UGFItemDataAsset;

UENUM(BlueprintType)
enum class EGFEquipmentSlot : uint8
{
	PrimaryWeapon	UMETA(DisplayName = "Primary Weapon"),
	SecondaryWeapon	UMETA(DisplayName = "Secondary Weapon"),
	ShieldGenerator	UMETA(DisplayName = "Shield Generator")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGFEquipmentChangedDelegate, FName, SlotId, FName, ItemId);

UCLASS(ClassGroup = (GF), meta = (BlueprintSpawnableComponent))
class GALAXYFRONTIER_API UGFEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGFEquipmentComponent();

public:
	// Equipped item ids (NAME_None = slot empty). The inventory
	// still owns the item — equipping does not remove it from the
	// inventory, so unequipping can never lose an item.
	UPROPERTY(BlueprintReadOnly, Category = "GF|Equipment")
	FName EquippedPrimaryWeaponId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Equipment")
	FName EquippedSecondaryWeaponId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Equipment")
	FName EquippedShieldGeneratorId = NAME_None;

	UPROPERTY(BlueprintAssignable, Category = "GF|Equipment")
	FGFEquipmentChangedDelegate OnEquipmentChanged;

	// Equips a weapon item into a weapon slot. Returns false when
	// the item is not a weapon, is not owned, or its weapon class
	// is not in the combat loadout. On success the combat
	// component switches to that weapon.
	UFUNCTION(BlueprintCallable, Category = "GF|Equipment")
	bool EquipWeapon(UGFItemDataAsset* ItemData, EGFEquipmentSlot Slot);

	// Equips a shield generator. Applies its ShieldBonus to
	// MaxShield while equipped. Returns false for a non-generator
	// item, an unowned item, or a second generator while one is
	// already equipped (unequip first).
	UFUNCTION(BlueprintCallable, Category = "GF|Equipment")
	bool EquipShieldGenerator(UGFItemDataAsset* ItemData);

	// Clears a slot. For the shield slot this removes the
	// MaxShield bonus and clamps current shield to the new max.
	UFUNCTION(BlueprintCallable, Category = "GF|Equipment")
	void UnequipSlot(EGFEquipmentSlot Slot);

	UFUNCTION(BlueprintCallable, Category = "GF|Equipment")
	void UnequipAll();

	UFUNCTION(BlueprintPure, Category = "GF|Equipment")
	FName GetEquippedItemId(EGFEquipmentSlot Slot) const;

	UFUNCTION(BlueprintPure, Category = "GF|Equipment")
	bool IsSlotEmpty(EGFEquipmentSlot Slot) const { return GetEquippedItemId(Slot).IsNone(); }

protected:
	// Applies (+1) or removes (-1) a shield generator's bonus.
	// Returns false when there is no health component.
	bool ApplyShieldBonus(float Bonus, bool bAdd);

	FName& GetSlotRef(EGFEquipmentSlot Slot);
	const FName& GetSlotRef(EGFEquipmentSlot Slot) const;
};

// Galaxy Frontier — UGFEquipmentComponent.cpp (Phase 05)
// Slot rules: own the item, match the slot type, and (for weapons)
// be in the combat loadout. Nothing here spawns items or weapons.

#include "Inventory/UGFEquipmentComponent.h"

#include "Combat/UGFCombatComponent.h"
#include "Components/UGFHealthComponent.h"
#include "Inventory/UGFInventoryComponent.h"
#include "Inventory/UGFItemDataAsset.h"
#include "Weapons/AGFWeaponBase.h"
#include "Weapons/UGFWeaponDataAsset.h"

UGFEquipmentComponent::UGFEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UGFEquipmentComponent::EquipWeapon(UGFItemDataAsset* ItemData, EGFEquipmentSlot Slot)
{
	if (Slot == EGFEquipmentSlot::ShieldGenerator)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFEquipmentComponent::EquipWeapon — use EquipShieldGenerator for the shield slot."));
		return false;
	}
	AActor* Owner = GetOwner();
	if (!Owner || !ItemData)
	{
		return false;
	}
	if (ItemData->ItemType != EGFItemType::Weapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFEquipmentComponent::EquipWeapon — %s is not a weapon item."),
			*ItemData->ItemId.ToString());
		return false;
	}
	UGFInventoryComponent* Inventory = Owner->FindComponentByClass<UGFInventoryComponent>();
	if (!Inventory || !Inventory->HasItem(ItemData->ItemId, 1))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFEquipmentComponent::EquipWeapon — %s is not owned; equip refused."),
			*ItemData->ItemId.ToString());
		return false;
	}
	UGFCombatComponent* Combat = Owner->FindComponentByClass<UGFCombatComponent>();
	if (!Combat)
	{
		return false;
	}
	// Find the spawned weapon with the same WeaponData. A weapon
	// item with no spawned actor is an Editor setup gap (its class
	// is missing from WeaponClasses) — equipping must not spawn a
	// weapon behind the combat component's back.
	int32 WeaponIndex = -1;
	for (int32 i = 0; i < Combat->Weapons.Num(); ++i)
	{
		AGFWeaponBase* Weapon = Combat->Weapons[i];
		if (Weapon && Weapon->WeaponData == ItemData->WeaponData)
		{
			WeaponIndex = i;
			break;
		}
	}
	if (WeaponIndex < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFEquipmentComponent::EquipWeapon — weapon %s has no spawned actor in the loadout; add its class to WeaponClasses in the Editor."),
			*ItemData->ItemId.ToString());
		return false;
	}
	if (!Combat->SwitchWeapon(WeaponIndex))
	{
		return false;
	}
	GetSlotRef(Slot) = ItemData->ItemId;
	OnEquipmentChanged.Broadcast(
		Slot == EGFEquipmentSlot::PrimaryWeapon ? TEXT("PrimaryWeapon") : TEXT("SecondaryWeapon"),
		ItemData->ItemId);
	return true;
}

bool UGFEquipmentComponent::EquipShieldGenerator(UGFItemDataAsset* ItemData)
{
	AActor* Owner = GetOwner();
	if (!Owner || !ItemData)
	{
		return false;
	}
	if (ItemData->ItemType != EGFItemType::ShieldGenerator)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFEquipmentComponent::EquipShieldGenerator — %s is not a shield generator."),
			*ItemData->ItemId.ToString());
		return false;
	}
	UGFInventoryComponent* Inventory = Owner->FindComponentByClass<UGFInventoryComponent>();
	if (!Inventory || !Inventory->HasItem(ItemData->ItemId, 1))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFEquipmentComponent::EquipShieldGenerator — %s is not owned; equip refused."),
			*ItemData->ItemId.ToString());
		return false;
	}
	if (!EquippedShieldGeneratorId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFEquipmentComponent::EquipShieldGenerator — a generator is already equipped; unequip it first."));
		return false;
	}
	if (!ApplyShieldBonus(ItemData->ShieldBonus, true))
	{
		return false;
	}
	EquippedShieldGeneratorId = ItemData->ItemId;
	OnEquipmentChanged.Broadcast(TEXT("ShieldGenerator"), ItemData->ItemId);
	return true;
}

void UGFEquipmentComponent::UnequipSlot(EGFEquipmentSlot Slot)
{
	const FName ItemId = GetEquippedItemId(Slot);
	if (ItemId.IsNone())
	{
		return;
	}
	if (Slot == EGFEquipmentSlot::ShieldGenerator)
	{
		// The bonus is removed from the item that is actually
		// equipped. Find its data through the inventory (the item
		// is still owned — equipping never removes it).
		AActor* Owner = GetOwner();
		UGFInventoryComponent* Inventory = Owner ? Owner->FindComponentByClass<UGFInventoryComponent>() : nullptr;
		UGFItemDataAsset* ItemData = Inventory ? Inventory->FindItemData(ItemId) : nullptr;
		if (ItemData)
		{
			ApplyShieldBonus(ItemData->ShieldBonus, false);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[GF] UGFEquipmentComponent::UnequipSlot — equipped generator %s is no longer in the inventory; its bonus cannot be removed cleanly."),
				*ItemId.ToString());
		}
	}
	GetSlotRef(Slot) = NAME_None;
	OnEquipmentChanged.Broadcast(
		Slot == EGFEquipmentSlot::PrimaryWeapon ? TEXT("PrimaryWeapon")
		: Slot == EGFEquipmentSlot::SecondaryWeapon ? TEXT("SecondaryWeapon")
		: TEXT("ShieldGenerator"), NAME_None);
}

void UGFEquipmentComponent::UnequipAll()
{
	UnequipSlot(EGFEquipmentSlot::PrimaryWeapon);
	UnequipSlot(EGFEquipmentSlot::SecondaryWeapon);
	UnequipSlot(EGFEquipmentSlot::ShieldGenerator);
}

FName UGFEquipmentComponent::GetEquippedItemId(EGFEquipmentSlot Slot) const
{
	return GetSlotRef(Slot);
}

bool UGFEquipmentComponent::ApplyShieldBonus(float Bonus, bool bAdd)
{
	AActor* Owner = GetOwner();
	UGFHealthComponent* Health = Owner ? Owner->FindComponentByClass<UGFHealthComponent>() : nullptr;
	if (!Health || Bonus <= 0.0f || !FMath::IsFinite(Bonus))
	{
		return false;
	}
	const float SignedBonus = bAdd ? Bonus : -Bonus;
	Health->MaxShield = FMath::Max(0.0f, Health->MaxShield + SignedBonus);
	// Removing the bonus must never leave current shield above the
	// new maximum; adding it never heals (current shield is
	// unchanged — the generator raises the ceiling, not the charge).
	Health->CurrentShield = FMath::Clamp(Health->CurrentShield, 0.0f, Health->MaxShield);
	return true;
}

FName& UGFEquipmentComponent::GetSlotRef(EGFEquipmentSlot Slot)
{
	switch (Slot)
	{
	case EGFEquipmentSlot::PrimaryWeapon: return EquippedPrimaryWeaponId;
	case EGFEquipmentSlot::SecondaryWeapon: return EquippedSecondaryWeaponId;
	case EGFEquipmentSlot::ShieldGenerator: default: return EquippedShieldGeneratorId;
	}
}

const FName& UGFEquipmentComponent::GetSlotRef(EGFEquipmentSlot Slot) const
{
	switch (Slot)
	{
	case EGFEquipmentSlot::PrimaryWeapon: return EquippedPrimaryWeaponId;
	case EGFEquipmentSlot::SecondaryWeapon: return EquippedSecondaryWeaponId;
	case EGFEquipmentSlot::ShieldGenerator: default: return EquippedShieldGeneratorId;
	}
}

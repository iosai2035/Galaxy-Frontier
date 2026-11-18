// Galaxy Frontier — UGFInventoryComponent.cpp (Phase 05)
// Slot/stack enforcement. Every public entry point validates its
// input; invalid input changes nothing and returns 0 / false.

#include "Inventory/UGFInventoryComponent.h"

#include "Combat/UGFCombatComponent.h"
#include "Components/UGFHealthComponent.h"
#include "Weapons/AGFWeaponBase.h"

UGFInventoryComponent::UGFInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

int32 UGFInventoryComponent::AddItem(UGFItemDataAsset* ItemData, int32 Quantity)
{
	if (!ItemData || Quantity <= 0)
	{
		return 0;
	}
	if (!ItemData->Validate())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFInventoryComponent::AddItem — item %s failed validation; refused."),
			*ItemData->ItemId.ToString());
		return 0;
	}

	int32 Remaining = Quantity;
	const int32 MaxStack = ItemData->MaxStackSize;

	// Fill partial stacks of the same item first.
	while (Remaining > 0)
	{
		const int32 StackIndex = FindStackWithSpace(ItemData);
		if (StackIndex < 0)
		{
			break;
		}
		const int32 Space = MaxStack - Entries[StackIndex].Quantity;
		const int32 ToAdd = FMath::Min(Remaining, Space);
		Entries[StackIndex].Quantity += ToAdd;
		Remaining -= ToAdd;
	}

	// Then open new slots.
	while (Remaining > 0)
	{
		if (Entries.Num() >= MaxSlots)
		{
			break;
		}
		FGFInventoryEntry NewEntry;
		NewEntry.ItemData = ItemData;
		NewEntry.Quantity = FMath::Min(Remaining, MaxStack);
		Entries.Add(NewEntry);
		Remaining -= NewEntry.Quantity;
	}

	const int32 Added = Quantity - Remaining;
	if (Added > 0)
	{
		BroadcastChange(ItemData->ItemId);
	}
	if (Remaining > 0)
	{
		// The inventory is full (or the item cannot stack further).
		// The caller keeps the un-added units — nothing is dropped.
		OnInventoryFull.Broadcast();
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFInventoryComponent::AddItem — inventory full: added %d of %d %s."),
			Added, Quantity, *ItemData->ItemId.ToString());
	}
	return Added;
}

bool UGFInventoryComponent::CanAddItem(UGFItemDataAsset* ItemData, int32 Quantity) const
{
	if (!ItemData || Quantity <= 0)
	{
		return false;
	}
	int32 FreeSpace = 0;
	for (const FGFInventoryEntry& Entry : Entries)
	{
		if (Entry.ItemData == ItemData)
		{
			FreeSpace += ItemData->MaxStackSize - Entry.Quantity;
		}
	}
	const int32 FreeSlots = MaxSlots - Entries.Num();
	FreeSpace += FreeSlots * ItemData->MaxStackSize;
	return FreeSpace >= Quantity;
}

int32 UGFInventoryComponent::RemoveItem(FName ItemId, int32 Quantity)
{
	if (ItemId.IsNone() || Quantity <= 0)
	{
		return 0;
	}
	int32 Remaining = Quantity;
	while (Remaining > 0)
	{
		const int32 Index = FindEntryIndex(ItemId);
		if (Index < 0)
		{
			break;
		}
		const int32 ToRemove = FMath::Min(Remaining, Entries[Index].Quantity);
		Entries[Index].Quantity -= ToRemove;
		Remaining -= ToRemove;
		if (Entries[Index].Quantity <= 0)
		{
			Entries.RemoveAt(Index);
		}
	}
	const int32 Removed = Quantity - Remaining;
	if (Removed > 0)
	{
		BroadcastChange(ItemId);
	}
	return Removed;
}

bool UGFInventoryComponent::UseItem(FName ItemId)
{
	if (ItemId.IsNone())
	{
		return false;
	}
	const int32 Index = FindEntryIndex(ItemId);
	if (Index < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFInventoryComponent::UseItem — %s is not owned."), *ItemId.ToString());
		return false;
	}
	UGFItemDataAsset* ItemData = Entries[Index].ItemData;
	if (!ItemData || !ItemData->bIsConsumable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFInventoryComponent::UseItem — %s is not consumable."), *ItemId.ToString());
		return false;
	}
	// Apply first, consume second: a failed effect (full health,
	// no weapon for ammo) never eats the item. RestoreHealth /
	// RestoreShield return 0 at full value, which counts as
	// "cannot apply" here so the player does not waste a pack.
	if (!ApplyItemEffect(ItemData))
	{
		return false;
	}
	RemoveItem(ItemId, 1);
	OnItemUsed.Broadcast(ItemId);
	return true;
}

void UGFInventoryComponent::ClearInventory()
{
	Entries.Empty();
	// No single-item broadcast makes sense for a full clear; the
	// widget re-reads the whole inventory on the next change or
	// on open (WBP_Inventory reads Entries on construct).
	OnInventoryChanged.Broadcast(NAME_None, 0);
}

int32 UGFInventoryComponent::GetQuantity(FName ItemId) const
{
	int32 Total = 0;
	for (const FGFInventoryEntry& Entry : Entries)
	{
		if (Entry.ItemData && Entry.ItemData->ItemId == ItemId)
		{
			Total += Entry.Quantity;
		}
	}
	return Total;
}

bool UGFInventoryComponent::HasItem(FName ItemId, int32 Quantity) const
{
	return Quantity > 0 && GetQuantity(ItemId) >= Quantity;
}

int32 UGFInventoryComponent::GetFreeSlotCount() const
{
	return FMath::Max(0, MaxSlots - Entries.Num());
}

UGFItemDataAsset* UGFInventoryComponent::FindItemData(FName ItemId) const
{
	const int32 Index = FindEntryIndex(ItemId);
	return Index >= 0 ? Entries[Index].ItemData : nullptr;
}

bool UGFInventoryComponent::ApplyItemEffect(UGFItemDataAsset* ItemData)
{
	AActor* Owner = GetOwner();
	if (!Owner || !ItemData)
	{
		return false;
	}
	switch (ItemData->UseEffect)
	{
	case EGFItemUseEffect::RestoreHealth:
	{
		UGFHealthComponent* Health = Owner->FindComponentByClass<UGFHealthComponent>();
		if (!Health || Health->IsDead())
		{
			return false;
		}
		// RestoreHealth returns 0 at full health — using a pack
		// then would waste it, so that counts as "cannot apply".
		return Health->RestoreHealth(ItemData->EffectAmount) > 0.0f;
	}
	case EGFItemUseEffect::RestoreShield:
	{
		UGFHealthComponent* Health = Owner->FindComponentByClass<UGFHealthComponent>();
		if (!Health || Health->IsDead())
		{
			return false;
		}
		return Health->RestoreShield(ItemData->EffectAmount) > 0.0f;
	}
	case EGFItemUseEffect::AddAmmo:
	{
		UGFCombatComponent* Combat = Owner->FindComponentByClass<UGFCombatComponent>();
		AGFWeaponBase* Weapon = Combat ? Combat->GetCurrentWeapon() : nullptr;
		if (!Weapon)
		{
			UE_LOG(LogTemp, Warning, TEXT("[GF] UGFInventoryComponent::ApplyItemEffect — no current weapon for ammo."));
			return false;
		}
		const int32 Rounds = FMath::Max(0, FMath::RoundToInt(ItemData->EffectAmount));
		if (Rounds <= 0)
		{
			return false;
		}
		Weapon->SetAmmo(Weapon->GetAmmo(), Weapon->GetReserveAmmo() + Rounds);
		return true;
	}
	case EGFItemUseEffect::None:
	default:
		return false;
	}
}

int32 UGFInventoryComponent::FindEntryIndex(FName ItemId) const
{
	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		if (Entries[i].ItemData && Entries[i].ItemData->ItemId == ItemId)
		{
			return i;
		}
	}
	return -1;
}

int32 UGFInventoryComponent::FindStackWithSpace(UGFItemDataAsset* ItemData) const
{
	if (!ItemData)
	{
		return -1;
	}
	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		if (Entries[i].ItemData == ItemData && Entries[i].Quantity < ItemData->MaxStackSize)
		{
			return i;
		}
	}
	return -1;
}

void UGFInventoryComponent::BroadcastChange(FName ItemId)
{
	OnInventoryChanged.Broadcast(ItemId, GetQuantity(ItemId));
}

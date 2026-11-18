// Galaxy Frontier — UGFInventoryComponent.h (Phase 05)
// The player's inventory: a fixed number of slots, each holding
// one stack of one item. Stacking, caps and overflow are enforced
// here — callers never touch the Entries array directly. The
// component knows nothing about widgets; it broadcasts changes
// and WBP_Inventory (Docs/UI-SCREENS.md) listens.
//
// Rules (Docs/EXPANSION.md):
// - MaxSlots = 24. Adding past the cap adds nothing and fires
//   OnInventoryFull — items are never silently dropped.
// - A stack never exceeds its item's MaxStackSize.
// - UseItem works only on consumables the player actually owns;
//   one use consumes exactly one unit.
// - No weight system in Phase 05: slots are the only limit.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/UGFItemDataAsset.h"
#include "UGFInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGFInventoryChangedDelegate, FName, ItemId, int32, NewQuantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGFItemUsedDelegate, FName, ItemId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGFInventoryFullDelegate);

UCLASS(ClassGroup = (GF), meta = (BlueprintSpawnableComponent))
class GALAXYFRONTIER_API UGFInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGFInventoryComponent();

public:
	// Slots in the inventory. 24 is the Phase 05 cap (see
	// Docs/EXPANSION.md) — raising it needs a HUD layout check,
	// because WBP_Inventory shows a fixed grid.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Inventory", meta = (ClampMin = "1", ClampMax = "64"))
	int32 MaxSlots = 24;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Inventory")
	TArray<FGFInventoryEntry> Entries;

	UPROPERTY(BlueprintAssignable, Category = "GF|Inventory")
	FGFInventoryChangedDelegate OnInventoryChanged;

	UPROPERTY(BlueprintAssignable, Category = "GF|Inventory")
	FGFItemUsedDelegate OnItemUsed;

	UPROPERTY(BlueprintAssignable, Category = "GF|Inventory")
	FGFInventoryFullDelegate OnInventoryFull;

	// Adds up to Quantity of ItemData, filling partial stacks
	// first, then empty slots. Returns how many were actually
	// added (0 when the inventory is full or the input is
	// invalid). Never adds a partial stack beyond MaxStackSize.
	UFUNCTION(BlueprintCallable, Category = "GF|Inventory")
	int32 AddItem(UGFItemDataAsset* ItemData, int32 Quantity = 1);

	// True when Quantity of ItemData would fit right now.
	UFUNCTION(BlueprintCallable, Category = "GF|Inventory")
	bool CanAddItem(UGFItemDataAsset* ItemData, int32 Quantity = 1) const;

	// Removes up to Quantity of ItemId. Returns how many were
	// actually removed (0 when the player owns none).
	UFUNCTION(BlueprintCallable, Category = "GF|Inventory")
	int32 RemoveItem(FName ItemId, int32 Quantity = 1);

	// Uses one unit of ItemId (consumables only). Returns false
	// and changes nothing for a non-consumable, an unknown id,
	// or zero owned. The effect is applied by ApplyItemEffect().
	UFUNCTION(BlueprintCallable, Category = "GF|Inventory")
	bool UseItem(FName ItemId);

	// Removes every entry. Used when a save is loaded (the
	// loaded entries replace the current ones).
	UFUNCTION(BlueprintCallable, Category = "GF|Inventory")
	void ClearInventory();

	UFUNCTION(BlueprintPure, Category = "GF|Inventory")
	int32 GetQuantity(FName ItemId) const;

	UFUNCTION(BlueprintPure, Category = "GF|Inventory")
	bool HasItem(FName ItemId, int32 Quantity = 1) const;

	UFUNCTION(BlueprintPure, Category = "GF|Inventory")
	int32 GetUsedSlotCount() const { return Entries.Num(); }

	UFUNCTION(BlueprintPure, Category = "GF|Inventory")
	int32 GetFreeSlotCount() const;

	UFUNCTION(BlueprintPure, Category = "GF|Inventory")
	UGFItemDataAsset* FindItemData(FName ItemId) const;

protected:
	// Applies one unit's effect to the owner. Returns false when
	// the effect cannot apply (no health component, no current
	// weapon for ammo) — the unit is NOT consumed in that case.
	bool ApplyItemEffect(UGFItemDataAsset* ItemData);

	int32 FindEntryIndex(FName ItemId) const;
	int32 FindStackWithSpace(UGFItemDataAsset* ItemData) const;
	void BroadcastChange(FName ItemId);
};

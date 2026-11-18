// Galaxy Frontier — UGFItemDataAsset.h (Phase 05)
// Data-only item definition. Logic lives in UGFInventoryComponent /
// UGFEquipmentComponent; this asset holds the numbers, so new items
// are data (Editor Data Assets, e.g. DA_HealthPack), not code.
// Item instances are created in the Editor — no .uasset is faked here.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UGFItemDataAsset.generated.h"

class UGFWeaponDataAsset;
class AGFWeaponBase;

UENUM(BlueprintType)
enum class EGFItemType : uint8
{
	Weapon			UMETA(DisplayName = "Weapon"),
	Ammo			UMETA(DisplayName = "Ammo"),
	HealthPack		UMETA(DisplayName = "Health Pack"),
	ShieldCell		UMETA(DisplayName = "Shield Cell"),
	ShieldGenerator	UMETA(DisplayName = "Shield Generator"),
	MissionItem		UMETA(DisplayName = "Mission Item"),
	Material		UMETA(DisplayName = "Material")
};

UENUM(BlueprintType)
enum class EGFItemRarity : uint8
{
	Common		UMETA(DisplayName = "Common"),
	Uncommon	UMETA(DisplayName = "Uncommon"),
	Rare		UMETA(DisplayName = "Rare"),
	Epic		UMETA(DisplayName = "Epic")
};

// What using a consumable item does. Rarity is cosmetic only:
// it never changes these numbers (see Docs/EXPANSION.md — no
// pay-to-win and no rarity-based stat bonuses anywhere).
UENUM(BlueprintType)
enum class EGFItemUseEffect : uint8
{
	None			UMETA(DisplayName = "None"),
	RestoreHealth	UMETA(DisplayName = "Restore Health"),
	RestoreShield	UMETA(DisplayName = "Restore Shield"),
	AddAmmo			UMETA(DisplayName = "Add Ammo (current weapon)")
};

// One inventory slot. The inventory stores entries, not loose
// counts, so a stack's item data always travels with its quantity.
USTRUCT(BlueprintType)
struct FGFInventoryEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Inventory")
	TObjectPtr<UGFItemDataAsset> ItemData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Inventory", meta = (ClampMin = "0"))
	int32 Quantity = 0;
};

UCLASS(BlueprintType)
class GALAXYFRONTIER_API UGFItemDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// Stable id, e.g. "HealthPack". Used by the inventory, the
	// equipment slots and the save file — never rename an id that
	// a released save may contain (see Docs/EXPANSION.md).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Item")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Item")
	FText DisplayName = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Item")
	FText Description = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Item")
	EGFItemType ItemType = EGFItemType::Material;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Item")
	EGFItemRarity Rarity = EGFItemRarity::Common;

	// Largest stack in one inventory slot. Weapons and mission
	// items are 1 (no stacking); Health/Shield 5; Ammo 200.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Item", meta = (ClampMin = "1", ClampMax = "999"))
	int32 MaxStackSize = 1;

	// True only for items the player can use from the inventory
	// (HealthPack, ShieldCell, Ammo). Weapons are equipped, not
	// used; mission items and materials are never consumable.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Item")
	bool bIsConsumable = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Item")
	EGFItemUseEffect UseEffect = EGFItemUseEffect::None;

	// Health/shield points restored, or reserve rounds added, by
	// one use. Ignored unless bIsConsumable and UseEffect != None.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Item", meta = (ClampMin = "0.0"))
	float EffectAmount = 0.0f;

	// Weapon items only: the weapon's numbers (damage, fire rate,
	// magazine). The weapon actor class is WeaponClass below.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Item")
	TObjectPtr<UGFWeaponDataAsset> WeaponData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Item")
	TSubclassOf<AGFWeaponBase> WeaponClass = nullptr;

	// Shield Generator items only: MaxShield bonus while equipped.
	// 0 for every other type. Unequipping removes the bonus and
	// clamps current shield to the new maximum (see
	// UGFEquipmentComponent).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Item", meta = (ClampMin = "0.0"))
	float ShieldBonus = 0.0f;

	// Checks the asset's own consistency (ids, stack size,
	// consumable/effect agreement, weapon data for weapon items).
	// Returns false and logs the first problem found.
	UFUNCTION(BlueprintCallable, Category = "GF|Item")
	bool Validate() const;
};

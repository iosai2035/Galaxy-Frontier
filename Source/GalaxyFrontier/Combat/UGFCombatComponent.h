// Galaxy Frontier — UGFCombatComponent.h (Phase 02)
// Owns the player's weapons: spawn, equip, switch, fire-rate timer,
// reload. The character only forwards input here; all weapon state
// lives in this component + AGFWeaponBase. Fire is timer-driven
// (no Tick), so holding fire costs no per-frame work.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UGFCombatComponent.generated.h"

class AGFWeaponBase;
class UGFWeaponDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGFWeaponSwitchedDelegate, int32, NewWeaponIndex);

UCLASS(ClassGroup = (GF), meta = (BlueprintSpawnableComponent))
class GALAXYFRONTIER_API UGFCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGFCombatComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Weapon classes to spawn at BeginPlay, in loadout order.
	// Data assets are assigned on each weapon class in the Editor;
	// a class with no data asset is skipped with a warning, not spawned.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Combat")
	TArray<TSubclassOf<AGFWeaponBase>> WeaponClasses;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Combat")
	TArray<TObjectPtr<AGFWeaponBase>> Weapons;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Combat")
	int32 CurrentWeaponIndex = -1;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Combat")
	bool bIsFiring = false;

	UPROPERTY(BlueprintAssignable, Category = "GF|Combat")
	FGFWeaponSwitchedDelegate OnWeaponSwitched;

	UFUNCTION(BlueprintCallable, Category = "GF|Combat")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "GF|Combat")
	void StopFire();

	// One shot, respecting CanFire(). Returns false when blocked
	// (reloading, no ammo, no weapon, dead owner — see CanFire).
	UFUNCTION(BlueprintCallable, Category = "GF|Combat")
	bool Fire();

	UFUNCTION(BlueprintCallable, Category = "GF|Combat")
	void Reload();

	UFUNCTION(BlueprintCallable, Category = "GF|Combat")
	bool SwitchWeapon(int32 NewIndex);

	UFUNCTION(BlueprintCallable, Category = "GF|Combat")
	bool SwitchToNextWeapon();

	UFUNCTION(BlueprintCallable, Category = "GF|Combat")
	bool CanFire() const;

	UFUNCTION(BlueprintPure, Category = "GF|Combat")
	AGFWeaponBase* GetCurrentWeapon() const;

	UFUNCTION(BlueprintPure, Category = "GF|Combat")
	int32 GetCurrentAmmo() const;

	UFUNCTION(BlueprintPure, Category = "GF|Combat")
	int32 GetReserveAmmo() const;

protected:
	void SpawnWeapons();
	void OnFireTimer();
	void StopFireTimer();

	FTimerHandle FireTimerHandle;
};

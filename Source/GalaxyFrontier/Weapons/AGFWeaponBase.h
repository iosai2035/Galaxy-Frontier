// Galaxy Frontier — AGFWeaponBase.h (Phase 02)
// One equippable weapon. All numbers come from its UGFWeaponDataAsset
// — the weapon owns ammo/reload state, never the character. Fire is
// blocked while reloading and at 0 ammo. Muzzle / mesh assets are
// assigned in the Editor; no mesh is faked here (see Docs/WEAPONS.md).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AGFWeaponBase.generated.h"

class UGFWeaponDataAsset;
class AGFProjectile;
class USkeletalMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGFWeaponAmmoChangedDelegate, int32, CurrentAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGFWeaponReloadedDelegate);

UCLASS()
class GALAXYFRONTIER_API AGFWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AGFWeaponBase();

protected:
	virtual void BeginPlay() override;

public:
	// Data asset — must be assigned in the Editor (or by the combat
	// component at spawn). Fire() refuses to run without one.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Weapon")
	TObjectPtr<UGFWeaponDataAsset> WeaponData;

	// Visible mesh, assigned in the Editor. May be null on a data-only
	// test weapon — firing does not depend on it.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|Weapon")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	// Muzzle socket name on WeaponMesh. If the socket is missing, the
	// projectile spawns at the weapon location (logged, not a crash).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Weapon")
	FName MuzzleSocketName = TEXT("Muzzle");

	UPROPERTY(BlueprintReadOnly, Category = "GF|Weapon")
	int32 CurrentAmmo = 0;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Weapon")
	int32 ReserveAmmo = 0;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Weapon")
	bool bIsReloading = false;

	UPROPERTY(BlueprintAssignable, Category = "GF|Weapon")
	FGFWeaponAmmoChangedDelegate OnAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "GF|Weapon")
	FGFWeaponReloadedDelegate OnReloaded;

	UFUNCTION(BlueprintCallable, Category = "GF|Weapon")
	bool Fire();

	UFUNCTION(BlueprintCallable, Category = "GF|Weapon")
	void Reload();

	UFUNCTION(BlueprintCallable, Category = "GF|Weapon")
	bool CanFire() const;

	UFUNCTION(BlueprintCallable, Category = "GF|Weapon")
	bool CanReload() const;

	UFUNCTION(BlueprintCallable, Category = "GF|Weapon")
	void Equip();

	UFUNCTION(BlueprintCallable, Category = "GF|Weapon")
	void Unequip();

	UFUNCTION(BlueprintCallable, Category = "GF|Weapon")
	void SetAmmo(int32 InCurrentAmmo, int32 InReserveAmmo);

	UFUNCTION(BlueprintPure, Category = "GF|Weapon")
	int32 GetAmmo() const { return CurrentAmmo; }

	UFUNCTION(BlueprintPure, Category = "GF|Weapon")
	int32 GetReserveAmmo() const { return ReserveAmmo; }

	UFUNCTION(BlueprintCallable, Category = "GF|Weapon")
	void ApplyRecoil();

	// Spawns one projectile from the muzzle. Returns null on failure
	// (no data asset / no projectile class / no world) — the caller
	// must handle null, never assume a projectile exists.
	UFUNCTION(BlueprintCallable, Category = "GF|Weapon")
	AGFProjectile* SpawnProjectile();

protected:
	void OnReloadFinished();

	FTimerHandle ReloadTimerHandle;
};

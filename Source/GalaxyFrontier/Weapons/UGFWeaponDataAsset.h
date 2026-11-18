// Galaxy Frontier — UGFWeaponDataAsset.h (Phase 02)
// Data-only weapon definition. Logic lives in AGFWeaponBase /
// UGFCombatComponent; this asset holds the numbers, so balance can
// be tuned in the Editor without touching code. The four starting
// weapons below are the master prompt's starting balance — they are
// NOT final; see Docs/WEAPONS.md for the table and tuning notes.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UGFWeaponDataAsset.generated.h"

class AGFProjectile;

UENUM(BlueprintType)
enum class EGFWeaponType : uint8
{
	Pistol			UMETA(DisplayName = "Pistol"),
	AssaultRifle	UMETA(DisplayName = "Assault Rifle"),
	Sniper			UMETA(DisplayName = "Sniper Rifle"),
	EnergyRifle		UMETA(DisplayName = "Energy Rifle"),
	Laser			UMETA(DisplayName = "Laser Weapon"),
	Melee			UMETA(DisplayName = "Melee Weapon")
};

UCLASS(BlueprintType)
class GALAXYFRONTIER_API UGFWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Weapon")
	FText WeaponName = FText::FromString(TEXT("Unnamed Weapon"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Weapon")
	EGFWeaponType WeaponType = EGFWeaponType::Pistol;

	// Damage per projectile hit. Starting balance (master prompt §10):
	// Pistol 20, Assault Rifle 18, Sniper 85, Energy Rifle 24.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Weapon", meta = (ClampMin = "0.0"))
	float Damage = 20.0f;

	// Shots per second. Starting balance: Pistol 3, Assault 10,
	// Sniper 0.8, Energy Rifle 5.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Weapon", meta = (ClampMin = "0.1"))
	float FireRate = 3.0f;

	// Rounds per magazine. Starting balance: Pistol 12, Assault 30,
	// Sniper 5, Energy Rifle 24.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Weapon", meta = (ClampMin = "1"))
	int32 MagazineSize = 12;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Weapon", meta = (ClampMin = "0.0"))
	float ReloadTimeSeconds = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Weapon", meta = (ClampMin = "0.0"))
	float ProjectileSpeed = 4000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Weapon", meta = (ClampMin = "0.0"))
	float MaxRange = 8000.0f;

	// Degrees of random spread per shot (0 = perfectly accurate).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Weapon", meta = (ClampMin = "0.0"))
	float SpreadDegrees = 1.0f;

	// Pitch kick applied to the owning controller per shot.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Weapon", meta = (ClampMin = "0.0"))
	float RecoilPitch = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Weapon")
	bool bIsAutomatic = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Weapon")
	TSubclassOf<AGFProjectile> ProjectileClass;
};

// Galaxy Frontier — AGFWeaponBase.cpp (Phase 02)

#include "Weapons/AGFWeaponBase.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Weapons/AGFProjectile.h"
#include "Weapons/UGFWeaponDataAsset.h"

AGFWeaponBase::AGFWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = WeaponMesh;
}

void AGFWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	if (!WeaponData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFWeaponBase %s has no WeaponData asset — it cannot fire until one is assigned."), *GetName());
		return;
	}
	// Start with a full magazine and a reasonable reserve.
	CurrentAmmo = WeaponData->MagazineSize;
	ReserveAmmo = WeaponData->MagazineSize * 4;
	OnAmmoChanged.Broadcast(CurrentAmmo);
}

bool AGFWeaponBase::CanFire() const
{
	return WeaponData != nullptr && !bIsReloading && CurrentAmmo > 0;
}

bool AGFWeaponBase::CanReload() const
{
	return WeaponData != nullptr && !bIsReloading
		&& CurrentAmmo < WeaponData->MagazineSize && ReserveAmmo > 0;
}

bool AGFWeaponBase::Fire()
{
	if (!CanFire())
	{
		if (WeaponData && CurrentAmmo <= 0 && !bIsReloading)
		{
			// Dry fire: no ammo is a state, not a crash. The combat
			// component may auto-reload on this; the weapon only logs.
			UE_LOG(LogTemp, Verbose, TEXT("[GF] %s dry fire — magazine empty."), *GetName());
		}
		return false;
	}

	AGFProjectile* Projectile = SpawnProjectile();
	if (!Projectile)
	{
		return false;
	}

	CurrentAmmo = FMath::Max(0, CurrentAmmo - 1);
	OnAmmoChanged.Broadcast(CurrentAmmo);
	ApplyRecoil();
	return true;
}

void AGFWeaponBase::Reload()
{
	if (!CanReload())
	{
		return;
	}
	bIsReloading = true;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ReloadTimerHandle, this, &AGFWeaponBase::OnReloadFinished,
			FMath::Max(0.1f, WeaponData->ReloadTimeSeconds), false);
	}
	else
	{
		// No world (should not happen in game) — reload instantly
		// rather than leaving the weapon stuck reloading forever.
		OnReloadFinished();
	}
}

void AGFWeaponBase::OnReloadFinished()
{
	bIsReloading = false;
	if (!WeaponData)
	{
		return;
	}
	const int32 Needed = WeaponData->MagazineSize - CurrentAmmo;
	const int32 Taken = FMath::Min(Needed, ReserveAmmo);
	CurrentAmmo += Taken;
	ReserveAmmo -= Taken;
	OnAmmoChanged.Broadcast(CurrentAmmo);
	OnReloaded.Broadcast();
}

void AGFWeaponBase::Equip()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(false);
}

void AGFWeaponBase::Unequip()
{
	SetActorHiddenInGame(true);
	// Cancel a reload in progress: a hidden weapon must never stay
	// in the reloading state when it is equipped again later.
	bIsReloading = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReloadTimerHandle);
	}
}

void AGFWeaponBase::SetAmmo(int32 InCurrentAmmo, int32 InReserveAmmo)
{
	if (!WeaponData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFWeaponBase::SetAmmo without WeaponData — ignored."));
		return;
	}
	CurrentAmmo = FMath::Clamp(InCurrentAmmo, 0, WeaponData->MagazineSize);
	ReserveAmmo = FMath::Max(0, InReserveAmmo);
	OnAmmoChanged.Broadcast(CurrentAmmo);
}

void AGFWeaponBase::ApplyRecoil()
{
	if (!WeaponData || WeaponData->RecoilPitch <= 0.0f)
	{
		return;
	}
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}
	if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
	{
		// Negative pitch = muzzle climbs. Small, per-shot, and
		// proportional to the data asset — never a hard-coded kick.
		PC->AddPitchInput(-WeaponData->RecoilPitch);
	}
}

AGFProjectile* AGFWeaponBase::SpawnProjectile()
{
	if (!WeaponData || !WeaponData->ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFWeaponBase::SpawnProjectile failed — no WeaponData or no ProjectileClass on %s."), *GetName());
		return nullptr;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FVector MuzzleLocation = GetActorLocation();
	FRotator MuzzleRotation = GetActorRotation();
	if (WeaponMesh && WeaponMesh->DoesSocketExist(MuzzleSocketName))
	{
		MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
		MuzzleRotation = WeaponMesh->GetSocketRotation(MuzzleSocketName);
	}
	else if (WeaponMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] Muzzle socket %s missing on %s — spawning at weapon location."),
			*MuzzleSocketName.ToString(), *GetName());
	}

	// Spread: rotate the aim direction by a small random cone.
	const float Spread = WeaponData->SpreadDegrees;
	FVector AimDirection = MuzzleRotation.Vector();
	if (Spread > 0.0f)
	{
		AimDirection = AimDirection.RotateAngleAxis(FMath::FRandRange(-Spread, Spread), FVector::UpVector);
		AimDirection = AimDirection.RotateAngleAxis(FMath::FRandRange(-Spread, Spread), FVector::RightVector);
		AimDirection = AimDirection.GetSafeNormal();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AGFProjectile* Projectile = World->SpawnActor<AGFProjectile>(
		WeaponData->ProjectileClass, MuzzleLocation, MuzzleRotation, SpawnParams);
	if (Projectile)
	{
		Projectile->InitProjectile(WeaponData->Damage, WeaponData->MaxRange, AimDirection, WeaponData->ProjectileSpeed);
	}
	return Projectile;
}

// Galaxy Frontier — UGFCombatComponent.cpp (Phase 02)

#include "Combat/UGFCombatComponent.h"

#include "Components/UGFHealthComponent.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Weapons/AGFWeaponBase.h"
#include "Weapons/UGFWeaponDataAsset.h"

UGFCombatComponent::UGFCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGFCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	SpawnWeapons();
}

void UGFCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopFireTimer();
	Super::EndPlay(EndPlayReason);
}

void UGFCombatComponent::SpawnWeapons()
{
	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();
	if (!World || !Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFCombatComponent::SpawnWeapons — no world/owner, no weapons spawned."));
		return;
	}

	for (TSubclassOf<AGFWeaponBase> WeaponClass : WeaponClasses)
	{
		if (!WeaponClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("[GF] UGFCombatComponent: null entry in WeaponClasses — skipped."));
			continue;
		}
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Owner;
		SpawnParams.Instigator = Cast<APawn>(Owner);
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AGFWeaponBase* Weapon = World->SpawnActor<AGFWeaponBase>(
			WeaponClass, Owner->GetActorLocation(), Owner->GetActorRotation(), SpawnParams);
		if (!Weapon)
		{
			UE_LOG(LogTemp, Warning, TEXT("[GF] UGFCombatComponent: failed to spawn weapon %s."), *WeaponClass->GetName());
			continue;
		}
		if (!Weapon->WeaponData)
		{
			// A weapon without data cannot fire. Destroy it rather
			// than keeping a dead entry in the loadout.
			UE_LOG(LogTemp, Warning, TEXT("[GF] UGFCombatComponent: weapon %s has no WeaponData — destroyed, not equipped."),
				*Weapon->GetName());
			Weapon->Destroy();
			continue;
		}
		Weapon->Unequip();
		Weapons.Add(Weapon);
	}

	if (Weapons.Num() > 0)
	{
		SwitchWeapon(0);
	}
}

bool UGFCombatComponent::CanFire() const
{
	// Dead owners cannot fire — the health component is the authority.
	if (UGFHealthComponent* Health = GetOwner() ? GetOwner()->FindComponentByClass<UGFHealthComponent>() : nullptr)
	{
		if (Health->IsDead())
		{
			return false;
		}
	}
	AGFWeaponBase* Weapon = GetCurrentWeapon();
	return Weapon != nullptr && Weapon->CanFire();
}

bool UGFCombatComponent::Fire()
{
	if (!CanFire())
	{
		return false;
	}
	return GetCurrentWeapon()->Fire();
}

void UGFCombatComponent::StartFire()
{
	if (bIsFiring)
	{
		return;
	}
	bIsFiring = true;
	AGFWeaponBase* Weapon = GetCurrentWeapon();
	if (!Weapon || !Weapon->WeaponData)
	{
		bIsFiring = false;
		return;
	}
	// First shot immediately, then a timer at the weapon's fire rate.
	// Automatic weapons keep firing via the timer; semi-automatic
	// weapons fire once per trigger pull (see StopFire / Fire).
	Fire();
	if (Weapon->WeaponData->bIsAutomatic && Weapon->WeaponData->FireRate > 0.0f)
	{
		const float Interval = 1.0f / Weapon->WeaponData->FireRate;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				FireTimerHandle, this, &UGFCombatComponent::OnFireTimer, Interval, true);
		}
	}
}

void UGFCombatComponent::OnFireTimer()
{
	if (!bIsFiring || !Fire())
	{
		// Out of ammo / reloading / dead: stop the automatic timer
		// instead of spinning it forever.
		StopFireTimer();
	}
}

void UGFCombatComponent::StopFire()
{
	bIsFiring = false;
	StopFireTimer();
}

void UGFCombatComponent::StopFireTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FireTimerHandle);
	}
}

void UGFCombatComponent::Reload()
{
	AGFWeaponBase* Weapon = GetCurrentWeapon();
	if (Weapon)
	{
		Weapon->Reload();
	}
}

bool UGFCombatComponent::SwitchWeapon(int32 NewIndex)
{
	if (NewIndex < 0 || NewIndex >= Weapons.Num() || NewIndex == CurrentWeaponIndex)
	{
		return false;
	}
	StopFire();
	if (AGFWeaponBase* OldWeapon = GetCurrentWeapon())
	{
		OldWeapon->Unequip();
	}
	CurrentWeaponIndex = NewIndex;
	if (AGFWeaponBase* NewWeapon = GetCurrentWeapon())
	{
		// Attach to the owner's mesh socket if present, else to the
		// root — a missing socket is logged in the weapon, not a crash.
		ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
		if (OwnerCharacter && OwnerCharacter->GetMesh())
		{
			NewWeapon->AttachToComponent(
				OwnerCharacter->GetMesh(),
				FAttachmentTransformRules::SnapToTargetIncludingScale,
				TEXT("WeaponSocket"));
		}
		else if (GetOwner())
		{
			NewWeapon->AttachToActor(GetOwner(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		}
		NewWeapon->Equip();
	}
	OnWeaponSwitched.Broadcast(CurrentWeaponIndex);
	return true;
}

bool UGFCombatComponent::SwitchToNextWeapon()
{
	if (Weapons.Num() <= 1)
	{
		return false;
	}
	return SwitchWeapon((CurrentWeaponIndex + 1) % Weapons.Num());
}

AGFWeaponBase* UGFCombatComponent::GetCurrentWeapon() const
{
	if (CurrentWeaponIndex < 0 || CurrentWeaponIndex >= Weapons.Num())
	{
		return nullptr;
	}
	return Weapons[CurrentWeaponIndex];
}

int32 UGFCombatComponent::GetCurrentAmmo() const
{
	AGFWeaponBase* Weapon = GetCurrentWeapon();
	return Weapon ? Weapon->GetAmmo() : 0;
}

int32 UGFCombatComponent::GetReserveAmmo() const
{
	AGFWeaponBase* Weapon = GetCurrentWeapon();
	return Weapon ? Weapon->GetReserveAmmo() : 0;
}

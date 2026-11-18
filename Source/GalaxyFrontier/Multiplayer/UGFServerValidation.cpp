// Galaxy Frontier — UGFServerValidation.cpp (Phase 06)

#include "Multiplayer/UGFServerValidation.h"

#include "Characters/AGFCharacter.h"
#include "Combat/UGFCombatComponent.h"
#include "Components/UGFHealthComponent.h"
#include "GameFramework/PlayerState.h"
#include "Multiplayer/AGFGameMode.h"
#include "Multiplayer/AGFGameState.h"
#include "Multiplayer/AGFPlayerState.h"
#include "Multiplayer/UGFAntiCheatManager.h"
#include "Weapons/AGFWeaponBase.h"
#include "Weapons/UGFWeaponDataAsset.h"
#include "World/IGFInteractable.h"

UGFServerValidation::UGFServerValidation()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGFServerValidation::BeginPlay()
{
	Super::BeginPlay();
	if (AActor* Owner = GetOwner())
	{
		LastValidatedPosition = Owner->GetActorLocation();
		if (UWorld* World = GetWorld())
		{
			LastValidatedPositionTime = World->GetTimeSeconds();
		}
		bHasValidatedPosition = true;
	}
}

bool UGFServerValidation::IsServerAuthority() const
{
	const AActor* Owner = GetOwner();
	return Owner && Owner->HasAuthority();
}

AGFCharacter* UGFServerValidation::GetOwnerCharacter() const
{
	return Cast<AGFCharacter>(GetOwner());
}

AGFPlayerState* UGFServerValidation::GetOwnerPlayerState() const
{
	if (const APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		return Cast<AGFPlayerState>(Pawn->GetPlayerState());
	}
	return nullptr;
}

void UGFServerValidation::ReportViolation(int32 ViolationTypeInt)
{
	// Single-player: nothing to report (there is no other player
	// to protect, and the local player is the authority).
	if (!IsServerAuthority())
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		if (World->GetNetMode() == NM_Standalone)
		{
			return;
		}
	}
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI)
	{
		return;
	}
	if (UGFAntiCheatManager* AntiCheat = GI->GetSubsystem<UGFAntiCheatManager>())
	{
		AntiCheat->ReportViolation(GetOwnerPlayerState(), static_cast<EGFViolationType>(ViolationTypeInt));
	}
}

void UGFServerValidation::RecordPosition(const FVector& Position)
{
	LastValidatedPosition = Position;
	if (UWorld* World = GetWorld())
	{
		LastValidatedPositionTime = World->GetTimeSeconds();
	}
	bHasValidatedPosition = true;
}

bool UGFServerValidation::ValidateFireRequest()
{
	// Single-player: the local player IS the authority.
	if (UWorld* World = GetWorld())
	{
		if (World->GetNetMode() == NM_Standalone)
		{
			return true;
		}
	}
	if (!IsServerAuthority())
	{
		return false;
	}
	AGFCharacter* Character = GetOwnerCharacter();
	if (!Character || !Character->GetCombatComponent())
	{
		return false;
	}
	AGFWeaponBase* Weapon = Character->GetCombatComponent()->GetCurrentWeapon();
	if (!Weapon || !Weapon->WeaponData || Weapon->WeaponData->FireRate <= 0.0f)
	{
		return false;
	}
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}
	const double Now = World->GetTimeSeconds();
	const double Interval = 1.0 / static_cast<double>(Weapon->WeaponData->FireRate);
	const double MinInterval = Interval * static_cast<double>(FireRateTolerance) - FireNetworkSlackSeconds;
	if (Now - LastFireTimeSeconds < MinInterval)
	{
		ReportViolation(static_cast<int32>(EGFViolationType::FireRate));
		return false;
	}
	LastFireTimeSeconds = Now;
	return true;
}

bool UGFServerValidation::ValidateMovement(const FVector& NewLocation, float DeltaTime)
{
	if (UWorld* World = GetWorld())
	{
		if (World->GetNetMode() == NM_Standalone)
		{
			RecordPosition(NewLocation);
			return true;
		}
	}
	if (!IsServerAuthority() || DeltaTime <= 0.0f || !FMath::IsFinite(DeltaTime))
	{
		return false;
	}
	if (!bHasValidatedPosition)
	{
		RecordPosition(NewLocation);
		return true;
	}
	AGFCharacter* Character = GetOwnerCharacter();
	if (!Character)
	{
		return false;
	}
	// Top speed is the character's SprintSpeed (700 cm/s, Phase 01).
	const float MaxSpeed = Character->SprintSpeed * SpeedTolerance;
	const float Distance = FVector::Dist(LastValidatedPosition, NewLocation);
	const float Allowed = MaxSpeed * DeltaTime + TeleportSlackCm;
	const float GrossLimit = Character->SprintSpeed * 2.0f * DeltaTime + TeleportSlackCm;
	if (Distance > GrossLimit)
	{
		ReportViolation(static_cast<int32>(EGFViolationType::Teleport));
		return false;
	}
	if (Distance > Allowed)
	{
		ReportViolation(static_cast<int32>(EGFViolationType::SpeedHack));
		return false;
	}
	RecordPosition(NewLocation);
	return true;
}

bool UGFServerValidation::ValidateDamageRequest(AActor* Target, float Damage, UGFWeaponDataAsset* WeaponData)
{
	if (UWorld* World = GetWorld())
	{
		if (World->GetNetMode() == NM_Standalone)
		{
			return true;
		}
	}
	if (!IsServerAuthority() || !Target || !WeaponData)
	{
		return false;
	}
	// Match must be Playing (a lobby shot is not a shot).
	if (const AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr)
	{
		if (const AGFGameState* GFGS = Cast<AGFGameState>(GS))
		{
			if (!GFGS->IsMatchPlaying())
			{
				return false;
			}
		}
	}
	AGFCharacter* Attacker = GetOwnerCharacter();
	if (!Attacker || Attacker == Target)
	{
		return false;
	}
	UGFHealthComponent* AttackerHealth = Attacker->GetHealthComponent();
	UGFHealthComponent* TargetHealth = Target->FindComponentByClass<UGFHealthComponent>();
	if (!AttackerHealth || AttackerHealth->IsDead() || !TargetHealth || TargetHealth->IsDead())
	{
		return false;
	}
	// The damage must be exactly what the weapon deals. A client
	// sending 999 (or any other number) is not a weapon — it is
	// tampering.
	if (!FMath::IsFinite(Damage) || Damage <= 0.0f ||
		!FMath::IsNearlyEqual(Damage, WeaponData->Damage, 0.01f))
	{
		ReportViolation(static_cast<int32>(EGFViolationType::ImpossibleDamage));
		return false;
	}
	if (FVector::Dist(Attacker->GetActorLocation(), Target->GetActorLocation()) > WeaponData->MaxRange)
	{
		return false;
	}
	return true;
}

bool UGFServerValidation::ValidateProjectileHit(AActor* Target, float Damage, const FVector& ShotOrigin, float MaxRange)
{
	if (UWorld* World = GetWorld())
	{
		if (World->GetNetMode() == NM_Standalone)
		{
			return true;
		}
	}
	if (!IsServerAuthority() || !Target)
	{
		return false;
	}
	if (const AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr)
	{
		if (const AGFGameState* GFGS = Cast<AGFGameState>(GS))
		{
			if (!GFGS->IsMatchPlaying())
			{
				return false;
			}
		}
	}
	AGFCharacter* Attacker = GetOwnerCharacter();
	if (!Attacker || Attacker == Target)
	{
		return false;
	}
	UGFHealthComponent* AttackerHealth = Attacker->GetHealthComponent();
	UGFHealthComponent* TargetHealth = Target->FindComponentByClass<UGFHealthComponent>();
	if (!AttackerHealth || AttackerHealth->IsDead() || !TargetHealth || TargetHealth->IsDead())
	{
		return false;
	}
	if (!FMath::IsFinite(Damage) || Damage <= 0.0f || Damage > MaxWeaponDamage)
	{
		ReportViolation(static_cast<int32>(EGFViolationType::ImpossibleDamage));
		return false;
	}
	if (FVector::Dist(ShotOrigin, Target->GetActorLocation()) > MaxRange)
	{
		return false;
	}
	return true;
}

float UGFServerValidation::ApplyValidatedDamage(AActor* Target, float Damage, const FVector& ShotOrigin, float MaxRange)
{
	if (!ValidateProjectileHit(Target, Damage, ShotOrigin, MaxRange))
	{
		return 0.0f;
	}
	UGFHealthComponent* TargetHealth = Target->FindComponentByClass<UGFHealthComponent>();
	if (!TargetHealth)
	{
		return 0.0f;
	}
	const float Applied = TargetHealth->ApplyDamage(Damage, GetOwner());
	if (Applied > 0.0f && TargetHealth->IsDead())
	{
		// Elimination credit: the instigator is the attacker.
		// Zone deaths never come through here (UGFZoneManager
		// calls AGFGameMode::NotifyElimination directly, with a
		// null killer).
		if (UWorld* World = GetWorld())
		{
			if (AGFGameMode* GM = Cast<AGFGameMode>(World->GetAuthGameMode()))
			{
				AGFPlayerState* KillerState = GetOwnerPlayerState();
				AGFPlayerState* VictimState = nullptr;
				if (APawn* VictimPawn = Cast<APawn>(Target))
				{
					VictimState = Cast<AGFPlayerState>(VictimPawn->GetPlayerState());
				}
				if (VictimState)
				{
					GM->NotifyElimination(VictimState, KillerState);
				}
			}
		}
	}
	return Applied;
}

bool UGFServerValidation::ValidateInteractRequest(AActor* Target)
{
	if (UWorld* World = GetWorld())
	{
		if (World->GetNetMode() == NM_Standalone)
		{
			return true;
		}
	}
	if (!IsServerAuthority() || !Target)
	{
		return false;
	}
	if (const AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr)
	{
		if (const AGFGameState* GFGS = Cast<AGFGameState>(GS))
		{
			if (!GFGS->IsMatchPlaying())
			{
				return false;
			}
		}
	}
	AGFCharacter* Character = GetOwnerCharacter();
	if (!Character)
	{
		return false;
	}
	if (FVector::Dist(Character->GetActorLocation(), Target->GetActorLocation()) > InteractRangeCm)
	{
		ReportViolation(static_cast<int32>(EGFViolationType::InvalidInteract));
		return false;
	}
	IGFInteractable* Interactable = Cast<IGFInteractable>(Target);
	if (!Interactable || !Interactable->CanInteract(Character))
	{
		ReportViolation(static_cast<int32>(EGFViolationType::InvalidInteract));
		return false;
	}
	return true;
}

// Galaxy Frontier — AGFPlayerController.cpp (Phase 06)

#include "Multiplayer/AGFPlayerController.h"

#include "Characters/AGFCharacter.h"
#include "Combat/UGFCombatComponent.h"
#include "Components/UGFHealthComponent.h"
#include "Multiplayer/AGFPlayerState.h"
#include "Multiplayer/UGFServerValidation.h"
#include "Progression/UGFProgressionComponent.h"
#include "Weapons/AGFWeaponBase.h"
#include "Weapons/UGFWeaponDataAsset.h"
#include "World/UGFInteractionComponent.h"

AGFPlayerController::AGFPlayerController()
{
}

void AGFPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Networked client only: send the saved progression level to
	// the server once, so the PlayerState (and matchmaking) knows
	// it. In single-player there is no server to tell, and the
	// local progression component already holds the level.
	if (IsLocalController() && GetWorld() && GetWorld()->GetNetMode() == NM_Client && !bSentPlayerLevel)
	{
		bSentPlayerLevel = true;
		int32 Level = 1;
		if (AGFCharacter* Character = GetGFCharacter())
		{
			if (UGFProgressionComponent* Progression = Character->GetProgressionComponent())
			{
				Level = Progression->PlayerLevel;
			}
		}
		ServerSetPlayerLevel(Level);
	}
}

AGFCharacter* AGFPlayerController::GetGFCharacter() const
{
	return Cast<AGFCharacter>(GetPawn());
}

// --- Server RPCs ----------------------------------------------------
// _Validate runs on the server before _Implementation. It must
// stay permissive (null checks only): returning false here
// disconnects the client, so the real rules (fire rate, range,
// damage) are enforced in _Implementation via
// UGFServerValidation, where a failure only rejects the request.

void AGFPlayerController::ServerRequestStartFire_Implementation()
{
	AGFCharacter* Character = GetGFCharacter();
	if (!Character || !Character->GetCombatComponent())
	{
		return;
	}
	UGFServerValidation* Validation = Character->FindComponentByClass<UGFServerValidation>();
	if (Validation && !Validation->ValidateFireRequest())
	{
		return;
	}
	Character->GetCombatComponent()->StartFire();
}

bool AGFPlayerController::ServerRequestStartFire_Validate()
{
	return GetPawn() != nullptr;
}

void AGFPlayerController::ServerRequestStopFire_Implementation()
{
	if (AGFCharacter* Character = GetGFCharacter())
	{
		if (Character->GetCombatComponent())
		{
			Character->GetCombatComponent()->StopFire();
		}
	}
}

bool AGFPlayerController::ServerRequestStopFire_Validate()
{
	return GetPawn() != nullptr;
}

void AGFPlayerController::ServerRequestInteract_Implementation()
{
	AGFCharacter* Character = GetGFCharacter();
	if (!Character || !Character->GetInteractionComponent())
	{
		return;
	}
	// The interaction component knows the current target (its
	// 0.15 s scan, Phase 03). Validation checks that target is
	// real, reachable and usable before anything starts.
	UGFServerValidation* Validation = Character->FindComponentByClass<UGFServerValidation>();
	if (Validation)
	{
		// The target is the component's current interactable
		// (its 0.15 s scan, Phase 03). A null target fails
		// validation (nothing to interact with — not a violation).
		if (AActor* Target = Character->GetInteractionComponent()->CurrentInteractable)
		{
			if (!Validation->ValidateInteractRequest(Target))
			{
				return;
			}
		}
		else
		{
			return;
		}
	}
	Character->GetInteractionComponent()->BeginInteraction();
}

bool AGFPlayerController::ServerRequestInteract_Validate()
{
	return GetPawn() != nullptr;
}

void AGFPlayerController::ServerRequestDamage_Implementation(AActor* TargetActor, float Damage)
{
	AGFCharacter* Character = GetGFCharacter();
	if (!Character || !TargetActor || !Character->GetCombatComponent())
	{
		return;
	}
	AGFWeaponBase* Weapon = Character->GetCombatComponent()->GetCurrentWeapon();
	if (!Weapon || !Weapon->WeaponData)
	{
		return;
	}
	UGFServerValidation* Validation = Character->FindComponentByClass<UGFServerValidation>();
	if (Validation && !Validation->ValidateDamageRequest(TargetActor, Damage, Weapon->WeaponData))
	{
		return;
	}
	if (UGFHealthComponent* Health = TargetActor->FindComponentByClass<UGFHealthComponent>())
	{
		Health->ApplyDamage(Damage, Character);
	}
}

bool AGFPlayerController::ServerRequestDamage_Validate(AActor* TargetActor, float Damage)
{
	// Reject only malformed requests here (null target, absurd
	// numbers). The real rules live in _Implementation.
	return TargetActor != nullptr && FMath::IsFinite(Damage) && Damage > 0.0f && Damage <= 10000.0f;
}

void AGFPlayerController::ServerSetPlayerLevel_Implementation(int32 Level)
{
	if (AGFPlayerState* PS = Cast<AGFPlayerState>(PlayerState))
	{
		// Clamped to 1..20 inside SetPlayerLevel — a client
		// sending 99 does not become level 99.
		PS->SetPlayerLevel(Level);
	}
}

bool AGFPlayerController::ServerSetPlayerLevel_Validate(int32 Level)
{
	return Level >= 1 && Level <= 100;
}

// --- Client RPC -----------------------------------------------------

void AGFPlayerController::ClientShowMatchResult_Implementation(int32 Placement, bool bWon)
{
	// The result screen is a widget (WBP_MultiplayerHUD /
	// match-result screen, Docs/UI-SCREENS.md). The values are
	// exposed to Blueprint here so the widget can read them on
	// construct; there is no C++ widget in this repo.
	UE_LOG(LogTemp, Log, TEXT("[GF] Match result: placement %d, won=%s."),
		Placement, bWon ? TEXT("true") : TEXT("false"));
}

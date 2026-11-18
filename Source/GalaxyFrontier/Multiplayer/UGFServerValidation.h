// Galaxy Frontier — UGFServerValidation.h (Phase 06)
// Server-authoritative validation, an ActorComponent on
// AGFCharacter. Every gameplay request that arrives from a client
// passes through here ON THE SERVER before anything is applied:
// fire requests, movement, damage, projectile hits, interactions
// and pickups. The client only ever *asks*; the server decides.
//
// Rules enforced (see Docs/MULTIPLAYER.md for the full table):
// - Fire rate: a shot may not arrive faster than the weapon's
//   FireRate allows (15% tolerance + 50 ms network slack).
// - Movement: speed may not exceed the character's SprintSpeed
//   (700 cm/s) + 10%, and a position jump may not exceed what
//   that speed allows in the elapsed time (+ 500 cm slack).
// - Damage: the damage must equal the weapon's Damage, the
//   target must be alive, in range and in a Playing match, and
//   the attacker must not be the target.
// - Interact / pickup: the target must be within 250 cm and
//   usable (IGFInteractable::CanInteract).
//
// In single-player (NM_Standalone) every check passes trivially
// on the local authority — the component never blocks local play.
// A failed check reports a violation to UGFAntiCheatManager
// (server only) and returns false; it never crashes and never
// applies partial state.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UGFServerValidation.generated.h"

class AGFCharacter;
class AGFPlayerState;
class UGFWeaponDataAsset;
class AActor;

UCLASS(ClassGroup = (GF), meta = (BlueprintSpawnableComponent))
class GALAXYFRONTIER_API UGFServerValidation : public UActorComponent
{
	GENERATED_BODY()

public:
	UGFServerValidation();

	// --- Fire ---
	// True when the owner may fire now, by the current weapon's
	// fire rate. Records the shot time on success. Call on the
	// server only; in single-player it always returns true.
	UFUNCTION(BlueprintCallable, Category = "GF|Validation")
	bool ValidateFireRequest();

	// --- Movement ---
	// True when NewLocation is reachable from the last validated
	// position at the character's top speed in DeltaTime seconds.
	// Records the position on success. DeltaTime <= 0 is rejected.
	UFUNCTION(BlueprintCallable, Category = "GF|Validation")
	bool ValidateMovement(const FVector& NewLocation, float DeltaTime);

	// --- Damage ---
	// Full check for a damage request: authority, Playing match,
	// attacker and target alive, attacker != target, damage
	// finite and equal to the weapon's Damage, target within
	// MaxRange of the attacker. Does NOT apply the damage.
	UFUNCTION(BlueprintCallable, Category = "GF|Validation")
	bool ValidateDamageRequest(AActor* Target, float Damage, UGFWeaponDataAsset* WeaponData);

	// Projectile-hit check: same as ValidateDamageRequest, but
	// the range is measured from the shot's origin (the
	// projectile's SpawnLocation) and the damage cap is the
	// highest weapon damage in the game (Sniper 85), because a
	// projectile carries no weapon asset with it.
	UFUNCTION(BlueprintCallable, Category = "GF|Validation")
	bool ValidateProjectileHit(AActor* Target, float Damage, const FVector& ShotOrigin, float MaxRange);

	// Validates AND applies damage on the server. Returns the
	// damage actually applied (0 when rejected). On a kill it
	// notifies the GameMode (elimination credit + placement).
	// This is the only path projectiles use to damage players
	// (AGFProjectile::OnHit).
	UFUNCTION(BlueprintCallable, Category = "GF|Validation")
	float ApplyValidatedDamage(AActor* Target, float Damage, const FVector& ShotOrigin, float MaxRange);

	// --- Interaction / pickup ---
	// True when Target is within 250 cm, usable, and the match
	// is Playing (or single-player). Does not start the interaction.
	UFUNCTION(BlueprintCallable, Category = "GF|Validation")
	bool ValidateInteractRequest(AActor* Target);

	// --- Bookkeeping (server-side, used by the checks above) ---
	UFUNCTION(BlueprintCallable, Category = "GF|Validation")
	void RecordPosition(const FVector& Position);

	UFUNCTION(BlueprintPure, Category = "GF|Validation")
	bool IsServerAuthority() const;

protected:
	virtual void BeginPlay() override;

	AGFCharacter* GetOwnerCharacter() const;
	AGFPlayerState* GetOwnerPlayerState() const;
	void ReportViolation(int32 ViolationTypeInt);

	// Last validated shot / position, server-side only.
	double LastFireTimeSeconds = -1000.0;
	FVector LastValidatedPosition = FVector::ZeroVector;
	double LastValidatedPositionTime = -1000.0;
	bool bHasValidatedPosition = false;

	// Tolerances (documented in Docs/MULTIPLAYER.md).
	static constexpr float FireRateTolerance = 0.85f;	// 15% early
	static constexpr float FireNetworkSlackSeconds = 0.05f;
	static constexpr float SpeedTolerance = 1.10f;		// 10% over top speed
	static constexpr float TeleportSlackCm = 500.0f;
	static constexpr float InteractRangeCm = 250.0f;
	// Highest damage any weapon in the game can deal (Sniper 85).
	static constexpr float MaxWeaponDamage = 85.0f;
};

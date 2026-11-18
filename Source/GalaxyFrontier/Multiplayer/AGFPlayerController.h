// Galaxy Frontier — AGFPlayerController.h (Phase 06)
// Player controller for networked matches. All gameplay requests
// from a client arrive here as Server RPCs and are executed on
// the server only after UGFServerValidation approves them:
//   ServerRequestStartFire / ServerRequestStopFire — fire control
//   ServerRequestInteract — interaction (doors, pickups)
//   ServerRequestDamage — direct damage (used by hitscan-style
//     paths; projectiles use UGFServerValidation::
//     ApplyValidatedDamage in AGFProjectile::OnHit instead)
//   ServerSetPlayerLevel — the client's saved level (1..20,
//     clamped on the server), sent once at login.
//
// In single-player games this controller is never used for
// gameplay input — AGFCharacter calls its CombatComponent
// directly (Phase 02), exactly as before. The RPCs exist only
// when the game is networked (World NetMode != Standalone).
//
// Client RPC: ClientShowMatchResult — the server's final result
// for this player (placement + win flag), shown by
// WBP_MultiplayerHUD / the match-result screen.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AGFPlayerController.generated.h"

UCLASS()
class GALAXYFRONTIER_API AGFPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGFPlayerController();

protected:
	virtual void BeginPlay() override;

public:
	// --- Server RPCs (client asks, server validates + executes) ---
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestStartFire();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestStopFire();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestInteract();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestDamage(AActor* TargetActor, float Damage);

	// The client's saved progression level, sent once at login.
	// The server clamps it to 1..20 and stores it on the
	// PlayerState (AGFPlayerState::SetPlayerLevel).
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetPlayerLevel(int32 Level);

	// --- Client RPC (server -> this client only) ---
	UFUNCTION(Client, Reliable)
	void ClientShowMatchResult(int32 Placement, bool bWon);

protected:
	bool bSentPlayerLevel = false;

	class AGFCharacter* GetGFCharacter() const;
};

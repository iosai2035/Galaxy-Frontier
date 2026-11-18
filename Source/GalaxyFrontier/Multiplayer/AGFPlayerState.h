// Galaxy Frontier — AGFPlayerState.h (Phase 06)
// Per-player replicated data, owned by the server. The client's
// own level is sent once at login (AGFPlayerController::
// ServerSetPlayerLevel, clamped 1..20 on the server) — the
// client never writes these fields directly. Eliminations and
// alive/placement state are written by AGFGameMode /
// UGFMatchManager only. In single-player games the default
// PlayerState is used; nothing in Phases 00-05 reads this class.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AGFPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGFPlayerAliveChangedDelegate, bool, bIsAlive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGFEliminationsChangedDelegate, int32, Eliminations);

UCLASS()
class GALAXYFRONTIER_API AGFPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AGFPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Progression level from the player's save (Phase 05,
	// UGFProgressionComponent, cap 20). Used for matchmaking
	// (UGFSessionManager) and shown in the lobby / HUD. It is
	// cosmetic: levels grant no combat stats.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GF|Player")
	int32 PlayerLevel = 1;

	// Kills in this match. Written by the server only, on a
	// validated elimination (AGFGameMode::NotifyElimination).
	UPROPERTY(ReplicatedUsing = OnRep_Eliminations, BlueprintReadOnly, Category = "GF|Player")
	int32 Eliminations = 0;

	// Alive in this match. False after elimination; in Battle
	// Royale there is no revive (UGFMatchManager), so this only
	// ever goes true -> false within a match.
	UPROPERTY(ReplicatedUsing = OnRep_IsAlive, BlueprintReadOnly, Category = "GF|Player")
	bool bIsAlive = true;

	// Final placement (1 = winner). 0 = not placed yet (still
	// alive or match not ended). Written by UGFMatchManager.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GF|Player")
	int32 Placement = 0;

	UPROPERTY(BlueprintAssignable, Category = "GF|Player")
	FGFPlayerAliveChangedDelegate OnAliveChanged;

	UPROPERTY(BlueprintAssignable, Category = "GF|Player")
	FGFEliminationsChangedDelegate OnEliminationsChanged;

	// --- Server-only setters (clients get a warning, no change) ---
	UFUNCTION(BlueprintCallable, Category = "GF|Player")
	void SetPlayerLevel(int32 NewLevel);

	UFUNCTION(BlueprintCallable, Category = "GF|Player")
	void AddElimination();

	UFUNCTION(BlueprintCallable, Category = "GF|Player")
	void SetAlive(bool bNewAlive);

	UFUNCTION(BlueprintCallable, Category = "GF|Player")
	void SetPlacement(int32 NewPlacement);

protected:
	UFUNCTION()
	void OnRep_Eliminations();

	UFUNCTION()
	void OnRep_IsAlive();
};

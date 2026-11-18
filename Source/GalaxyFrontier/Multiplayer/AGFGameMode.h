// Galaxy Frontier — AGFGameMode.h (Phase 06)
// Server-authoritative GameMode for multiplayer matches. Only used
// when a network session is running (assigned in DefaultGame.ini
// via the session flow, Docs/MULTIPLAYER.md) — single-player
// games use the default GameMode and never touch this class.
//
// Match flow (server only): Lobby (players join, 30 s or full)
// -> Countdown (5 s) -> Playing -> Ended (one player alive).
// Death is handled here: NotifyElimination() is called by the
// server-side damage paths only (UGFServerValidation::
// ApplyValidatedDamage, UGFZoneManager zone damage). There is no
// respawn in Battle Royale — RestartPlayer is never called for a
// Battle Royale match, and a dead player's pawn stays dead until
// the match ends.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Multiplayer/AGFGameState.h"
#include "AGFGameMode.generated.h"

class AGFPlayerState;
class UGFZoneManager;
class UGFMatchManager;

UCLASS()
class GALAXYFRONTIER_API AGFGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGFGameMode();

	// --- Match tuning (starting values, all editable) ---
	// Lobby waits this long, or until MaxPlayers join.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Match")
	float LobbyDurationSeconds = 30.0f;

	// Short countdown after the lobby closes, before play starts.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Match")
	float CountdownDurationSeconds = 5.0f;

	// Design target for Battle Royale (Docs/BATTLE-ROYALE.md).
	// NOT a tested number — the two-player test comes first.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Match", meta = (ClampMin = "2", ClampMax = "60"))
	int32 MaxPlayers = 60;

	// Minimum players to start a match when the lobby timer ends.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Match", meta = (ClampMin = "2"))
	int32 MinPlayersToStart = 2;

	// True for Battle Royale (no respawn, zone, placements).
	// False for a plain multiplayer match (same flow, but the
	// zone is disabled and eliminations still end the match
	// when one player is left — see Docs/MULTIPLAYER.md).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Match")
	bool bIsBattleRoyale = true;

	// --- Owned match systems (server only, created here) ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|Match")
	TObjectPtr<UGFZoneManager> ZoneManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|Match")
	TObjectPtr<UGFMatchManager> MatchManager;

	// Called by server-side damage paths when a player dies.
	// KillerState may be null (zone death, or a death with no
	// credited killer). Sets the victim dead, records the
	// elimination + placement, and ends the match when one
	// player is left. Safe to call twice for the same victim
	// (second call is a no-op).
	UFUNCTION(BlueprintCallable, Category = "GF|Match")
	void NotifyElimination(AGFPlayerState* VictimState, AGFPlayerState* KillerState);

	// Server-only match control. StartMatch moves Lobby ->
	// Countdown; EndMatch moves Playing -> Ended and awards XP.
	UFUNCTION(BlueprintCallable, Category = "GF|Match")
	void StartMatch();

	UFUNCTION(BlueprintCallable, Category = "GF|Match")
	void EndMatch();

	UFUNCTION(BlueprintPure, Category = "GF|Match")
	AGFGameState* GetGFGameState() const;

	UFUNCTION(BlueprintPure, Category = "GF|Match")
	bool IsMatchPlaying() const;

protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void BeginPlay() override;

	// Spawn distribution is delegated to UGFMatchManager
	// (round-robin over PlayerStarts, grid fallback).
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// 1 s match timer: drives Lobby -> Countdown -> Playing and
	// the per-second match clock. Timers only, no Tick.
	void OnMatchTimerTick();
	void EnterCountdown();
	void EnterPlaying();

	// Recomputes Alive/Total counts on the GameState from the
	// PlayerArray (server only).
	void RefreshPlayerCounts();

	FTimerHandle MatchTimerHandle;
	float LobbyTimeRemaining = 0.0f;
	float CountdownTimeRemaining = 0.0f;
};

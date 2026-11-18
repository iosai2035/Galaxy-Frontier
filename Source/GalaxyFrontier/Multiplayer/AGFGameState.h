// Galaxy Frontier — AGFGameState.h (Phase 06)
// Replicated match state, owned by the server. Clients never write
// these values — they read them (via OnRep) to drive the HUD,
// the lobby and the zone display. Single-player games never use
// this class (the default GameState is used instead); nothing in
// Phases 00-05 reads it, so single-player behaviour is unchanged.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "AGFGameState.generated.h"

// Match lifecycle. Lobby -> Countdown -> Playing -> Ended. Ended is
// terminal for the match; a new match needs a new session/level load.
UENUM(BlueprintType)
enum class EGFMatchState : uint8
{
	Lobby		UMETA(DisplayName = "Lobby"),
	Countdown	UMETA(DisplayName = "Countdown"),
	Playing		UMETA(DisplayName = "Playing"),
	Ended		UMETA(DisplayName = "Ended")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGFMatchStateChangedDelegate, EGFMatchState, NewState);

UCLASS()
class GALAXYFRONTIER_API AGFGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AGFGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- Replicated match state (server writes, clients read) ---
	UPROPERTY(ReplicatedUsing = OnRep_MatchState, BlueprintReadOnly, Category = "GF|Match")
	EGFMatchState MatchState = EGFMatchState::Lobby;

	// Seconds left in the current phase (lobby wait / countdown /
	// match). 0 when the phase has no timer.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GF|Match")
	float MatchTimerRemaining = 0.0f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GF|Match")
	int32 AlivePlayerCount = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GF|Match")
	int32 TotalPlayerCount = 0;

	// --- Replicated safe-zone state (written by UGFZoneManager) ---
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GF|Zone")
	FVector ZoneCenter = FVector::ZeroVector;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GF|Zone")
	float ZoneRadius = 6000.0f;

	// Fired on clients (and the server) when MatchState changes.
	// Widgets bind here — they never poll the state every frame.
	UPROPERTY(BlueprintAssignable, Category = "GF|Match")
	FGFMatchStateChangedDelegate OnMatchStateChanged;

	// Server-only setters. They do nothing on clients (the values
	// are replicated, not locally written) — a client calling one
	// gets a warning and no change.
	UFUNCTION(BlueprintCallable, Category = "GF|Match")
	void SetMatchState(EGFMatchState NewState);

	UFUNCTION(BlueprintCallable, Category = "GF|Match")
	void SetMatchTimer(float SecondsRemaining);

	UFUNCTION(BlueprintCallable, Category = "GF|Match")
	void SetPlayerCounts(int32 AliveCount, int32 TotalCount);

	UFUNCTION(BlueprintCallable, Category = "GF|Zone")
	void SetZone(const FVector& Center, float Radius);

	UFUNCTION(BlueprintPure, Category = "GF|Match")
	bool IsMatchPlaying() const { return MatchState == EGFMatchState::Playing; }

protected:
	UFUNCTION()
	void OnRep_MatchState();
};

// Galaxy Frontier — UGFReplicationManager.h (Phase 06)
// Replication policy for the player character, an ActorComponent
// on AGFCharacter. It does NOT replicate anything itself — the
// engine replicates the character (movement, the replicated
// properties in AGFPlayerState / AGFGameState). This component
// only answers the policy questions, so the numbers live in one
// place and can be tuned without touching the engine's
// NetUpdateFrequency settings:
//
// - Priority: near players matter most (a far player you cannot
//   see does not need fast updates).
// - Update interval by distance: < 1500 cm -> 20 Hz, < 4000 cm
//   -> 10 Hz, further -> 5 Hz.
// - Movement delta: a player who has not moved does not send
//   position updates at all (the idle threshold is 5 cm).
// - Heartbeat: the server records the last validated position
//   time; a player silent for longer than DisconnectTimeout is
//   treated as disconnected by AGFGameMode::Logout handling
//   (the engine's own timeout fires first — this is a fallback
//   check, see Docs/MULTIPLAYER.md).
//
// In single-player every function is still callable and returns
// the closest-priority values; nothing in Phases 00-05 calls
// this component, so single-player behaviour is unchanged.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UGFReplicationManager.generated.h"

UCLASS(ClassGroup = (GF), meta = (BlueprintSpawnableComponent))
class GALAXYFRONTIER_API UGFReplicationManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UGFReplicationManager();

	// --- Tuning (starting values, all editable) ---
	// Inside this distance a player is "near" (full rate).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Replication")
	float NearDistanceCm = 1500.0f;

	// Inside this distance a player is "mid" (half rate).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Replication")
	float MidDistanceCm = 4000.0f;

	// A move smaller than this does not need a position update.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Replication")
	float IdleDeltaCm = 5.0f;

	// Silence longer than this marks the player disconnected
	// (fallback — the engine timeout normally fires first).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Replication")
	float DisconnectTimeoutSeconds = 30.0f;

	// 1.0 = highest priority (near), 0.3 = lowest (far / idle).
	// Used by the server to order replication work.
	UFUNCTION(BlueprintPure, Category = "GF|Replication")
	float ComputeReplicationPriority(const FVector& ViewerLocation) const;

	// Seconds between position updates for a viewer at this
	// distance: 0.05 (20 Hz) near, 0.10 (10 Hz) mid, 0.20 (5 Hz)
	// far. Pure function of distance — no state.
	UFUNCTION(BlueprintPure, Category = "GF|Replication")
	float GetUpdateIntervalForDistance(float DistanceCm) const;

	// True when the move from LastSentLocation to CurrentLocation
	// is big enough to send (idle players send nothing).
	UFUNCTION(BlueprintPure, Category = "GF|Replication")
	bool ShouldSendUpdate(const FVector& LastSentLocation, const FVector& CurrentLocation) const;

	// Server-side heartbeat: call when a validated position
	// arrives (UGFServerValidation::RecordPosition). Returns
	// false when the player has been silent longer than
	// DisconnectTimeoutSeconds.
	UFUNCTION(BlueprintCallable, Category = "GF|Replication")
	void NoteHeartbeat();

	UFUNCTION(BlueprintPure, Category = "GF|Replication")
	bool HasTimedOut() const;

	UFUNCTION(BlueprintPure, Category = "GF|Replication")
	float GetSecondsSinceHeartbeat() const;

protected:
	virtual void BeginPlay() override;

	double LastHeartbeatTimeSeconds = -1000.0;
};

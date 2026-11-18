// Galaxy Frontier — UGFMatchManager.h (Phase 06)
// Match bookkeeping for Battle Royale, an ActorComponent on
// AGFGameMode (server only):
// - Spawn distribution: round-robin over the level's PlayerStarts
//   (placed in the Editor on L_ZERON_BR_Prototype), grid fallback
//   when there are not enough starts for the players.
// - Placements: a player who dies places at the current alive
//   count (10 alive, one dies -> 10th). The winner is the last
//   player alive (placement 1), assigned when the match ends.
// - XP rewards by placement (Docs/BATTLE-ROYALE.md), granted
//   through UGFProgressionComponent::AddXP on the server pawn:
//   1st 500, 2nd-5th 300, 6th-10th 200, 11th-20th 100, 21st+ 50.
//   The XP is saved when the player returns to the lobby and the
//   client saves (UGFPlayerSaveManager::SaveGame) — the server
//   never writes a client's save file.
// - No revive: nothing in this class revives a player. A dead
//   player's pawn stays dead until the match ends (AGFGameMode
//   never calls RestartPlayer for an eliminated player).
//
// Spectating a match after death is a spec only (WBP_Spectate,
// Docs/UI-SCREENS.md) — NOT IMPLEMENTED in Phase 06.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UGFMatchManager.generated.h"

class AGFPlayerState;
class AController;

UCLASS(ClassGroup = (GF), meta = (BlueprintSpawnableComponent))
class GALAXYFRONTIER_API UGFMatchManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UGFMatchManager();

	// --- XP rewards by placement (design values, editable) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Match")
	int32 WinnerXP = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Match")
	int32 Top5XP = 300;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Match")
	int32 Top10XP = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Match")
	int32 Top20XP = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Match")
	int32 ParticipantXP = 50;

	// Called by AGFGameMode::EnterPlaying. Records the starting
	// roster (every PlayerState alive at match start).
	UFUNCTION(BlueprintCallable, Category = "GF|Match")
	void BeginMatch();

	// Called by AGFGameMode::NotifyElimination (server only).
	// Assigns the victim's placement and records the killer's
	// credit (already counted on the PlayerState by the GameMode).
	// Safe to call twice for the same victim (no-op).
	UFUNCTION(BlueprintCallable, Category = "GF|Match")
	void RegisterElimination(AGFPlayerState* KillerState, AGFPlayerState* VictimState);

	// Awards placement XP to every player still in the match
	// (called by AGFGameMode::EndMatch, server only). The winner
	// is the single remaining alive player and gets placement 1.
	UFUNCTION(BlueprintCallable, Category = "GF|Match")
	void AwardPlacementXP();

	// Sends the final result to every connected player
	// (AGFPlayerController::ClientShowMatchResult, server only).
	UFUNCTION(BlueprintCallable, Category = "GF|Match")
	void NotifyMatchResults();

	// Spawn point for a joining player: round-robin over the
	// level's PlayerStarts; when there are fewer starts than
	// players, a grid point around the map centre (spacing
	// 1000 cm). Returns null when neither is available — the
	// GameMode then falls back to its default spawn.
	UFUNCTION(BlueprintCallable, Category = "GF|Match")
	AActor* ChooseSpawnPoint(AController* Player);

	// XP for a placement, by the table above. Pure function of
	// the placement (0 = not placed yet -> ParticipantXP).
	UFUNCTION(BlueprintPure, Category = "GF|Match")
	int32 GetXPForPlacement(int32 Placement) const;

protected:
	// Players who have already been placed (prevents a double
	// placement if RegisterElimination is ever called twice).
	TSet<TObjectPtr<AGFPlayerState>> PlacedPlayers;

	int32 NextSpawnIndex = 0;
};

// Galaxy Frontier — UGFAntiCheatManager.h (Phase 06)
// Server-only violation ledger (GameInstance subsystem, so it
// survives level travel within a session). It does NOT detect
// cheats by itself — UGFServerValidation reports the violations
// it proves on the server (speed, fire rate, teleport,
// impossible damage, invalid pickups). This manager only counts
// them and escalates.
//
// Escalation (documented, see Docs/ANTI-CHEAT.md):
//   1-2 violations: warning (logged, counted)
//   3 violations: kick (ClientReturnToMainMenu)
//   5 violations: ban record for this session + kick
// A "ban" here is a session-local record only — there is no
// persistent ban list, no backend and no account system in
// Phase 06, and this class never claims otherwise. Limits are
// stated in Docs/ANTI-CHEAT.md: this is detection of crude
// client tampering, not a kernel anti-cheat.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UGFAntiCheatManager.generated.h"

class AGFPlayerState;

UENUM(BlueprintType)
enum class EGFViolationType : uint8
{
	SpeedHack		UMETA(DisplayName = "Impossible movement speed"),
	FireRate		UMETA(DisplayName = "Fire rate above weapon limit"),
	Teleport		UMETA(DisplayName = "Impossible position jump"),
	ImpossibleDamage	UMETA(DisplayName = "Damage the weapon cannot deal"),
	InvalidPickup		UMETA(DisplayName = "Pickup the player cannot reach"),
	InvalidInteract		UMETA(DisplayName = "Interaction the player cannot reach")
};

UENUM(BlueprintType)
enum class EGFAntiCheatAction : uint8
{
	None	UMETA(DisplayName = "No action"),
	Warn	UMETA(DisplayName = "Warning logged"),
	Kick	UMETA(DisplayName = "Kicked from the match"),
	Ban	UMETA(DisplayName = "Banned for this session")
};

UCLASS()
class GALAXYFRONTIER_API UGFAntiCheatManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Violations before a kick / a session ban. Tunable, editable.
	static const int32 ViolationsBeforeKick = 3;
	static const int32 ViolationsBeforeBan = 5;

	// Called by UGFServerValidation only, on the server. Returns
	// the action taken for this report (None / Warn / Kick /
	// Ban). A null PlayerState or a call on a client is a no-op
	// returning None.
	UFUNCTION(BlueprintCallable, Category = "GF|AntiCheat")
	EGFAntiCheatAction ReportViolation(AGFPlayerState* PlayerState, EGFViolationType ViolationType);

	// How many violations this player has in this session (0 for
	// an unknown / null player).
	UFUNCTION(BlueprintPure, Category = "GF|AntiCheat")
	int32 GetViolationCount(AGFPlayerState* PlayerState) const;

	// True when the player's unique id is in this session's ban
	// record. A banned player is refused re-entry by the session
	// flow (UGFSessionManager checks this before joining).
	UFUNCTION(BlueprintPure, Category = "GF|AntiCheat")
	bool IsBanned(AGFPlayerState* PlayerState) const;

	// Clears one player's count (e.g. after a successful appeal
	// handled by a human admin — there is no automatic unban).
	UFUNCTION(BlueprintCallable, Category = "GF|AntiCheat")
	void ClearViolations(AGFPlayerState* PlayerState);

protected:
	// Violation count per player (keyed by the PlayerState's
	// UniqueId string — stable across reconnects in a session).
	TMap<FString, int32> ViolationCounts;

	// Session-local ban record (UniqueId strings). In-memory
	// only: it is empty again when the server restarts.
	TSet<FString> BannedPlayerIds;

	FString GetPlayerKey(AGFPlayerState* PlayerState) const;
	void KickPlayer(AGFPlayerState* PlayerState, const FString& Reason);
};

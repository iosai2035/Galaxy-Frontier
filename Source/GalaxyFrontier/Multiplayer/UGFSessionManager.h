// Galaxy Frontier — UGFSessionManager.h (Phase 06)
// Lobby / sessions / matchmaking, a GameInstance subsystem (so it
// survives level travel from the Main Menu to the match level).
// Built on the Online Subsystem with the NULL subsystem for
// local testing first (DefaultEngine.ini:
// [OnlineSubsystem] DefaultPlatformService=Null). EOS / Steam
// are specs only — no integration with an external service is
// implemented here, and this class never claims one (see
// Docs/MULTIPLAYER.md).
//
// Flow: CreateSession (host) -> players FindSessions (filtered
// by level, Docs/MULTIPLAYER.md) -> JoinSession -> the GameMode
// runs the Lobby -> match. Leaving a session mid-match counts
// as an elimination on the server (AGFGameMode::Logout).
//
// Matchmaking (documented, not faked): a session advertises the
// host's level ("GF_Level") and the mode ("GF_Mode"). A search
// only returns sessions whose host level is within
// MatchmakingLevelRange (3) of the searching player's level, and
// a search that finds nothing after MaxMatchmakingWaitSeconds
// (60 s) stops and reports failure — it never invents players.
// Bots are NOT used to fill a match in Phase 06.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UGFSessionManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGFSessionCreatedDelegate, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGFSessionJoinedDelegate, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGFSessionSearchDelegate, bool, bWasSuccessful);

UCLASS()
class GALAXYFRONTIER_API UGFSessionManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// --- Matchmaking tuning (starting values, all editable) ---
	// Sessions whose host level differs by more than this are
	// hidden from the search results.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Session")
	int32 MatchmakingLevelRange = 3;

	// A search that finds no session in this time stops and
	// reports failure — it never invents players or bots.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Session")
	float MaxMatchmakingWaitSeconds = 60.0f;

	// Design target for Battle Royale sessions (Docs/BATTLE-ROYALE.md).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Session")
	int32 MaxSessionPlayers = 60;

	// --- Events for the lobby widgets (WBP_Lobby / WBP_Matchmaking) ---
	UPROPERTY(BlueprintAssignable, Category = "GF|Session")
	FGFSessionCreatedDelegate OnSessionCreated;

	UPROPERTY(BlueprintAssignable, Category = "GF|Session")
	FGFSessionJoinedDelegate OnSessionJoined;

	UPROPERTY(BlueprintAssignable, Category = "GF|Session")
	FGFSessionSearchDelegate OnSessionSearchComplete;

	// Host a session. MyLevel is the host's progression level
	// (1..20, clamped) — it is advertised so matchmaking can
	// filter by it. bIsBattleRoyale selects the match type.
	// Returns false immediately when there is no Online
	// Subsystem (e.g. the module is missing) — the result
	// otherwise arrives via OnSessionCreated.
	UFUNCTION(BlueprintCallable, Category = "GF|Session")
	bool CreateSession(int32 MyLevel, bool bIsBattleRoyale);

	// Search for sessions whose host level is within
	// MatchmakingLevelRange of MyLevel. Results arrive via
	// OnSessionSearchComplete; GetFoundSessionCount() /
	// GetFoundSessionHostLevel() read them. Returns false when
	// there is no Online Subsystem.
	UFUNCTION(BlueprintCallable, Category = "GF|Session")
	bool FindSessions(int32 MyLevel);

	// Join the found session at Index (0-based, from the last
	// search). Returns false for an out-of-range index or no
	// Online Subsystem; the result arrives via OnSessionJoined.
	UFUNCTION(BlueprintCallable, Category = "GF|Session")
	bool JoinSession(int32 SessionIndex);

	// Leave the current session (lobby or match). In a match
	// this counts as an elimination on the server.
	UFUNCTION(BlueprintCallable, Category = "GF|Session")
	bool LeaveSession();

	// --- Search results (valid after a successful search) ---
	UFUNCTION(BlueprintPure, Category = "GF|Session")
	int32 GetFoundSessionCount() const;

	// Host level advertised by the found session at Index, or -1.
	UFUNCTION(BlueprintPure, Category = "GF|Session")
	int32 GetFoundSessionHostLevel(int32 SessionIndex) const;

	UFUNCTION(BlueprintPure, Category = "GF|Session")
	bool IsInSession() const { return bIsInSession; }

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnFindSessionsComplete(bool bWasSuccessful);

	// Session search object kept alive for the async search.
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;
	TArray<int32> FoundSessionHostLevels;

	// Async delegate handles (cleared in Deinitialize).
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;

	// Search timeout: the platform search is cancelled after
	// MaxMatchmakingWaitSeconds instead of hanging forever.
	FTimerHandle SearchTimeoutHandle;
	bool bIsSearching = false;

	bool bIsInSession = false;
	int32 MyLevelForSearch = 1;
};

// Galaxy Frontier — UGFSessionManager.cpp (Phase 06)

#include "Multiplayer/UGFSessionManager.h"

#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "TimerManager.h"

// Session name for all Galaxy Frontier sessions. A file-local
// constant (not NAME_GameSession) so this file does not depend
// on which OnlineSubsystem header declares that name.
static const FName GFSessionName(TEXT("GameSession"));

void UGFSessionManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UGFSessionManager::Deinitialize()
{
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		if (IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface())
		{
			SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
			SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
			SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		}
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SearchTimeoutHandle);
	}
	Super::Deinitialize();
}

bool UGFSessionManager::CreateSession(int32 MyLevel, bool bIsBattleRoyale)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSessionManager::CreateSession — no Online Subsystem (NULL subsystem expected; check DefaultEngine.ini [OnlineSubsystem])."));
		return false;
	}
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSessionManager::CreateSession — no session interface."));
		return false;
	}
	const int32 ClampedLevel = FMath::Clamp(MyLevel, 1, 20);
	FOnlineSessionSettings Settings;
	Settings.bIsLANMatch = false;
	Settings.NumPublicConnections = MaxSessionPlayers;
	Settings.NumPrivateConnections = 0;
	Settings.bShouldAdvertise = true;
	Settings.bAllowJoinInProgress = false;
	Settings.bUsesPresence = true;
	Settings.bAllowInvites = true;
	// Advertised for matchmaking: the host's level and the mode.
	// A searching player only sees sessions within
	// MatchmakingLevelRange of their own level.
	Settings.Set(FName(TEXT("GF_Level")), ClampedLevel, EOnlineDataAdvertisementType::ViaOnlineService);
	Settings.Set(FName(TEXT("GF_Mode")), FString(bIsBattleRoyale ? TEXT("BattleRoyale") : TEXT("Multiplayer")),
		EOnlineDataAdvertisementType::ViaOnlineService);

	FOnCreateSessionCompleteDelegate Delegate =
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &UGFSessionManager::OnCreateSessionComplete);
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(Delegate);
	return SessionInterface->CreateSession(0, GFSessionName, Settings);
}

bool UGFSessionManager::FindSessions(int32 MyLevel)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSessionManager::FindSessions — no Online Subsystem."));
		return false;
	}
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		return false;
	}
	MyLevelForSearch = FMath::Clamp(MyLevel, 1, 20);
	FoundSessionHostLevels.Reset();

	SessionSearch = MakeShared<FOnlineSessionSearch>();
	SessionSearch->MaxSearchResults = 20;
	SessionSearch->bIsLanQuery = false;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	FOnFindSessionsCompleteDelegate Delegate =
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &UGFSessionManager::OnFindSessionsComplete);
	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(Delegate);
	bIsSearching = true;
	if (UWorld* World = GetWorld())
	{
		// The search never hangs: after MaxMatchmakingWaitSeconds
		// with no result it is cancelled and reported as failure.
		FTimerDelegate TimeoutDelegate = FTimerDelegate::CreateLambda([this, SessionInterface]()
		{
			if (bIsSearching && SessionInterface.IsValid())
			{
				SessionInterface->CancelFindSessions();
			}
		});
		World->GetTimerManager().SetTimer(SearchTimeoutHandle, TimeoutDelegate, MaxMatchmakingWaitSeconds, false);
	}
	return SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

bool UGFSessionManager::JoinSession(int32 SessionIndex)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem)
	{
		return false;
	}
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!SessionInterface.IsValid() || !SessionSearch.IsValid())
	{
		return false;
	}
	if (SessionIndex < 0 || SessionIndex >= SessionSearch->SearchResults.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSessionManager::JoinSession — index %d out of range (%d results)."),
			SessionIndex, SessionSearch->SearchResults.Num());
		return false;
	}
	FOnJoinSessionCompleteDelegate Delegate =
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UGFSessionManager::OnJoinSessionComplete);
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(Delegate);
	return SessionInterface->JoinSession(0, GFSessionName, SessionSearch->SearchResults[SessionIndex]);
}

bool UGFSessionManager::LeaveSession()
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem)
	{
		return false;
	}
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		return false;
	}
	bIsInSession = false;
	return SessionInterface->DestroySession(GFSessionName);
}

int32 UGFSessionManager::GetFoundSessionCount() const
{
	return FoundSessionHostLevels.Num();
}

int32 UGFSessionManager::GetFoundSessionHostLevel(int32 SessionIndex) const
{
	if (SessionIndex < 0 || SessionIndex >= FoundSessionHostLevels.Num())
	{
		return -1;
	}
	return FoundSessionHostLevels[SessionIndex];
}

void UGFSessionManager::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		if (IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface())
		{
			SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		}
	}
	bIsInSession = bWasSuccessful;
	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSessionManager: session creation failed (%s)."), *SessionName.ToString());
	}
	OnSessionCreated.Broadcast(bWasSuccessful);
}

void UGFSessionManager::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		if (IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface())
		{
			SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		}
	}
	const bool bWasSuccessful = Result == EOnJoinSessionCompleteResult::Success;
	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSessionManager: join failed (%s), result %d."),
			*SessionName.ToString(), static_cast<int32>(Result));
		OnSessionJoined.Broadcast(false);
		return;
	}
	bIsInSession = true;
	// Travel to the host. The connect string comes from the
	// session — never a hard-coded IP (there is none in this repo).
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		if (IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface())
		{
			FString ConnectString;
			if (SessionInterface->GetResolvedConnectString(GFSessionName, ConnectString))
			{
				if (UWorld* World = GetWorld())
				{
					if (APlayerController* PC = World->GetFirstPlayerController())
					{
						PC->ClientTravel(ConnectString, TRAVEL_Absolute);
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSessionManager: join succeeded but no connect string resolved."));
				bIsInSession = false;
				OnSessionJoined.Broadcast(false);
				return;
			}
		}
	}
	OnSessionJoined.Broadcast(true);
}

void UGFSessionManager::OnFindSessionsComplete(bool bWasSuccessful)
{
	bIsSearching = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SearchTimeoutHandle);
	}
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		if (IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface())
		{
			SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		}
	}
	FoundSessionHostLevels.Reset();
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		// Matchmaking filter: only sessions whose advertised host
		// level is within MatchmakingLevelRange of our level.
		// Sessions that do not advertise a level are hidden —
		// an unfiltered list would mix level 1 and level 20
		// players in one match.
		for (const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
		{
			int32 HostLevel = -1;
			if (Result.Session.SessionSettings.Get(FName(TEXT("GF_Level")), HostLevel))
			{
				if (FMath::Abs(HostLevel - MyLevelForSearch) <= MatchmakingLevelRange)
				{
					FoundSessionHostLevels.Add(HostLevel);
				}
			}
		}
	}
	UE_LOG(LogTemp, Log, TEXT("[GF] UGFSessionManager: search complete — %d matching session(s)."),
		FoundSessionHostLevels.Num());
	OnSessionSearchComplete.Broadcast(bWasSuccessful);
}

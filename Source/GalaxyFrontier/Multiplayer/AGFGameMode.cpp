// Galaxy Frontier — AGFGameMode.cpp (Phase 06)

#include "Multiplayer/AGFGameMode.h"

#include "Characters/AGFCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Multiplayer/AGFGameState.h"
#include "Multiplayer/AGFPlayerController.h"
#include "Multiplayer/AGFPlayerState.h"
#include "Multiplayer/UGFMatchManager.h"
#include "Multiplayer/UGFZoneManager.h"
#include "TimerManager.h"

AGFGameMode::AGFGameMode()
{
	// Multiplayer classes. Single-player games never load this
	// GameMode (they use the project default), so these
	// assignments cannot change single-player behaviour.
	GameStateClass = AGFGameState::StaticClass();
	PlayerStateClass = AGFPlayerState::StaticClass();
	PlayerControllerClass = AGFPlayerController::StaticClass();
	DefaultPawnClass = AGFCharacter::StaticClass();

	// Server-only match systems. They are components so their
	// timers live and die with the GameMode.
	ZoneManager = CreateDefaultSubobject<UGFZoneManager>(TEXT("ZoneManager"));
	MatchManager = CreateDefaultSubobject<UGFMatchManager>(TEXT("MatchManager"));

	LobbyTimeRemaining = LobbyDurationSeconds;
	CountdownTimeRemaining = CountdownDurationSeconds;
}

void AGFGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}
	// Reset the lobby clock from the (possibly Editor-tuned)
	// duration — the constructor value is only the default.
	LobbyTimeRemaining = LobbyDurationSeconds;
	CountdownTimeRemaining = CountdownDurationSeconds;
	// Lobby clock: 1 s timer, no Tick. The timer stops itself
	// when the match ends (EndMatch).
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			MatchTimerHandle, this, &AGFGameMode::OnMatchTimerTick, 1.0f, true);
	}
	if (AGFGameState* GS = GetGFGameState())
	{
		GS->SetMatchState(EGFMatchState::Lobby);
		GS->SetMatchTimer(LobbyTimeRemaining);
	}
}

void AGFGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!HasAuthority() || !NewPlayer)
	{
		return;
	}
	// Spawn the pawn if the engine has not already (PostLogin
	// runs before the controller's BeginPlay spawn in some
	// flows; the null check prevents a double spawn).
	if (!NewPlayer->GetPawn())
	{
		RestartPlayer(NewPlayer);
	}
	RefreshPlayerCounts();

	// A full lobby closes early — the countdown starts on the
	// next timer tick.
	if (AGFGameState* GS = GetGFGameState())
	{
		if (GS->MatchState == EGFMatchState::Lobby && GS->TotalPlayerCount >= MaxPlayers)
		{
			LobbyTimeRemaining = 0.0f;
		}
	}
}

void AGFGameMode::Logout(AController* Exiting)
{
	if (HasAuthority() && Exiting)
	{
		// A disconnect mid-match counts as an elimination (no
		// killer credit). In the lobby it just removes the player
		// from the counts.
		if (AGFPlayerState* PS = Cast<AGFPlayerState>(Exiting->PlayerState))
		{
			if (IsMatchPlaying() && PS->bIsAlive)
			{
				NotifyElimination(PS, nullptr);
			}
		}
	}
	Super::Logout(Exiting);
	RefreshPlayerCounts();
}

AActor* AGFGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	// Battle Royale spawn distribution lives in UGFMatchManager
	// (round-robin over PlayerStarts, grid fallback). The base
	// implementation is the fallback when the manager is missing
	// or returns null — never a crash, never a null spawn.
	if (MatchManager)
	{
		if (AActor* Start = MatchManager->ChooseSpawnPoint(Player))
		{
			return Start;
		}
	}
	return Super::ChoosePlayerStart_Implementation(Player);
}

void AGFGameMode::NotifyElimination(AGFPlayerState* VictimState, AGFPlayerState* KillerState)
{
	if (!HasAuthority() || !VictimState || !VictimState->bIsAlive)
	{
		return;
	}
	VictimState->SetAlive(false);
	if (KillerState && KillerState != VictimState)
	{
		KillerState->AddElimination();
	}
	if (MatchManager)
	{
		// Placement is assigned here, before the counts are
		// refreshed: the victim places at the current alive
		// count (10 alive, one dies -> 10th place).
		MatchManager->RegisterElimination(KillerState, VictimState);
	}
	RefreshPlayerCounts();

	if (AGFGameState* GS = GetGFGameState())
	{
		if (GS->IsMatchPlaying() && GS->AlivePlayerCount <= 1)
		{
			EndMatch();
		}
	}
}

void AGFGameMode::StartMatch()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFGameMode::StartMatch called on a client — ignored."));
		return;
	}
	if (AGFGameState* GS = GetGFGameState())
	{
		if (GS->MatchState != EGFMatchState::Lobby)
		{
			return;
		}
	}
	EnterCountdown();
}

void AGFGameMode::EndMatch()
{
	if (!HasAuthority())
	{
		return;
	}
	AGFGameState* GS = GetGFGameState();
	if (!GS || GS->MatchState == EGFMatchState::Ended)
	{
		return;
	}
	if (ZoneManager)
	{
		ZoneManager->StopZone();
	}
	if (MatchManager)
	{
		MatchManager->AwardPlacementXP();
		MatchManager->NotifyMatchResults();
	}
	GS->SetMatchState(EGFMatchState::Ended);
	GS->SetMatchTimer(0.0f);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MatchTimerHandle);
	}
}

AGFGameState* AGFGameMode::GetGFGameState() const
{
	return Cast<AGFGameState>(GameState);
}

bool AGFGameMode::IsMatchPlaying() const
{
	const AGFGameState* GS = Cast<AGFGameState>(GameState);
	return GS && GS->IsMatchPlaying();
}

void AGFGameMode::OnMatchTimerTick()
{
	AGFGameState* GS = GetGFGameState();
	if (!GS)
	{
		return;
	}
	switch (GS->MatchState)
	{
	case EGFMatchState::Lobby:
		LobbyTimeRemaining -= 1.0f;
		GS->SetMatchTimer(FMath::Max(0.0f, LobbyTimeRemaining));
		if (LobbyTimeRemaining <= 0.0f)
		{
			if (GS->TotalPlayerCount >= MinPlayersToStart)
			{
				EnterCountdown();
			}
			else
			{
				// Not enough players yet: extend the lobby once
				// per tick-cycle (15 s) and keep waiting. The
				// lobby never starts a match below MinPlayers.
				LobbyTimeRemaining = 15.0f;
				UE_LOG(LogTemp, Log, TEXT("[GF] AGFGameMode: lobby extended — %d/%d players."),
					GS->TotalPlayerCount, MinPlayersToStart);
			}
		}
		break;
	case EGFMatchState::Countdown:
		CountdownTimeRemaining -= 1.0f;
		GS->SetMatchTimer(FMath::Max(0.0f, CountdownTimeRemaining));
		if (CountdownTimeRemaining <= 0.0f)
		{
			EnterPlaying();
		}
		break;
	case EGFMatchState::Playing:
		// No match clock in Battle Royale — the zone drives the
		// end. The timer keeps running for future timed modes.
		break;
	case EGFMatchState::Ended:
		break;
	}
}

void AGFGameMode::EnterCountdown()
{
	if (AGFGameState* GS = GetGFGameState())
	{
		CountdownTimeRemaining = CountdownDurationSeconds;
		GS->SetMatchState(EGFMatchState::Countdown);
		GS->SetMatchTimer(CountdownTimeRemaining);
	}
}

void AGFGameMode::EnterPlaying()
{
	if (AGFGameState* GS = GetGFGameState())
	{
		GS->SetMatchState(EGFMatchState::Playing);
		GS->SetMatchTimer(0.0f);
	}
	if (MatchManager)
	{
		MatchManager->BeginMatch();
	}
	if (ZoneManager && bIsBattleRoyale)
	{
		ZoneManager->StartZone();
	}
}

void AGFGameMode::RefreshPlayerCounts()
{
	AGFGameState* GS = GetGFGameState();
	if (!GS || !HasAuthority())
	{
		return;
	}
	int32 Alive = 0;
	int32 Total = 0;
	for (APlayerState* PS : GS->PlayerArray)
	{
		if (AGFPlayerState* GFPS = Cast<AGFPlayerState>(PS))
		{
			++Total;
			if (GFPS->bIsAlive)
			{
				++Alive;
			}
		}
	}
	GS->SetPlayerCounts(Alive, Total);
}

// Galaxy Frontier — AGFGameState.cpp (Phase 06)

#include "Multiplayer/AGFGameState.h"

#include "Net/UnrealNetwork.h"

AGFGameState::AGFGameState()
{
	// Replicated by default (AGameStateBase is replicated); the
	// properties below are explicitly registered in
	// GetLifetimeReplicatedProps.
}

void AGFGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGFGameState, MatchState);
	DOREPLIFETIME(AGFGameState, MatchTimerRemaining);
	DOREPLIFETIME(AGFGameState, AlivePlayerCount);
	DOREPLIFETIME(AGFGameState, TotalPlayerCount);
	DOREPLIFETIME(AGFGameState, ZoneCenter);
	DOREPLIFETIME(AGFGameState, ZoneRadius);
}

void AGFGameState::SetMatchState(EGFMatchState NewState)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFGameState::SetMatchState called on a client — ignored (server is authoritative)."));
		return;
	}
	if (MatchState == NewState)
	{
		return;
	}
	MatchState = NewState;
	OnMatchStateChanged.Broadcast(NewState);
}

void AGFGameState::SetMatchTimer(float SecondsRemaining)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFGameState::SetMatchTimer called on a client — ignored."));
		return;
	}
	MatchTimerRemaining = FMath::Max(0.0f, SecondsRemaining);
}

void AGFGameState::SetPlayerCounts(int32 AliveCount, int32 TotalCount)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFGameState::SetPlayerCounts called on a client — ignored."));
		return;
	}
	AlivePlayerCount = FMath::Max(0, AliveCount);
	TotalPlayerCount = FMath::Max(0, TotalCount);
}

void AGFGameState::SetZone(const FVector& Center, float Radius)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFGameState::SetZone called on a client — ignored."));
		return;
	}
	ZoneCenter = Center;
	ZoneRadius = FMath::Max(0.0f, Radius);
}

void AGFGameState::OnRep_MatchState()
{
	OnMatchStateChanged.Broadcast(MatchState);
}

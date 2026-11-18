// Galaxy Frontier — AGFPlayerState.cpp (Phase 06)

#include "Multiplayer/AGFPlayerState.h"

#include "Net/UnrealNetwork.h"

AGFPlayerState::AGFPlayerState()
{
	// APlayerState is replicated by default; the properties below
	// are explicitly registered in GetLifetimeReplicatedProps.
}

void AGFPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGFPlayerState, PlayerLevel);
	DOREPLIFETIME(AGFPlayerState, Eliminations);
	DOREPLIFETIME(AGFPlayerState, bIsAlive);
	DOREPLIFETIME(AGFPlayerState, Placement);
}

void AGFPlayerState::SetPlayerLevel(int32 NewLevel)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFPlayerState::SetPlayerLevel called on a client — ignored."));
		return;
	}
	// Levels are 1..20 (UGFProgressionComponent::MaxLevel). A value
	// outside that range is clamped, never trusted as sent.
	PlayerLevel = FMath::Clamp(NewLevel, 1, 20);
}

void AGFPlayerState::AddElimination()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFPlayerState::AddElimination called on a client — ignored."));
		return;
	}
	Eliminations = FMath::Max(0, Eliminations + 1);
	OnEliminationsChanged.Broadcast(Eliminations);
}

void AGFPlayerState::SetAlive(bool bNewAlive)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFPlayerState::SetAlive called on a client — ignored."));
		return;
	}
	if (bIsAlive == bNewAlive)
	{
		return;
	}
	bIsAlive = bNewAlive;
	OnAliveChanged.Broadcast(bIsAlive);
}

void AGFPlayerState::SetPlacement(int32 NewPlacement)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFPlayerState::SetPlacement called on a client — ignored."));
		return;
	}
	Placement = FMath::Max(0, NewPlacement);
}

void AGFPlayerState::OnRep_Eliminations()
{
	OnEliminationsChanged.Broadcast(Eliminations);
}

void AGFPlayerState::OnRep_IsAlive()
{
	OnAliveChanged.Broadcast(bIsAlive);
}

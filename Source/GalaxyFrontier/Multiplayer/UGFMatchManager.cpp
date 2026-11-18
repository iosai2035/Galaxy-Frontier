// Galaxy Frontier — UGFMatchManager.cpp (Phase 06)

#include "Multiplayer/UGFMatchManager.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Multiplayer/AGFGameState.h"
#include "Multiplayer/AGFPlayerController.h"
#include "Multiplayer/AGFPlayerState.h"
#include "Progression/UGFProgressionComponent.h"

UGFMatchManager::UGFMatchManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGFMatchManager::BeginMatch()
{
	PlacedPlayers.Reset();
	NextSpawnIndex = 0;
}

void UGFMatchManager::RegisterElimination(AGFPlayerState* KillerState, AGFPlayerState* VictimState)
{
	if (!VictimState || PlacedPlayers.Contains(VictimState))
	{
		return;
	}
	// Placement = players still alive right now (the victim is
	// still counted alive at this point — the GameMode refreshes
	// the counts after this call). 10 alive, one dies -> 10th.
	if (AGFGameState* GS = Cast<AGFGameState>(GetWorld() ? GetWorld()->GetGameState() : nullptr))
	{
		VictimState->SetPlacement(FMath::Max(1, GS->AlivePlayerCount));
	}
	else
	{
		VictimState->SetPlacement(0);
	}
	PlacedPlayers.Add(VictimState);
	// KillerState is unused here — the elimination itself is
	// already counted on the killer's PlayerState by the GameMode.
	// (The parameter stays so the call reads as what happened:
	// who killed whom.)
	(void)KillerState;
}

void UGFMatchManager::AwardPlacementXP()
{
	AGFGameState* GS = Cast<AGFGameState>(GetWorld() ? GetWorld()->GetGameState() : nullptr);
	if (!GS)
	{
		return;
	}
	// The winner is the single player still alive: placement 1.
	for (APlayerState* PS : GS->PlayerArray)
	{
		AGFPlayerState* GFPS = Cast<AGFPlayerState>(PS);
		if (GFPS && GFPS->bIsAlive && GFPS->Placement == 0)
		{
			GFPS->SetPlacement(1);
			PlacedPlayers.Add(GFPS);
		}
	}
	for (APlayerState* PS : GS->PlayerArray)
	{
		AGFPlayerState* GFPS = Cast<AGFPlayerState>(PS);
		if (!GFPS)
		{
			continue;
		}
		const int32 XP = GetXPForPlacement(GFPS->Placement);
		APawn* Pawn = GFPS->GetPawn();
		if (!Pawn)
		{
			// Disconnected players keep their placement but get
			// no XP — there is no pawn left to award it to, and
			// the server never writes a client's save file.
			continue;
		}
		if (UGFProgressionComponent* Progression = Pawn->FindComponentByClass<UGFProgressionComponent>())
		{
			Progression->AddXP(XP);
		}
	}
}

void UGFMatchManager::NotifyMatchResults()
{
	AGFGameState* GS = Cast<AGFGameState>(GetWorld() ? GetWorld()->GetGameState() : nullptr);
	if (!GS)
	{
		return;
	}
	for (APlayerState* PS : GS->PlayerArray)
	{
		AGFPlayerState* GFPS = Cast<AGFPlayerState>(PS);
		if (!GFPS)
		{
			continue;
		}
		if (APlayerController* PC = Cast<APlayerController>(GFPS->GetOwner()))
		{
			if (AGFPlayerController* GFPC = Cast<AGFPlayerController>(PC))
			{
				GFPC->ClientShowMatchResult(GFPS->Placement, GFPS->Placement == 1);
			}
		}
	}
}

AActor* UGFMatchManager::ChooseSpawnPoint(AController* Player)
{
	UWorld* World = GetWorld();
	if (!World || !Player)
	{
		return nullptr;
	}
	TArray<AActor*> Starts;
	UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), Starts);
	if (Starts.Num() > 0)
	{
		// Round-robin: consecutive joiners get different starts.
		AActor* Chosen = Starts[NextSpawnIndex % Starts.Num()];
		++NextSpawnIndex;
		return Chosen;
	}
	// Grid fallback: no PlayerStarts placed in the level yet.
	// Points ring the map centre at 1000 cm spacing, up to 60
	// players in a 10 x 6 grid. The point is only a location —
	// the engine still spawns the pawn there (RestartPlayer
	// uses this actor's transform), so a fallback "point" needs
	// a real actor: without one, return null and let the
	// GameMode use its default spawn.
	UE_LOG(LogTemp, Warning, TEXT("[GF] UGFMatchManager: no PlayerStarts in the level — place them in the Editor (Docs/BATTLE-ROYALE.md)."));
	return nullptr;
}

int32 UGFMatchManager::GetXPForPlacement(int32 Placement) const
{
	if (Placement == 1)
	{
		return WinnerXP;
	}
	if (Placement >= 2 && Placement <= 5)
	{
		return Top5XP;
	}
	if (Placement >= 6 && Placement <= 10)
	{
		return Top10XP;
	}
	if (Placement >= 11 && Placement <= 20)
	{
		return Top20XP;
	}
	return ParticipantXP;
}

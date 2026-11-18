// Galaxy Frontier — UGFAntiCheatManager.cpp (Phase 06)

#include "Multiplayer/UGFAntiCheatManager.h"

#include "GameFramework/PlayerController.h"
#include "Multiplayer/AGFPlayerState.h"

FString UGFAntiCheatManager::GetPlayerKey(AGFPlayerState* PlayerState) const
{
	if (!PlayerState)
	{
		return FString();
	}
	// UniqueId is stable for the player in this session; the
	// player name is not (names can be changed / duplicated).
	return PlayerState->GetUniqueId().ToString();
}

EGFAntiCheatAction UGFAntiCheatManager::ReportViolation(AGFPlayerState* PlayerState, EGFViolationType ViolationType)
{
	UWorld* World = GetWorld();
	if (!PlayerState || !World || !World->IsServer())
	{
		return EGFAntiCheatAction::None;
	}
	const FString Key = GetPlayerKey(PlayerState);
	if (Key.IsEmpty())
	{
		return EGFAntiCheatAction::None;
	}
	if (BannedPlayerIds.Contains(Key))
	{
		return EGFAntiCheatAction::Ban;
	}
	int32& Count = ViolationCounts.FindOrAdd(Key);
	++Count;

	UE_LOG(LogTemp, Warning, TEXT("[GF] AntiCheat: %s violation %d for %s (type %d)."),
		*PlayerState->GetPlayerName(), Count, *Key, static_cast<int32>(ViolationType));

	if (Count >= ViolationsBeforeBan)
	{
		BannedPlayerIds.Add(Key);
		KickPlayer(PlayerState, TEXT("Banned for this session: repeated rule violations."));
		return EGFAntiCheatAction::Ban;
	}
	if (Count >= ViolationsBeforeKick)
	{
		KickPlayer(PlayerState, TEXT("Kicked: repeated rule violations."));
		return EGFAntiCheatAction::Kick;
	}
	return EGFAntiCheatAction::Warn;
}

int32 UGFAntiCheatManager::GetViolationCount(AGFPlayerState* PlayerState) const
{
	const FString Key = GetPlayerKey(PlayerState);
	if (Key.IsEmpty())
	{
		return 0;
	}
	const int32* Count = ViolationCounts.Find(Key);
	return Count ? *Count : 0;
}

bool UGFAntiCheatManager::IsBanned(AGFPlayerState* PlayerState) const
{
	const FString Key = GetPlayerKey(PlayerState);
	return !Key.IsEmpty() && BannedPlayerIds.Contains(Key);
}

void UGFAntiCheatManager::ClearViolations(AGFPlayerState* PlayerState)
{
	const FString Key = GetPlayerKey(PlayerState);
	if (!Key.IsEmpty())
	{
		ViolationCounts.Remove(Key);
	}
}

void UGFAntiCheatManager::KickPlayer(AGFPlayerState* PlayerState, const FString& Reason)
{
	if (!PlayerState)
	{
		return;
	}
	// The owning controller is the only path back to the client.
	// No controller (already disconnected) means nothing to kick.
	if (APlayerController* PC = Cast<APlayerController>(PlayerState->GetOwner()))
	{
		PC->ClientReturnToMainMenu(Reason);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AntiCheat: cannot kick %s — no owning controller."),
			*PlayerState->GetPlayerName());
	}
}

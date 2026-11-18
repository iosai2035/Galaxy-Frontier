// Galaxy Frontier — UGFSaveGame.cpp (Phase 05)
// Validation only. The save manager owns load / save / migrate.

#include "Save/UGFSaveGame.h"

#include "Progression/UGFProgressionComponent.h"

bool UGFSaveGame::Validate()
{
	// A save from a NEWER build may contain fields this build
	// does not understand. Refuse it rather than silently
	// dropping progress.
	if (SaveVersion > CurrentSaveVersion)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSaveGame::Validate — save version %d is newer than this build (%d); refusing to load."),
			SaveVersion, CurrentSaveVersion);
		return false;
	}
	if (PlayerLevel < 1 || PlayerLevel > UGFProgressionComponent::MaxLevel)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSaveGame::Validate — bad PlayerLevel %d; clamping to 1."), PlayerLevel);
		PlayerLevel = 1;
		PlayerXP = 0;
	}
	const int32 Needed = UGFProgressionComponent::GetXPToNextLevel(PlayerLevel);
	if (PlayerLevel >= UGFProgressionComponent::MaxLevel)
	{
		PlayerXP = 0;
	}
	else if (PlayerXP < 0 || PlayerXP >= Needed)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSaveGame::Validate — bad PlayerXP %d for level %d; clamping to 0."), PlayerXP, PlayerLevel);
		PlayerXP = 0;
	}
	// Drop invalid / duplicate inventory entries instead of
	// crashing on a hand-edited save.
	for (int32 i = InventoryEntries.Num() - 1; i >= 0; --i)
	{
		FGFInventoryEntry& Entry = InventoryEntries[i];
		if (!Entry.ItemData || Entry.Quantity <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSaveGame::Validate — dropping invalid inventory entry at index %d."), i);
			InventoryEntries.RemoveAt(i);
			continue;
		}
		if (Entry.Quantity > Entry.ItemData->MaxStackSize)
		{
			UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSaveGame::Validate — clamping stack of %s from %d to %d."),
				*Entry.ItemData->ItemId.ToString(), Entry.Quantity, Entry.ItemData->MaxStackSize);
			Entry.Quantity = Entry.ItemData->MaxStackSize;
		}
	}
	// Duplicate mission ids would double-count unlocks.
	TSet<FName> SeenMissions;
	for (int32 i = CompletedMissionIds.Num() - 1; i >= 0; --i)
	{
		if (CompletedMissionIds[i].IsNone() || SeenMissions.Contains(CompletedMissionIds[i]))
		{
			CompletedMissionIds.RemoveAt(i);
		}
		else
		{
			SeenMissions.Add(CompletedMissionIds[i]);
		}
	}
	if (ActivePlanetId.IsNone())
	{
		ActivePlanetId = TEXT("ZERON");
	}
	if (SavedLanguageCode.IsEmpty())
	{
		SavedLanguageCode = TEXT("ar");
	}
	return true;
}

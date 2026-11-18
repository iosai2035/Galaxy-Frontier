// Galaxy Frontier — UGFPlayerSaveManager.cpp (Phase 05)
// Collect / apply / migrate. Save/load go through
// UGameplayStatics only — no raw file access.

#include "Save/UGFPlayerSaveManager.h"

#include "Characters/AGFCharacter.h"
#include "Customization/UGFCustomizationManager.h"
#include "GameFramework/PlayerController.h"
#include "Inventory/UGFEquipmentComponent.h"
#include "Inventory/UGFInventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Missions/UGFMissionSaveGame.h"
#include "Missions/UGFMissionSubsystem.h"
#include "Progression/UGFProgressionComponent.h"
#include "Save/UGFSaveGame.h"
#include "Settings/UGFSettingsManager.h"

const FString UGFPlayerSaveManager::SaveSlotName = TEXT("GF_PlayerSave");

bool UGFPlayerSaveManager::SaveGame()
{
	AGFCharacter* Character = GetPlayerCharacter();
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFPlayerSaveManager::SaveGame — no player character; nothing saved."));
		return false;
	}
	UGFSaveGame* SaveData = CollectPlayerState(Character);
	if (!SaveData)
	{
		return false;
	}
	return UGameplayStatics::SaveGameToSlot(SaveData, SaveSlotName, 0);
}

bool UGFPlayerSaveManager::LoadGame()
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		return false;
	}
	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0);
	UGFSaveGame* SaveData = Cast<UGFSaveGame>(Loaded);
	if (!SaveData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFPlayerSaveManager::LoadGame — slot %s is not a UGFSaveGame; refusing to load."), *SaveSlotName);
		return false;
	}
	if (SaveData->SaveVersion < UGFSaveGame::CurrentSaveVersion)
	{
		if (!MigrateSave(SaveData))
		{
			return false;
		}
	}
	if (!SaveData->Validate())
	{
		return false;
	}
	AGFCharacter* Character = GetPlayerCharacter();
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFPlayerSaveManager::LoadGame — save is valid but there is no player character yet; call LoadGame again after spawn."));
		return false;
	}
	return ApplyPlayerState(SaveData, Character);
}

bool UGFPlayerSaveManager::DeleteSave()
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		return false;
	}
	return UGameplayStatics::DeleteGameInSlot(SaveSlotName, 0);
}

bool UGFPlayerSaveManager::HasSaveGame() const
{
	return UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0);
}

UGFSaveGame* UGFPlayerSaveManager::CollectPlayerState(AGFCharacter* Character) const
{
	if (!Character)
	{
		return nullptr;
	}
	UGFSaveGame* SaveData = Cast<UGFSaveGame>(UGameplayStatics::CreateSaveGameObject(UGFSaveGame::StaticClass()));
	if (!SaveData)
	{
		return nullptr;
	}
	SaveData->SaveVersion = UGFSaveGame::CurrentSaveVersion;

	if (UGFProgressionComponent* Progression = Character->FindComponentByClass<UGFProgressionComponent>())
	{
		SaveData->PlayerLevel = Progression->PlayerLevel;
		SaveData->PlayerXP = Progression->CurrentXP;
	}
	// Completed missions come from the live mission subsystem
	// (current run) plus the previous save (earlier runs). The
	// subsystem only knows the current mission, so the previous
	// save's list is merged in — without it, loading then saving
	// would forget every mission finished in an earlier session.
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		if (UGFSaveGame* Previous = Cast<UGFSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0)))
		{
			SaveData->CompletedMissionIds = Previous->CompletedMissionIds;
		}
	}
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UGFMissionSubsystem* Missions = GI->GetSubsystem<UGFMissionSubsystem>())
		{
			if (Missions->bMissionCompleted && Missions->CurrentMission)
			{
				SaveData->CompletedMissionIds.AddUnique(Missions->CurrentMission->MissionId);
			}
		}
	}
	if (UGFInventoryComponent* Inventory = Character->FindComponentByClass<UGFInventoryComponent>())
	{
		SaveData->InventoryEntries = Inventory->Entries;
	}
	if (UGFEquipmentComponent* Equipment = Character->FindComponentByClass<UGFEquipmentComponent>())
	{
		SaveData->EquippedPrimaryWeaponId = Equipment->EquippedPrimaryWeaponId;
		SaveData->EquippedSecondaryWeaponId = Equipment->EquippedSecondaryWeaponId;
		SaveData->EquippedShieldGeneratorId = Equipment->EquippedShieldGeneratorId;
	}
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UGFCustomizationManager* Customization = GI->GetSubsystem<UGFCustomizationManager>())
		{
			SaveData->CustomizationProfileId = Customization->GetActiveProfileId();
		}
		if (UGFSettingsManager* Settings = GI->GetSubsystem<UGFSettingsManager>())
		{
			// Reference copy only — GF_SettingsSave is authoritative.
			if (UGFSettingsSaveGame* SettingsData = Settings->GetSettings())
			{
				SaveData->SavedLanguageCode = SettingsData->LanguageCode;
			}
		}
	}
	// The active planet is the level the player is standing in.
	// The Level Blueprint sets it via the spaceship's
	// CurrentPlanet on travel; until then the save keeps the
	// previous value (ZERON on a first save).
	return SaveData;
}

bool UGFPlayerSaveManager::ApplyPlayerState(UGFSaveGame* SaveData, AGFCharacter* Character)
{
	if (!SaveData || !Character)
	{
		return false;
	}
	if (UGFProgressionComponent* Progression = Character->FindComponentByClass<UGFProgressionComponent>())
	{
		if (!Progression->SetProgress(SaveData->PlayerLevel, SaveData->PlayerXP))
		{
			UE_LOG(LogTemp, Warning, TEXT("[GF] UGFPlayerSaveManager::ApplyPlayerState — saved level/XP rejected; keeping current progress."));
		}
	}
	if (UGFInventoryComponent* Inventory = Character->FindComponentByClass<UGFInventoryComponent>())
	{
		Inventory->ClearInventory();
		for (const FGFInventoryEntry& Entry : SaveData->InventoryEntries)
		{
			if (Entry.ItemData && Entry.Quantity > 0)
			{
				// A full inventory drops the overflow with a
				// warning (AddItem broadcasts OnInventoryFull) —
				// the save itself is left untouched.
				Inventory->AddItem(Entry.ItemData, Entry.Quantity);
			}
		}
	}
	if (UGFEquipmentComponent* Equipment = Character->FindComponentByClass<UGFEquipmentComponent>())
	{
		Equipment->UnequipAll();
		UGFInventoryComponent* Inventory = Character->FindComponentByClass<UGFInventoryComponent>();
		auto EquipById = [&](FName ItemId, EGFEquipmentSlot Slot)
		{
			if (ItemId.IsNone() || !Inventory)
			{
				return;
			}
			if (UGFItemDataAsset* ItemData = Inventory->FindItemData(ItemId))
			{
				if (Slot == EGFEquipmentSlot::ShieldGenerator)
				{
					Equipment->EquipShieldGenerator(ItemData);
				}
				else
				{
					Equipment->EquipWeapon(ItemData, Slot);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[GF] UGFPlayerSaveManager::ApplyPlayerState — saved equipped item %s is not in the loaded inventory; slot left empty."),
					*ItemId.ToString());
			}
		};
		EquipById(SaveData->EquippedPrimaryWeaponId, EGFEquipmentSlot::PrimaryWeapon);
		EquipById(SaveData->EquippedSecondaryWeaponId, EGFEquipmentSlot::SecondaryWeapon);
		EquipById(SaveData->EquippedShieldGeneratorId, EGFEquipmentSlot::ShieldGenerator);
	}
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UGFCustomizationManager* Customization = GI->GetSubsystem<UGFCustomizationManager>())
		{
			if (!SaveData->CustomizationProfileId.IsNone())
			{
				Customization->SetActiveProfileById(SaveData->CustomizationProfileId);
			}
		}
	}
	return true;
}

bool UGFPlayerSaveManager::MigrateSave(UGFSaveGame* SaveData)
{
	if (!SaveData)
	{
		return false;
	}
	if (SaveData->SaveVersion >= UGFSaveGame::CurrentSaveVersion)
	{
		return true;
	}
	// Version 0: pre-Phase 05 saves have no version field and no
	// player state — only the Phase 03 mission slot may hold
	// progress. Import a completed mission from it so the player
	// keeps their ZERON completion (and the NEXUS unlock it grants).
	if (SaveData->SaveVersion == 0)
	{
		if (UGameplayStatics::DoesSaveGameExist(UGFMissionSubsystem::SaveSlotName, 0))
		{
			if (UGFMissionSaveGame* MissionSave = Cast<UGFMissionSaveGame>(
				UGameplayStatics::LoadGameFromSlot(UGFMissionSubsystem::SaveSlotName, 0)))
			{
				if (MissionSave->bMissionCompleted && !MissionSave->SavedMissionId.IsNone())
				{
					SaveData->CompletedMissionIds.AddUnique(MissionSave->SavedMissionId);
				}
			}
		}
	}
	SaveData->SaveVersion = UGFSaveGame::CurrentSaveVersion;
	return true;
}

AGFCharacter* UGFPlayerSaveManager::GetPlayerCharacter() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC)
	{
		return nullptr;
	}
	return Cast<AGFCharacter>(PC->GetPawn());
}

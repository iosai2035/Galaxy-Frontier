// Galaxy Frontier — UGFProgressionComponent.cpp (Phase 05)
// XP curve: 100 * level per level-up. Mission rewards only.

#include "Progression/UGFProgressionComponent.h"

#include "Inventory/UGFInventoryComponent.h"
#include "Inventory/UGFItemDataAsset.h"
#include "Missions/UGFMissionDataAsset.h"
#include "Missions/UGFMissionSubsystem.h"

UGFProgressionComponent::UGFProgressionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGFProgressionComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UGFMissionSubsystem* Missions = GI->GetSubsystem<UGFMissionSubsystem>())
		{
			Missions->OnMissionCompleted.AddDynamic(this, &UGFProgressionComponent::HandleMissionCompleted);
		}
	}
}

void UGFProgressionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UGFMissionSubsystem* Missions = GI->GetSubsystem<UGFMissionSubsystem>())
		{
			Missions->OnMissionCompleted.RemoveDynamic(this, &UGFProgressionComponent::HandleMissionCompleted);
		}
	}
	Super::EndPlay(EndPlayReason);
}

int32 UGFProgressionComponent::GetXPToNextLevel(int32 Level)
{
	if (Level < 1)
	{
		return 100;
	}
	if (Level >= MaxLevel)
	{
		return 0;
	}
	return 100 * Level;
}

int32 UGFProgressionComponent::GetTotalXPForLevel(int32 Level)
{
	if (Level <= 1)
	{
		return 0;
	}
	const int32 ClampedLevel = FMath::Clamp(Level, 1, MaxLevel);
	return 50 * ClampedLevel * (ClampedLevel - 1);
}

int32 UGFProgressionComponent::AddXP(int32 Amount)
{
	if (Amount <= 0 || PlayerLevel >= MaxLevel)
	{
		return 0;
	}
	int32 Remaining = Amount;
	while (Remaining > 0 && PlayerLevel < MaxLevel)
	{
		const int32 Needed = GetXPToNextLevel(PlayerLevel);
		const int32 ToAdd = FMath::Min(Remaining, Needed - CurrentXP);
		CurrentXP += ToAdd;
		Remaining -= ToAdd;
		if (CurrentXP >= Needed)
		{
			CurrentXP = 0;
			++PlayerLevel;
			OnLevelUp.Broadcast(PlayerLevel);
			UE_LOG(LogTemp, Log, TEXT("[GF] UGFProgressionComponent — level up! Now level %d."), PlayerLevel);
		}
	}
	const int32 Added = Amount - Remaining;
	if (Added > 0)
	{
		OnXPChanged.Broadcast(CurrentXP, GetXPToNextLevel(PlayerLevel));
	}
	return Added;
}

bool UGFProgressionComponent::GrantMissionReward(UGFMissionDataAsset* MissionData)
{
	if (!MissionData || MissionData->MissionId.IsNone())
	{
		return false;
	}
	if (GrantedMissionRewards.Contains(MissionData->MissionId))
	{
		// Already granted — a second broadcast must never pay twice.
		return false;
	}
	GrantedMissionRewards.Add(MissionData->MissionId);
	AddXP(MissionData->CompletionXPReward);
	if (MissionData->CompletionRewardItem)
	{
		AActor* Owner = GetOwner();
		UGFInventoryComponent* Inventory = Owner ? Owner->FindComponentByClass<UGFInventoryComponent>() : nullptr;
		if (!Inventory)
		{
			UE_LOG(LogTemp, Warning, TEXT("[GF] UGFProgressionComponent::GrantMissionReward — no inventory; reward item %s for mission %s was NOT granted."),
				*MissionData->CompletionRewardItem->ItemId.ToString(), *MissionData->MissionId.ToString());
			return false;
		}
		if (Inventory->AddItem(MissionData->CompletionRewardItem, 1) <= 0)
		{
			// Inventory full: the XP was still granted, but the
			// item is not silently lost — the reward is marked
			// un-granted for the item part only if the player
			// clears space and the mission is re-completed? No:
			// re-completing a mission is impossible, so the item
			// is recorded as pending in the log and the mission
			// stays marked granted. The player picks the item up
			// from the mission's end-state pickup in the level
			// (see Docs/MISSIONS.md — the Level Blueprint places
			// the reward item as an AGFPickupActor).
			UE_LOG(LogTemp, Warning, TEXT("[GF] UGFProgressionComponent::GrantMissionReward — inventory full; reward item %s for mission %s was not added. Pick it up from the mission reward pickup in the level."),
				*MissionData->CompletionRewardItem->ItemId.ToString(), *MissionData->MissionId.ToString());
			return false;
		}
	}
	return true;
}

bool UGFProgressionComponent::SetProgress(int32 NewLevel, int32 NewXP)
{
	if (NewLevel < 1 || NewLevel > MaxLevel || NewXP < 0)
	{
		return false;
	}
	const int32 Needed = GetXPToNextLevel(NewLevel);
	if (NewLevel >= MaxLevel)
	{
		PlayerLevel = MaxLevel;
		CurrentXP = 0;
	}
	else
	{
		if (NewXP >= Needed)
		{
			return false;
		}
		PlayerLevel = NewLevel;
		CurrentXP = NewXP;
	}
	OnXPChanged.Broadcast(CurrentXP, GetXPToNextLevel(PlayerLevel));
	return true;
}

float UGFProgressionComponent::GetLevelProgressFraction() const
{
	const int32 Needed = GetXPToNextLevel(PlayerLevel);
	if (Needed <= 0)
	{
		return 1.0f;
	}
	return FMath::Clamp(static_cast<float>(CurrentXP) / static_cast<float>(Needed), 0.0f, 1.0f);
}

void UGFProgressionComponent::HandleMissionCompleted(FName MissionId)
{
	if (MissionId.IsNone() || GrantedMissionRewards.Contains(MissionId))
	{
		return;
	}
	UGameInstance* GI = GetGameInstance();
	UGFMissionSubsystem* Missions = GI ? GI->GetSubsystem<UGFMissionSubsystem>() : nullptr;
	if (!Missions || !Missions->CurrentMission || Missions->CurrentMission->MissionId != MissionId)
	{
		// The completed mission is not the subsystem's current
		// one — do not guess at its reward data.
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFProgressionComponent::HandleMissionCompleted — mission %s is not the current mission; reward not granted."),
			*MissionId.ToString());
		return;
	}
	GrantMissionReward(Missions->CurrentMission);
}

// Galaxy Frontier — UGFObjectiveComponent.cpp (Phase 03)

#include "Missions/UGFObjectiveComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Missions/UGFMissionSubsystem.h"

UGFObjectiveComponent::UGFObjectiveComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UGFObjectiveComponent::CompleteObjective()
{
	if (bIsCompleted)
	{
		return false;
	}
	if (ObjectiveId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFObjectiveComponent on %s has no ObjectiveId — set it in the Editor."),
			*GetOwner()->GetName());
		return false;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}
	UGFMissionSubsystem* Subsystem = World->GetGameInstance()
		? World->GetGameInstance()->GetSubsystem<UGFMissionSubsystem>()
		: nullptr;
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFObjectiveComponent::CompleteObjective — no UGFMissionSubsystem (GameInstance subsystem not registered?)."));
		return false;
	}
	// The subsystem is the authority on order. If it rejects this
	// id, this component does NOT mark itself completed.
	if (!Subsystem->CompleteObjective(ObjectiveId))
	{
		return false;
	}
	bIsCompleted = true;
	return true;
}

void UGFObjectiveComponent::ResetObjective()
{
	bIsCompleted = false;
}

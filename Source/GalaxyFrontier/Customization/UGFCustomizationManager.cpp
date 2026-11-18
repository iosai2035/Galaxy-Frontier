// Galaxy Frontier — UGFCustomizationManager.cpp (Phase 05)
// Profile lookup + application. No profile is ever created here.

#include "Customization/UGFCustomizationManager.h"

#include "Characters/AGFCharacter.h"
#include "Customization/UGFCharacterCustomizationData.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

bool UGFCustomizationManager::SetActiveProfile(UGFCharacterCustomizationData* ProfileData)
{
	if (!ProfileData)
	{
		return false;
	}
	if (!ProfileData->Validate())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFCustomizationManager::SetActiveProfile — profile failed validation; refused."));
		return false;
	}
	if (!AvailableProfiles.Contains(ProfileData))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFCustomizationManager::SetActiveProfile — profile %s is not in AvailableProfiles; add it in the Editor."),
			*ProfileData->ProfileId.ToString());
		return false;
	}
	ActiveProfile = ProfileData;
	return ApplyActiveProfile();
}

bool UGFCustomizationManager::SetActiveProfileById(FName ProfileId)
{
	UGFCharacterCustomizationData* ProfileData = FindProfileById(ProfileId);
	if (!ProfileData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFCustomizationManager::SetActiveProfileById — no profile with id %s."),
			*ProfileId.ToString());
		return false;
	}
	return SetActiveProfile(ProfileData);
}

bool UGFCustomizationManager::ApplyActiveProfile()
{
	if (!ActiveProfile)
	{
		// No profile chosen yet: fall back to the first valid
		// available profile instead of leaving the character bare.
		for (UGFCharacterCustomizationData* Candidate : AvailableProfiles)
		{
			if (Candidate && Candidate->Validate())
			{
				ActiveProfile = Candidate;
				break;
			}
		}
	}
	if (!ActiveProfile)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFCustomizationManager::ApplyActiveProfile — no profile available; assign profiles in the Editor."));
		return false;
	}
	AGFCharacter* Character = GetPlayerCharacter();
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFCustomizationManager::ApplyActiveProfile — no player character yet (call again after spawn)."));
		return false;
	}
	return ActiveProfile->ApplyToCharacter(Character);
}

UGFCharacterCustomizationData* UGFCustomizationManager::FindProfileById(FName ProfileId) const
{
	if (ProfileId.IsNone())
	{
		return nullptr;
	}
	for (UGFCharacterCustomizationData* ProfileData : AvailableProfiles)
	{
		if (ProfileData && ProfileData->ProfileId == ProfileId)
		{
			return ProfileData;
		}
	}
	return nullptr;
}

FName UGFCustomizationManager::GetActiveProfileId() const
{
	return ActiveProfile ? ActiveProfile->ProfileId : NAME_None;
}

AGFCharacter* UGFCustomizationManager::GetPlayerCharacter() const
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

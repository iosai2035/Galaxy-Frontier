// Galaxy Frontier — UGFSettingsManager.cpp (Phase 04)

#include "Settings/UGFSettingsManager.h"

#include "Characters/AGFCharacter.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Internationalization/Internationalization.h"
#include "Kismet/GameplayStatics.h"

const TArray<FString> UGFSettingsManager::SupportedLanguageCodes =
{
	TEXT("ar"), TEXT("en"), TEXT("ru"), TEXT("es"), TEXT("fr"),
	TEXT("de"), TEXT("tr"), TEXT("fa"), TEXT("pt"), TEXT("id"),
	TEXT("zh"), TEXT("ja"), TEXT("ko"), TEXT("hi"), TEXT("ur"),
	TEXT("bn"), TEXT("th"), TEXT("vi"), TEXT("ms"), TEXT("ta")
};

const FString UGFSettingsManager::SaveSlotName = TEXT("GF_SettingsSave");

void UGFSettingsManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadSettings();
}

bool UGFSettingsManager::LoadSettings()
{
	USaveGame* Loaded = nullptr;
	if (GetGameInstance())
	{
		Loaded = UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0);
	}

	if (UGFSettingsSaveGame* LoadedSettings = Cast<UGFSettingsSaveGame>(Loaded))
	{
		CurrentSettings = LoadedSettings;
	}
	else
	{
		// No save (first run) or a save of the wrong class: start
		// from defaults rather than crashing or keeping stale data.
		if (Loaded)
		{
			UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSettingsManager::LoadSettings — save slot %s held a non-settings object; using defaults."), *SaveSlotName);
		}
		CurrentSettings = NewObject<UGFSettingsSaveGame>(this);
		if (!CurrentSettings)
		{
			UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSettingsManager::LoadSettings — could not create settings object."));
			return false;
		}
		ValidateSettings(CurrentSettings);
		ApplySettings();
		return false;
	}

	ValidateSettings(CurrentSettings);
	ApplySettings();
	return true;
}

bool UGFSettingsManager::SaveSettings()
{
	if (!CurrentSettings)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSettingsManager::SaveSettings — no settings loaded."));
		return false;
	}
	if (!UGameplayStatics::SaveGameToSlot(CurrentSettings, SaveSlotName, 0))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSettingsManager::SaveSettings — failed to write slot %s."), *SaveSlotName);
		return false;
	}
	return true;
}

void UGFSettingsManager::ApplySettings()
{
	if (!CurrentSettings)
	{
		return;
	}
	ApplyGraphicsQuality(CurrentSettings->GraphicsQuality);
	ApplyCameraSettings();
	ApplyLanguage();
}

void UGFSettingsManager::ResetToDefaults()
{
	UGFSettingsSaveGame* Defaults = NewObject<UGFSettingsSaveGame>(this);
	if (!Defaults)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSettingsManager::ResetToDefaults — could not create defaults object."));
		return;
	}
	CurrentSettings = Defaults;
	SaveSettings();
	ApplySettings();
}

bool UGFSettingsManager::ValidateSettings(UGFSettingsSaveGame* Settings)
{
	if (!Settings)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSettingsManager::ValidateSettings — null settings."));
		return false;
	}

	bool bChanged = false;
	auto ClampField = [&bChanged](float& Value, float Min, float Max, float Fallback)
	{
		if (!FMath::IsFinite(Value))
		{
			Value = Fallback;
			bChanged = true;
		}
		else if (Value < Min || Value > Max)
		{
			Value = FMath::Clamp(Value, Min, Max);
			bChanged = true;
		}
	};

	ClampField(Settings->CameraSensitivity, 0.1f, 5.0f, 1.0f);
	ClampField(Settings->ButtonScale, 0.5f, 2.0f, 1.0f);
	ClampField(Settings->JoystickAnchorX, 0.05f, 0.95f, 0.12f);
	ClampField(Settings->JoystickAnchorY, 0.05f, 0.95f, 0.75f);
	ClampField(Settings->MasterVolume, 0.0f, 1.0f, 1.0f);
	ClampField(Settings->MusicVolume, 0.0f, 1.0f, 0.7f);
	ClampField(Settings->SFXVolume, 0.0f, 1.0f, 0.8f);

	const int32 QualityInt = static_cast<int32>(Settings->GraphicsQuality);
	if (QualityInt < 0 || QualityInt > static_cast<int32>(EGFGraphicsQuality::High))
	{
		Settings->GraphicsQuality = EGFGraphicsQuality::Medium;
		bChanged = true;
	}
	const int32 SideInt = static_cast<int32>(Settings->JoystickSide);
	if (SideInt < 0 || SideInt > static_cast<int32>(EGFJoystickSide::Right))
	{
		Settings->JoystickSide = EGFJoystickSide::Left;
		bChanged = true;
	}
	if (!IsSupportedLanguageCode(Settings->LanguageCode))
	{
		Settings->LanguageCode = TEXT("ar");
		bChanged = true;
	}
	return bChanged;
}

bool UGFSettingsManager::IsSupportedLanguageCode(const FString& LanguageCode)
{
	return SupportedLanguageCodes.Contains(LanguageCode);
}

void UGFSettingsManager::ApplyGraphicsQuality(EGFGraphicsQuality Quality)
{
	UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
	if (!UserSettings)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSettingsManager::ApplyGraphicsQuality — no GameUserSettings."));
		return;
	}
	// Scalability level: 0 Low, 1 Medium, 2 High (3 Epic is never
	// used on mobile — see Docs/PERFORMANCE.md).
	const int32 Level = static_cast<int32>(Quality);
	UserSettings->SetOverallScalabilityLevel(Level);
	UserSettings->ApplySettings(false);

	// Streaming pool matches the per-quality budget in
	// Docs/DEVICE_TEST_MATRIX.md (High 512 MB, else 256 MB).
	if (IConsoleVariable* PoolVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streaming.PoolSize")))
	{
		PoolVar->Set(Quality == EGFGraphicsQuality::High ? 512 : 256);
	}
}

void UGFSettingsManager::ApplyCameraSettings()
{
	AGFCharacter* Character = GetPlayerCharacter();
	if (!Character || !CurrentSettings)
	{
		return;
	}
	Character->LookSensitivity = CurrentSettings->CameraSensitivity;
	Character->bInvertLookPitch = CurrentSettings->bInvertCameraPitch;
}

void UGFSettingsManager::ApplyLanguage()
{
	if (!CurrentSettings)
	{
		return;
	}
	// Switches the active culture. Full translations need the
	// Editor localisation targets (Docs/SETTINGS.md) — switching
	// the culture without loc data falls back to the source text,
	// it never crashes and never invents a translation.
	FInternationalization::Get().SetCurrentCulture(CurrentSettings->LanguageCode);
}

AGFCharacter* UGFSettingsManager::GetPlayerCharacter() const
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (APlayerController* PC = GI->GetFirstLocalPlayerController())
		{
			return Cast<AGFCharacter>(PC->GetPawn());
		}
	}
	return nullptr;
}

bool UGFSettingsManager::SetCameraSensitivity(float NewSensitivity)
{
	if (!CurrentSettings || !FMath::IsFinite(NewSensitivity))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSettingsManager::SetCameraSensitivity — rejected non-finite value."));
		return false;
	}
	CurrentSettings->CameraSensitivity = FMath::Clamp(NewSensitivity, 0.1f, 5.0f);
	ApplyCameraSettings();
	return SaveSettings();
}

void UGFSettingsManager::SetInvertCameraPitch(bool bInvert)
{
	if (!CurrentSettings)
	{
		return;
	}
	CurrentSettings->bInvertCameraPitch = bInvert;
	ApplyCameraSettings();
	SaveSettings();
}

bool UGFSettingsManager::SetButtonScale(float NewScale)
{
	if (!CurrentSettings || !FMath::IsFinite(NewScale))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSettingsManager::SetButtonScale — rejected non-finite value."));
		return false;
	}
	CurrentSettings->ButtonScale = FMath::Clamp(NewScale, 0.5f, 2.0f);
	// Button scale is read by WBP_MobileHUD at construct time; a
	// live HUD re-reads it when the Settings screen closes.
	return SaveSettings();
}

void UGFSettingsManager::SetJoystickSide(EGFJoystickSide NewSide)
{
	if (!CurrentSettings)
	{
		return;
	}
	CurrentSettings->JoystickSide = NewSide;
	SaveSettings();
}

bool UGFSettingsManager::SetJoystickAnchor(float AnchorX, float AnchorY)
{
	if (!CurrentSettings || !FMath::IsFinite(AnchorX) || !FMath::IsFinite(AnchorY))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSettingsManager::SetJoystickAnchor — rejected non-finite value."));
		return false;
	}
	CurrentSettings->JoystickAnchorX = FMath::Clamp(AnchorX, 0.05f, 0.95f);
	CurrentSettings->JoystickAnchorY = FMath::Clamp(AnchorY, 0.05f, 0.95f);
	return SaveSettings();
}

void UGFSettingsManager::SetGraphicsQuality(EGFGraphicsQuality NewQuality)
{
	if (!CurrentSettings)
	{
		return;
	}
	CurrentSettings->GraphicsQuality = NewQuality;
	ApplyGraphicsQuality(NewQuality);
	SaveSettings();
}

void UGFSettingsManager::SetAutoQualityEnabled(bool bEnabled)
{
	if (!CurrentSettings)
	{
		return;
	}
	CurrentSettings->bAutoQualityEnabled = bEnabled;
	SaveSettings();
}

bool UGFSettingsManager::SetMasterVolume(float NewVolume)
{
	if (!CurrentSettings || !FMath::IsFinite(NewVolume))
	{
		return false;
	}
	CurrentSettings->MasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
	return SaveSettings();
}

bool UGFSettingsManager::SetMusicVolume(float NewVolume)
{
	if (!CurrentSettings || !FMath::IsFinite(NewVolume))
	{
		return false;
	}
	CurrentSettings->MusicVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
	return SaveSettings();
}

bool UGFSettingsManager::SetSFXVolume(float NewVolume)
{
	if (!CurrentSettings || !FMath::IsFinite(NewVolume))
	{
		return false;
	}
	CurrentSettings->SFXVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
	return SaveSettings();
}

bool UGFSettingsManager::SetLanguage(const FString& NewLanguageCode)
{
	if (!CurrentSettings || !IsSupportedLanguageCode(NewLanguageCode))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFSettingsManager::SetLanguage — unsupported code '%s'."), *NewLanguageCode);
		return false;
	}
	CurrentSettings->LanguageCode = NewLanguageCode;
	ApplyLanguage();
	return SaveSettings();
}

void UGFSettingsManager::SetShowDiagnostics(bool bShow)
{
	if (!CurrentSettings)
	{
		return;
	}
	CurrentSettings->bShowDiagnostics = bShow;
	SaveSettings();
}

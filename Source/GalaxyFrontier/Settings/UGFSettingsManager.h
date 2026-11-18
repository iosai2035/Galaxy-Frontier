// Galaxy Frontier — UGFSettingsManager.h (Phase 04)
// GameInstance subsystem: owns the player's settings. Loads the
// save slot "GF_SettingsSave" (or safe defaults when there is no
// save / the save is corrupt), validates every value, applies the
// ones that have a code path (graphics, camera sensitivity,
// language culture), and saves on every change.
//
// What is NOT applied in code (honest split, see Docs/SETTINGS.md):
// audio volumes are validated + saved, but the live mix needs a
// SoundMix asset authored in the Editor — there is none in this
// repo, so ApplySettings() does not claim to change any volume.
// HUD button scale / joystick layout are read by WBP_MobileHUD
// (Blueprint) from GetSettings() at construct time.

#pragma once

#include "CoreMinimal.h"
#include "Settings/UGFSettingsSaveGame.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UGFSettingsManager.generated.h"

class AGFCharacter;

UCLASS()
class GALAXYFRONTIER_API UGFSettingsManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// The 20 supported UI languages (same set as the download bot's
	// i18n list — the game localises to these when loc data exists
	// in the Editor; see Docs/SETTINGS.md).
	static const TArray<FString> SupportedLanguageCodes;

	static const FString SaveSlotName;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Loads the save slot, or fresh defaults if there is no save.
	// Always validates before applying. Returns false only when
	// there was no save to load (defaults are then in use).
	UFUNCTION(BlueprintCallable, Category = "GF|Settings")
	bool LoadSettings();

	// Writes the current settings to the save slot. Returns false
	// when the save fails (disk/full) — the in-memory settings are
	// still valid, only persistence failed.
	UFUNCTION(BlueprintCallable, Category = "GF|Settings")
	bool SaveSettings();

	// Applies the current settings to the running game: graphics
	// scalability, camera sensitivity / invert on the player
	// character, and the active culture for LanguageCode.
	UFUNCTION(BlueprintCallable, Category = "GF|Settings")
	void ApplySettings();

	// Restores factory defaults (in memory), then saves + applies.
	UFUNCTION(BlueprintCallable, Category = "GF|Settings")
	void ResetToDefaults();

	// Clamps every field of Settings into its documented range and
	// fixes an unsupported LanguageCode to "ar". Returns true when
	// anything was changed. Never crashes on a null Settings.
	UFUNCTION(BlueprintCallable, Category = "GF|Settings")
	bool ValidateSettings(UGFSettingsSaveGame* Settings);

	UFUNCTION(BlueprintPure, Category = "GF|Settings")
	UGFSettingsSaveGame* GetSettings() const { return CurrentSettings; }

	UFUNCTION(BlueprintPure, Category = "GF|Settings")
	static bool IsSupportedLanguageCode(const FString& LanguageCode);

	// --- Individual setters: validate the single value, save and
	// apply. Each returns false when the value was rejected
	// (non-finite numbers are rejected, never clamped silently
	// into a valid-looking value — the caller is warned).
	UFUNCTION(BlueprintCallable, Category = "GF|Settings")
	bool SetCameraSensitivity(float NewSensitivity);

	UFUNCTION(BlueprintCallable, Category = "GF|Settings")
	void SetInvertCameraPitch(bool bInvert);

	UFUNCTION(BlueprintCallable, Category = "GF|Settings")
	bool SetButtonScale(float NewScale);

	UFUNCTION(BlueprintCallable, Category = "GF|Settings")
	void SetJoystickSide(EGFJoystickSide NewSide);

	UFUNCTION(BlueprintCallable, Category = "GF|Settings")
	bool SetJoystickAnchor(float AnchorX, float AnchorY);

	UFUNCTION(BlueprintCallable, Category = "GF|Settings")
	void SetGraphicsQuality(EGFGraphicsQuality NewQuality);

	UFUNCTION(BlueprintCallable, Category = "GF|Settings")
	void SetAutoQualityEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "GF|Settings")
	bool SetMasterVolume(float NewVolume);

	UFUNCTION(BlueprintCallable, Category = "GF|Settings")
	bool SetMusicVolume(float NewVolume);

	UFUNCTION(BlueprintCallable, Category = "GF|Settings")
	bool SetSFXVolume(float NewVolume);

	// Returns false (and changes nothing) for an unsupported code.
	UFUNCTION(BlueprintCallable, Category = "GF|Settings")
	bool SetLanguage(const FString& NewLanguageCode);

	UFUNCTION(BlueprintCallable, Category = "GF|Settings")
	void SetShowDiagnostics(bool bShow);

protected:
	// Applies only the graphics part (scalability level + streaming
	// pool). Used by ApplySettings() and by UGFPerformanceManager's
	// auto step-down, which must NOT rewrite the saved quality.
	void ApplyGraphicsQuality(EGFGraphicsQuality Quality);

	void ApplyCameraSettings();
	void ApplyLanguage();

	AGFCharacter* GetPlayerCharacter() const;

	UPROPERTY()
	TObjectPtr<UGFSettingsSaveGame> CurrentSettings;
};

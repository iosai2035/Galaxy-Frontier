// Galaxy Frontier — UGFMobileHUDWidget.h (Phase 04)
// C++ base class for the WBP_MobileHUD Blueprint (see
// Docs/MOBILE-HUD.md). The Blueprint child must be parented to
// this class and must name its widgets exactly as below — every
// binding is BindWidgetOptional, so a missing widget is logged
// and skipped, never a crash.
//
// The widget never polls: it binds to UGFHUDController's events
// in NativeConstruct and only updates when an event fires.
// The virtual joystick and the camera-drag area are Blueprint
// touch logic (OnTouchStarted / OnTouchMoved on the canvas —
// see Docs/MOBILE-HUD.md); the C++ side only owns the buttons
// and the readouts, because those call character/combat APIs.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Performance/UGFPerformanceManager.h"
#include "UGFMobileHUDWidget.generated.h"

class UButton;
class UProgressBar;
class UTextBlock;
class UGFHUDController;

UCLASS(Blueprintable)
class GALAXYFRONTIER_API UGFMobileHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	// --- Readouts (names must match the Blueprint widgets) ---
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ShieldBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HealthText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ShieldText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AmmoText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WeaponNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ObjectiveText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> InteractPromptText;

	// Diagnostics line ("FPS 58 | RAM 412 MB | High"). Shown only
	// when UGFSettingsSaveGame::bShowDiagnostics is true.
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DiagnosticsText;

	// --- Buttons (names must match the Blueprint widgets) ---
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> FireButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> AimButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> JumpButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> SprintButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> InteractButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ReloadButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> SwitchWeaponButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> PauseButton;

protected:
	// --- Event handlers (bound to UGFHUDController) ---
	UFUNCTION()
	void UpdateHealth(float CurrentHealth, float MaxHealth, float Delta);

	UFUNCTION()
	void UpdateShield(float CurrentShield, float MaxShield, float Delta);

	UFUNCTION()
	void UpdateAmmo(int32 CurrentAmmo);

	UFUNCTION()
	void UpdateWeapon(int32 NewWeaponIndex);

	UFUNCTION()
	void UpdateObjective(FText ObjectiveTitle);

	UFUNCTION()
	void UpdateInteractPrompt(FText PromptText);

	UFUNCTION()
	void UpdateDiagnostics(FGFPerformanceMetrics Metrics);

	// --- Button handlers ---
	UFUNCTION()
	void OnFirePressed();

	UFUNCTION()
	void OnFireReleased();

	UFUNCTION()
	void OnAimPressed();

	UFUNCTION()
	void OnAimReleased();

	UFUNCTION()
	void OnJumpClicked();

	UFUNCTION()
	void OnSprintPressed();

	UFUNCTION()
	void OnSprintReleased();

	UFUNCTION()
	void OnInteractClicked();

	UFUNCTION()
	void OnReloadClicked();

	UFUNCTION()
	void OnSwitchWeaponClicked();

	UFUNCTION()
	void OnPauseClicked();

	UGFHUDController* GetHUDController() const;

	TObjectPtr<UGFHUDController> BoundHUDController;
	TObjectPtr<UGFPerformanceManager> BoundPerformanceManager;
};

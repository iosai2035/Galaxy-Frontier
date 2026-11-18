// Galaxy Frontier — UGFHUDController.h (Phase 04)
// Actor component on AGFCharacter: the single bridge between
// gameplay and the UMG widgets. It binds to the existing
// delegates (health / shield / ammo / weapon switched / mission /
// interaction) and re-broadcasts them for the widgets, so no
// widget polls anything every frame. It also owns the HUD, Pause,
// Game Over and Mission Complete widgets and the pause state.
//
// Widget classes are assigned in the Editor (WBP_MobileHUD etc.
// — Blueprint assets, see Docs/MOBILE-HUD.md and
// Docs/UI-SCREENS.md). Every widget class is optional: if one is
// not assigned, the game still runs and the missing screen is
// logged, never crashed — a missing Pause widget still pauses
// the game, it just shows no menu.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/UGFHealthComponent.h"
#include "Combat/UGFCombatComponent.h"
#include "Missions/UGFMissionSubsystem.h"
#include "Weapons/AGFWeaponBase.h"
#include "World/UGFInteractionComponent.h"
#include "UGFHUDController.generated.h"

class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGFHUDObjectiveDelegate, FText, ObjectiveTitle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGFHUDPromptDelegate, FText, PromptText);

UCLASS(ClassGroup = (GF), meta = (BlueprintSpawnableComponent))
class GALAXYFRONTIER_API UGFHUDController : public UActorComponent
{
	GENERATED_BODY()

public:
	UGFHUDController();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// --- Widget classes (assigned in the Editor) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|UI")
	TSubclassOf<UUserWidget> PauseWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|UI")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|UI")
	TSubclassOf<UUserWidget> MissionCompleteWidgetClass;

	// --- Live widgets (null until created) ---
	UPROPERTY(BlueprintReadOnly, Category = "GF|UI")
	TObjectPtr<UUserWidget> HUDWidget;

	UPROPERTY(BlueprintReadOnly, Category = "GF|UI")
	TObjectPtr<UUserWidget> PauseWidget;

	UPROPERTY(BlueprintReadOnly, Category = "GF|UI")
	TObjectPtr<UUserWidget> GameOverWidget;

	UPROPERTY(BlueprintReadOnly, Category = "GF|UI")
	TObjectPtr<UUserWidget> MissionCompleteWidget;

	UPROPERTY(BlueprintReadOnly, Category = "GF|UI")
	bool bIsPaused = false;

	// --- Events for the widgets (Blueprint binds here) ---
	// Health / shield / death are the UGFHealthComponent
	// delegates, re-broadcast unchanged.
	UPROPERTY(BlueprintAssignable, Category = "GF|UI")
	FGFHealthChangedDelegate OnHUDHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GF|UI")
	FGFShieldChangedDelegate OnHUDShieldChanged;

	UPROPERTY(BlueprintAssignable, Category = "GF|UI")
	FGFDeathDelegate OnHUDPlayerDied;

	// Ammo: current magazine only (AGFWeaponBase::OnAmmoChanged).
	// The widget pulls the reserve count via GetReserveAmmo().
	UPROPERTY(BlueprintAssignable, Category = "GF|UI")
	FGFWeaponAmmoChangedDelegate OnHUDAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "GF|UI")
	FGFWeaponReloadedDelegate OnHUDReloaded;

	UPROPERTY(BlueprintAssignable, Category = "GF|UI")
	FGFWeaponSwitchedDelegate OnHUDWeaponSwitched;

	UPROPERTY(BlueprintAssignable, Category = "GF|UI")
	FGFHUDObjectiveDelegate OnHUDObjectiveChanged;

	UPROPERTY(BlueprintAssignable, Category = "GF|UI")
	FGFHUDPromptDelegate OnHUDInteractPromptChanged;

	UPROPERTY(BlueprintAssignable, Category = "GF|UI")
	FGFMissionDelegate OnHUDMissionCompleted;

	// --- API ---
	// Creates the HUD widget and adds it to the viewport.
	// Returns false when there is no player controller or no
	// HUDWidgetClass assigned (logged, not a crash).
	UFUNCTION(BlueprintCallable, Category = "GF|UI")
	bool CreateHUD();

	// Toggles pause. Returns the new paused state. Pausing stops
	// firing, freezes the game and shows the Pause widget when
	// one is assigned. Resuming restores game-only input.
	UFUNCTION(BlueprintCallable, Category = "GF|UI")
	bool TogglePause();

	UFUNCTION(BlueprintCallable, Category = "GF|UI")
	bool OpenPauseMenu();

	UFUNCTION(BlueprintCallable, Category = "GF|UI")
	bool ClosePauseMenu();

	// Pulls every current value once and re-broadcasts it, so a
	// widget created late (or rebound) shows correct values
	// without waiting for the next change event.
	UFUNCTION(BlueprintCallable, Category = "GF|UI")
	void RefreshAll();

	// --- Pull getters for widgets (called on events, never Tick) ---
	UFUNCTION(BlueprintPure, Category = "GF|UI")
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintPure, Category = "GF|UI")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintPure, Category = "GF|UI")
	float GetCurrentShield() const;

	UFUNCTION(BlueprintPure, Category = "GF|UI")
	float GetMaxShield() const;

	UFUNCTION(BlueprintPure, Category = "GF|UI")
	int32 GetCurrentAmmo() const;

	UFUNCTION(BlueprintPure, Category = "GF|UI")
	int32 GetReserveAmmo() const;

	UFUNCTION(BlueprintPure, Category = "GF|UI")
	FText GetCurrentWeaponName() const;

	UFUNCTION(BlueprintPure, Category = "GF|UI")
	FText GetCurrentObjectiveTitle() const;

	UFUNCTION(BlueprintPure, Category = "GF|UI")
	FText GetCurrentPrompt() const;

	UFUNCTION(BlueprintPure, Category = "GF|UI")
	bool IsPaused() const { return bIsPaused; }

protected:
	// Binds all gameplay delegates. Called from BeginPlay and
	// again after a retry, if the player controller was not ready.
	void BindGameplayDelegates();
	void BindCurrentWeapon();
	void UnbindCurrentWeapon();

	// Timer callback (timers need a void function): retries the
	// HUD creation until the player controller is ready.
	void RetryCreateHUD();

	// Event handlers — each re-broadcasts to the widgets.
	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth, float Delta);

	UFUNCTION()
	void HandleShieldChanged(float CurrentShield, float MaxShield, float Delta);

	UFUNCTION()
	void HandleDeath();

	UFUNCTION()
	void HandleAmmoChanged(int32 CurrentAmmo);

	UFUNCTION()
	void HandleReloaded();

	UFUNCTION()
	void HandleWeaponSwitched(int32 NewWeaponIndex);

	UFUNCTION()
	void HandleObjectiveCompleted(FName ObjectiveId);

	UFUNCTION()
	void HandleMissionCompleted(FName MissionId);

	UFUNCTION()
	void HandleMissionFailed(FName MissionId);

	UFUNCTION()
	void HandleInteractableChanged(AActor* NewInteractable);

	void ShowWidget(TSubclassOf<UUserWidget> WidgetClass, TObjectPtr<UUserWidget>& OutWidget);
	void HideWidget(TObjectPtr<UUserWidget>& Widget);

	UGFHealthComponent* GetHealthComponent() const;
	UGFCombatComponent* GetCombatComponent() const;
	UGFInteractionComponent* GetInteractionComponent() const;
	UGFMissionSubsystem* GetMissionSubsystem() const;
	APlayerController* GetPlayerController() const;

	TObjectPtr<AGFWeaponBase> BoundWeapon;
	bool bDelegatesBound = false;
	FTimerHandle HUDRetryTimerHandle;
	int32 HUDRetryCount = 0;
};

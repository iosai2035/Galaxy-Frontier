// Galaxy Frontier — UGFHUDController.cpp (Phase 04)

#include "UI/UGFHUDController.h"

#include "Blueprint/UserWidget.h"
#include "Characters/AGFCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Performance/UGFPerformanceManager.h"
#include "TimerManager.h"
#include "Weapons/UGFWeaponDataAsset.h"

UGFHUDController::UGFHUDController()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGFHUDController::BeginPlay()
{
	Super::BeginPlay();

	// The performance manager's sampling timer needs a world.
	// This component lives in one, so it is the reliable starter
	// (see UGFPerformanceManager::StartSampling).
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UGFPerformanceManager* Perf = GI->GetSubsystem<UGFPerformanceManager>())
		{
			Perf->StartSampling();
		}
	}

	BindGameplayDelegates();
	if (!CreateHUD())
	{
		// The player controller may not be ready yet at BeginPlay
		// (possession order). Retry a few times, then stop and log
		// — a HUD that never appears is a visible setup error, not
		// a crash, and the game still runs.
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				HUDRetryTimerHandle, this, &UGFHUDController::RetryCreateHUD,
				0.5f, true);
		}
	}
}

void UGFHUDController::RetryCreateHUD()
{
	CreateHUD();
}

void UGFHUDController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HUDRetryTimerHandle);
	}
	UnbindCurrentWeapon();
	HideWidget(HUDWidget);
	HideWidget(PauseWidget);
	HideWidget(GameOverWidget);
	HideWidget(MissionCompleteWidget);
	Super::EndPlay(EndPlayReason);
}

void UGFHUDController::BindGameplayDelegates()
{
	if (bDelegatesBound)
	{
		return;
	}
	if (UGFHealthComponent* Health = GetHealthComponent())
	{
		Health->OnHealthChanged.AddDynamic(this, &UGFHUDController::HandleHealthChanged);
		Health->OnShieldChanged.AddDynamic(this, &UGFHUDController::HandleShieldChanged);
		Health->OnDeath.AddDynamic(this, &UGFHUDController::HandleDeath);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFHUDController — no HealthComponent on owner; health HUD will stay empty."));
	}
	if (UGFCombatComponent* Combat = GetCombatComponent())
	{
		Combat->OnWeaponSwitched.AddDynamic(this, &UGFHUDController::HandleWeaponSwitched);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFHUDController — no CombatComponent on owner; ammo/weapon HUD will stay empty."));
	}
	if (UGFInteractionComponent* Interaction = GetInteractionComponent())
	{
		Interaction->OnInteractableChanged.AddDynamic(this, &UGFHUDController::HandleInteractableChanged);
	}
	if (UGFMissionSubsystem* Missions = GetMissionSubsystem())
	{
		Missions->OnObjectiveCompleted.AddDynamic(this, &UGFHUDController::HandleObjectiveCompleted);
		Missions->OnMissionCompleted.AddDynamic(this, &UGFHUDController::HandleMissionCompleted);
		Missions->OnMissionFailed.AddDynamic(this, &UGFHUDController::HandleMissionFailed);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFHUDController — no MissionSubsystem; objective HUD will stay empty."));
	}
	BindCurrentWeapon();
	bDelegatesBound = true;
}

void UGFHUDController::BindCurrentWeapon()
{
	UnbindCurrentWeapon();
	if (UGFCombatComponent* Combat = GetCombatComponent())
	{
		if (AGFWeaponBase* Weapon = Combat->GetCurrentWeapon())
		{
			BoundWeapon = Weapon;
			Weapon->OnAmmoChanged.AddDynamic(this, &UGFHUDController::HandleAmmoChanged);
			Weapon->OnReloaded.AddDynamic(this, &UGFHUDController::HandleReloaded);
		}
	}
}

void UGFHUDController::UnbindCurrentWeapon()
{
	if (BoundWeapon)
	{
		BoundWeapon->OnAmmoChanged.RemoveDynamic(this, &UGFHUDController::HandleAmmoChanged);
		BoundWeapon->OnReloaded.RemoveDynamic(this, &UGFHUDController::HandleReloaded);
		BoundWeapon = nullptr;
	}
}

bool UGFHUDController::CreateHUD()
{
	if (HUDWidget)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(HUDRetryTimerHandle);
		}
		return true;
	}
	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		HUDRetryCount++;
		if (HUDRetryCount > 10)
		{
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().ClearTimer(HUDRetryTimerHandle);
			}
			UE_LOG(LogTemp, Warning, TEXT("[GF] UGFHUDController::CreateHUD — no player controller after 10 retries; HUD not created."));
		}
		return false;
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HUDRetryTimerHandle);
	}
	if (!HUDWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFHUDController::CreateHUD — HUDWidgetClass (WBP_MobileHUD) is not assigned in the Editor. See Docs/MOBILE-HUD.md."));
		return false;
	}
	ShowWidget(HUDWidgetClass, HUDWidget);
	RefreshAll();
	return HUDWidget != nullptr;
}

bool UGFHUDController::TogglePause()
{
	return bIsPaused ? ClosePauseMenu() : OpenPauseMenu();
}

bool UGFHUDController::OpenPauseMenu()
{
	UWorld* World = GetWorld();
	APlayerController* PC = GetPlayerController();
	if (!World || !PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFHUDController::OpenPauseMenu — no world/player controller."));
		return false;
	}
	if (bIsPaused)
	{
		return true;
	}
	// Stop firing first: a held Fire button must not keep
	// shooting while the game is paused.
	if (UGFCombatComponent* Combat = GetCombatComponent())
	{
		Combat->StopFire();
	}
	UGameplayStatics::SetGamePaused(World, true);
	bIsPaused = true;
	if (PauseWidgetClass)
	{
		ShowWidget(PauseWidgetClass, PauseWidget);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFHUDController::OpenPauseMenu — game paused, but PauseWidgetClass (WBP_PauseMenu) is not assigned; no menu shown."));
	}
	PC->SetInputMode(FInputModeUIOnly());
	PC->bShowMouseCursor = true;
	return true;
}

bool UGFHUDController::ClosePauseMenu()
{
	UWorld* World = GetWorld();
	APlayerController* PC = GetPlayerController();
	if (!World || !PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFHUDController::ClosePauseMenu — no world/player controller."));
		return false;
	}
	if (!bIsPaused)
	{
		return true;
	}
	HideWidget(PauseWidget);
	UGameplayStatics::SetGamePaused(World, false);
	bIsPaused = false;
	PC->SetInputMode(FInputModeGameOnly());
	PC->bShowMouseCursor = false;
	return true;
}

void UGFHUDController::RefreshAll()
{
	// Pull once, broadcast once. No per-frame polling: widgets
	// only re-read on these events.
	OnHUDHealthChanged.Broadcast(GetCurrentHealth(), GetMaxHealth(), 0.0f);
	OnHUDShieldChanged.Broadcast(GetCurrentShield(), GetMaxShield(), 0.0f);
	OnHUDAmmoChanged.Broadcast(GetCurrentAmmo());
	if (UGFCombatComponent* Combat = GetCombatComponent())
	{
		OnHUDWeaponSwitched.Broadcast(Combat->CurrentWeaponIndex);
	}
	OnHUDObjectiveChanged.Broadcast(GetCurrentObjectiveTitle());
	OnHUDInteractPromptChanged.Broadcast(GetCurrentPrompt());
}

float UGFHUDController::GetCurrentHealth() const
{
	if (UGFHealthComponent* Health = GetHealthComponent())
	{
		return Health->CurrentHealth;
	}
	return 0.0f;
}

float UGFHUDController::GetMaxHealth() const
{
	if (UGFHealthComponent* Health = GetHealthComponent())
	{
		return Health->MaxHealth;
	}
	return 100.0f;
}

float UGFHUDController::GetCurrentShield() const
{
	if (UGFHealthComponent* Health = GetHealthComponent())
	{
		return Health->CurrentShield;
	}
	return 0.0f;
}

float UGFHUDController::GetMaxShield() const
{
	if (UGFHealthComponent* Health = GetHealthComponent())
	{
		return Health->MaxShield;
	}
	return 100.0f;
}

int32 UGFHUDController::GetCurrentAmmo() const
{
	if (UGFCombatComponent* Combat = GetCombatComponent())
	{
		return Combat->GetCurrentAmmo();
	}
	return 0;
}

int32 UGFHUDController::GetReserveAmmo() const
{
	if (UGFCombatComponent* Combat = GetCombatComponent())
	{
		return Combat->GetReserveAmmo();
	}
	return 0;
}

FText UGFHUDController::GetCurrentWeaponName() const
{
	if (UGFCombatComponent* Combat = GetCombatComponent())
	{
		if (AGFWeaponBase* Weapon = Combat->GetCurrentWeapon())
		{
			if (Weapon->WeaponData)
			{
				return Weapon->WeaponData->WeaponName;
			}
		}
	}
	return FText::FromString(TEXT("—"));
}

FText UGFHUDController::GetCurrentObjectiveTitle() const
{
	if (UGFMissionSubsystem* Missions = GetMissionSubsystem())
	{
		return Missions->GetCurrentObjectiveTitle();
	}
	return FText::GetEmpty();
}

FText UGFHUDController::GetCurrentPrompt() const
{
	if (UGFInteractionComponent* Interaction = GetInteractionComponent())
	{
		return Interaction->GetCurrentPrompt();
	}
	return FText::GetEmpty();
}

void UGFHUDController::HandleHealthChanged(float CurrentHealth, float MaxHealth, float Delta)
{
	OnHUDHealthChanged.Broadcast(CurrentHealth, MaxHealth, Delta);
}

void UGFHUDController::HandleShieldChanged(float CurrentShield, float MaxShield, float Delta)
{
	OnHUDShieldChanged.Broadcast(CurrentShield, MaxShield, Delta);
}

void UGFHUDController::HandleDeath()
{
	OnHUDPlayerDied.Broadcast();
	if (UGFCombatComponent* Combat = GetCombatComponent())
	{
		Combat->StopFire();
	}
	if (GameOverWidgetClass)
	{
		ShowWidget(GameOverWidgetClass, GameOverWidget);
		if (APlayerController* PC = GetPlayerController())
		{
			PC->SetInputMode(FInputModeUIOnly());
			PC->bShowMouseCursor = true;
		}
	}
}

void UGFHUDController::HandleAmmoChanged(int32 CurrentAmmo)
{
	OnHUDAmmoChanged.Broadcast(CurrentAmmo);
}

void UGFHUDController::HandleReloaded()
{
	OnHUDReloaded.Broadcast();
	OnHUDAmmoChanged.Broadcast(GetCurrentAmmo());
}

void UGFHUDController::HandleWeaponSwitched(int32 NewWeaponIndex)
{
	// The new weapon has its own ammo delegates — rebind, then
	// push the new weapon's state to the HUD.
	BindCurrentWeapon();
	OnHUDWeaponSwitched.Broadcast(NewWeaponIndex);
	OnHUDAmmoChanged.Broadcast(GetCurrentAmmo());
}

void UGFHUDController::HandleObjectiveCompleted(FName ObjectiveId)
{
	(void)ObjectiveId;
	OnHUDObjectiveChanged.Broadcast(GetCurrentObjectiveTitle());
}

void UGFHUDController::HandleMissionCompleted(FName MissionId)
{
	OnHUDMissionCompleted.Broadcast(MissionId);
	OnHUDObjectiveChanged.Broadcast(GetCurrentObjectiveTitle());
	if (MissionCompleteWidgetClass)
	{
		ShowWidget(MissionCompleteWidgetClass, MissionCompleteWidget);
	}
}

void UGFHUDController::HandleMissionFailed(FName MissionId)
{
	(void)MissionId;
	OnHUDObjectiveChanged.Broadcast(GetCurrentObjectiveTitle());
}

void UGFHUDController::HandleInteractableChanged(AActor* NewInteractable)
{
	(void)NewInteractable;
	OnHUDInteractPromptChanged.Broadcast(GetCurrentPrompt());
}

void UGFHUDController::ShowWidget(TSubclassOf<UUserWidget> WidgetClass, TObjectPtr<UUserWidget>& OutWidget)
{
	if (OutWidget)
	{
		return;
	}
	APlayerController* PC = GetPlayerController();
	if (!PC || !WidgetClass)
	{
		return;
	}
	UUserWidget* Widget = CreateWidget<UUserWidget>(PC, WidgetClass);
	if (!Widget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFHUDController::ShowWidget — CreateWidget failed for %s."), *WidgetClass->GetName());
		return;
	}
	Widget->AddToViewport();
	OutWidget = Widget;
}

void UGFHUDController::HideWidget(TObjectPtr<UUserWidget>& Widget)
{
	if (Widget)
	{
		Widget->RemoveFromParent();
		Widget = nullptr;
	}
}

UGFHealthComponent* UGFHUDController::GetHealthComponent() const
{
	if (AActor* Owner = GetOwner())
	{
		return Owner->FindComponentByClass<UGFHealthComponent>();
	}
	return nullptr;
}

UGFCombatComponent* UGFHUDController::GetCombatComponent() const
{
	if (AActor* Owner = GetOwner())
	{
		return Owner->FindComponentByClass<UGFCombatComponent>();
	}
	return nullptr;
}

UGFInteractionComponent* UGFHUDController::GetInteractionComponent() const
{
	if (AActor* Owner = GetOwner())
	{
		return Owner->FindComponentByClass<UGFInteractionComponent>();
	}
	return nullptr;
}

UGFMissionSubsystem* UGFHUDController::GetMissionSubsystem() const
{
	if (UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UGFMissionSubsystem>();
	}
	return nullptr;
}

APlayerController* UGFHUDController::GetPlayerController() const
{
	if (AGFCharacter* Character = Cast<AGFCharacter>(GetOwner()))
	{
		return Cast<APlayerController>(Character->GetController());
	}
	return nullptr;
}

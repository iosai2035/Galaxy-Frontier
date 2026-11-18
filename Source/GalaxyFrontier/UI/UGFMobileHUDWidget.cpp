// Galaxy Frontier — UGFMobileHUDWidget.cpp (Phase 04)

#include "UI/UGFMobileHUDWidget.h"

#include "Characters/AGFCharacter.h"
#include "Combat/UGFCombatComponent.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/Character.h"
#include "Settings/UGFSettingsManager.h"
#include "UI/UGFHUDController.h"
#include "World/UGFInteractionComponent.h"

void UGFMobileHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BoundHUDController = GetHUDController();
	if (!BoundHUDController)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFMobileHUDWidget — no UGFHUDController on the owning pawn; HUD will not update."));
		return;
	}
	BoundHUDController->OnHUDHealthChanged.AddDynamic(this, &UGFMobileHUDWidget::UpdateHealth);
	BoundHUDController->OnHUDShieldChanged.AddDynamic(this, &UGFMobileHUDWidget::UpdateShield);
	BoundHUDController->OnHUDAmmoChanged.AddDynamic(this, &UGFMobileHUDWidget::UpdateAmmo);
	BoundHUDController->OnHUDWeaponSwitched.AddDynamic(this, &UGFMobileHUDWidget::UpdateWeapon);
	BoundHUDController->OnHUDObjectiveChanged.AddDynamic(this, &UGFMobileHUDWidget::UpdateObjective);
	BoundHUDController->OnHUDInteractPromptChanged.AddDynamic(this, &UGFMobileHUDWidget::UpdateInteractPrompt);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UGFPerformanceManager* Perf = GI->GetSubsystem<UGFPerformanceManager>())
		{
			BoundPerformanceManager = Perf;
			Perf->OnMetricsUpdated.AddDynamic(this, &UGFMobileHUDWidget::UpdateDiagnostics);
		}
	}

	// Buttons: press/release pairs for hold actions (Fire, Aim,
	// Sprint), single clicks for the rest. A missing button is a
	// Blueprint setup error — logged, not a crash.
	if (FireButton)
	{
		FireButton->OnPressed.AddDynamic(this, &UGFMobileHUDWidget::OnFirePressed);
		FireButton->OnReleased.AddDynamic(this, &UGFMobileHUDWidget::OnFireReleased);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFMobileHUDWidget — FireButton is not bound (name it FireButton in WBP_MobileHUD)."));
	}
	if (AimButton)
	{
		AimButton->OnPressed.AddDynamic(this, &UGFMobileHUDWidget::OnAimPressed);
		AimButton->OnReleased.AddDynamic(this, &UGFMobileHUDWidget::OnAimReleased);
	}
	if (JumpButton)
	{
		JumpButton->OnClicked.AddDynamic(this, &UGFMobileHUDWidget::OnJumpClicked);
	}
	if (SprintButton)
	{
		SprintButton->OnPressed.AddDynamic(this, &UGFMobileHUDWidget::OnSprintPressed);
		SprintButton->OnReleased.AddDynamic(this, &UGFMobileHUDWidget::OnSprintReleased);
	}
	if (InteractButton)
	{
		InteractButton->OnClicked.AddDynamic(this, &UGFMobileHUDWidget::OnInteractClicked);
	}
	if (ReloadButton)
	{
		ReloadButton->OnClicked.AddDynamic(this, &UGFMobileHUDWidget::OnReloadClicked);
	}
	if (SwitchWeaponButton)
	{
		SwitchWeaponButton->OnClicked.AddDynamic(this, &UGFMobileHUDWidget::OnSwitchWeaponClicked);
	}
	if (PauseButton)
	{
		PauseButton->OnClicked.AddDynamic(this, &UGFMobileHUDWidget::OnPauseClicked);
	}

	// Pull the current values once so the HUD is correct from
	// the first frame, without waiting for a change event.
	BoundHUDController->RefreshAll();
	if (BoundPerformanceManager)
	{
		UpdateDiagnostics(BoundPerformanceManager->GetMetrics());
	}
}

void UGFMobileHUDWidget::NativeDestruct()
{
	if (BoundHUDController)
	{
		BoundHUDController->OnHUDHealthChanged.RemoveDynamic(this, &UGFMobileHUDWidget::UpdateHealth);
		BoundHUDController->OnHUDShieldChanged.RemoveDynamic(this, &UGFMobileHUDWidget::UpdateShield);
		BoundHUDController->OnHUDAmmoChanged.RemoveDynamic(this, &UGFMobileHUDWidget::UpdateAmmo);
		BoundHUDController->OnHUDWeaponSwitched.RemoveDynamic(this, &UGFMobileHUDWidget::UpdateWeapon);
		BoundHUDController->OnHUDObjectiveChanged.RemoveDynamic(this, &UGFMobileHUDWidget::UpdateObjective);
		BoundHUDController->OnHUDInteractPromptChanged.RemoveDynamic(this, &UGFMobileHUDWidget::UpdateInteractPrompt);
	}
	if (BoundPerformanceManager)
	{
		BoundPerformanceManager->OnMetricsUpdated.RemoveDynamic(this, &UGFMobileHUDWidget::UpdateDiagnostics);
	}
	Super::NativeDestruct();
}

UGFHUDController* UGFMobileHUDWidget::GetHUDController() const
{
	if (APawn* Pawn = GetOwningPlayerPawn())
	{
		return Pawn->FindComponentByClass<UGFHUDController>();
	}
	return nullptr;
}

void UGFMobileHUDWidget::UpdateHealth(float CurrentHealth, float MaxHealth, float Delta)
{
	(void)Delta;
	if (HealthBar)
	{
		HealthBar->SetPercent(MaxHealth > 0.0f ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f) : 0.0f);
	}
	if (HealthText)
	{
		HealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurrentHealth, MaxHealth)));
	}
}

void UGFMobileHUDWidget::UpdateShield(float CurrentShield, float MaxShield, float Delta)
{
	(void)Delta;
	if (ShieldBar)
	{
		ShieldBar->SetPercent(MaxShield > 0.0f ? FMath::Clamp(CurrentShield / MaxShield, 0.0f, 1.0f) : 0.0f);
	}
	if (ShieldText)
	{
		ShieldText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurrentShield, MaxShield)));
	}
}

void UGFMobileHUDWidget::UpdateAmmo(int32 CurrentAmmo)
{
	if (!AmmoText)
	{
		return;
	}
	const int32 ReserveAmmo = BoundHUDController ? BoundHUDController->GetReserveAmmo() : 0;
	AmmoText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentAmmo, ReserveAmmo)));
}

void UGFMobileHUDWidget::UpdateWeapon(int32 NewWeaponIndex)
{
	(void)NewWeaponIndex;
	if (WeaponNameText && BoundHUDController)
	{
		WeaponNameText->SetText(BoundHUDController->GetCurrentWeaponName());
	}
	// Ammo belongs to the new weapon — refresh the counter too.
	if (BoundHUDController)
	{
		UpdateAmmo(BoundHUDController->GetCurrentAmmo());
	}
}

void UGFMobileHUDWidget::UpdateObjective(FText ObjectiveTitle)
{
	if (ObjectiveText)
	{
		ObjectiveText->SetText(ObjectiveTitle);
	}
}

void UGFMobileHUDWidget::UpdateInteractPrompt(FText PromptText)
{
	if (InteractPromptText)
	{
		InteractPromptText->SetText(PromptText);
	}
	// The Interact button is only usable when a prompt exists —
	// hiding it prevents a dead button with no target.
	if (InteractButton)
	{
		InteractButton->SetVisibility(PromptText.IsEmpty()
			? ESlateVisibility::Hidden
			: ESlateVisibility::Visible);
	}
}

void UGFMobileHUDWidget::UpdateDiagnostics(FGFPerformanceMetrics Metrics)
{
	if (!DiagnosticsText)
	{
		return;
	}
	bool bShow = false;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UGFSettingsManager* Settings = GI->GetSubsystem<UGFSettingsManager>())
		{
			if (UGFSettingsSaveGame* Save = Settings->GetSettings())
			{
				bShow = Save->bShowDiagnostics;
			}
		}
	}
	if (!bShow)
	{
		DiagnosticsText->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	DiagnosticsText->SetVisibility(ESlateVisibility::HitTestInvisible);
	DiagnosticsText->SetText(FText::FromString(FString::Printf(
		TEXT("FPS %.0f (min %.0f) | RAM %.0f MB | %s%s"),
		Metrics.AvgFPS,
		Metrics.MinFPS,
		Metrics.UsedRAMMB,
		Metrics.CurrentQuality == EGFGraphicsQuality::High ? TEXT("High")
			: Metrics.CurrentQuality == EGFGraphicsQuality::Medium ? TEXT("Medium") : TEXT("Low"),
		Metrics.bAutoDowngraded ? TEXT(" (auto-lowered)") : TEXT(""))));
}

void UGFMobileHUDWidget::OnFirePressed()
{
	if (AGFCharacter* Character = Cast<AGFCharacter>(GetOwningPlayerPawn()))
	{
		if (UGFCombatComponent* Combat = Character->GetCombatComponent())
		{
			Combat->StartFire();
		}
	}
}

void UGFMobileHUDWidget::OnFireReleased()
{
	if (AGFCharacter* Character = Cast<AGFCharacter>(GetOwningPlayerPawn()))
	{
		if (UGFCombatComponent* Combat = Character->GetCombatComponent())
		{
			Combat->StopFire();
		}
	}
}

void UGFMobileHUDWidget::OnAimPressed()
{
	if (AGFCharacter* Character = Cast<AGFCharacter>(GetOwningPlayerPawn()))
	{
		Character->SetAiming(true);
	}
}

void UGFMobileHUDWidget::OnAimReleased()
{
	if (AGFCharacter* Character = Cast<AGFCharacter>(GetOwningPlayerPawn()))
	{
		Character->SetAiming(false);
	}
}

void UGFMobileHUDWidget::OnJumpClicked()
{
	if (ACharacter* Character = Cast<ACharacter>(GetOwningPlayerPawn()))
	{
		Character->Jump();
	}
}

void UGFMobileHUDWidget::OnSprintPressed()
{
	if (AGFCharacter* Character = Cast<AGFCharacter>(GetOwningPlayerPawn()))
	{
		Character->SetSprinting(true);
	}
}

void UGFMobileHUDWidget::OnSprintReleased()
{
	if (AGFCharacter* Character = Cast<AGFCharacter>(GetOwningPlayerPawn()))
	{
		Character->SetSprinting(false);
	}
}

void UGFMobileHUDWidget::OnInteractClicked()
{
	if (AGFCharacter* Character = Cast<AGFCharacter>(GetOwningPlayerPawn()))
	{
		if (UGFInteractionComponent* Interaction = Character->GetInteractionComponent())
		{
			Interaction->BeginInteraction();
		}
	}
}

void UGFMobileHUDWidget::OnReloadClicked()
{
	if (AGFCharacter* Character = Cast<AGFCharacter>(GetOwningPlayerPawn()))
	{
		if (UGFCombatComponent* Combat = Character->GetCombatComponent())
		{
			Combat->Reload();
		}
	}
}

void UGFMobileHUDWidget::OnSwitchWeaponClicked()
{
	if (AGFCharacter* Character = Cast<AGFCharacter>(GetOwningPlayerPawn()))
	{
		if (UGFCombatComponent* Combat = Character->GetCombatComponent())
		{
			Combat->SwitchToNextWeapon();
		}
	}
}

void UGFMobileHUDWidget::OnPauseClicked()
{
	if (BoundHUDController)
	{
		BoundHUDController->TogglePause();
	}
}

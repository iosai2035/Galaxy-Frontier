// Galaxy Frontier — UGFPerformanceManager.cpp (Phase 04)

#include "Performance/UGFPerformanceManager.h"

#include "Engine/World.h"
#include "GameFramework/GameUserSettings.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformTime.h"
#include "Settings/UGFSettingsManager.h"
#include "TimerManager.h"

FGFGraphicsBudget UGFPerformanceManager::GetBudgetForQuality(EGFGraphicsQuality Quality)
{
	FGFGraphicsBudget Budget;
	switch (Quality)
	{
	case EGFGraphicsQuality::Low:
		Budget.MaxDrawCalls = 80;
		Budget.MaxPolygons = 200000;
		Budget.MaxTextureMemoryMB = 256;
		Budget.MaxDynamicLights = 1;
		Budget.StreamingPoolMB = 256;
		break;
	case EGFGraphicsQuality::Medium:
		Budget.MaxDrawCalls = 120;
		Budget.MaxPolygons = 350000;
		Budget.MaxTextureMemoryMB = 384;
		Budget.MaxDynamicLights = 1;
		Budget.StreamingPoolMB = 256;
		break;
	case EGFGraphicsQuality::High:
	default:
		Budget.MaxDrawCalls = 150;
		Budget.MaxPolygons = 500000;
		Budget.MaxTextureMemoryMB = 512;
		Budget.MaxDynamicLights = 2;
		Budget.StreamingPoolMB = 512;
		break;
	}
	return Budget;
}

float UGFPerformanceManager::GetTargetFPSForQuality(EGFGraphicsQuality Quality)
{
	return Quality == EGFGraphicsQuality::High ? 60.0f : 30.0f;
}

void UGFPerformanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Start from the saved user quality; the live level equals it
	// until the auto step-down fires. The settings subsystem may
	// not have initialised yet — a null result just means the
	// default (Medium) is used until the next quality change.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UGFSettingsManager* Settings = GI->GetSubsystem<UGFSettingsManager>())
		{
			if (UGFSettingsSaveGame* Save = Settings->GetSettings())
			{
				UserQuality = Save->GraphicsQuality;
			}
		}
	}
	CurrentQuality = UserQuality;
	LastMetrics.CurrentQuality = CurrentQuality;
	LastMetrics.UserQuality = UserQuality;

	StartSampling();
}

void UGFPerformanceManager::StartSampling()
{
	if (bSamplingStarted)
	{
		return;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		// No world yet (normal at subsystem Initialize time).
		// UGFHUDController calls StartSampling() again from
		// BeginPlay, when a world exists.
		return;
	}
	World->GetTimerManager().SetTimer(
		SampleTimerHandle, this, &UGFPerformanceManager::OnSampleTimer,
		SampleIntervalSeconds, true);
	bSamplingStarted = true;
	LastSampleTimeSeconds = FPlatformTime::Seconds();
}

void UGFPerformanceManager::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SampleTimerHandle);
	}
	Super::Deinitialize();
}

void UGFPerformanceManager::SetQuality(EGFGraphicsQuality NewQuality, const FString& Reason)
{
	// A manual set is the user choosing: it becomes the ceiling
	// and the live level together.
	UserQuality = NewQuality;
	if (UGFSettingsManager* Settings = GetGameInstance()->GetSubsystem<UGFSettingsManager>())
	{
		Settings->SetGraphicsQuality(NewQuality);
	}
	ApplyLiveQuality(NewQuality, Reason);
}

void UGFPerformanceManager::RestoreUserQuality()
{
	if (CurrentQuality != UserQuality)
	{
		ApplyLiveQuality(UserQuality, TEXT("Restored to the quality chosen in Settings."));
	}
}

void UGFPerformanceManager::ApplyLiveQuality(EGFGraphicsQuality NewQuality, const FString& Reason)
{
	if (NewQuality == CurrentQuality)
	{
		return;
	}
	const EGFGraphicsQuality PreviousQuality = CurrentQuality;
	CurrentQuality = NewQuality;
	LastMetrics.CurrentQuality = CurrentQuality;
	LastMetrics.UserQuality = UserQuality;
	LastMetrics.bAutoDowngraded = CurrentQuality < UserQuality;
	LowFPSTime = 0.0f;
	HighFPSTime = 0.0f;
	TimeSinceLastChange = 0.0f;

	if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
	{
		UserSettings->SetOverallScalabilityLevel(static_cast<int32>(NewQuality));
		UserSettings->ApplySettings(false);
	}
	const FGFGraphicsBudget Budget = GetBudgetForQuality(NewQuality);
	if (IConsoleVariable* PoolVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streaming.PoolSize")))
	{
		PoolVar->Set(Budget.StreamingPoolMB);
	}

	if (PreviousQuality != CurrentQuality)
	{
		OnQualityChanged.Broadcast(CurrentQuality, Reason);
		UE_LOG(LogTemp, Log, TEXT("[GF] UGFPerformanceManager — quality %d -> %d (%s)"),
			static_cast<int32>(PreviousQuality), static_cast<int32>(CurrentQuality), *Reason);
	}
}

void UGFPerformanceManager::OnSampleTimer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const double NowSeconds = FPlatformTime::Seconds();
	const double DeltaSeconds = NowSeconds - LastSampleTimeSeconds;
	LastSampleTimeSeconds = NowSeconds;
	if (DeltaSeconds <= 0.0 || !FMath::IsFinite(static_cast<float>(DeltaSeconds)))
	{
		return;
	}

	// One sample per timer tick (0.5 s): FPS is the reciprocal of
	// the real elapsed time, clamped so a hitch cannot report 0
	// or an absurd value. No per-frame work happens anywhere here.
	const float SampleFPS = FMath::Clamp(static_cast<float>(1.0 / DeltaSeconds), 1.0f, 240.0f);
	if (LastMetrics.AvgFPS <= 0.0f)
	{
		LastMetrics.AvgFPS = SampleFPS;
		WindowMinFPS = SampleFPS;
	}
	else
	{
		LastMetrics.AvgFPS = FMath::Lerp(LastMetrics.AvgFPS, SampleFPS, 0.25f);
		WindowMinFPS = FMath::Min(WindowMinFPS, SampleFPS);
	}
	LastMetrics.MinFPS = WindowMinFPS;

	const FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
	const float BytesToMB = 1.0f / (1024.0f * 1024.0f);
	LastMetrics.UsedRAMMB = static_cast<float>(MemoryStats.UsedPhysical) * BytesToMB;
	LastMetrics.TotalRAMMB = static_cast<float>(MemoryStats.TotalPhysical) * BytesToMB;
	// Thermal state: Unknown in this build (iOS platform reader
	// not implemented — see header). Never invent a reading.
	LastMetrics.ThermalState = EGFThermalState::Unknown;

	TimeSinceLastChange += static_cast<float>(DeltaSeconds);
	EvaluateAutoQuality(static_cast<float>(DeltaSeconds));
	OnMetricsUpdated.Broadcast(LastMetrics);
}

void UGFPerformanceManager::EvaluateAutoQuality(float DeltaSeconds)
{
	UGFSettingsManager* Settings = nullptr;
	if (UGameInstance* GI = GetGameInstance())
	{
		Settings = GI->GetSubsystem<UGFSettingsManager>();
	}
	const bool bAutoEnabled = Settings && Settings->GetSettings() ? Settings->GetSettings()->bAutoQualityEnabled : true;
	if (!bAutoEnabled || LastMetrics.AvgFPS <= 0.0f)
	{
		LowFPSTime = 0.0f;
		HighFPSTime = 0.0f;
		return;
	}
	if (TimeSinceLastChange < ChangeCooldownSeconds)
	{
		return;
	}

	const float TargetFPS = GetTargetFPSForQuality(CurrentQuality);
	if (LastMetrics.AvgFPS < TargetFPS - 5.0f)
	{
		// Sustained low FPS: accumulate, step down once the
		// threshold time is reached. High time resets — the two
		// directions never accumulate at once (hysteresis).
		LowFPSTime += DeltaSeconds;
		HighFPSTime = 0.0f;
		if (LowFPSTime >= DowngradeAfterSeconds && CurrentQuality != EGFGraphicsQuality::Low)
		{
			const EGFGraphicsQuality Lower = CurrentQuality == EGFGraphicsQuality::High
				? EGFGraphicsQuality::Medium : EGFGraphicsQuality::Low;
			ApplyLiveQuality(Lower, FString::Printf(
				TEXT("Auto step-down: avg FPS %.1f stayed below %.1f for %.1fs."),
				LastMetrics.AvgFPS, TargetFPS - 5.0f, DowngradeAfterSeconds));
		}
	}
	else if (LastMetrics.AvgFPS > TargetFPS + 10.0f)
	{
		// Sustained headroom: step back up, but never above the
		// user's chosen ceiling.
		HighFPSTime += DeltaSeconds;
		LowFPSTime = 0.0f;
		if (HighFPSTime >= UpgradeAfterSeconds && CurrentQuality < UserQuality)
		{
			const EGFGraphicsQuality Higher = CurrentQuality == EGFGraphicsQuality::Low
				? EGFGraphicsQuality::Medium : EGFGraphicsQuality::High;
			ApplyLiveQuality(Higher, FString::Printf(
				TEXT("Auto step-up: avg FPS %.1f stayed above %.1f for %.1fs."),
				LastMetrics.AvgFPS, TargetFPS + 10.0f, UpgradeAfterSeconds));
		}
	}
	else
	{
		// Inside the dead band: neither timer accumulates, so a
		// borderline FPS cannot flip the quality back and forth.
		LowFPSTime = 0.0f;
		HighFPSTime = 0.0f;
	}
}

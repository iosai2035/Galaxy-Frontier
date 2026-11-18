// Galaxy Frontier — UGFPerformanceManager.h (Phase 04)
// GameInstance subsystem: measures FPS / RAM on a 0.5 s timer
// (never per-frame polling by the UI) and steps the live graphics
// quality down — and back up — with hysteresis, so a thermally
// limited phone does not oscillate between High and Medium every
// second.
//
// Honest limits (see Docs/PERFORMANCE.md):
// - Draw calls / polygons / texture memory are BUDGETS, not live
//   measurements: they are measured in the Editor with `stat RHI`
//   and Unreal Insights, not by this manager. The diagnostics
//   overlay shows measured FPS / RAM / current quality only.
// - Thermal state is Unknown on this build: the real iOS value
//   comes from NSProcessInfo.thermalState, which needs iOS
//   platform code on the macOS build machine (NOT IMPLEMENTED).
// - The saved quality in UGFSettingsSaveGame is the player's
//   chosen ceiling. Auto step-down never rewrites it and never
//   steps above it.

#pragma once

#include "CoreMinimal.h"
#include "Settings/UGFSettingsSaveGame.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UGFPerformanceManager.generated.h"

// Thermal state. Only Unknown is ever produced by this build —
// see the header comment. The other values exist so the iOS
// platform reader (macOS, Phase 04 device work) has somewhere
// to put a real reading without changing this API.
UENUM(BlueprintType)
enum class EGFThermalState : uint8
{
	Unknown		UMETA(DisplayName = "Unknown"),
	Nominal		UMETA(DisplayName = "Nominal"),
	Fair		UMETA(DisplayName = "Fair"),
	Serious		UMETA(DisplayName = "Serious"),
	Critical	UMETA(DisplayName = "Critical")
};

// One quality tier's budget (targets from Docs/QUALITY.md rule 21
// and Docs/DEVICE_TEST_MATRIX.md — to measure, never a promise).
USTRUCT(BlueprintType)
struct FGFGraphicsBudget
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Performance")
	int32 MaxDrawCalls = 150;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Performance")
	int32 MaxPolygons = 500000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Performance")
	int32 MaxTextureMemoryMB = 512;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Performance")
	int32 MaxDynamicLights = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Performance")
	int32 StreamingPoolMB = 512;
};

// The numbers the diagnostics overlay shows. FPS / RAM are
// measured; draw calls / polygons are NOT in here on purpose.
USTRUCT(BlueprintType)
struct FGFPerformanceMetrics
{
	GENERATED_BODY()

	// Exponential moving average of FPS.
	UPROPERTY(BlueprintReadOnly, Category = "GF|Performance")
	float AvgFPS = 0.0f;

	// Lowest single sample since sampling started (not a rolling
	// window — the value only ever decreases, so a dip is never
	// hidden by later good frames).
	UPROPERTY(BlueprintReadOnly, Category = "GF|Performance")
	float MinFPS = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Performance")
	float UsedRAMMB = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Performance")
	float TotalRAMMB = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Performance")
	EGFThermalState ThermalState = EGFThermalState::Unknown;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Performance")
	EGFGraphicsQuality CurrentQuality = EGFGraphicsQuality::Medium;

	// The quality the player chose in Settings (the ceiling).
	UPROPERTY(BlueprintReadOnly, Category = "GF|Performance")
	EGFGraphicsQuality UserQuality = EGFGraphicsQuality::Medium;

	// True when the live quality is below the user's choice
	// because the auto step-down fired.
	UPROPERTY(BlueprintReadOnly, Category = "GF|Performance")
	bool bAutoDowngraded = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FGFQualityChangedDelegate,
	EGFGraphicsQuality, NewQuality,
	FString, Reason);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FGFMetricsUpdatedDelegate,
	FGFPerformanceMetrics, Metrics);

UCLASS()
class GALAXYFRONTIER_API UGFPerformanceManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Starts the 0.5 s sampling timer. Safe to call repeatedly.
	// Called from Initialize() when a world already exists, and
	// from UGFHUDController::BeginPlay() otherwise: a
	// GameInstanceSubsystem initialises before the first world
	// exists, so the timer cannot always start here. The HUD
	// controller lives in a world, so it is the reliable starter.
	UFUNCTION(BlueprintCallable, Category = "GF|Performance")
	void StartSampling();

	// Budgets per tier (Docs/DEVICE_TEST_MATRIX.md).
	UFUNCTION(BlueprintPure, Category = "GF|Performance")
	static FGFGraphicsBudget GetBudgetForQuality(EGFGraphicsQuality Quality);

	// FPS the tier is expected to hold. High 60, Medium/Low 30.
	UFUNCTION(BlueprintPure, Category = "GF|Performance")
	static float GetTargetFPSForQuality(EGFGraphicsQuality Quality);

	// Live quality right now (may be below the user's choice).
	UFUNCTION(BlueprintPure, Category = "GF|Performance")
	EGFGraphicsQuality GetCurrentQuality() const { return CurrentQuality; }

	UFUNCTION(BlueprintPure, Category = "GF|Performance")
	FGFPerformanceMetrics GetMetrics() const { return LastMetrics; }

	// Forces a quality level now (Settings screen). This also
	// becomes the new ceiling: it updates the saved user quality
	// via UGFSettingsManager when one exists.
	UFUNCTION(BlueprintCallable, Category = "GF|Performance")
	void SetQuality(EGFGraphicsQuality NewQuality, const FString& Reason);

	// Restores the live level to the user's chosen quality
	// (e.g. after the phone cools down / user taps "Restore").
	UFUNCTION(BlueprintCallable, Category = "GF|Performance")
	void RestoreUserQuality();

	UPROPERTY(BlueprintAssignable, Category = "GF|Performance")
	FGFQualityChangedDelegate OnQualityChanged;

	UPROPERTY(BlueprintAssignable, Category = "GF|Performance")
	FGFMetricsUpdatedDelegate OnMetricsUpdated;

	// --- Hysteresis tuning (all editable, all in seconds) ---
	// FPS must stay below (Target - 5) this long before a step down.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Performance", meta = (ClampMin = "1.0"))
	float DowngradeAfterSeconds = 5.0f;

	// FPS must stay above (Target + 10) this long before a step up.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Performance", meta = (ClampMin = "1.0"))
	float UpgradeAfterSeconds = 10.0f;

	// Minimum gap between two quality changes, either direction.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Performance", meta = (ClampMin = "0.0"))
	float ChangeCooldownSeconds = 10.0f;

	// How often a sample is taken (seconds). 0.5 s — never per frame.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Performance", meta = (ClampMin = "0.1"))
	float SampleIntervalSeconds = 0.5f;

protected:
	void OnSampleTimer();
	void EvaluateAutoQuality(float DeltaSeconds);
	void ApplyLiveQuality(EGFGraphicsQuality NewQuality, const FString& Reason);

	EGFGraphicsQuality CurrentQuality = EGFGraphicsQuality::Medium;
	EGFGraphicsQuality UserQuality = EGFGraphicsQuality::Medium;

	FGFPerformanceMetrics LastMetrics;

	FTimerHandle SampleTimerHandle;
	double LastSampleTimeSeconds = 0.0;
	float LowFPSTime = 0.0f;
	float HighFPSTime = 0.0f;
	float TimeSinceLastChange = 0.0f;
	float WindowMinFPS = 0.0f;
	bool bSamplingStarted = false;
};

// Galaxy Frontier — UGFSettingsSaveGame.h (Phase 04)
// Persistent player settings. Saved to slot "GF_SettingsSave" by
// UGFSettingsManager. This is a plain data object: every value is
// validated/clamped by UGFSettingsManager::ValidateSettings() before
// it is applied, so a hand-edited or older save can never drive a
// setting out of range. Full player progress (level, health,
// inventory) is UGFSaveGame in Phase 05 — this class is settings only.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "UGFSettingsSaveGame.generated.h"

// Graphics quality. Maps to the Scalability groups in
// Config/DefaultScalability.ini (Low/Medium/High) and to the budgets
// in Docs/DEVICE_TEST_MATRIX.md. There is deliberately no "Epic"
// on mobile — see Docs/PERFORMANCE.md.
UENUM(BlueprintType)
enum class EGFGraphicsQuality : uint8
{
	Low		UMETA(DisplayName = "Low"),
	Medium	UMETA(DisplayName = "Medium"),
	High	UMETA(DisplayName = "High")
};

// Which side of the screen the virtual joystick sits on.
// Right-handed players who aim with the left thumb can flip it.
UENUM(BlueprintType)
enum class EGFJoystickSide : uint8
{
	Left	UMETA(DisplayName = "Left"),
	Right	UMETA(DisplayName = "Right")
};

UCLASS()
class GALAXYFRONTIER_API UGFSettingsSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// --- Camera ---
	// Look sensitivity multiplier. Range 0.1–5.0, default 1.0.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Settings|Camera")
	float CameraSensitivity = 1.0f;

	// When true, dragging up looks down (and vice versa).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Settings|Camera")
	bool bInvertCameraPitch = false;

	// --- Touch layout (adjustable buttons, Phase 04) ---
	// Scale applied to every HUD touch button (Fire, Jump, ...).
	// Range 0.5–2.0, default 1.0. The HUD layout is anchor-based
	// (Docs/MOBILE-HUD.md), so scaling never breaks Safe Areas.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Settings|Touch")
	float ButtonScale = 1.0f;

	// Joystick side. Default Left.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Settings|Touch")
	EGFJoystickSide JoystickSide = EGFJoystickSide::Left;

	// Joystick anchor, as a fraction of the screen (0–1). The
	// player can drag the joystick base to a new position in the
	// Settings screen; the value is stored here. Range 0.05–0.95
	// on both axes so the base can never leave the screen.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Settings|Touch")
	float JoystickAnchorX = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Settings|Touch")
	float JoystickAnchorY = 0.75f;

	// --- Graphics ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Settings|Graphics")
	EGFGraphicsQuality GraphicsQuality = EGFGraphicsQuality::Medium;

	// Automatic quality step-down when FPS stays low (see
	// UGFPerformanceManager). Default true. The saved quality is
	// the player's chosen ceiling — auto-downgrade never rewrites
	// this value, it only lowers the live level temporarily.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Settings|Graphics")
	bool bAutoQualityEnabled = true;

	// --- Audio ---
	// Volumes are 0.0–1.0. They are validated and saved here; the
	// live mix is applied by the SoundMix asset in the Editor
	// (see Docs/SETTINGS.md) — there is no SoundMix asset in this
	// repo yet, so no volume is faked as "applied" in code.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Settings|Audio")
	float MasterVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Settings|Audio")
	float MusicVolume = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Settings|Audio")
	float SFXVolume = 0.8f;

	// --- Language ---
	// BCP-47-ish code, e.g. "ar" / "en". Default "ar". Must be one
	// of UGFSettingsManager::SupportedLanguageCodes (20 languages);
	// anything else is reset to "ar" by ValidateSettings().
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Settings|Language")
	FString LanguageCode = TEXT("ar");

	// --- Diagnostics ---
	// Shows the FPS / RAM / quality diagnostics overlay on the HUD.
	// Default false — it is a developer/tester aid, not gameplay.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Settings|Diagnostics")
	bool bShowDiagnostics = false;
};

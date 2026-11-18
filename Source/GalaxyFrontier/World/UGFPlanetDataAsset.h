// Galaxy Frontier — UGFPlanetDataAsset.h (Phase 05)
// Data-only planet definition. The level itself (L_<NAME>_Prototype)
// is built in the Unreal Editor on macOS — no .umap is faked here.
// Planets are data so travel, unlock gating and mission lists can
// be tuned without touching code. ZERON (Phase 03) is the first
// entry; NEXUS / VELTRON / ASTRALIS are defined in Docs/PLANETS.md.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UGFPlanetDataAsset.generated.h"

UENUM(BlueprintType)
enum class EGFPlanetWeather : uint8
{
	Clear		UMETA(DisplayName = "Clear"),
	DustStorm	UMETA(DisplayName = "Dust Storm"),
	AcidRain	UMETA(DisplayName = "Acid Rain"),
	IonStorm	UMETA(DisplayName = "Ion Storm"),
	Blizzard	UMETA(DisplayName = "Blizzard")
};

UCLASS(BlueprintType)
class GALAXYFRONTIER_API UGFPlanetDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// Stable id, e.g. "NEXUS". Stored in UGFSaveGame as the active
	// planet — never rename an id a released save may contain.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Planet")
	FName PlanetId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Planet")
	FText PlanetName = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Planet")
	FText Description = FText::GetEmpty();

	// Gravity multiplier applied to the character on this planet
	// (1.0 = Earth-like). Range 0.3–2.0; the Level Blueprint
	// applies it to the character's movement on spawn (see
	// Docs/PLANETS.md). It is data here, not a live effect.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Planet", meta = (ClampMin = "0.3", ClampMax = "2.0"))
	float GravityScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Planet")
	EGFPlanetWeather Weather = EGFPlanetWeather::Clear;

	// Level to load when travelling here (e.g. "L_NEXUS_Prototype").
	// The level asset itself is built in the Editor.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Planet")
	FName LevelName = NAME_None;

	// Suggested player level for this planet's missions
	// (Docs/EXPANSION.md). A suggestion only — travel is never
	// blocked by level, only by the unlock rule below.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Planet", meta = (ClampMin = "1", ClampMax = "20"))
	int32 RecommendedLevel = 1;

	// True when the planet is open from the start (ZERON only).
	// Otherwise the planet unlocks by completing UnlockMissionId.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Planet")
	bool bUnlockedByDefault = false;

	// Mission whose completion unlocks this planet (ignored when
	// bUnlockedByDefault is true). Checked against
	// UGFSaveGame::CompletedMissionIds.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Planet")
	FName UnlockMissionId = NAME_None;

	// Mission ids available on this planet (data assets created
	// in the Editor; the subsystem starts them by id).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Planet")
	TArray<FName> AvailableMissionIds;

	// True when the saved progress lists this planet as unlocked
	// (or it is unlocked by default). Pure data check — the caller
	// passes the completed-mission list from the save.
	UFUNCTION(BlueprintCallable, Category = "GF|Planet")
	bool IsUnlocked(const TArray<FName>& CompletedMissionIds) const;

	UFUNCTION(BlueprintCallable, Category = "GF|Planet")
	bool Validate() const;
};

// Galaxy Frontier — AGFSpaceship.h (Phase 05)
// Interplanetary ship ("Starhopper" in Docs/EXPANSION.md).
// An APawn the player can board at a landing pad; travel is a
// real level load, not a cutscene that pretends to fly:
//
//   1. TravelToPlanet(Planet) validates the planet and the unlock
//      (via UGFPlanetDataAsset::IsUnlocked + the save's completed
//      missions), then starts a 5 s spool-up.
//   2. When the timer fires, the planet's level
//      (Planet->LevelName, e.g. L_NEXUS_Prototype) is opened with
//      UGameplayStatics::OpenLevel.
//
// There is NO space combat in Phase 05 — the ship is transport
// only. The mesh is assigned in the Editor (no mesh is faked).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AGFSpaceship.generated.h"

class UGFPlanetDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGFSpaceshipTravelDelegate, FName, PlanetId);

UCLASS()
class GALAXYFRONTIER_API AGFSpaceship : public APawn
{
	GENERATED_BODY()

public:
	AGFSpaceship();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Spool-up time before the level load (seconds). The HUD
	// shows this as a real countdown driven by the timer — never
	// a fake progress bar (see Docs/UI-SCREENS.md).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Spaceship", meta = (ClampMin = "1.0"))
	float TravelSpoolUpSeconds = 5.0f;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Spaceship")
	bool bIsTravelling = false;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Spaceship")
	TObjectPtr<UGFPlanetDataAsset> CurrentPlanet = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Spaceship")
	TObjectPtr<UGFPlanetDataAsset> DestinationPlanet = nullptr;

	UPROPERTY(BlueprintAssignable, Category = "GF|Spaceship")
	FGFSpaceshipTravelDelegate OnTravelStarted;

	UPROPERTY(BlueprintAssignable, Category = "GF|Spaceship")
	FGFSpaceshipTravelDelegate OnTravelCompleted;

	// Starts travel to Planet. CompletedMissionIds comes from the
	// player save (UGFPlayerSaveManager) and gates locked planets.
	// Returns false for a null/invalid planet, a locked planet,
	// a planet with no level, or travel already in progress.
	UFUNCTION(BlueprintCallable, Category = "GF|Spaceship")
	bool TravelToPlanet(UGFPlanetDataAsset* Planet, const TArray<FName>& CompletedMissionIds);

	// Cancels a spool-up in progress. Returns false when not
	// travelling. A level load that already started cannot be
	// cancelled.
	UFUNCTION(BlueprintCallable, Category = "GF|Spaceship")
	bool CancelTravel();

	UFUNCTION(BlueprintPure, Category = "GF|Spaceship")
	float GetTravelTimeRemaining() const;

protected:
	void OnTravelTimer();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|Spaceship")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	FTimerHandle TravelTimerHandle;
	float TravelTimeRemaining = 0.0f;
};

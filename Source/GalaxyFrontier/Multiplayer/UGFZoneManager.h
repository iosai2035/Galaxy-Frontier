// Galaxy Frontier — UGFZoneManager.h (Phase 06)
// Battle Royale safe zone, an ActorComponent on AGFGameMode
// (server only). The zone shrinks in fixed phases: each phase
// waits, then shrinks the radius over a fixed time, then the
// damage outside the zone rises. Players outside the current
// radius take damage every second — the damage is applied by
// the server directly (it is not a client request, so no
// validation round-trip is needed; see Docs/BATTLE-ROYALE.md).
//
// Phase table (Docs/BATTLE-ROYALE.md — the numbers are the
// design, not measurements):
//   Phase  Radius    Wait  Shrink  DPS outside
//   0      6000 cm   30 s  —       0
//   1      4000 cm   20 s  60 s    1
//   2      2500 cm   15 s  45 s    2
//   3      1500 cm   15 s  40 s    5
//   4      800 cm    10 s  35 s    10
//   5      300 cm    10 s  30 s    15
//   6      100 cm    5 s   25 s    20
//   7      0 cm      60 s  20 s    25 (final, no safe area left;
//                                  persists until the match ends)
// The zone centre is the map centre (0,0) for the prototype
// island — a random centre per match is a later phase, not
// silently added here. The current centre/radius are written
// to AGFGameState (replicated) so clients can draw the zone.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UGFZoneManager.generated.h"

// One zone phase. RadiusCm is the radius the zone shrinks TO;
// DamagePerSecond is what players outside the zone take while
// this phase is active (including its wait + shrink).
USTRUCT(BlueprintType)
struct FGFZonePhase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Zone")
	float RadiusCm = 6000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Zone")
	float WaitSeconds = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Zone")
	float ShrinkSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Zone")
	float DamagePerSecond = 0.0f;
};

UCLASS(ClassGroup = (GF), meta = (BlueprintSpawnableComponent))
class GALAXYFRONTIER_API UGFZoneManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UGFZoneManager();

	// The 8 phases from the design table above. Editable in the
	// Editor; the defaults are the documented numbers.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Zone")
	TArray<FGFZonePhase> Phases;

	// Centre of the safe zone for the prototype island
	// (L_ZERON_BR_Prototype, Docs/BATTLE-ROYALE.md). Fixed for
	// now — random centres are a later phase.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Zone")
	FVector ZoneCenter = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Zone")
	int32 CurrentPhaseIndex = -1;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Zone")
	bool bZoneActive = false;

	// Starts the zone (called by AGFGameMode::EnterPlaying).
	// Does nothing on clients or when already active.
	UFUNCTION(BlueprintCallable, Category = "GF|Zone")
	void StartZone();

	// Stops the zone (called by AGFGameMode::EndMatch).
	UFUNCTION(BlueprintCallable, Category = "GF|Zone")
	void StopZone();

	UFUNCTION(BlueprintPure, Category = "GF|Zone")
	float GetCurrentRadius() const;

	UFUNCTION(BlueprintPure, Category = "GF|Zone")
	float GetCurrentDamagePerSecond() const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 1 s damage tick: every alive player outside the radius
	// takes DamagePerSecond. Server only, direct application —
	// zone damage is never a client request.
	void ApplyZoneDamage();

	// Advances the phase machine (wait -> shrink -> next phase).
	void AdvancePhase(float DeltaTime);

	void PublishZoneToGameState() const;

	float CurrentRadius = 6000.0f;
	float PhaseTimeRemaining = 0.0f;
	bool bIsShrinking = false;
	float ShrinkFromRadius = 6000.0f;
	float ShrinkElapsed = 0.0f;
	float DamageTimer = 0.0f;

	FTimerHandle DamageTimerHandle;
};

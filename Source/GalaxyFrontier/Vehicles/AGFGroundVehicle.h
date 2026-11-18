// Galaxy Frontier — AGFGroundVehicle.h (Phase 05)
// Drivable ground vehicle ("Rover RX-7" in Docs/EXPANSION.md).
// An APawn with floating movement: throttle / brake / steering
// are axis inputs, and the player enters/exits through possession.
//
// Driving rules:
// - While driving, the player character is hidden and its
//   collision is off; damage goes to the vehicle (its own
//   UGFHealthComponent, 250 HP), never to the hidden character.
// - Exiting puts the character back beside the vehicle and
//   re-possesses it. Exiting while the vehicle is moving is
//   refused — the player must stop first.
// - The mesh is assigned in the Editor (no mesh is faked).
//   Movement uses UFloatingPawnMovementComponent (Engine module),
//   not the Chaos vehicle system — no ChaosVehicles dependency
//   is added in Phase 05 (see Docs/EXPANSION.md).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AGFGroundVehicle.generated.h"

class AGFCharacter;
class UGFHealthComponent;
class UFloatingPawnMovementComponent;
class UInputAction;

struct FInputActionValue;

UCLASS()
class GALAXYFRONTIER_API AGFGroundVehicle : public APawn
{
	GENERATED_BODY()

public:
	AGFGroundVehicle();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// --- Tuning (starting values, all editable) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Vehicle", meta = (ClampMin = "0.0"))
	float MaxSpeed = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Vehicle", meta = (ClampMin = "0.0"))
	float Acceleration = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Vehicle", meta = (ClampMin = "0.0"))
	float BrakeDeceleration = 2400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Vehicle", meta = (ClampMin = "0.0"))
	float TurnRateDegrees = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Vehicle", meta = (ClampMin = "0.0"))
	float VehicleMaxHealth = 250.0f;

	// --- State ---
	UPROPERTY(BlueprintReadOnly, Category = "GF|Vehicle")
	bool bIsOccupied = false;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Vehicle")
	TObjectPtr<AGFCharacter> Driver = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|Vehicle")
	TObjectPtr<UGFHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|Vehicle")
	TObjectPtr<UFloatingPawnMovementComponent> MovementComponent;

	// --- Enhanced Input (actions created in the Editor) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Vehicle|Input")
	TObjectPtr<UInputAction> ThrottleAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Vehicle|Input")
	TObjectPtr<UInputAction> SteerAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Vehicle|Input")
	TObjectPtr<UInputAction> ExitVehicleAction;

	// Puts Character in the driver's seat: hides the character,
	// disables its collision, and possesses this vehicle.
	// Returns false for a null/dead character or an occupied
	// vehicle. Called by the interaction / prompt in the Editor.
	UFUNCTION(BlueprintCallable, Category = "GF|Vehicle")
	bool EnterVehicle(AGFCharacter* Character);

	// Puts the driver back beside the vehicle and re-possesses
	// the character. Returns false when not occupied, or when
	// the vehicle is still moving faster than 50 cm/s.
	UFUNCTION(BlueprintCallable, Category = "GF|Vehicle")
	bool ExitVehicle();

	UFUNCTION(BlueprintPure, Category = "GF|Vehicle")
	float GetCurrentSpeed() const;

protected:
	void HandleThrottle(const FInputActionValue& Value);
	void HandleSteer(const FInputActionValue& Value);
	void HandleExitVehicle();

	// Hides/shows the driver character + toggles its collision.
	void SetDriverHidden(bool bHidden);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|Vehicle")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	float ThrottleInput = 0.0f;
	float SteerInput = 0.0f;
};

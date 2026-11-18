// Galaxy Frontier — AGFCharacter.h (Phase 01)
// Third-person player character. Movement, camera and input only.
// Combat (Fire / Reload / weapons), interaction and inventory are
// declared as stubs and implemented in later phases — see .cpp.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AGFCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class GALAXYFRONTIER_API AGFCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGFCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// --- Movement values (starting values for testing, all editable) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Movement")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Movement")
	float RunSpeed = 550.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Movement")
	float SprintSpeed = 700.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Movement")
	float CrouchSpeed = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Movement")
	float JumpZVelocityValue = 520.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Movement")
	float AirControlValue = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Movement")
	float RotationRateValue = 540.0f;

	// --- Camera values ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Camera")
	float CameraArmLength = 320.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Camera")
	float CameraFOV = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Camera")
	float AimFOV = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Camera")
	float AimArmLength = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Camera")
	float LookSensitivity = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "GF|State")
	bool bIsSprinting = false;

	UPROPERTY(BlueprintReadOnly, Category = "GF|State")
	bool bIsAiming = false;

	// --- Components ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	// --- Enhanced Input (assets created in the Editor — see Docs/INPUT.md) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Input")
	TObjectPtr<UInputMappingContext> PlayerMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Input")
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Input")
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Input")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Input")
	TObjectPtr<UInputAction> ReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Input")
	TObjectPtr<UInputAction> SwitchWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Input")
	TObjectPtr<UInputAction> OpenInventoryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GF|Input")
	TObjectPtr<UInputAction> PauseAction;

protected:
	// Input handlers — implemented in Phase 01.
	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleJump();
	void HandleSprintStarted();
	void HandleSprintCompleted();
	void HandleCrouchStarted();
	void HandleCrouchCompleted();
	void HandleAimStarted();
	void HandleAimCompleted();

	// Declared in Phase 01, implemented in later phases. Each body logs
	// a warning and does nothing else — no fake behaviour is simulated.
	void HandleFire();
	void HandleReload();
	void HandleInteract();
	void HandleSwitchWeapon();
	void HandleOpenInventory();
	void HandlePause();

	void UpdateMovementSpeed();
};

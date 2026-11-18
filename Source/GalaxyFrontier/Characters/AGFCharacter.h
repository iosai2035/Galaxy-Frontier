// Galaxy Frontier — AGFCharacter.h (Phases 01-03)
// Third-person player character. Movement, camera and input (Phase 01),
// health + combat (Phase 02, real implementations), interaction
// (Phase 03). Inventory / Pause remain later phases — see .cpp.
// Realistic-character hooks: material slots, LOD and customisation
// (Docs/CHARACTERS.md). Meshes themselves are authored in Blender /
// MetaHuman-style tooling on macOS/Windows — no mesh binary is faked.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AGFCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UGFHealthComponent;
class UGFCombatComponent;
class UGFInteractionComponent;
struct FInputActionValue;

// Customisation applied to the character's materials (Phase 02/05).
// Colours are material parameters, not new meshes: SkinTone tints the
// skin material instance, ArmourTint tints armour/cloth. The actual
// Skeletal Mesh (with its slots) is assigned in the Editor.
USTRUCT(BlueprintType)
struct FGFCharacterCustomization
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Customization")
	FLinearColor SkinTone = FLinearColor(0.72f, 0.53f, 0.42f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Customization")
	FLinearColor HairColour = FLinearColor(0.12f, 0.08f, 0.05f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Customization")
	FLinearColor ArmourTint = FLinearColor(0.15f, 0.25f, 0.35f);

	// Material slot indices on the character Skeletal Mesh.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Customization")
	int32 SkinMaterialSlot = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Customization")
	int32 ArmourMaterialSlot = 1;
};

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

	// Phase 02: health + combat. Created in the constructor, never
	// null in game — guarded with null checks in the .cpp anyway.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|Combat")
	TObjectPtr<UGFHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|Combat")
	TObjectPtr<UGFCombatComponent> CombatComponent;

	// Phase 03: interaction. Created in the constructor.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|World")
	TObjectPtr<UGFInteractionComponent> InteractionComponent;

	// --- Realistic-character hooks (Docs/CHARACTERS.md) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Character")
	FGFCharacterCustomization Customization;

	// Forced LOD index for the character mesh, -1 = automatic.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Character")
	int32 ForcedLOD = -1;

	UFUNCTION(BlueprintCallable, Category = "GF|Character")
	void ApplyCustomization(const FGFCharacterCustomization& NewCustomization);

	UFUNCTION(BlueprintCallable, Category = "GF|Character")
	void ApplyMaterialToSlot(int32 SlotIndex, UMaterialInterface* Material);

	UFUNCTION(BlueprintCallable, Category = "GF|Character")
	void ApplyForcedLOD();

	UFUNCTION(BlueprintPure, Category = "GF|Combat")
	UGFHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UFUNCTION(BlueprintPure, Category = "GF|Combat")
	UGFCombatComponent* GetCombatComponent() const { return CombatComponent; }

	UFUNCTION(BlueprintPure, Category = "GF|World")
	UGFInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }

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

	// Input handlers — Fire / Reload / SwitchWeapon implemented in
	// Phase 02 (delegated to UGFCombatComponent), Interact in Phase 03
	// (delegated to UGFInteractionComponent). OpenInventory / Pause
	// remain stubs for Phases 05 / 04 — each logs a warning and does
	// nothing else, no fake behaviour is simulated.
	void HandleFireStarted();
	void HandleFireCompleted();
	void HandleReload();
	void HandleInteract();
	void HandleSwitchWeapon();
	void HandleOpenInventory();
	void HandlePause();

	void UpdateMovementSpeed();
};

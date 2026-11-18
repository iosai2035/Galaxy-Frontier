// Galaxy Frontier — AGFCharacter.h (Phases 01-05)
// Third-person player character. Movement, camera and input (Phase 01),
// health + combat (Phase 02, real implementations), interaction
// (Phase 03). Inventory / equipment / progression (Phase 05).
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
class UGFHUDController;
class UGFInventoryComponent;
class UGFEquipmentComponent;
class UGFProgressionComponent;
class UGFServerValidation;
class UGFReplicationManager;
struct FInputActionValue;

// Two base bodies (male / female, Phase 05 customisation) share
// the same skeleton so one Animation Blueprint retargets to both
// (Docs/CHARACTERS.md). The body is a mesh choice made in the
// Editor — this value only records which one is active.
UENUM(BlueprintType)
enum class EGFBodyType : uint8
{
	Male		UMETA(DisplayName = "Male"),
	Female		UMETA(DisplayName = "Female")
};

// Customisation applied to the character's materials (Phase 02/05).
// Colours are material parameters, not new meshes: SkinTone tints the
// skin material instance, ArmourTint tints armour/cloth. The actual
// Skeletal Mesh (with its slots) is assigned in the Editor.
USTRUCT(BlueprintType)
struct FGFCharacterCustomization
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Customization")
	EGFBodyType BodyType = EGFBodyType::Male;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Customization")
	FLinearColor SkinTone = FLinearColor(0.72f, 0.53f, 0.42f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Customization")
	FLinearColor HairColour = FLinearColor(0.12f, 0.08f, 0.05f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Customization")
	FLinearColor ArmourTint = FLinearColor(0.15f, 0.25f, 0.35f);

	// Phase 05: accent colour for the metal/tech slot (slot 2) —
	// visor trim, lights, edge details. Cosmetic only.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Customization")
	FLinearColor AccentColour = FLinearColor(0.10f, 0.55f, 0.65f);

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

	// Phase 04: touch look pitch inversion, applied by
	// UGFSettingsManager::ApplyCameraSettings(). When true,
	// dragging up looks down.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Camera")
	bool bInvertLookPitch = false;

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

	// Phase 04: UI bridge (UGFHUDController). Created in the
	// constructor; owns the HUD / Pause / Game Over widgets and
	// binds the gameplay delegates for them. Never null in game.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|UI")
	TObjectPtr<UGFHUDController> HUDController;

	// Phase 05: inventory + equipment + progression. Created in
	// the constructor; every use site still null-checks them
	// (Docs/RELIABILITY.md).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|Inventory")
	TObjectPtr<UGFInventoryComponent> InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|Equipment")
	TObjectPtr<UGFEquipmentComponent> EquipmentComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|Progression")
	TObjectPtr<UGFProgressionComponent> ProgressionComponent;

	// Phase 06: network. Created in the constructor. In
	// single-player they exist but never block anything (the
	// validation component passes every check in Standalone,
	// and the replication manager is only read by server code).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|Network")
	TObjectPtr<UGFServerValidation> ServerValidation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|Network")
	TObjectPtr<UGFReplicationManager> ReplicationManager;

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

	UFUNCTION(BlueprintPure, Category = "GF|UI")
	UGFHUDController* GetHUDController() const { return HUDController; }

	UFUNCTION(BlueprintPure, Category = "GF|Inventory")
	UGFInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	UFUNCTION(BlueprintPure, Category = "GF|Equipment")
	UGFEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }

	UFUNCTION(BlueprintPure, Category = "GF|Progression")
	UGFProgressionComponent* GetProgressionComponent() const { return ProgressionComponent; }

	UFUNCTION(BlueprintPure, Category = "GF|Network")
	UGFServerValidation* GetServerValidation() const { return ServerValidation; }

	UFUNCTION(BlueprintPure, Category = "GF|Network")
	UGFReplicationManager* GetReplicationManager() const { return ReplicationManager; }

	// Phase 06: true when this game is networked (a client or a
	// server in a session). False in single-player (Standalone).
	// Input handlers use it to route requests through the
	// PlayerController's Server RPCs instead of executing them
	// locally — the server is authoritative in a session.
	UFUNCTION(BlueprintPure, Category = "GF|Network")
	bool IsNetworkedGame() const;

	// Phase 04: touch-button state setters. The HUD buttons call
	// these (the keyboard handlers set the same flags directly).
	// They update the movement speed, so aiming still slows the
	// character to WalkSpeed — setting bIsAiming directly would not.
	UFUNCTION(BlueprintCallable, Category = "GF|Character")
	void SetAiming(bool bNewAiming);

	UFUNCTION(BlueprintCallable, Category = "GF|Character")
	void SetSprinting(bool bNewSprinting);

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
	// (delegated to UGFInteractionComponent), Pause in Phase 04
	// (delegated to UGFHUDController), OpenInventory in Phase 05
	// (delegated to UGFHUDController::ToggleInventory). No Phase 01
	// stub remains.
	void HandleFireStarted();
	void HandleFireCompleted();
	void HandleReload();
	void HandleInteract();
	void HandleSwitchWeapon();
	void HandleOpenInventory();
	void HandlePause();

	void UpdateMovementSpeed();
};

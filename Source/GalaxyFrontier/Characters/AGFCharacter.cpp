// Galaxy Frontier — AGFCharacter.cpp (Phases 01-05)
// Third-person movement, camera and Enhanced Input (Phase 01).
// Combat handlers delegate to UGFCombatComponent (Phase 02, real),
// Interact delegates to UGFInteractionComponent (Phase 03, real),
// Pause delegates to UGFHUDController (Phase 04, real),
// OpenInventory delegates to UGFHUDController (Phase 05, real).
// Inventory / equipment / progression components are Phase 05.

#include "AGFCharacter.h"

#include "Camera/CameraComponent.h"
#include "Combat/UGFCombatComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/UGFHealthComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "Inventory/UGFEquipmentComponent.h"
#include "Inventory/UGFInventoryComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Progression/UGFProgressionComponent.h"
#include "UI/UGFHUDController.h"
#include "World/UGFInteractionComponent.h"

AGFCharacter::AGFCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Character faces movement direction; controller rotates the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->bOrientRotationToMovement = true;
	MoveComp->RotationRate = FRotator(0.0f, RotationRateValue, 0.0f);
	MoveComp->JumpZVelocity = JumpZVelocityValue;
	MoveComp->AirControl = AirControlValue;
	MoveComp->MaxWalkSpeed = RunSpeed;
	MoveComp->MaxWalkSpeedCrouched = CrouchSpeed;
	// Note: UCharacterMovementComponent has no built-in MaxSprintSpeed.
	// Sprint is implemented by AGFCharacter::UpdateMovementSpeed(),
	// which sets MaxWalkSpeed to SprintSpeed (700) while sprinting.

	// Camera boom: follows the player, blocks on walls, rotates with
	// the controller (touch drag / mouse look).
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = CameraArmLength;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bDoCollisionTest = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->FieldOfView = CameraFOV;

	// Phase 02/03/04/05 components. Created here so they are never
	// null in game; every use site still null-checks them
	// (Docs/RELIABILITY.md).
	HealthComponent = CreateDefaultSubobject<UGFHealthComponent>(TEXT("HealthComponent"));
	CombatComponent = CreateDefaultSubobject<UGFCombatComponent>(TEXT("CombatComponent"));
	InteractionComponent = CreateDefaultSubobject<UGFInteractionComponent>(TEXT("InteractionComponent"));
	HUDController = CreateDefaultSubobject<UGFHUDController>(TEXT("HUDController"));
	InventoryComponent = CreateDefaultSubobject<UGFInventoryComponent>(TEXT("InventoryComponent"));
	EquipmentComponent = CreateDefaultSubobject<UGFEquipmentComponent>(TEXT("EquipmentComponent"));
	ProgressionComponent = CreateDefaultSubobject<UGFProgressionComponent>(TEXT("ProgressionComponent"));
}

void AGFCharacter::ApplyCustomization(const FGFCharacterCustomization& NewCustomization)
{
	Customization = NewCustomization;
	USkeletalMeshComponent* Mesh = GetMesh();
	if (!Mesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFCharacter::ApplyCustomization — no Skeletal Mesh (assign one in the Editor)."));
		return;
	}
	// Dynamic material instances per slot, tinted by the customisation
	// colours. Parameter names must match the materials authored in
	// the Editor (see Docs/CHARACTERS.md); a missing parameter is a
	// no-op in UMaterialInstanceDynamic, never a crash.
	UMaterialInstanceDynamic* SkinMID = Mesh->CreateDynamicMaterialInstance(
		Customization.SkinMaterialSlot, Mesh->GetMaterial(Customization.SkinMaterialSlot));
	if (SkinMID)
	{
		SkinMID->SetVectorParameterValue(TEXT("SkinTone"), Customization.SkinTone);
	}
	UMaterialInstanceDynamic* ArmourMID = Mesh->CreateDynamicMaterialInstance(
		Customization.ArmourMaterialSlot, Mesh->GetMaterial(Customization.ArmourMaterialSlot));
	if (ArmourMID)
	{
		ArmourMID->SetVectorParameterValue(TEXT("ArmourTint"), Customization.ArmourTint);
	}
	// Phase 05: accent colour on the metal/tech slot (slot 2) —
	// visor trim, lights, edge details. Slot 2 is the metal/tech
	// slot by the material convention in Docs/CHARACTERS.md.
	// A mesh with fewer than 3 slots simply gets no accent pass.
	if (Mesh->GetNumMaterials() > 2)
	{
		UMaterialInstanceDynamic* AccentMID = Mesh->CreateDynamicMaterialInstance(
			2, Mesh->GetMaterial(2));
		if (AccentMID)
		{
			AccentMID->SetVectorParameterValue(TEXT("AccentColour"), Customization.AccentColour);
		}
	}
	ApplyForcedLOD();
}

void AGFCharacter::ApplyMaterialToSlot(int32 SlotIndex, UMaterialInterface* Material)
{
	USkeletalMeshComponent* Mesh = GetMesh();
	if (!Mesh || !Material || SlotIndex < 0 || SlotIndex >= Mesh->GetNumMaterials())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFCharacter::ApplyMaterialToSlot — invalid mesh/material/slot %d."), SlotIndex);
		return;
	}
	Mesh->SetMaterial(SlotIndex, Material);
}

void AGFCharacter::ApplyForcedLOD()
{
	USkeletalMeshComponent* Mesh = GetMesh();
	if (!Mesh)
	{
		return;
	}
	if (ForcedLOD >= 0)
	{
		const int32 LODCount = Mesh->GetNumLODs();
		Mesh->SetForcedLOD(FMath::Clamp(ForcedLOD, 0, FMath::Max(0, LODCount - 1)));
	}
	else
	{
		Mesh->SetForcedLOD(-1);
	}
}

void AGFCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Apply the mapping context to the local player. If the context or
	// actions are not assigned in the Editor, input simply does nothing
	// — this is checked, not assumed.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (PlayerMappingContext)
			{
				Subsystem->AddMappingContext(PlayerMappingContext, 0);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[GF] AGFCharacter: PlayerMappingContext (IMC_Player) is not assigned. Create it in the Editor — see Docs/INPUT.md."));
			}
		}
	}

	// Apply the designer-set customisation / LOD. A missing mesh is
	// logged inside ApplyCustomization, never a crash.
	ApplyCustomization(Customization);
}

void AGFCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Smooth FOV / arm transition between normal and aim mode.
	const float TargetFOV = bIsAiming ? AimFOV : CameraFOV;
	const float TargetArm = bIsAiming ? AimArmLength : CameraArmLength;
	if (FollowCamera)
	{
		FollowCamera->FieldOfView = FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOV, DeltaTime, 8.0f);
	}
	if (CameraBoom)
	{
		CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArm, DeltaTime, 8.0f);
	}
}

void AGFCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	// Bind only actions that are assigned; a missing action is a setup
	// error to fix in the Editor, not a crash.
	if (MoveAction) { EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGFCharacter::HandleMove); }
	if (LookAction) { EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGFCharacter::HandleLook); }
	if (JumpAction) { EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &AGFCharacter::HandleJump); }
	if (SprintAction)
	{
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &AGFCharacter::HandleSprintStarted);
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &AGFCharacter::HandleSprintCompleted);
	}
	if (CrouchAction)
	{
		EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Started, this, &AGFCharacter::HandleCrouchStarted);
		EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AGFCharacter::HandleCrouchCompleted);
	}
	if (AimAction)
	{
		EnhancedInput->BindAction(AimAction, ETriggerEvent::Started, this, &AGFCharacter::HandleAimStarted);
		EnhancedInput->BindAction(AimAction, ETriggerEvent::Completed, this, &AGFCharacter::HandleAimCompleted);
	}
	if (FireAction)
	{
		// Started = trigger pull (StartFire, timer handles automatic
		// weapons), Completed = release (StopFire). Never fire on
		// Triggered alone — that would bypass the fire-rate timer.
		EnhancedInput->BindAction(FireAction, ETriggerEvent::Started, this, &AGFCharacter::HandleFireStarted);
		EnhancedInput->BindAction(FireAction, ETriggerEvent::Completed, this, &AGFCharacter::HandleFireCompleted);
	}
	if (ReloadAction) { EnhancedInput->BindAction(ReloadAction, ETriggerEvent::Started, this, &AGFCharacter::HandleReload); }
	if (InteractAction) { EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &AGFCharacter::HandleInteract); }
	if (SwitchWeaponAction) { EnhancedInput->BindAction(SwitchWeaponAction, ETriggerEvent::Started, this, &AGFCharacter::HandleSwitchWeapon); }
	if (OpenInventoryAction) { EnhancedInput->BindAction(OpenInventoryAction, ETriggerEvent::Started, this, &AGFCharacter::HandleOpenInventory); }
	if (PauseAction) { EnhancedInput->BindAction(PauseAction, ETriggerEvent::Started, this, &AGFCharacter::HandlePause); }
}

void AGFCharacter::HandleMove(const FInputActionValue& Value)
{
	const FVector2D MoveVector = Value.Get<FVector2D>();
	if (!Controller || MoveVector.IsNearlyZero())
	{
		return;
	}

	const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MoveVector.Y);
	AddMovementInput(RightDirection, MoveVector.X);
}

void AGFCharacter::HandleLook(const FInputActionValue& Value)
{
	const FVector2D LookVector = Value.Get<FVector2D>();
	if (!Controller || LookVector.IsNearlyZero())
	{
		return;
	}

	AddControllerYawInput(LookVector.X * LookSensitivity);
	// Phase 04: pitch inversion is a player setting
	// (UGFSettingsManager applies it to bInvertLookPitch).
	AddControllerPitchInput(LookVector.Y * LookSensitivity * (bInvertLookPitch ? -1.0f : 1.0f));
}

void AGFCharacter::HandleJump()
{
	Jump();
}

void AGFCharacter::HandleSprintStarted()
{
	bIsSprinting = true;
	UpdateMovementSpeed();
}

void AGFCharacter::HandleSprintCompleted()
{
	bIsSprinting = false;
	UpdateMovementSpeed();
}

void AGFCharacter::HandleCrouchStarted()
{
	Crouch();
}

void AGFCharacter::HandleCrouchCompleted()
{
	UnCrouch();
}

void AGFCharacter::HandleAimStarted()
{
	bIsAiming = true;
	UpdateMovementSpeed();
}

void AGFCharacter::HandleAimCompleted()
{
	bIsAiming = false;
	UpdateMovementSpeed();
}

void AGFCharacter::SetAiming(bool bNewAiming)
{
	bIsAiming = bNewAiming;
	UpdateMovementSpeed();
}

void AGFCharacter::SetSprinting(bool bNewSprinting)
{
	bIsSprinting = bNewSprinting;
	UpdateMovementSpeed();
}

void AGFCharacter::UpdateMovementSpeed()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		return;
	}

	// Aiming slows the character to WalkSpeed (aim-walk). Sprinting
	// overrides run speed only when not aiming and not crouched.
	if (bIsAiming)
	{
		MoveComp->MaxWalkSpeed = WalkSpeed;
	}
	else if (bIsSprinting)
	{
		MoveComp->MaxWalkSpeed = SprintSpeed;
	}
	else
	{
		MoveComp->MaxWalkSpeed = RunSpeed;
	}
}

// --- Phase 02 / 03 — real implementations ---------------------------
// Fire / Reload / SwitchWeapon delegate to UGFCombatComponent.
// Interact delegates to UGFInteractionComponent. A missing component
// is logged and ignored — the character never crashes on input.

void AGFCharacter::HandleFireStarted()
{
	if (!CombatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFCharacter::HandleFireStarted — no CombatComponent."));
		return;
	}
	CombatComponent->StartFire();
}

void AGFCharacter::HandleFireCompleted()
{
	if (CombatComponent)
	{
		CombatComponent->StopFire();
	}
}

void AGFCharacter::HandleReload()
{
	if (!CombatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFCharacter::HandleReload — no CombatComponent."));
		return;
	}
	CombatComponent->Reload();
}

void AGFCharacter::HandleInteract()
{
	if (!InteractionComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFCharacter::HandleInteract — no InteractionComponent."));
		return;
	}
	InteractionComponent->BeginInteraction();
}

void AGFCharacter::HandleSwitchWeapon()
{
	if (!CombatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFCharacter::HandleSwitchWeapon — no CombatComponent."));
		return;
	}
	CombatComponent->SwitchToNextWeapon();
}

void AGFCharacter::HandleOpenInventory()
{
	if (!HUDController)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFCharacter::HandleOpenInventory — no HUDController."));
		return;
	}
	HUDController->ToggleInventory();
}

void AGFCharacter::HandlePause()
{
	if (!HUDController)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFCharacter::HandlePause — no HUDController."));
		return;
	}
	HUDController->TogglePause();
}

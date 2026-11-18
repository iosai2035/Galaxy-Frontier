// Galaxy Frontier — AGFCharacter.cpp (Phase 01)
// Third-person movement, camera and Enhanced Input. Combat, interaction
// and inventory handlers at the bottom are NOT IMPLEMENTED stubs
// (Phase 02 / Phase 05) — they log a warning and return.

#include "AGFCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"

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
	if (FireAction) { EnhancedInput->BindAction(FireAction, ETriggerEvent::Triggered, this, &AGFCharacter::HandleFire); }
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
	AddControllerPitchInput(LookVector.Y * LookSensitivity);
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

// --- NOT IMPLEMENTED stubs (Phase 02 / Phase 05) -------------------------
// Each logs once per call and does nothing else. No ammo, damage,
// weapon, inventory or pause behaviour is faked here.

void AGFCharacter::HandleFire()
{
	UE_LOG(LogTemp, Warning, TEXT("[GF] AGFCharacter::HandleFire — NOT IMPLEMENTED (Phase 02 Combat)."));
}

void AGFCharacter::HandleReload()
{
	UE_LOG(LogTemp, Warning, TEXT("[GF] AGFCharacter::HandleReload — NOT IMPLEMENTED (Phase 02 Combat)."));
}

void AGFCharacter::HandleInteract()
{
	UE_LOG(LogTemp, Warning, TEXT("[GF] AGFCharacter::HandleInteract — NOT IMPLEMENTED (Phase 03 World)."));
}

void AGFCharacter::HandleSwitchWeapon()
{
	UE_LOG(LogTemp, Warning, TEXT("[GF] AGFCharacter::HandleSwitchWeapon — NOT IMPLEMENTED (Phase 02 Combat)."));
}

void AGFCharacter::HandleOpenInventory()
{
	UE_LOG(LogTemp, Warning, TEXT("[GF] AGFCharacter::HandleOpenInventory — NOT IMPLEMENTED (Phase 05 Expansion)."));
}

void AGFCharacter::HandlePause()
{
	UE_LOG(LogTemp, Warning, TEXT("[GF] AGFCharacter::HandlePause — NOT IMPLEMENTED (Phase 04 Mobile UI)."));
}

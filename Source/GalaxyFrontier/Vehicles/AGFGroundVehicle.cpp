// Galaxy Frontier — AGFGroundVehicle.cpp (Phase 05)
// Possession-based driving. The character is hidden while driving;
// damage is routed to the vehicle's own health component.

#include "Vehicles/AGFGroundVehicle.h"

#include "Characters/AGFCharacter.h"
#include "Components/UGFHealthComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"

AGFGroundVehicle::AGFGroundVehicle()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovementComponent>(TEXT("MovementComponent"));
	MovementComponent->MaxSpeed = MaxSpeed;
	MovementComponent->Acceleration = Acceleration;
	MovementComponent->Deceleration = BrakeDeceleration;

	HealthComponent = CreateDefaultSubobject<UGFHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->MaxHealth = VehicleMaxHealth;
	HealthComponent->MaxShield = 0.0f;
	HealthComponent->CurrentShield = 0.0f;
}

void AGFGroundVehicle::BeginPlay()
{
	Super::BeginPlay();
	// Apply the designer-set tuning (defaults above are the
	// starting values; the Editor may override them).
	if (MovementComponent)
	{
		MovementComponent->MaxSpeed = MaxSpeed;
		MovementComponent->Acceleration = Acceleration;
		MovementComponent->Deceleration = BrakeDeceleration;
	}
	if (HealthComponent)
	{
		HealthComponent->MaxHealth = VehicleMaxHealth;
		HealthComponent->ResetHealth();
	}
}

void AGFGroundVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bIsOccupied)
	{
		return;
	}
	// Forward / reverse from the throttle axis; steering turns
	// the vehicle around its up axis. Steering authority scales
	// with speed so the rover cannot spin on the spot.
	const float Speed = GetCurrentSpeed();
	const float SteerAuthority = FMath::Clamp(Speed / 300.0f, 0.0f, 1.0f);
	if (!FMath::IsNearlyZero(SteerInput) && SteerAuthority > 0.0f)
	{
		AddControllerYawInput(SteerInput * TurnRateDegrees * SteerAuthority * DeltaTime);
	}
	if (!FMath::IsNearlyZero(ThrottleInput))
	{
		AddMovementInput(GetActorForwardVector(), ThrottleInput);
	}
}

void AGFGroundVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (ThrottleAction) { EnhancedInput->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &AGFGroundVehicle::HandleThrottle); }
	if (SteerAction) { EnhancedInput->BindAction(SteerAction, ETriggerEvent::Triggered, this, &AGFGroundVehicle::HandleSteer); }
	if (ExitVehicleAction) { EnhancedInput->BindAction(ExitVehicleAction, ETriggerEvent::Started, this, &AGFGroundVehicle::HandleExitVehicle); }
}

bool AGFGroundVehicle::EnterVehicle(AGFCharacter* Character)
{
	if (!Character)
	{
		return false;
	}
	if (bIsOccupied)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFGroundVehicle::EnterVehicle — vehicle %s is already occupied."), *GetName());
		return false;
	}
	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFGroundVehicle::EnterVehicle — character %s has no player controller."), *Character->GetName());
		return false;
	}
	if (HealthComponent && HealthComponent->IsDead())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFGroundVehicle::EnterVehicle — vehicle %s is destroyed; it cannot be driven."), *GetName());
		return false;
	}
	Driver = Character;
	bIsOccupied = true;
	SetDriverHidden(true);
	PC->Possess(this);
	return true;
}

bool AGFGroundVehicle::ExitVehicle()
{
	if (!bIsOccupied || !Driver)
	{
		return false;
	}
	if (GetCurrentSpeed() > 50.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFGroundVehicle::ExitVehicle — vehicle is still moving (%.0f cm/s); stop first."), GetCurrentSpeed());
		return false;
	}
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return false;
	}
	// Step out to the vehicle's right side, at ground level.
	const FVector ExitLocation = GetActorLocation()
		+ GetActorRightVector() * 250.0f
		+ FVector(0.0f, 0.0f, 50.0f);
	Driver->SetActorLocation(ExitLocation, false, nullptr, ETeleportType::TeleportPhysics);
	SetDriverHidden(false);
	AGFCharacter* ExitingDriver = Driver;
	Driver = nullptr;
	bIsOccupied = false;
	ThrottleInput = 0.0f;
	SteerInput = 0.0f;
	PC->Possess(ExitingDriver);
	return true;
}

float AGFGroundVehicle::GetCurrentSpeed() const
{
	return GetVelocity().Size();
}

void AGFGroundVehicle::HandleThrottle(const FInputActionValue& Value)
{
	ThrottleInput = FMath::Clamp(Value.Get<float>(), -1.0f, 1.0f);
}

void AGFGroundVehicle::HandleSteer(const FInputActionValue& Value)
{
	SteerInput = FMath::Clamp(Value.Get<float>(), -1.0f, 1.0f);
}

void AGFGroundVehicle::HandleExitVehicle()
{
	ExitVehicle();
}

void AGFGroundVehicle::SetDriverHidden(bool bHidden)
{
	if (!Driver)
	{
		return;
	}
	Driver->SetActorHiddenInGame(bHidden);
	Driver->SetActorEnableCollision(!bHidden);
}

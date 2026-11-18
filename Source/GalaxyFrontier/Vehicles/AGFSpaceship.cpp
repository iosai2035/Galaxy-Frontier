// Galaxy Frontier — AGFSpaceship.cpp (Phase 05)
// Travel = validated spool-up + real level load. No space combat.

#include "Vehicles/AGFSpaceship.h"

#include "Components/StaticMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/GameplayStatics.h"
#include "World/UGFPlanetDataAsset.h"

AGFSpaceship::AGFSpaceship()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// Hover-capable movement for pad manoeuvres. The ship is
	// transport, not a combat craft — speed is deliberately low.
	UFloatingPawnMovementComponent* Movement = CreateDefaultSubobject<UFloatingPawnMovementComponent>(TEXT("MovementComponent"));
	Movement->MaxSpeed = 600.0f;
	Movement->Acceleration = 400.0f;
	Movement->Deceleration = 800.0f;
}

void AGFSpaceship::BeginPlay()
{
	Super::BeginPlay();
	TravelTimeRemaining = 0.0f;
}

void AGFSpaceship::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TravelTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

bool AGFSpaceship::TravelToPlanet(UGFPlanetDataAsset* Planet, const TArray<FName>& CompletedMissionIds)
{
	if (bIsTravelling)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFSpaceship::TravelToPlanet — travel already in progress; finish or cancel it first."));
		return false;
	}
	if (!Planet || !Planet->Validate())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFSpaceship::TravelToPlanet — null or invalid planet."));
		return false;
	}
	if (!Planet->IsUnlocked(CompletedMissionIds))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFSpaceship::TravelToPlanet — planet %s is locked; complete %s first."),
			*Planet->PlanetId.ToString(), *Planet->UnlockMissionId.ToString());
		return false;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}
	DestinationPlanet = Planet;
	bIsTravelling = true;
	TravelTimeRemaining = TravelSpoolUpSeconds;
	OnTravelStarted.Broadcast(Planet->PlanetId);
	World->GetTimerManager().SetTimer(TravelTimerHandle, this, &AGFSpaceship::OnTravelTimer, TravelSpoolUpSeconds, false);
	return true;
}

bool AGFSpaceship::CancelTravel()
{
	if (!bIsTravelling)
	{
		return false;
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TravelTimerHandle);
	}
	bIsTravelling = false;
	DestinationPlanet = nullptr;
	TravelTimeRemaining = 0.0f;
	return true;
}

float AGFSpaceship::GetTravelTimeRemaining() const
{
	if (!bIsTravelling)
	{
		return 0.0f;
	}
	if (UWorld* World = GetWorld())
	{
		return FMath::Max(0.0f, World->GetTimerManager().GetTimerRemaining(TravelTimerHandle));
	}
	return TravelTimeRemaining;
}

void AGFSpaceship::OnTravelTimer()
{
	if (!DestinationPlanet)
	{
		bIsTravelling = false;
		return;
	}
	// The destination becomes current before the load so the
	// save (written by the Level Blueprint on BeginPlay of the
	// new level) records the right planet.
	const FName PlanetId = DestinationPlanet->PlanetId;
	const FName LevelName = DestinationPlanet->LevelName;
	CurrentPlanet = DestinationPlanet;
	DestinationPlanet = nullptr;
	bIsTravelling = false;
	TravelTimeRemaining = 0.0f;
	OnTravelCompleted.Broadcast(PlanetId);
	UGameplayStatics::OpenLevel(GetWorld(), LevelName);
}

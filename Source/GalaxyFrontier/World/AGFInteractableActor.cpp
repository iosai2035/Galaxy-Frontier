// Galaxy Frontier — AGFInteractableActor.cpp (Phase 03)

#include "World/AGFInteractableActor.h"

#include "Components/StaticMeshComponent.h"

AGFInteractableActor::AGFInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	// Mesh asset is assigned in the Editor. Collision is query-only
	// so the interaction overlap can find this actor.
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	RootComponent = MeshComponent;
}

void AGFInteractableActor::Interact(AActor* Interactor)
{
	if (!CanInteract(Interactor))
	{
		return;
	}
	bHasBeenUsed = true;
	if (InteractableType == EGFInteractableType::Door)
	{
		bIsDoorOpen = !bIsDoorOpen;
	}
	OnInteracted.Broadcast(Interactor);
}

FText AGFInteractableActor::GetInteractionPrompt() const
{
	return PromptText;
}

bool AGFInteractableActor::CanInteract(AActor* Interactor) const
{
	if (!Interactor)
	{
		return false;
	}
	if (bHasBeenUsed && !bCanBeUsedMultipleTimes)
	{
		return false;
	}
	return true;
}

float AGFInteractableActor::GetInteractionDuration() const
{
	return FMath::Max(0.0f, InteractionDurationSeconds);
}

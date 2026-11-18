// Galaxy Frontier — AGFPickupActor.cpp (Phase 05)
// Grants items through the inventory only — the pickup never
// writes health, ammo or mission state directly.

#include "World/AGFPickupActor.h"

#include "Components/StaticMeshComponent.h"
#include "Inventory/UGFInventoryComponent.h"
#include "Inventory/UGFItemDataAsset.h"

AGFPickupActor::AGFPickupActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
}

void AGFPickupActor::BeginPlay()
{
	Super::BeginPlay();
	RemainingQuantity = FMath::Max(0, Quantity);
	bIsAvailable = RemainingQuantity > 0 && ItemData != nullptr;
	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFPickupActor %s has no ItemData; it can never be picked up."), *GetName());
	}
}

void AGFPickupActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RespawnTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void AGFPickupActor::Interact(AActor* Interactor)
{
	if (!CanInteract(Interactor))
	{
		return;
	}
	UGFInventoryComponent* Inventory = Interactor->FindComponentByClass<UGFInventoryComponent>();
	if (!Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFPickupActor::Interact — interactor %s has no inventory."), *Interactor->GetName());
		return;
	}
	const int32 Added = Inventory->AddItem(ItemData, RemainingQuantity);
	if (Added <= 0)
	{
		// Inventory full (AddItem already broadcast
		// OnInventoryFull). The pickup stays, nothing is lost.
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFPickupActor::Interact — inventory full; %s not taken."), *ItemData->ItemId.ToString());
		return;
	}
	RemainingQuantity -= Added;
	if (RemainingQuantity <= 0)
	{
		Consume();
	}
}

FText AGFPickupActor::GetInteractionPrompt() const
{
	if (!ItemData)
	{
		return FText::FromString(TEXT("Take"));
	}
	if (RemainingQuantity > 1)
	{
		return FText::Format(NSLOCTEXT("GF", "PickupPromptMultiple", "Take {0} x{1}"),
			ItemData->DisplayName, RemainingQuantity);
	}
	return FText::Format(NSLOCTEXT("GF", "PickupPromptSingle", "Take {0}"), ItemData->DisplayName);
}

bool AGFPickupActor::CanInteract(AActor* Interactor) const
{
	if (!bIsAvailable || !ItemData || RemainingQuantity <= 0)
	{
		return false;
	}
	if (!Interactor)
	{
		return false;
	}
	UGFInventoryComponent* Inventory = Interactor->FindComponentByClass<UGFInventoryComponent>();
	if (!Inventory)
	{
		return false;
	}
	// Offer the pickup only when at least one unit fits. When
	// the inventory is full the prompt disappears instead of
	// letting the player interact and get nothing.
	return Inventory->CanAddItem(ItemData, 1);
}

void AGFPickupActor::Consume()
{
	bIsAvailable = false;
	SetPickupVisible(false);
	if (bRespawns)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(RespawnTimerHandle, this, &AGFPickupActor::OnRespawnTimer, RespawnSeconds, false);
		}
	}
}

void AGFPickupActor::OnRespawnTimer()
{
	RemainingQuantity = FMath::Max(0, Quantity);
	bIsAvailable = RemainingQuantity > 0 && ItemData != nullptr;
	SetPickupVisible(bIsAvailable);
}

void AGFPickupActor::SetPickupVisible(bool bVisible)
{
	if (MeshComponent)
	{
		MeshComponent->SetVisibility(bVisible);
		MeshComponent->SetCollisionEnabled(bVisible
			? ECollisionEnabled::QueryAndPhysics
			: ECollisionEnabled::NoCollision);
	}
	SetActorHiddenInGame(!bVisible);
}

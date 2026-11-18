// Galaxy Frontier — UGFInteractionComponent.cpp (Phase 03)

#include "World/UGFInteractionComponent.h"

#include "TimerManager.h"
#include "World/IGFInteractable.h"

UGFInteractionComponent::UGFInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGFInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ScanTimerHandle, this, &UGFInteractionComponent::OnScanTimer,
			FMath::Max(0.05f, ScanIntervalSeconds), true);
	}
}

void UGFInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CancelInteraction();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ScanTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void UGFInteractionComponent::OnScanTimer()
{
	// Do not re-scan while an interaction is running — the target
	// must not change under the player's finger.
	if (bIsInteracting)
	{
		return;
	}
	FindInteractable();
}

AActor* UGFInteractionComponent::FindInteractable()
{
	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();
	if (!World || !Owner)
	{
		SetCurrentInteractable(nullptr);
		return nullptr;
	}

	// Overlap a sphere around the owner. Interactables are actors
	// that implement IGFInteractable; anything else is ignored.
	TArray<FOverlapResult> Overlaps;
	const FCollisionShape Sphere = FCollisionShape::MakeSphere(InteractionRange);
	const FCollisionQueryParams QueryParams(NAME_None, false, Owner);
	World->OverlapMultiByObjectType(
		Overlaps, Owner->GetActorLocation(), FQuat::Identity,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_WorldDynamic),
		Sphere, QueryParams);

	AActor* Best = nullptr;
	float BestDistanceSq = InteractionRange * InteractionRange;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate || Candidate == Owner)
		{
			continue;
		}
		// C++-only interface: cast to IGFInteractable and call it
		// directly. (Execute_* wrappers are only generated for
		// UFUNCTION interface methods — these are plain C++ virtuals.)
		IGFInteractable* Interactable = Cast<IGFInteractable>(Candidate);
		if (!Interactable)
		{
			continue;
		}
		// CanInteract is checked on the interface, not on the class.
		if (!Interactable->CanInteract(Owner))
		{
			continue;
		}
		const float DistanceSq = FVector::DistSquared(Owner->GetActorLocation(), Candidate->GetActorLocation());
		if (DistanceSq <= BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			Best = Candidate;
		}
	}
	SetCurrentInteractable(Best);
	return Best;
}

void UGFInteractionComponent::SetCurrentInteractable(AActor* NewInteractable)
{
	if (CurrentInteractable == NewInteractable)
	{
		return;
	}
	CurrentInteractable = NewInteractable;
	OnInteractableChanged.Broadcast(NewInteractable);
}

void UGFInteractionComponent::BeginInteraction()
{
	if (bIsInteracting)
	{
		return;
	}
	if (!CurrentInteractable)
	{
		FindInteractable();
	}
	if (!CurrentInteractable)
	{
		return; // Nothing in range: a press with no target is a no-op.
	}
	AActor* Owner = GetOwner();
	IGFInteractable* Interactable = Cast<IGFInteractable>(CurrentInteractable);
	if (!Interactable || !Interactable->CanInteract(Owner))
	{
		// Target became unusable since the scan — re-scan next tick.
		SetCurrentInteractable(nullptr);
		return;
	}
	const float Duration = Interactable->GetInteractionDuration();
	if (Duration <= 0.0f)
	{
		CompleteInteraction();
		return;
	}
	bIsInteracting = true;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			InteractionTimerHandle, this, &UGFInteractionComponent::CompleteInteraction,
			FMath::Max(0.05f, Duration), false);
	}
	else
	{
		bIsInteracting = false;
	}
}

void UGFInteractionComponent::CompleteInteraction()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InteractionTimerHandle);
	}
	bIsInteracting = false;
	if (!CurrentInteractable)
	{
		return;
	}
	// Re-validate at completion time: range and usability can change
	// during a hold-to-interact. A target that moved away or became
	// unusable does NOT complete.
	AActor* Owner = GetOwner();
	if (Owner && FVector::Dist(Owner->GetActorLocation(), CurrentInteractable->GetActorLocation()) > InteractionRange)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFInteractionComponent::CompleteInteraction — target moved out of range, interaction cancelled."));
		SetCurrentInteractable(nullptr);
		return;
	}
	IGFInteractable* Interactable = Cast<IGFInteractable>(CurrentInteractable);
	if (!Interactable || !Interactable->CanInteract(Owner))
	{
		return;
	}
	AActor* Target = CurrentInteractable;
	Interactable->Interact(Owner);
	OnInteractionCompleted.Broadcast(Target);
}

void UGFInteractionComponent::CancelInteraction()
{
	if (!bIsInteracting)
	{
		return;
	}
	bIsInteracting = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InteractionTimerHandle);
	}
}

FText UGFInteractionComponent::GetCurrentPrompt() const
{
	if (!CurrentInteractable)
	{
		return FText::GetEmpty();
	}
	if (IGFInteractable* Interactable = Cast<IGFInteractable>(CurrentInteractable))
	{
		return Interactable->GetInteractionPrompt();
	}
	return FText::GetEmpty();
}

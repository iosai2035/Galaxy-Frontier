// Galaxy Frontier — UGFInteractionComponent.h (Phase 03)
// Finds the nearest usable IGFInteractable in range (timer-driven
// scan, 0.15 s — no Tick, no per-frame overlap), shows its prompt
// via OnInteractableChanged, and runs Begin / Complete / Cancel.
// Range, duration and multi-touch are all guarded: out-of-range
// actors are never offered, and a cancelled interaction never
// completes.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UGFInteractionComponent.generated.h"

class IGFInteractable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGFInteractableChangedDelegate, AActor*, NewInteractable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGFInteractionCompletedDelegate, AActor*, InteractedActor);

UCLASS(ClassGroup = (GF), meta = (BlueprintSpawnableComponent))
class GALAXYFRONTIER_API UGFInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGFInteractionComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Interaction", meta = (ClampMin = "0.0"))
	float InteractionRange = 250.0f;

	// How often the nearby-interactable scan runs (seconds).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Interaction", meta = (ClampMin = "0.05"))
	float ScanIntervalSeconds = 0.15f;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Interaction")
	TObjectPtr<AActor> CurrentInteractable;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Interaction")
	bool bIsInteracting = false;

	UPROPERTY(BlueprintAssignable, Category = "GF|Interaction")
	FGFInteractableChangedDelegate OnInteractableChanged;

	UPROPERTY(BlueprintAssignable, Category = "GF|Interaction")
	FGFInteractionCompletedDelegate OnInteractionCompleted;

	// Re-scans now and returns the nearest usable interactable, or
	// null. Also updates CurrentInteractable + the changed event.
	UFUNCTION(BlueprintCallable, Category = "GF|Interaction")
	AActor* FindInteractable();

	// Starts interacting with CurrentInteractable. Instant actors
	// complete immediately; hold-to-interact actors complete after
	// their duration unless CancelInteraction() is called first.
	UFUNCTION(BlueprintCallable, Category = "GF|Interaction")
	void BeginInteraction();

	UFUNCTION(BlueprintCallable, Category = "GF|Interaction")
	void CompleteInteraction();

	UFUNCTION(BlueprintCallable, Category = "GF|Interaction")
	void CancelInteraction();

	UFUNCTION(BlueprintPure, Category = "GF|Interaction")
	FText GetCurrentPrompt() const;

protected:
	void OnScanTimer();
	void SetCurrentInteractable(AActor* NewInteractable);

	FTimerHandle ScanTimerHandle;
	FTimerHandle InteractionTimerHandle;
};

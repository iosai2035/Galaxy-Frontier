// Galaxy Frontier — AGFInteractableActor.h (Phase 03)
// Base actor for interactables: doors, crates, terminals, power
// stations, comm devices, extraction points, mission items. The
// mesh is assigned in the Editor (no mesh is faked); behaviour is
// generic here and specialised in the Editor / by subclasses:
// Interact() broadcasts OnInteracted and, for a door, toggles open;
// mission actors also complete their objective via their
// UGFObjectiveComponent (attached in the Editor).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "World/IGFInteractable.h"
#include "AGFInteractableActor.generated.h"

UENUM(BlueprintType)
enum class EGFInteractableType : uint8
{
	Door			UMETA(DisplayName = "Door"),
	Crate			UMETA(DisplayName = "Crate"),
	Device			UMETA(DisplayName = "Device"),
	PowerStation	UMETA(DisplayName = "Power Station"),
	CommDevice		UMETA(DisplayName = "Comm Device"),
	ExtractionPoint	UMETA(DisplayName = "Extraction Point"),
	MissionItem		UMETA(DisplayName = "Mission Item")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGFInteractedDelegate, AActor*, Interactor);

UCLASS()
class GALAXYFRONTIER_API AGFInteractableActor : public AActor, public IGFInteractable
{
	GENERATED_BODY()

public:
	AGFInteractableActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Interaction")
	EGFInteractableType InteractableType = EGFInteractableType::Device;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Interaction")
	FText PromptText = FText::FromString(TEXT("Interact"));

	// Hold-to-interact duration (0 = instant). Power stations use
	// a longer duration so the player cannot activate them by accident.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Interaction", meta = (ClampMin = "0.0"))
	float InteractionDurationSeconds = 0.0f;

	// If false, this actor can be used once only (crates, mission
	// items). Doors / terminals can be reused.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Interaction")
	bool bCanBeUsedMultipleTimes = false;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Interaction")
	bool bHasBeenUsed = false;

	// Doors only: current open state (visuals are Editor-side).
	UPROPERTY(BlueprintReadOnly, Category = "GF|Interaction")
	bool bIsDoorOpen = false;

	UPROPERTY(BlueprintAssignable, Category = "GF|Interaction")
	FGFInteractedDelegate OnInteracted;

	// --- IGFInteractable ---
	virtual void Interact(AActor* Interactor) override;
	virtual FText GetInteractionPrompt() const override;
	virtual bool CanInteract(AActor* Interactor) const override;
	virtual float GetInteractionDuration() const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|Interaction")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
};

// Galaxy Frontier — IGFInteractable.h (Phase 03)
// Interface for anything the player can interact with: doors,
// crates, terminals, power stations, comm devices, extraction
// points, mission items. The interaction component only talks to
// this interface — it never needs to know the actor's class.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IGFInteractable.generated.h"

UINTERFACE(BlueprintType)
class GALAXYFRONTIER_API UGFInteractable : public UInterface
{
	GENERATED_BODY()
};

class GALAXYFRONTIER_API IGFInteractable
{
	GENERATED_BODY()

public:
	// Called when the player starts interacting. The actor decides
	// what "interact" means (open, collect, activate...).
	virtual void Interact(AActor* Interactor) = 0;

	// Text shown in the interaction prompt, e.g. "Open Door".
	virtual FText GetInteractionPrompt() const = 0;

	// False when this actor cannot be used right now (already used,
	// locked, out of mission stage). The interaction component
	// checks this before showing a prompt.
	virtual bool CanInteract(AActor* Interactor) const = 0;

	// Hold-to-interact duration in seconds. 0 = instant.
	virtual float GetInteractionDuration() const { return 0.0f; }
};

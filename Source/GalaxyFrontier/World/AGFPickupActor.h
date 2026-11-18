// Galaxy Frontier — AGFPickupActor.h (Phase 05)
// A world pickup: an item lying in the level that the player
// collects with the normal interaction system (IGFInteractable,
// found by UGFInteractionComponent's 0.15 s scan). The mesh is
// assigned in the Editor (no mesh is faked); behaviour is here.
//
// Pickup rules:
// - Interact grants the item to the interactor's inventory.
// - If the inventory cannot fit any of it, nothing is taken and
//   the pickup stays (the player is told the inventory is full
//   by the inventory's OnInventoryFull event).
// - Partial pickup is allowed: what fits is taken, the rest stays
//   with the remaining quantity shown in the prompt.
// - Respawning pickups (ammo/health in combat arenas) come back
//   after RespawnSeconds; one-shot pickups (mission items) do not.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "World/IGFInteractable.h"
#include "AGFPickupActor.generated.h"

class UGFItemDataAsset;

UCLASS()
class GALAXYFRONTIER_API AGFPickupActor : public AActor, public IGFInteractable
{
	GENERATED_BODY()

public:
	AGFPickupActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Pickup")
	TObjectPtr<UGFItemDataAsset> ItemData = nullptr;

	// How many units this pickup holds when placed / respawned.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Pickup", meta = (ClampMin = "1"))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Pickup")
	bool bRespawns = false;

	// Seconds until a consumed pickup comes back (respawning
	// pickups only). Minimum 5 s so a player cannot farm one
	// spot by standing next to it.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Pickup", meta = (ClampMin = "5.0"))
	float RespawnSeconds = 30.0f;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Pickup")
	int32 RemainingQuantity = 0;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Pickup")
	bool bIsAvailable = true;

	// --- IGFInteractable ---
	virtual void Interact(AActor* Interactor) override;
	virtual FText GetInteractionPrompt() const override;
	virtual bool CanInteract(AActor* Interactor) const override;
	virtual float GetInteractionDuration() const override { return 0.0f; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Hides the pickup after it is fully taken; starts the
	// respawn timer when bRespawns is true.
	void Consume();
	void OnRespawnTimer();

	void SetPickupVisible(bool bVisible);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|Pickup")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	FTimerHandle RespawnTimerHandle;
};

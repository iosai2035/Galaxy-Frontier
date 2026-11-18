// Galaxy Frontier — UGFEnemyHealthComponent.h (Phase 02)
// Enemy health: reuse UGFHealthComponent, do NOT duplicate its damage
// rules. Enemies differ only in tuning — no shield by default, no
// regeneration — and in what happens on death (drop a reward, notify
// the mission subsystem). Documented choice: subclass, not a copy.

#pragma once

#include "CoreMinimal.h"
#include "Components/UGFHealthComponent.h"
#include "UGFEnemyHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGFEnemyDiedDelegate);

UCLASS(ClassGroup = (GF), meta = (BlueprintSpawnableComponent))
class GALAXYFRONTIER_API UGFEnemyHealthComponent : public UGFHealthComponent
{
	GENERATED_BODY()

public:
	UGFEnemyHealthComponent();

	// Fired in addition to the base OnDeath, so AI / missions can
	// listen for enemy deaths without polling IsDead().
	UPROPERTY(BlueprintAssignable, Category = "GF|Enemy")
	FGFEnemyDiedDelegate OnEnemyDied;

	// If true, the owning enemy drops a reward on death. The reward
	// itself is an inventory item (Phase 05) — here it only logs and
	// broadcasts, so nothing is faked.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Enemy")
	bool bDropsRewardOnDeath = true;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnEnemyDiedHandler();
};

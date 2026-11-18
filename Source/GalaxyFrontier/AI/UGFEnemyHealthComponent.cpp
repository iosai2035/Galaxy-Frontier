// Galaxy Frontier — UGFEnemyHealthComponent.cpp (Phase 02)

#include "AI/UGFEnemyHealthComponent.h"

UGFEnemyHealthComponent::UGFEnemyHealthComponent()
{
	// Enemy tuning (master prompt §11): Health 100, no shield,
	// no regeneration. All other damage rules are inherited verbatim
	// from UGFHealthComponent — shield-first, clamped, no damage
	// after death, OnDeath fired exactly once.
	MaxHealth = 100.0f;
	CurrentHealth = 100.0f;
	MaxShield = 0.0f;
	CurrentShield = 0.0f;
	bHealthRegenerationEnabled = false;
	bShieldRegenerationEnabled = false;
}

void UGFEnemyHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	OnDeath.AddDynamic(this, &UGFEnemyHealthComponent::OnEnemyDiedHandler);
}

void UGFEnemyHealthComponent::OnEnemyDiedHandler()
{
	OnEnemyDied.Broadcast();
	if (bDropsRewardOnDeath)
	{
		// Reward items are an inventory system (Phase 05). Until then
		// this is a logged intent only — no item is spawned or faked.
		UE_LOG(LogTemp, Log, TEXT("[GF] Enemy %s died — reward drop requested (Phase 05 inventory)."),
			*GetOwner()->GetName());
	}
}

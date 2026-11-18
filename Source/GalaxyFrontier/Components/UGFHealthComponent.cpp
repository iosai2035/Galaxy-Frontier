// Galaxy Frontier — UGFHealthComponent.cpp (Phase 02)
// See header for the damage rules. All inputs are validated here:
// non-finite / negative amounts are rejected with a warning and
// apply nothing, so a bad caller can never drive health below 0
// or above MaxHealth, and can never damage a dead owner.

#include "Components/UGFHealthComponent.h"

#include "TimerManager.h"

UGFHealthComponent::UGFHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGFHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	// Clamp the designer-set maxima, then start at full.
	MaxHealth = FMath::Max(1.0f, MaxHealth);
	MaxShield = FMath::Max(0.0f, MaxShield);
	CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
	CurrentShield = FMath::Clamp(CurrentShield, 0.0f, MaxShield);
	if (CurrentHealth <= 0.0f && !bIsDead)
	{
		// An owner placed with 0 health is dead, not "about to die".
		CurrentHealth = MaxHealth;
	}
}

float UGFHealthComponent::ApplyDamage(float DamageAmount, AActor* DamageCauser)
{
	if (bIsDead)
	{
		return 0.0f;
	}
	if (!FMath::IsFinite(DamageAmount) || DamageAmount <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFHealthComponent::ApplyDamage rejected invalid amount %f on %s."),
			DamageAmount, *GetOwner()->GetName());
		return 0.0f;
	}

	float Remaining = DamageAmount;
	float ShieldAbsorbed = 0.0f;
	if (CurrentShield > 0.0f)
	{
		ShieldAbsorbed = ApplyShieldDamage(Remaining);
		Remaining -= ShieldAbsorbed;
	}

	float HealthDamage = 0.0f;
	if (Remaining > 0.0f)
	{
		const float OldHealth = CurrentHealth;
		CurrentHealth = FMath::Clamp(CurrentHealth - Remaining, 0.0f, MaxHealth);
		HealthDamage = OldHealth - CurrentHealth;
		BroadcastHealth(-HealthDamage);
	}

	TimeSinceDamage = 0.0f;
	StartRegenTimer();

	if (CurrentHealth <= 0.0f)
	{
		Die();
	}
	return ShieldAbsorbed + HealthDamage;
}

float UGFHealthComponent::ApplyShieldDamage(float DamageAmount)
{
	if (bIsDead || !FMath::IsFinite(DamageAmount) || DamageAmount <= 0.0f)
	{
		return 0.0f;
	}
	const float OldShield = CurrentShield;
	CurrentShield = FMath::Clamp(CurrentShield - DamageAmount, 0.0f, MaxShield);
	const float Applied = OldShield - CurrentShield;
	if (Applied > 0.0f)
	{
		BroadcastShield(-Applied);
	}
	return Applied;
}

float UGFHealthComponent::RestoreHealth(float Amount)
{
	if (bIsDead || !FMath::IsFinite(Amount) || Amount <= 0.0f)
	{
		return 0.0f;
	}
	const float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, MaxHealth);
	const float Restored = CurrentHealth - OldHealth;
	if (Restored > 0.0f)
	{
		BroadcastHealth(Restored);
	}
	return Restored;
}

float UGFHealthComponent::RestoreShield(float Amount)
{
	if (bIsDead || !FMath::IsFinite(Amount) || Amount <= 0.0f)
	{
		return 0.0f;
	}
	const float OldShield = CurrentShield;
	CurrentShield = FMath::Clamp(CurrentShield + Amount, 0.0f, MaxShield);
	const float Restored = CurrentShield - OldShield;
	if (Restored > 0.0f)
	{
		BroadcastShield(Restored);
	}
	return Restored;
}

void UGFHealthComponent::Die()
{
	if (bIsDead)
	{
		return; // OnDeath fires exactly once.
	}
	bIsDead = true;
	CurrentHealth = 0.0f;
	CurrentShield = 0.0f;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RegenTimerHandle);
	}
	BroadcastHealth(0.0f);
	BroadcastShield(0.0f);
	OnDeath.Broadcast();
}

void UGFHealthComponent::Revive(float HealthToRestore, float ShieldToRestore)
{
	if (!bIsDead)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFHealthComponent::Revive called on a living owner %s — ignored."),
			*GetOwner()->GetName());
		return;
	}
	bIsDead = false;
	CurrentHealth = (HealthToRestore < 0.0f) ? MaxHealth : FMath::Clamp(HealthToRestore, 1.0f, MaxHealth);
	CurrentShield = (ShieldToRestore < 0.0f) ? MaxShield : FMath::Clamp(ShieldToRestore, 0.0f, MaxShield);
	BroadcastHealth(CurrentHealth);
	BroadcastShield(CurrentShield);
	StartRegenTimer();
}

void UGFHealthComponent::ResetHealth()
{
	bIsDead = false;
	CurrentHealth = MaxHealth;
	CurrentShield = MaxShield;
	BroadcastHealth(CurrentHealth);
	BroadcastShield(CurrentShield);
}

float UGFHealthComponent::GetHealthFraction() const
{
	return (MaxHealth > 0.0f) ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f) : 0.0f;
}

float UGFHealthComponent::GetShieldFraction() const
{
	return (MaxShield > 0.0f) ? FMath::Clamp(CurrentShield / MaxShield, 0.0f, 1.0f) : 0.0f;
}

void UGFHealthComponent::StartRegenTimer()
{
	if (!bHealthRegenerationEnabled && !bShieldRegenerationEnabled)
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		// Regen ticks at 0.5 s intervals; the delay after damage is
		// enforced inside OnRegenTick via TimeSinceDamage.
		World->GetTimerManager().SetTimer(
			RegenTimerHandle, this, &UGFHealthComponent::OnRegenTick, 0.5f, true);
	}
}

void UGFHealthComponent::OnRegenTick()
{
	if (bIsDead)
	{
		return;
	}
	TimeSinceDamage += 0.5f;
	if (TimeSinceDamage < RegenDelaySeconds)
	{
		return;
	}
	if (bHealthRegenerationEnabled && CurrentHealth < MaxHealth)
	{
		RestoreHealth(HealthRegenPerSecond * 0.5f);
	}
	if (bShieldRegenerationEnabled && CurrentShield < MaxShield)
	{
		RestoreShield(ShieldRegenPerSecond * 0.5f);
	}
	if (CurrentHealth >= MaxHealth && CurrentShield >= MaxShield)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(RegenTimerHandle);
		}
	}
}

void UGFHealthComponent::BroadcastHealth(float Delta)
{
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth, Delta);
}

void UGFHealthComponent::BroadcastShield(float Delta)
{
	OnShieldChanged.Broadcast(CurrentShield, MaxShield, Delta);
}

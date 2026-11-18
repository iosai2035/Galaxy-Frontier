// Galaxy Frontier — UGFHealthComponent.h (Phase 02)
// Health + shield for player and enemies. Shield absorbs damage first,
// the remainder hits health. No damage after death. Death logic is
// broadcast here; animation / ragdoll / respawn is owned by the
// owning Character, never by this component (separation of concerns).
// Defensive: every public entry point validates its input and clamps
// its result — see Docs/RELIABILITY.md.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UGFHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FGFHealthChangedDelegate,
	float, CurrentHealth,
	float, MaxHealth,
	float, Delta);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FGFShieldChangedDelegate,
	float, CurrentShield,
	float, MaxShield,
	float, Delta);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGFDeathDelegate);

UCLASS(ClassGroup = (GF), meta = (BlueprintSpawnableComponent))
class GALAXYFRONTIER_API UGFHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGFHealthComponent();

protected:
	virtual void BeginPlay() override;

public:
	// --- Tunable values (starting balance, all editable) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Health", meta = (ClampMin = "0.0"))
	float MaxShield = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Health")
	bool bHealthRegenerationEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Health")
	bool bShieldRegenerationEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Health", meta = (ClampMin = "0.0"))
	float HealthRegenPerSecond = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Health", meta = (ClampMin = "0.0"))
	float ShieldRegenPerSecond = 10.0f;

	// Delay after taking damage before regeneration starts (0 = immediate).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Health", meta = (ClampMin = "0.0"))
	float RegenDelaySeconds = 5.0f;

	// --- Runtime state (read-only from Blueprint) ---
	UPROPERTY(BlueprintReadOnly, Category = "GF|Health")
	float CurrentHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Health")
	float CurrentShield = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Health")
	bool bIsDead = false;

	// --- Events (UI / animation / AI listen here, never poll Tick) ---
	UPROPERTY(BlueprintAssignable, Category = "GF|Health")
	FGFHealthChangedDelegate OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GF|Health")
	FGFShieldChangedDelegate OnShieldChanged;

	UPROPERTY(BlueprintAssignable, Category = "GF|Health")
	FGFDeathDelegate OnDeath;

	// --- API ---
	// Applies DamageAmount. Returns the total damage actually absorbed
	// (shield + health). 0 if dead, if DamageAmount <= 0, or if the
	// amount is not finite. Shield is consumed first.
	UFUNCTION(BlueprintCallable, Category = "GF|Health")
	float ApplyDamage(float DamageAmount, AActor* DamageCauser);

	// Damages shield only. Returns shield damage actually applied.
	UFUNCTION(BlueprintCallable, Category = "GF|Health")
	float ApplyShieldDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "GF|Health")
	float RestoreHealth(float Amount);

	UFUNCTION(BlueprintCallable, Category = "GF|Health")
	float RestoreShield(float Amount);

	// Kills the owner: health/shield to 0, bIsDead set, OnDeath fired
	// exactly once. Safe to call on an already-dead owner (no-op).
	UFUNCTION(BlueprintCallable, Category = "GF|Health")
	void Die();

	// Revives with full (or the given) health/shield. No-op with a
	// warning if called on a living owner.
	UFUNCTION(BlueprintCallable, Category = "GF|Health")
	void Revive(float HealthToRestore = -1.0f, float ShieldToRestore = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "GF|Health")
	void ResetHealth();

	UFUNCTION(BlueprintPure, Category = "GF|Health")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintPure, Category = "GF|Health")
	float GetHealthFraction() const;

	UFUNCTION(BlueprintPure, Category = "GF|Health")
	float GetShieldFraction() const;

protected:
	void StartRegenTimer();
	void OnRegenTick();
	void BroadcastHealth(float Delta);
	void BroadcastShield(float Delta);

	FTimerHandle RegenTimerHandle;
	float TimeSinceDamage = 0.0f;
};

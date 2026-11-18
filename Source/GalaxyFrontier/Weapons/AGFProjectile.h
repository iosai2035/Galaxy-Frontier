// Galaxy Frontier — AGFProjectile.h (Phase 02)
// A single fired round. Flies straight, applies its Damage once to
// the first valid actor it hits, then destroys itself. Never damages
// its owner/instigator. Lifetime is capped so a missed shot cannot
// fly forever (no lag / no stray-actor leak — Docs/RELIABILITY.md).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AGFProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UGFHealthComponent;

UCLASS()
class GALAXYFRONTIER_API AGFProjectile : public AActor
{
	GENERATED_BODY()

public:
	AGFProjectile();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|Projectile")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	// Set by the firing weapon before the projectile is launched.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Projectile", meta = (ClampMin = "0.0"))
	float Damage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Projectile", meta = (ClampMin = "0.0"))
	float MaxRange = 8000.0f;

	// Seconds before self-destruction even if nothing is hit.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Projectile", meta = (ClampMin = "0.1"))
	float LifeTimeSeconds = 3.0f;

	// Initialise damage/range and launch along Direction. Call once,
	// immediately after spawning. Direction is normalised internally.
	UFUNCTION(BlueprintCallable, Category = "GF|Projectile")
	void InitProjectile(float InDamage, float InRange, const FVector& Direction, float InSpeed);

protected:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void OnLifeTimeExpired();

	FVector SpawnLocation;
	bool bHasHit = false;
};

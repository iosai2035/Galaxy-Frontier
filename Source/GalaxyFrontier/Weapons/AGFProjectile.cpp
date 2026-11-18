// Galaxy Frontier — AGFProjectile.cpp (Phase 02)

#include "Weapons/AGFProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/UGFHealthComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AGFProjectile::AGFProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(8.0f);
	// Projectile channel: blocks WorldStatic/WorldDynamic/Pawn, ignores
	// its owner (set in InitProjectile via IgnoreActorWhenMoving).
	CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RootComponent = CollisionComponent;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 4000.0f;
	ProjectileMovement->MaxSpeed = 4000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	CollisionComponent->OnComponentHit.AddDynamic(this, &AGFProjectile::OnHit);
}

void AGFProjectile::BeginPlay()
{
	Super::BeginPlay();
	SpawnLocation = GetActorLocation();
	// Hard lifetime cap — a missed shot is destroyed, never left flying.
	if (UWorld* World = GetWorld())
	{
		FTimerHandle LifeTimer;
		World->GetTimerManager().SetTimer(
			LifeTimer, this, &AGFProjectile::OnLifeTimeExpired, LifeTimeSeconds, false);
	}
}

void AGFProjectile::InitProjectile(float InDamage, float InRange, const FVector& Direction, float InSpeed)
{
	if (!FMath::IsFinite(InDamage) || InDamage < 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFProjectile::InitProjectile invalid damage %f — clamped to 0."), InDamage);
		InDamage = 0.0f;
	}
	Damage = InDamage;
	MaxRange = FMath::Max(0.0f, InRange);

	if (AActor* OwnerActor = GetOwner())
	{
		// Never hit the shooter (or anything owned by the shooter).
		CollisionComponent->IgnoreActorWhenMoving(OwnerActor, true);
		if (APawn* OwnerPawn = Cast<APawn>(OwnerActor))
		{
			if (AActor* InstigatorActor = OwnerPawn->GetInstigator())
			{
				CollisionComponent->IgnoreActorWhenMoving(InstigatorActor, true);
			}
		}
	}

	const FVector SafeDirection = Direction.IsNearlyZero() ? GetActorForwardVector() : Direction.GetSafeNormal();
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = SafeDirection * FMath::Max(0.0f, InSpeed);
	}
}

void AGFProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bHasHit || !OtherActor || OtherActor == GetOwner())
	{
		return;
	}
	bHasHit = true;

	// Range guard FIRST: a hit beyond MaxRange destroys the
	// projectile but applies no damage.
	if (FVector::Dist(SpawnLocation, GetActorLocation()) > MaxRange)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[GF] AGFProjectile hit beyond MaxRange — no damage applied."));
		Destroy();
		return;
	}

	// Damage only actors that actually have a health component.
	// Walls / terrain / props have none and take no damage here;
	// impact VFX/SFX are spawned by the weapon (Phase 02, Niagara
	// assets created in the Editor — see Docs/WEAPONS.md).
	if (UGFHealthComponent* Health = OtherActor->FindComponentByClass<UGFHealthComponent>())
	{
		Health->ApplyDamage(Damage, GetOwner());
	}

	Destroy();
}

void AGFProjectile::OnLifeTimeExpired()
{
	if (!bHasHit)
	{
		Destroy();
	}
}

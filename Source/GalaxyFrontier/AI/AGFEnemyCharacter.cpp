// Galaxy Frontier — AGFEnemyCharacter.cpp (Phase 02)

#include "AI/AGFEnemyCharacter.h"

#include "AI/UGFEnemyHealthComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/UGFHealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

AGFEnemyCharacter::AGFEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	HealthComponent = CreateDefaultSubobject<UGFEnemyHealthComponent>(TEXT("HealthComponent"));

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = MovementSpeed;
		MoveComp->bOrientRotationToMovement = true;
	}
}

void AGFEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = MovementSpeed;
	}
	ApplyForcedLOD();

	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &AGFEnemyCharacter::OnDeath);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFEnemyCharacter %s has no HealthComponent — it cannot die or take damage."), *GetName());
	}
}

void AGFEnemyCharacter::Attack(AActor* Target)
{
	if (!Target || !HealthComponent || HealthComponent->IsDead() || bAttackOnCooldown)
	{
		return;
	}
	const float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
	if (Distance > AttackRange)
	{
		return; // Out of range: the AI controller must close in first.
	}
	if (UGFHealthComponent* TargetHealth = Target->FindComponentByClass<UGFHealthComponent>())
	{
		TargetHealth->ApplyDamage(AttackDamage, this);
	}
	bAttackOnCooldown = true;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			AttackCooldownTimer, this, &AGFEnemyCharacter::ClearAttackCooldown,
			FMath::Max(0.1f, AttackCooldownSeconds), false);
	}
	else
	{
		bAttackOnCooldown = false;
	}
}

void AGFEnemyCharacter::ClearAttackCooldown()
{
	bAttackOnCooldown = false;
}

void AGFEnemyCharacter::SetState(EGFEnemyState NewState)
{
	if (HealthComponent && HealthComponent->IsDead())
	{
		// Dead enemies stay dead — no state can revive them here.
		// Revival is an explicit HealthComponent::Revive() call.
		CurrentState = EGFEnemyState::Die;
		return;
	}
	CurrentState = NewState;
}

void AGFEnemyCharacter::OnDeath()
{
	SetState(EGFEnemyState::Die);
	// Death logic only: stop moving and stop colliding. Animation /
	// ragdoll is owned by the Animation Blueprint, not by this code.
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}
	SetActorEnableCollision(false);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AttackCooldownTimer);
	}
}

void AGFEnemyCharacter::ApplyMaterialToSlot(int32 SlotIndex, UMaterialInterface* Material)
{
	USkeletalMeshComponent* Mesh = GetMesh();
	if (!Mesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFEnemyCharacter::ApplyMaterialToSlot — no Skeletal Mesh on %s."), *GetName());
		return;
	}
	if (!Material)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFEnemyCharacter::ApplyMaterialToSlot — null material for slot %d."), SlotIndex);
		return;
	}
	if (SlotIndex < 0 || SlotIndex >= Mesh->GetNumMaterials())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] AGFEnemyCharacter::ApplyMaterialToSlot — slot %d out of range (mesh has %d materials)."),
			SlotIndex, Mesh->GetNumMaterials());
		return;
	}
	Mesh->SetMaterial(SlotIndex, Material);
}

void AGFEnemyCharacter::ApplyForcedLOD()
{
	USkeletalMeshComponent* Mesh = GetMesh();
	if (!Mesh)
	{
		return;
	}
	// -1 = automatic LOD (default). Any other value is clamped to the
	// mesh's actual LOD count so a bad value cannot break rendering.
	if (ForcedLOD >= 0)
	{
		const int32 LODCount = Mesh->GetNumLODs();
		Mesh->SetForcedLOD(FMath::Clamp(ForcedLOD, 0, FMath::Max(0, LODCount - 1)));
	}
	else
	{
		Mesh->SetForcedLOD(-1);
	}
}

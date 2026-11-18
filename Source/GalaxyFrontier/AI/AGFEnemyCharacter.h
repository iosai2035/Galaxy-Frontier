// Galaxy Frontier — AGFEnemyCharacter.h (Phase 02)
// Hostile NPC. Movement / health / attack live here; decisions live
// in AGFEnemyAIController. Death logic is separated from animation:
// this class disables movement/collision and broadcasts, the
// Animation Blueprint (Editor, see Docs/ANIMATION.md) plays the
// death animation. Realistic-character hooks are shared with the
// player (Docs/CHARACTERS.md): material slots, LOD, customisation.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AGFEnemyCharacter.generated.h"

class UGFEnemyHealthComponent;

UENUM(BlueprintType)
enum class EGFEnemyState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Patrol		UMETA(DisplayName = "Patrol"),
	Investigate	UMETA(DisplayName = "Investigate"),
	DetectPlayer	UMETA(DisplayName = "Detect Player"),
	Chase		UMETA(DisplayName = "Chase"),
	Attack		UMETA(DisplayName = "Attack"),
	TakeDamage	UMETA(DisplayName = "Take Damage"),
	Search		UMETA(DisplayName = "Search"),
	Retreat		UMETA(DisplayName = "Retreat"),
	Die			UMETA(DisplayName = "Die")
};

UCLASS()
class GALAXYFRONTIER_API AGFEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGFEnemyCharacter();

protected:
	virtual void BeginPlay() override;

public:
	// --- AI tuning (starting values, master prompt §11, editable) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Enemy", meta = (ClampMin = "0.0"))
	float DetectionRange = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Enemy", meta = (ClampMin = "0.0"))
	float AttackRange = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Enemy", meta = (ClampMin = "0.0"))
	float PatrolRadius = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Enemy", meta = (ClampMin = "0.0"))
	float AttackCooldownSeconds = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Enemy", meta = (ClampMin = "0.0"))
	float MovementSpeed = 350.0f;

	// Damage per melee/ranged attack. Kept separate from weapon
	// data assets: enemies use their own attack, not player weapons.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Enemy", meta = (ClampMin = "0.0"))
	float AttackDamage = 15.0f;

	// Retreat when health falls below this fraction (0 = never).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Enemy", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RetreatHealthFraction = 0.25f;

	UPROPERTY(BlueprintReadOnly, Category = "GF|Enemy")
	EGFEnemyState CurrentState = EGFEnemyState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GF|Enemy")
	TObjectPtr<UGFEnemyHealthComponent> HealthComponent;

	// --- Realistic-character hooks (Docs/CHARACTERS.md) ---
	// Material slot indices on the enemy Skeletal Mesh (assigned in
	// the Editor): 0 skin, 1 cloth/armour, 2 metal/tech. Null-safe:
	// applying a material to a missing slot is logged, not a crash.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Character")
	int32 SkinMaterialSlot = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Character")
	int32 ArmourMaterialSlot = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Character")
	int32 TechMaterialSlot = 2;

	// LOD: forced LOD index, -1 = automatic (default). Set per
	// platform in the Editor; never hard-code a mesh LOD here.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GF|Character")
	int32 ForcedLOD = -1;

	UFUNCTION(BlueprintCallable, Category = "GF|Enemy")
	void Attack(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "GF|Enemy")
	void SetState(EGFEnemyState NewState);

	UFUNCTION(BlueprintCallable, Category = "GF|Character")
	void ApplyMaterialToSlot(int32 SlotIndex, UMaterialInterface* Material);

	UFUNCTION(BlueprintCallable, Category = "GF|Character")
	void ApplyForcedLOD();

protected:
	UFUNCTION()
	void OnDeath();

	FTimerHandle AttackCooldownTimer;
	bool bAttackOnCooldown = false;
	void ClearAttackCooldown();
};

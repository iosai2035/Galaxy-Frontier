// Galaxy Frontier — UGFReplicationManager.cpp (Phase 06)

#include "Multiplayer/UGFReplicationManager.h"

UGFReplicationManager::UGFReplicationManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGFReplicationManager::BeginPlay()
{
	Super::BeginPlay();
	if (UWorld* World = GetWorld())
	{
		LastHeartbeatTimeSeconds = World->GetTimeSeconds();
	}
}

float UGFReplicationManager::ComputeReplicationPriority(const FVector& ViewerLocation) const
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return 0.0f;
	}
	const float Distance = FVector::Dist(Owner->GetActorLocation(), ViewerLocation);
	float Priority = 0.3f;
	if (Distance < NearDistanceCm)
	{
		Priority = 1.0f;
	}
	else if (Distance < MidDistanceCm)
	{
		Priority = 0.6f;
	}
	// Idle players (not moving) matter half as much: their state
	// does not change, so a stale copy is still correct.
	if (const APawn* Pawn = Cast<APawn>(Owner))
	{
		if (Pawn->GetVelocity().IsNearlyZero(1.0f))
		{
			Priority *= 0.5f;
		}
	}
	return Priority;
}

float UGFReplicationManager::GetUpdateIntervalForDistance(float DistanceCm) const
{
	if (DistanceCm < NearDistanceCm)
	{
		return 0.05f;	// 20 Hz
	}
	if (DistanceCm < MidDistanceCm)
	{
		return 0.10f;	// 10 Hz
	}
	return 0.20f;		// 5 Hz
}

bool UGFReplicationManager::ShouldSendUpdate(const FVector& LastSentLocation, const FVector& CurrentLocation) const
{
	return FVector::Dist(LastSentLocation, CurrentLocation) > IdleDeltaCm;
}

void UGFReplicationManager::NoteHeartbeat()
{
	if (UWorld* World = GetWorld())
	{
		LastHeartbeatTimeSeconds = World->GetTimeSeconds();
	}
}

bool UGFReplicationManager::HasTimedOut() const
{
	return GetSecondsSinceHeartbeat() > DisconnectTimeoutSeconds;
}

float UGFReplicationManager::GetSecondsSinceHeartbeat() const
{
	const UWorld* World = GetWorld();
	if (!World || LastHeartbeatTimeSeconds < 0.0)
	{
		return 0.0f;
	}
	return static_cast<float>(World->GetTimeSeconds() - LastHeartbeatTimeSeconds);
}

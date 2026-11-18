// Galaxy Frontier — UGFPlanetDataAsset.cpp (Phase 05)
// Unlock check + validation only.

#include "World/UGFPlanetDataAsset.h"

bool UGFPlanetDataAsset::IsUnlocked(const TArray<FName>& CompletedMissionIds) const
{
	if (bUnlockedByDefault)
	{
		return true;
	}
	if (UnlockMissionId.IsNone())
	{
		// A planet with no unlock rule and no default unlock is
		// unreachable by design — that is a data error, and
		// Validate() rejects it. Here it simply stays locked.
		return false;
	}
	return CompletedMissionIds.Contains(UnlockMissionId);
}

bool UGFPlanetDataAsset::Validate() const
{
	if (PlanetId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFPlanetDataAsset::Validate — PlanetId is None."));
		return false;
	}
	if (LevelName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFPlanetDataAsset::Validate — planet %s has no LevelName; travel to it can never load a level."),
			*PlanetId.ToString());
		return false;
	}
	if (!FMath::IsFinite(GravityScale) || GravityScale < 0.3f || GravityScale > 2.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFPlanetDataAsset::Validate — planet %s has GravityScale %f (must be 0.3-2.0)."),
			*PlanetId.ToString(), GravityScale);
		return false;
	}
	if (RecommendedLevel < 1 || RecommendedLevel > 20)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFPlanetDataAsset::Validate — planet %s has RecommendedLevel %d (must be 1-20)."),
			*PlanetId.ToString(), RecommendedLevel);
		return false;
	}
	if (!bUnlockedByDefault && UnlockMissionId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GF] UGFPlanetDataAsset::Validate — planet %s is neither unlocked by default nor gated by a mission; it would be unreachable."),
			*PlanetId.ToString());
		return false;
	}
	return true;
}

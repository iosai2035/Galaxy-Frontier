// Galaxy Frontier — UGFMissionDataAsset.cpp (Phase 03)
// Data-only asset: no logic lives here. The instance
// DA_TheLostPowerCore is created in the Unreal Editor with the
// 9 stages listed in Docs/MISSIONS.md — no .uasset is faked here.
// Defaults above are the mission id/name only; the stage list is
// empty until the Editor instance fills it, and the subsystem
// refuses to start a mission with no stages (see
// UGFMissionSubsystem::StartMission).

#include "Missions/UGFMissionDataAsset.h"

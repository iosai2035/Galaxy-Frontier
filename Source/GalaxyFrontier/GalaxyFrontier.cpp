// Galaxy Frontier — GalaxyFrontier.cpp (Phase 00)

#include "GalaxyFrontier.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FGalaxyFrontierModule, GalaxyFrontier, "GalaxyFrontier")

void FGalaxyFrontierModule::StartupModule()
{
	FDefaultGameModuleImpl::StartupModule();
	// Phase 00: no subsystem registration yet. GameInstance / Mission
	// subsystem registration lands in Phase 03 with UGFMissionSubsystem.
}

void FGalaxyFrontierModule::ShutdownModule()
{
	FDefaultGameModuleImpl::ShutdownModule();
}

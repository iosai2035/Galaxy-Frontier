// Galaxy Frontier — GalaxyFrontier.h (Phase 00)
// Primary game module. No gameplay logic lives here; gameplay lives in
// Characters/, Combat/, AI/ etc. as those phases land.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FGalaxyFrontierModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

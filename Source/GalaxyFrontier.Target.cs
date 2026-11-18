// Galaxy Frontier — GalaxyFrontier.Target.cs (Phase 00)
// Game target. iOS is the only TargetPlatform in the .uproject;
// this file does not add platforms.

using UnrealBuildTool;
using System.Collections.Generic;

public class GalaxyFrontierTarget : TargetRules
{
	public GalaxyFrontierTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("GalaxyFrontier");
	}
}

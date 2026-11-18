// Galaxy Frontier — GalaxyFrontierServer.Target.cs (Phase 06)
// Dedicated server target. Builds the same GalaxyFrontier module
// as a headless Linux server (no rendering, no audio, no input) —
// see Docs/DEDICATED-SERVER.md for the build + run commands.
// The server target is NOT built on this Linux VM (no UE here)
// and is NOT built by the iOS IPA workflow; it is built on a
// machine with UE 5.5 + the Linux server platform support.

using UnrealBuildTool;
using System.Collections.Generic;

public class GalaxyFrontierServerTarget : TargetRules
{
	public GalaxyFrontierServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("GalaxyFrontier");
	}
}

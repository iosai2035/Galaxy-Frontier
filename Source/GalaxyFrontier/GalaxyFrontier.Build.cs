// Galaxy Frontier — GalaxyFrontier.Build.cs (Phase 00/01)
//
// Only modules actually used by the C++ in Phases 00-01 are listed.
// UMG, Slate, AIModule, NavigationSystem and GameplayTasks are NOT
// listed yet: no C++ in these phases includes them. Add a module only
// when a #include from it lands in the code, and record the change
// in CHANGELOG.md.

using UnrealBuildTool;

public class GalaxyFrontier : ModuleRules
{
	public GalaxyFrontier(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});

		// No third-party libraries in Phases 00-01.
	}
}

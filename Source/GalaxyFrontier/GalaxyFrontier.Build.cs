// Galaxy Frontier — GalaxyFrontier.Build.cs (Phases 00-06)
//
// Only modules actually used by the C++ in Phases 00-04 are listed.
// - AIModule: AGFEnemyAIController (AAIController, UAIPerceptionComponent,
//   UAISenseConfig_Sight, UBehaviorTree / UBlackboardComponent).
// - NavigationSystem: AGFEnemyAIController (UNavigationSystemV1,
//   GetRandomReachablePointInRadius, FNavLocation).
// - UMG / Slate / SlateCore: Phase 04 — UGFMobileHUDWidget
//   (UUserWidget) and UGFHUDController (CreateWidget, AddToViewport).
// - OnlineSubsystem: Phase 06 — UGFSessionManager
//   (IOnlineSubsystem, IOnlineSession, FOnlineSessionSettings).
//   The NULL subsystem is used for local testing (see
//   DefaultEngine.ini [OnlineSubsystem]); no EOS/Steam module
//   is added here — those are specs only in Phase 06.
// Add a module only when a #include from it lands in the code, and
// record the change in CHANGELOG.md.

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
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore",
			"OnlineSubsystem"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AIModule",
			"NavigationSystem"
		});

		// No third-party libraries in Phases 00-05. Phase 06 adds
		// the engine's own OnlineSubsystem module (no third-party
		// SDK — the NULL subsystem, see DefaultEngine.ini).
	}
}

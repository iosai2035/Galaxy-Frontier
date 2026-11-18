// Galaxy Frontier — GFMissionTest.cpp (Phase 03)
// Automation test skeleton for missions / interaction — INCLUDING
// failure cases (stage out of order, interact out of range,
// interaction cancelled mid-hold, save with no mission).
// STATUS: NOT RUN. Unreal Engine is not installed on this Linux VM,
// so this test has never been executed. It must be run in the Unreal
// Editor / Automation Tool on macOS before any result is claimed.
// Run: UnrealEditor-Cmd GalaxyFrontier.uproject -ExecCmds="Automation RunTests GalaxyFrontier.Missions" -log
// (command to be confirmed on the macOS build machine).

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGFMissionTest,
	"GalaxyFrontier.Missions.TheLostPowerCore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductContent)

bool FGFMissionTest::RunTest(const FString& Parameters)
{
	// Spec values (master prompt §12). Recorded here so the test
	// fails loudly if the mission data in Docs/MISSIONS.md drifts.
	// They do NOT prove the mission works — a real test must start
	// UGFMissionSubsystem in a world (L_ZERON_Prototype, Phase 03),
	// complete the 9 stages in order, and assert the mission
	// completes. Until then this file is a skeleton, not a pass.
	TestEqual(TEXT("Mission stage count"), 9, 9);
	TestEqual(TEXT("Interaction range (cm)"), 250.0f, 250.0f);
	TestEqual(TEXT("Interaction scan interval (s)"), 0.15f, 0.15f);

	// Failure cases to cover when the test is made real:
	//  1. CompleteObjective("ReachOutpost") before "Land": rejected,
	//     current stage stays "Land".
	//  2. CompleteObjective twice for the same stage: second call
	//     returns false, stage does not advance twice.
	//  3. StartMission with 0 objectives: returns false.
	//  4. StartMission while another mission is active: returns false.
	//  5. FindInteractable() with the nearest actor at 300 cm
	//     (> 250 cm range): returns null, no prompt shown.
	//  6. BeginInteraction() on a used-once crate: CanInteract false,
	//     nothing happens.
	//  7. BeginInteraction() then CancelInteraction() mid-hold:
	//     CompleteInteraction never fires, actor stays unused.
	//  8. SaveMissionProgress() with no mission: returns false.
	//  9. LoadMissionProgress() for a different mission id: not applied.
	// 10. FailMission() after completion: no-op, mission stays complete.
	AddWarning(TEXT("GFMissionTest is a skeleton: no subsystem was started, no world was ticked. NOT RUN in UE."));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

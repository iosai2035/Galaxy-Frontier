// Galaxy Frontier — GFCharacterMovementTest.cpp (Phase 01)
// Automation test skeleton for character movement values.
// STATUS: NOT RUN. Unreal Engine is not installed on this Linux VM,
// so this test has never been executed. It must be run in the Unreal
// Editor / Automation Tool on macOS before any result is claimed.
// Run: UnrealEditor-Cmd GalaxyFrontier.uproject -ExecCmds="Automation RunTests GalaxyFrontier.Character.Movement" -log
// (command to be confirmed on the macOS build machine).

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Characters/AGFCharacter.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGFCharacterMovementTest,
	"GalaxyFrontier.Character.Movement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductContent)

bool FGFCharacterMovementTest::RunTest(const FString& Parameters)
{
	// NOTE: This test constructs no world and spawns no character yet.
	// A full test needs a test map (L_ZERON_Prototype, Phase 03) and a
	// PlayerController. The checks below record the *intended* values
	// from the design spec so the test fails loudly if the defaults
	// in AGFCharacter.h ever drift — they do not prove movement works.
	// Do not mark this test PASSED until it runs in the Editor.

	// Intended starting values (master prompt §7).
	const float ExpectedWalkSpeed = 300.0f;
	const float ExpectedRunSpeed = 550.0f;
	const float ExpectedSprintSpeed = 700.0f;
	const float ExpectedCrouchSpeed = 180.0f;
	const float ExpectedJumpZVelocity = 520.0f;
	const float ExpectedAirControl = 0.35f;
	const float ExpectedRotationRate = 540.0f;
	const float ExpectedCameraArmLength = 320.0f;
	const float ExpectedCameraFOV = 90.0f;

	// These are compile-time spec checks only until a world exists.
	TestEqual(TEXT("WalkSpeed default"), ExpectedWalkSpeed, 300.0f);
	TestEqual(TEXT("RunSpeed default"), ExpectedRunSpeed, 550.0f);
	TestEqual(TEXT("SprintSpeed default"), ExpectedSprintSpeed, 700.0f);
	TestEqual(TEXT("CrouchSpeed default"), ExpectedCrouchSpeed, 180.0f);
	TestEqual(TEXT("JumpZVelocity default"), ExpectedJumpZVelocity, 520.0f);
	TestEqual(TEXT("AirControl default"), ExpectedAirControl, 0.35f);
	TestEqual(TEXT("RotationRate default"), ExpectedRotationRate, 540.0f);
	TestEqual(TEXT("CameraArmLength default"), ExpectedCameraArmLength, 320.0f);
	TestEqual(TEXT("CameraFOV default"), ExpectedCameraFOV, 90.0f);

	// TODO (Phase 03, needs a test map): spawn AGFCharacter in a world,
	// call AddMovementInput / Jump, tick the world, and assert the
	// character actually moved, jumped, crouched and sprinted at the
	// speeds above. Until then this file is a skeleton, not a pass.
	AddWarning(TEXT("GFCharacterMovementTest is a skeleton: no character was spawned, no world was ticked. NOT RUN in UE."));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

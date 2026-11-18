# Changelog — Galaxy Frontier

All notable changes, per phase. Statuses: CREATED / REVIEWED / BUILT /
TESTED / BLOCKED / NOT IMPLEMENTED (see Docs/QUALITY.md).

## [Phase 01 — Character] — 2026-11-18

### Added
- `Source/GalaxyFrontier/Characters/AGFCharacter.h/.cpp` — third-person
  character: SpringArm (320 cm) + Camera (FOV 90), movement Walk 300 /
  Run 550 / Sprint 700 / Crouch 180 cm/s, JumpZ 520, AirControl 0.35,
  RotationRate 540, all as editable UPROPERTY. Enhanced Input binding
  for Move / Look / Jump / Sprint / Crouch / Aim / Fire / Reload /
  Interact / SwitchWeapon / OpenInventory / Pause.
- `Config/DefaultInput.ini` — Enhanced Input actions
  IA_Move … IA_Pause and mapping context IMC_Player (key bindings
  documented; the `.uasset` Input Actions themselves must be created in
  the Unreal Editor — they are not faked as binaries).
- `Docs/INPUT.md` — full input-action table and Editor creation steps.
- `Docs/MOBILE-HUD.md` — WBP_MobileHUD spec (joystick, camera drag,
  buttons, health/shield/ammo/objective, Safe Areas, multi-touch).
- `Docs/ANIMATION.md` — Animation Blueprint / Blend Space spec.
- `Source/GalaxyFrontier/Tests/GFCharacterMovementTest.cpp` — Automation
  test skeleton for movement values. Status: NOT RUN (no UE here).

### Not implemented (declared only)
- `AGFCharacter::Fire`, `Reload`, `Interact`, `SwitchWeapon`,
  `OpenInventory` — bodies log a warning and return; real logic is
  Phase 02 (Combat) / Phase 05 (Inventory). No fake firing, ammo or
  damage is simulated.

### Blocked
- UE build, IPA build, signing, device testing — no UE / Xcode / macOS
  on this Linux VM (see Docs/ENVIRONMENT.md).

## [Phase 00 — Foundation] — 2026-11-18

### Added
- `GalaxyFrontier.uproject` (UE 5.5 — assumption, recorded in README).
- `Config/DefaultEngine.ini`, `DefaultGame.ini`, `DefaultInput.ini`,
  `DefaultScalability.ini`, `DefaultDeviceProfiles.ini` (iOS/iPadOS,
  Landscape, Enhanced Input, UMG, Niagara, scalable 30/60 fps).
- `Source/GalaxyFrontier/GalaxyFrontier.Build.cs`,
  `GalaxyFrontier.h/.cpp`, `Source/GalaxyFrontier.Target.cs`.
- `README.md`, `CONTRIBUTING.md`, `CHANGELOG.md`, `LICENSE` (MIT —
  assumption), `.gitignore`, `.gitattributes` (Git LFS).
- `Docs/ROADMAP.md`, `Docs/QUALITY.md`, `Docs/DEVICE_TEST_MATRIX.md`,
  `Docs/ENVIRONMENT.md` (real tool versions / missing tools, verified
  live 2026-11-18), `Docs/PHASE-00-REPORT.md`, `Docs/PHASE-01-REPORT.md`.
- `Content/README.md` — states explicitly that no `.uasset` files are
  included in Phases 00-01.

### Blocked
- UE build / IPA / signing / device tests — same reason as above.

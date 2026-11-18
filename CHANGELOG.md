# Changelog — Galaxy Frontier

All notable changes, per phase. Statuses: CREATED / REVIEWED / BUILT /
TESTED / BLOCKED / NOT IMPLEMENTED (see Docs/QUALITY.md).

## [Unreleased — SideStore IPA build path] — 2026-11-18

### Added
- `.github/workflows/build-ios-ipa.yml` — GitHub Actions workflow
  to build an **unsigned** iOS `.ipa` for SideStore: `macos-15`
  (NOT `macos-14` — retired 2026-11-02), Xcode 16.x, UE 5.5,
  `RunUAT BuildCookRun -platform=IOS -NoCodeSign`, `.ipa` uploaded
  as an Actions artifact (7-day retention). YAML validated by
  parsing (11 steps, no secrets). Workflow_dispatch + push only
  when the workflow file itself changes.
- `Docs/BUILD-IPA-SIDESTORE.md` — feasibility assessment with
  checked numbers, runner/engine/source citations, step-by-step
  run + SideStore install instructions, failure modes, and the
  realistic routes (self-hosted Mac recommended).
- `Docs/ENVIRONMENT.md` and `Docs/ROADMAP.md` — IPA-path facts
  only: standard-runner limits, macos-14 retirement, repo HEAD
  `d1f8965f` (repo no longer empty).

### Not built / blocked
- **BUILT: NONE. TESTED: NONE.** The workflow has never run; no
  IPA exists. A standard GitHub-hosted runner is **not feasible**
  (14 GB SSD vs 40–60 GB UE 5.5; 10 GB cache cap) — the workflow
  fails fast on a missing pre-installed engine / low disk by
  design. Do not attempt an IPA until Phase 03 is complete and
  plays in the Editor on macOS.

## [Phase 03 — World & Missions] — 2026-11-18

### Added
- `Source/GalaxyFrontier/Missions/UGFMissionSubsystem.h/.cpp` —
  GameInstance subsystem: one active mission, strict stage order,
  `StartMission` / `CompleteObjective` / `FailMission` /
  `CompleteMission` / `GetCurrentObjective` / `SaveMissionProgress`
  (slot `GF_MissionSave`) / `LoadMissionProgress`.
- `Source/GalaxyFrontier/Missions/UGFObjectiveComponent.h/.cpp` —
  per-stage component, forwards to the subsystem (the authority).
- `Source/GalaxyFrontier/Missions/UGFMissionDataAsset.h/.cpp` —
  `FGFObjectiveData` stage list; mission "The Lost Power Core",
  9 stages (Land → ReachOutpost → EnterBuilding → FindPowerCore →
  ActivateCore → DefeatEnemy → ReachExtractionZone →
  CompleteMission → ShowResult).
- `Source/GalaxyFrontier/Missions/UGFMissionSaveGame.h/.cpp` —
  mission-only save object (full player save is Phase 05).
- `Source/GalaxyFrontier/World/IGFInteractable.h` — C++-only
  interface (Interact / prompt / CanInteract / duration).
- `Source/GalaxyFrontier/World/UGFInteractionComponent.h/.cpp` —
  0.15 s timer scan (no Tick), nearest usable interactable in
  250 cm, Begin / Complete / Cancel with range + usability
  re-checked at completion.
- `Source/GalaxyFrontier/World/AGFInteractableActor.h/.cpp` —
  Door / Crate / Device / PowerStation / CommDevice /
  ExtractionPoint / MissionItem, single-use vs reusable.
- `Docs/WORLD-ZERON.md` — planet ZERON, `L_ZERON_Prototype`
  (~400 × 400 m test area), budgets, streaming decision,
  Editor creation steps. No `.umap` in this repo.
- `Docs/MISSIONS.md`, `Docs/INTERACTION.md`.
- `Source/GalaxyFrontier/Tests/GFMissionTest.cpp` — Automation
  test skeleton, 10 failure cases listed. Status: NOT RUN.

### Changed
- `AGFCharacter::HandleInteract` is now a real implementation
  (delegates to `UGFInteractionComponent`). The Phase 01 stub
  is replaced.

### Not implemented (declared / specced only)
- Level `L_ZERON_Prototype` itself (Editor asset, macOS only).
- Mission UI (`WBP_MissionPanel`, `WBP_MissionComplete`) — Phase 04.
- Mission rewards as inventory items — Phase 05.
- Interaction animations / SFX — Phase 04.

### Blocked
- UE build, IPA build, signing, device testing — no UE / Xcode /
  macOS on this Linux VM (see Docs/ENVIRONMENT.md).

## [Phase 02 — Combat & AI] — 2026-11-18

### Added
- `Source/GalaxyFrontier/Components/UGFHealthComponent.h/.cpp` —
  Health 100 / Shield 100, shield absorbs first, clamp 0..max,
  no damage after death, `Die()` fires `OnDeath` once, regen
  timer, health/shield change delegates.
- `Source/GalaxyFrontier/Combat/UGFCombatComponent.h/.cpp` —
  weapon spawn/equip/switch, fire-rate timer, automatic fire,
  `StartFire` / `StopFire` / `Reload` / `SwitchToNextWeapon`.
- `Source/GalaxyFrontier/Weapons/UGFWeaponDataAsset.h/.cpp`,
  `AGFWeaponBase.h/.cpp`, `AGFProjectile.h/.cpp` — data-driven
  weapons (Pistol 20/3/s/12, Assault 18/10/s/30, Sniper 85/0.8/s/5,
  Energy Rifle 24/5/s/24), no fire while reloading or at 0 ammo,
  no self-hit, no damage beyond MaxRange, 3 s projectile cap.
- `Source/GalaxyFrontier/AI/AGFEnemyCharacter.h/.cpp`,
  `AGFEnemyAIController.h/.cpp`, `UGFEnemyHealthComponent.h/.cpp` —
  enemy (Health 100, Detection 1500 cm, Attack 800 cm, Patrol
  1000 cm, cooldown 1.5 s, Speed 350 cm/s), AI Perception sight,
  Blackboard keys, Behavior Tree spec + 0.25 s timer fallback.
- `Docs/WEAPONS.md`, `Docs/AI.md`, `Docs/CHARACTERS.md`
  (realistic original humans only, PBR, LODs, customisation
  hooks — no mesh binary in this repo), `Docs/RELIABILITY.md`
  (per-system failure-mode checklist).
- `Source/GalaxyFrontier/Tests/GFCombatTest.cpp` — Automation
  test skeleton, 10 failure cases listed. Status: NOT RUN.
- `GalaxyFrontier.Build.cs` — added `AIModule` + `NavigationSystem`
  (Private, actually used by the AI controller).

### Changed
- `AGFCharacter::HandleFire` / `HandleReload` /
  `HandleSwitchWeapon` are now real implementations (delegated
  to `UGFCombatComponent`). The Phase 01 stubs are replaced.
- `AGFCharacter` gained `HealthComponent`, `CombatComponent`,
  `InteractionComponent`, `FGFCharacterCustomization`,
  `ApplyCustomization()` / `ApplyMaterialToSlot()` /
  `ApplyForcedLOD()`.
- `Config/DefaultScalability.ini`, `Config/DefaultDeviceProfiles.ini`
  — High group + iPhone Pro / iPad Pro profiles (60 fps is a
  target to measure, never a promise), Nanite/Lumen notes.
- `Docs/QUALITY.md`, `Docs/DEVICE_TEST_MATRIX.md` — budgets
  (draw calls, polygons, texture memory, dynamic lights ≤ 2).

### Not implemented (declared / specced only)
- Weapon meshes, muzzle-flash Niagara, firing SFX (Editor assets).
- Laser / Melee data assets, weapon pickups / inventory (Phase 05).
- BT / Blackboard `.uasset` files — spec in `Docs/AI.md` only.
- EQS, hearing, squad AI, ranged enemies.

### Blocked
- UE build, IPA build, signing, device testing — no UE / Xcode /
  macOS on this Linux VM (see Docs/ENVIRONMENT.md).

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

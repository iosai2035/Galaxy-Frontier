# Phase 04 — Mobile UI & Optimization — Report (2026-11-18)

- **Phase:** 04 Mobile UI & Optimization
- **Systems created:** HUD bridge + HUD widget base, settings
  save/manager, performance manager (sampling + hysteresis
  auto-quality), full UMG build specs, performance pass docs.
- **Files created / updated:** see list below (all CREATED, REVIEWED).

## Files

| File | Status |
|---|---|
| `Source/GalaxyFrontier/UI/UGFHUDController.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/UI/UGFHUDController.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/UI/UGFMobileHUDWidget.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/UI/UGFMobileHUDWidget.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Settings/UGFSettingsSaveGame.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Settings/UGFSettingsSaveGame.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Settings/UGFSettingsManager.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Settings/UGFSettingsManager.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Performance/UGFPerformanceManager.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Performance/UGFPerformanceManager.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Tests/GFPhase04Test.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Characters/AGFCharacter.h` (updated: HUDController, bInvertLookPitch, SetAiming/SetSprinting) | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Characters/AGFCharacter.cpp` (updated: HandlePause real, HandleLook invert, SetAiming/SetSprinting) | CREATED, REVIEWED |
| `Source/GalaxyFrontier/GalaxyFrontier.Build.cs` (updated: UMG + Slate + SlateCore) | CREATED, REVIEWED |
| `Config/DefaultScalability.ini` (updated: streaming pool per tier) | CREATED, REVIEWED |
| `Config/DefaultDeviceProfiles.ini` (updated: Phase 04 auto step-down note) | CREATED, REVIEWED |
| `Docs/MOBILE-HUD.md` (rewritten: full Phase 04 spec) | CREATED, REVIEWED |
| `Docs/UI-SCREENS.md` | CREATED, REVIEWED |
| `Docs/SETTINGS.md` | CREATED, REVIEWED |
| `Docs/PERFORMANCE.md` | CREATED, REVIEWED |
| `Docs/DEVICE_TEST_MATRIX.md` (updated: Phase 04 measurement protocol) | CREATED, REVIEWED |
| `Docs/RELIABILITY.md` (updated: Phase 04 failure tables) | CREATED, REVIEWED |
| `Docs/ROADMAP.md` (updated: Phase 04 done) | CREATED, REVIEWED |
| `README.md` (updated: Phase 04) | CREATED, REVIEWED |
| `CHANGELOG.md` (Phase 04 section) | CREATED, REVIEWED |

## Code implemented

- `UGFHUDController` (ActorComponent on `AGFCharacter`): binds
  `OnHealthChanged` / `OnShieldChanged` / `OnDeath` (health),
  `OnWeaponSwitched` (combat), `OnAmmoChanged` / `OnReloaded`
  (current weapon — rebound on every weapon switch, so the old
  weapon never updates the HUD again), `OnObjectiveCompleted` /
  `OnMissionCompleted` / `OnMissionFailed` (mission subsystem),
  `OnInteractableChanged` (interaction) and re-broadcasts them
  for the widgets. No widget polls anything per frame.
  `CreateHUD()` creates `WBP_MobileHUD` (class assigned in the
  Editor) with a 10× 0.5 s retry when the player controller is
  not ready yet. `TogglePause()` / `OpenPauseMenu()` /
  `ClosePauseMenu()` use `UGameplayStatics::SetGamePaused`,
  call `CombatComponent::StopFire()` on pause, and switch input
  mode UIOnly/GameOnly. `RefreshAll()` pulls every value once
  so a late-created widget is correct immediately. Game Over
  and Mission Complete widgets are created (when assigned) on
  `OnDeath` / `OnMissionCompleted`.
- `UGFMobileHUDWidget` (C++ base for `WBP_MobileHUD`): all
  widgets `BindWidgetOptional` with the exact names in
  `Docs/MOBILE-HUD.md` (HealthBar/ShieldBar, HealthText/
  ShieldText "87 / 100", AmmoText "24 / 120", WeaponNameText,
  ObjectiveText, InteractPromptText, DiagnosticsText, and the
  8 buttons). Button handlers call the combat / character /
  interaction APIs directly (Fire press/release,
  Aim/Sprint press/release via `SetAiming()`/`SetSprinting()`,
  Jump, Interact, Reload, SwitchWeapon, Pause). The Interact
  button hides when the prompt is empty. The diagnostics line
  shows measured FPS / RAM / live quality only when Settings
  `bShowDiagnostics` is true.
- `UGFSettingsSaveGame` + `UGFSettingsManager` (GameInstance
  subsystem, slot `GF_SettingsSave`): camera sensitivity
  0.1–5.0 (default 1.0), invert pitch, button scale 0.5–2.0,
  joystick side + anchor 0.05–0.95, graphics Low/Medium/High
  (default Medium), auto quality (default on), volumes 0–1,
  language (20 codes, default "ar"), diagnostics toggle.
  `ValidateSettings()` clamps / resets every field; non-finite
  numbers are rejected. Graphics + camera + language are
  applied in code; volumes are stored only (live mix needs a
  SoundMix asset — see `Docs/SETTINGS.md`).
- `UGFPerformanceManager` (GameInstance subsystem): 0.5 s timer
  sampling (never per-frame) of FPS (EMA) / lowest FPS since
  sampling started / RAM. Budgets High 150 draw calls / 500k polys / 512 MB / 2
  dynamic lights, Medium 120 / 350k / 384 MB / 1, Low 80 /
  200k / 256 MB / 1. Hysteresis auto-quality: step down after
  5 s below (target − 5), step up after 10 s above
  (target + 10), 10 s cooldown between changes, dead band in
  between, never above the user's chosen quality, never
  rewrites the saved quality. Thermal state is `Unknown` in
  this build (iOS reader NOT IMPLEMENTED); draw calls /
  polygons are Editor measurements (`stat RHI`), not live
  values — the diagnostics overlay shows none of them.
- `AGFCharacter`: `HandlePause` is now a real implementation
  (delegates to `UGFHUDController::TogglePause()`); the Phase 01
  stub is replaced. New `HUDController` component,
  `bInvertLookPitch` (honoured in `HandleLook`), and public
  `SetAiming()` / `SetSprinting()` for the HUD touch buttons
  (they update movement speed, so aim-walk stays WalkSpeed 300).
- `GalaxyFrontier.Build.cs`: added `UMG` + `Slate` +
  `SlateCore` (Public — actually used by the HUD widget /
  controller).

## UMG specs (Docs, not files)

- `Docs/MOBILE-HUD.md` — full `WBP_MobileHUD` spec: anchor
  table, Safe Areas, virtual joystick (fixed base, floating
  thumb, 0.15 dead zone, side/anchor from Settings),
  camera-drag area, button table with dp sizes, multi-touch
  (joystick + Fire + drag simultaneously), diagnostics overlay.
- `Docs/UI-SCREENS.md` — `WBP_MainMenu`, `WBP_PauseMenu`,
  `WBP_Settings`, `WBP_WeaponSelection`, `WBP_MissionPanel`,
  `WBP_LoadingScreen`, `WBP_GameOver`, `WBP_MissionComplete`
  (full specs); `WBP_Inventory` spec-only for Phase 05;
  `WBP_CharacterCustomization` brief spec only (3 colour
  pickers on the Phase 02 hooks — full customisation is
  Phase 05).
- `Docs/SETTINGS.md`, `Docs/PERFORMANCE.md` (budgets, LOD0–3,
  ASTC, HISM instancing, lighting/shadow budget, measurement
  protocol), updated `Docs/DEVICE_TEST_MATRIX.md` (Phase 04
  measurement protocol + thermal recording, FPS/RAM/thermal
  columns still empty).

## Tests

- `GFPhase04Test.cpp` — Automation test skeleton, spec values +
  15 failure cases listed (settings NaN / out of range /
  bad language, corrupt save, pause without a widget, fire
  held into pause, weapon-switch ammo rebind, hysteresis
  timing, no oscillation, never above the ceiling, death,
  multi-touch, Safe Areas). **NOT RUN** (no UE here). It
  creates no widget and ticks no world; a real test needs
  `WBP_MobileHUD` in `L_ZERON_Prototype` on macOS. Do not
  mark it PASSED.

## Errors found / fixed in review

- **Fixed:** an edit to `AGFCharacter.cpp` left a stray line
  `void UGFHUDController* DummyFunctionToRemove();` in place of
  the `UpdateMovementSpeed()` definition header. Removed —
  `SetSprinting()` and `UpdateMovementSpeed()` are now defined
  correctly; verified by grep that no "DummyFunction" text
  remains anywhere in the project.
- **Fixed:** `UGFPerformanceManager::ApplyLiveQuality()` had a
  confusing early return (`NewQuality == CurrentQuality &&
  Reason.IsEmpty()`) that let a same-quality call reset timers
  and re-apply settings when a reason string was present.
  Simplified to return immediately on same quality.
- **Fixed:** `UGFPerformanceManager::OnSampleTimer()` contained
  a dead `if (!bSamplingStarted)` branch (the flag is always
  true when the timer fires). Removed.
- **Fixed:** `MinFPS` was documented as a "window" minimum
  while the code keeps the lowest sample since sampling
  started. The header now says so explicitly — a dip is never
  hidden by later good frames.
- **Fixed:** `UGFPerformanceManager::Initialize()` called
  `GetGameInstance()->GetSubsystem()` without a null guard.
  Guarded with a local `UGameInstance*`.
- **Fixed:** two unused `#include "Engine/Engine.h"` lines
  (settings manager, performance manager). Removed.
- **Fixed:** `UGFSettingsManager::LoadSettings()` briefly held
  a `(void)GI;` outside the scope that declared `GI` (would
  not compile). Rewritten to a plain `GetGameInstance()` check.
- **Checked, no change:** `UGFHUDController` ammo binding is
  safe in both `BeginPlay` orders — if the HUD binds before
  the combat component spawns weapons, the combat
  `OnWeaponSwitched` broadcast from `SpawnWeapons()` triggers
  the rebind; if it binds after, `BindCurrentWeapon()` binds
  the already-spawned weapon directly.
- **Checked, no change:** timer callbacks (`RetryCreateHUD`,
  `OnSampleTimer`) are plain member functions, matching the
  established pattern in Phases 02–03 (`OnFireTimer`,
  `OnRegenTick`, `OnScanTimer`); only `AddDynamic` handlers
  are `UFUNCTION`s, as required.

## NOT IMPLEMENTED

- All UMG widgets as `.uasset` files — specs in `Docs/` only;
  built in the Editor on macOS (see the per-widget checklists).
- `WBP_Inventory` (Phase 05 backend) — `HandleOpenInventory`
  stays a stub that logs NOT IMPLEMENTED; it does not open any
  widget, because there is no inventory data to show.
- Full character customisation (Phase 05) — only the 3 colour
  pickers on the Phase 02 hooks are specced.
- Live audio volumes (SoundMix asset), full localisation data
  (Editor loc targets), iOS thermal reader — see
  `Docs/SETTINGS.md` / `Docs/PERFORMANCE.md`.
- Real device measurements: FPS / RAM / thermal columns in
  `Docs/DEVICE_TEST_MATRIX.md` are empty until a 15-minute
  session on a real iPhone/iPad.

## Changes to Phases 00–03 files (only linking changes)

- `AGFCharacter.h/.cpp` — `HUDController` component,
  `bInvertLookPitch`, `SetAiming()` / `SetSprinting()`,
  `HandlePause` real, `HandleLook` invert. No movement,
  combat, AI, mission or interaction logic was touched.
- `GalaxyFrontier.Build.cs` — UMG/Slate/SlateCore added.
- `Config/DefaultScalability.ini` — `r.Streaming.PoolSize`
  per tier (High 512, Medium/Low 256).
- `Config/DefaultDeviceProfiles.ini` — Phase 04 comment only.
- `Docs/DEVICE_TEST_MATRIX.md`, `Docs/RELIABILITY.md`,
  `Docs/ROADMAP.md`, `README.md`, `CHANGELOG.md` — Phase 04
  sections / notes only.
- No Phase 00–03 gameplay code, weapon, AI, mission or
  interaction file was modified.

## Build / test status

- **BUILT: NONE — BLOCKED.** No Unreal Engine on this Linux VM.
- **TESTED: NONE — BLOCKED.** No UE, no device, no Editor run.
- **IPA: BLOCKED.** macOS + Xcode + iOS SDK + signing required.
  (Workflow `.github/workflows/build-ios-ipa.yml` is on GitHub
  at commit `c62d890d`; this agent did not touch `.github/`.)
- **Device testing: BLOCKED.** No iPhone/iPad here.
- **`.uproject` still parses as JSON** (verified live).

## Delivery

- ZIP: `GalaxyFrontier-Phase-04-Mobile-UI-Optimization.zip` (in
  `~/workspace/galaxy-frontier/`). Verified with `unzip -l` and
  `unzip -t` — see sizes/counts in the parent delivery report.
- Refreshed: `GalaxyFrontier-All-Phases.zip` and
  `GalaxyFrontier-All-Phases-Source.zip` (overwrite the old ones).
- GitHub push: NOT DONE by this agent.

## Next step

**Phase 05 — Expansion:** inventory/equipment backend
(`UGFSaveGame`), character customisation profiles, ground
vehicle, spaceship, planets NEXUS / VELTRON / ASTRALIS, new
missions, player progression. Then the IPA build on a
self-hosted Mac + device testing (Phase 04 exit criteria).

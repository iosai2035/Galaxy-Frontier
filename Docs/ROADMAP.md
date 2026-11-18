# Roadmap — Galaxy Frontier

Single player first. Multiplayer and Battle Royale are Phase 06
(the network base is written and reviewed; the two-player test
that proves it has NOT run yet — see Phase 06 below).

## Phase 00 — Foundation ✅ CREATED / REVIEWED (2026-11-18)

- Project, `.uproject` (UE 5.5 — assumption), structure, C++ module,
  Config (Engine/Game/Input/Scalability/DeviceProfiles), README,
  CONTRIBUTING, CHANGELOG, LICENSE (MIT — assumption), `.gitignore`,
  `.gitattributes` (Git LFS), Docs.
- Output: `GalaxyFrontier-Phase-00-Foundation.zip`
- Blocked: UE build, IPA, signing, device tests (no UE/Xcode/macOS).

## Phase 01 — Character ✅ CREATED / REVIEWED (2026-11-18)

- `AGFCharacter` (movement, camera, Enhanced Input), Input Actions /
  Mapping Context spec, `WBP_MobileHUD` spec, Animation Blueprint /
  Blend Space spec, movement Automation test skeleton (NOT RUN).
- Output: `GalaxyFrontier-Phase-01-Character.zip`
- Not implemented: Fire / Reload / Interact / SwitchWeapon /
  OpenInventory / Pause (declared stubs, Phase 02/03/04/05).
- Blocked: UE build, IPA, signing, device tests.

## IPA build path — SideStore (2026-11-18, NOT BUILT / NOT TESTED)

- Workflow `.github/workflows/build-ios-ipa.yml` created
  (macos-15, Xcode 16.x, UE 5.5, unsigned `.ipa` via
  `RunUAT BuildCookRun -platform=IOS -NoCodeSign`); full analysis
  in `Docs/BUILD-IPA-SIDESTORE.md`.
- **Verdict: a standard GitHub-hosted runner is NOT feasible**
  (14 GB disk vs 40–60 GB UE 5.5; 10 GB cache cap). The workflow
  fails fast on a missing engine / low disk. Realistic route: a
  self-hosted Mac with UE 5.5 pre-installed, or a paid runner
  with verified larger disk.
- **Do not spend a build attempt until Phase 03** (ZERON + "The
  Lost Power Core") is complete and plays in the Editor on macOS.
  An IPA from Phases 00–01 would be a character with no map,
  mission, weapons or mobile HUD.
- SideStore install: unsigned `.ipa` → SideStore re-signs with
  the user's own free Apple ID (7-day signature, 3-app limit,
  refresh via LocalDevVPN).

## Phase 02 — Combat & AI ✅ CREATED / REVIEWED (2026-11-18)

- `UGFHealthComponent` (Health 100 / Shield 100, shield absorbs first),
  `UGFCombatComponent`, `AGFWeaponBase`, `AGFProjectile`,
  `UGFWeaponDataAsset` (Pistol 20 dmg / 3/s / 12, Assault 18 / 10/s / 30,
  Sniper 85 / 0.8/s / 5, Energy Rifle 24 / 5/s / 24 — starting balance),
  `AGFEnemyCharacter`, `AGFEnemyAIController`, Blackboard + Behavior
  Tree (Idle/Patrol/Investigate/Chase/Attack/Search/Retreat/Die),
  AI Perception, Navigation.
- `AGFCharacter::Fire` / `Reload` / `SwitchWeapon` are real now
  (Phase 01 stubs replaced). Realistic-character spec
  (`Docs/CHARACTERS.md`), reliability checklist
  (`Docs/RELIABILITY.md`), combat test skeleton (NOT RUN).
- Output: `GalaxyFrontier-Phase-02-Combat-AI.zip`
- Exit criteria NOT yet proven: playable combat prototype needs
  an Editor run on macOS (BUILT: NONE, TESTED: NONE — BLOCKED).

## Phase 03 — World & Missions ✅ CREATED / REVIEWED (2026-11-18)

- Planet ZERON, level `L_ZERON_Prototype` spec (small test area
  first — no `.umap` in this repo, Editor asset on macOS),
  mission "The Lost Power Core" (9 stages: Landing Zone → Outpost
  → enter building → find Power Core → activate → enemy encounter
  → Extraction Zone → complete / reward), `UGFMissionSubsystem`,
  `UGFObjectiveComponent`, `UGFMissionDataAsset`,
  `UGFInteractionComponent` (doors, crates, terminals, extraction
  points), `AGFInteractableActor`.
- `AGFCharacter::HandleInteract` is real now (Phase 01 stub replaced).
- Output: `GalaxyFrontier-Phase-03-World-Missions.zip`
- Exit criteria NOT yet proven: one mission completable start to
  finish needs an Editor run on macOS (BUILT: NONE, TESTED: NONE).

## Phase 04 — Mobile UI & Optimization ✅ CREATED / REVIEWED (2026-11-18)

- UMG specs: `WBP_MainMenu`, `WBP_MobileHUD` (full spec, C++ base
  `UGFMobileHUDWidget`), `WBP_PauseMenu`, `WBP_Settings`,
  `WBP_WeaponSelection`, `WBP_MissionPanel`, `WBP_LoadingScreen`,
  `WBP_GameOver`, `WBP_MissionComplete` (all in
  `Docs/MOBILE-HUD.md` + `Docs/UI-SCREENS.md` — specs only, the
  `.uasset` widgets are built in the Editor on macOS).
  `WBP_Inventory` is spec-only for Phase 05; `WBP_CharacterCustomization`
  is a brief spec (3 colour pickers on the Phase 02 hooks).
- C++ (real, no new stubs): `UGFHUDController` (event-driven HUD
  bridge, pause / Game Over / Mission Complete widgets),
  `UGFSettingsSaveGame` + `UGFSettingsManager` (slot
  `GF_SettingsSave`, validated ranges, 20 languages, default "ar"),
  `UGFPerformanceManager` (0.5 s sampling, hysteresis auto-quality,
  budgets High 150/500k/512 MB, Medium 120/350k/384 MB, Low 80/200k/
  256 MB, dynamic lights ≤ 2). `AGFCharacter::HandlePause` is real
  now; `HandleOpenInventory` stays a Phase 05 stub.
- Performance pass: LODs 0–3, ASTC textures, HISM instancing,
  lighting/shadow budget, per-device Scalability (see
  `Docs/PERFORMANCE.md` + updated `Docs/DEVICE_TEST_MATRIX.md`).
- Output: `GalaxyFrontier-Phase-04-Mobile-UI-Optimization.zip`
- Exit criteria NOT yet proven: UMG widgets unbuilt in the Editor,
  no real iPhone/iPad run — FPS/RAM/thermal columns are empty
  (BUILT: NONE, TESTED: NONE — BLOCKED).

## Phase 05 — Expansion ✅ CREATED / REVIEWED (2026-11-18)

- `UGFInventoryComponent` + `UGFItemDataAsset` (24 slots,
  stacking, overflow never drops items), `UGFEquipmentComponent`
  (Primary / Secondary / Shield Generator, +25 MaxShield Mk1),
  `UGFCharacterCustomizationData` + `UGFCustomizationManager`
  (cosmetic-only profiles: body, hair, armour, helmet,
  accessories, 4 colours), `AGFGroundVehicle` (Rover RX-7,
  1800 cm/s, 250 HP, possession-based),
  `AGFSpaceship` (Starhopper — travel is a real level load
  after a 5 s spool-up, no space combat),
  `UGFPlanetDataAsset` + `Docs/PLANETS.md` (ZERON / NEXUS /
  VELTRON / ASTRALIS, unlock chain, gravity, weather),
  `AGFPickupActor` (inventory-granted, respawning pickups),
  two new missions ("Signal in the Static" on NEXUS, 250 XP;
  "The Frozen Vault" on VELTRON, 400 XP),
  `UGFProgressionComponent` (XP curve 100 × level, cap 20,
  mission rewards once per mission),
  `UGFSaveGame` + `UGFPlayerSaveManager` (slot `GF_PlayerSave`,
  version 1, migration from `GF_MissionSave`).
- `AGFCharacter::HandleOpenInventory` is real now (no Phase 01
  stub remains). Full UMG specs for `WBP_Inventory` +
  `WBP_CharacterCustomization` in `Docs/UI-SCREENS.md`.
- Output: `GalaxyFrontier-Phase-05-Expansion.zip`
- Exit criteria NOT yet proven: inventory / save / vehicles
  need an Editor run on macOS (BUILT: NONE, TESTED: NONE).

## Phase 06 — Multiplayer / Battle Royale ✅ CREATED / REVIEWED (2026-11-18)

- `AGFGameMode` (Lobby → Countdown → Playing → Ended, 30 s / 5 s,
  MinPlayers 2, MaxPlayers 60 design target), `AGFGameState`
  (replicated match state + zone), `AGFPlayerState` (replicated
  level / eliminations / alive / placement),
  `AGFPlayerController` (Server RPCs: fire / stop / interact /
  damage / set level; `ClientShowMatchResult`),
  `UGFServerValidation` (fire rate, movement, damage, projectile
  hits, interact — server-authoritative, single-player unaffected),
  `UGFReplicationManager` (priority + update interval by distance,
  idle suppression, heartbeat timeout), `UGFAntiCheatManager`
  (violation ledger: warn / kick at 3 / session ban at 5),
  `UGFSessionManager` (Online Subsystem NULL first, level ±3
  matchmaking, 60 s search timeout, never invents players),
  `UGFZoneManager` (8 shrinking phases, 6000 → 0 cm, DPS 0 → 25),
  `UGFMatchManager` (spawn distribution, placements, XP by
  placement: 1st 500 / 2–5 300 / 6–10 200 / 11–20 100 / 21+ 50),
  `GalaxyFrontierServer.Target.cs` (dedicated server target).
- `AGFCharacter::HandleFireStarted/Completed` and
  `HandleInteract` route through Server RPCs only in networked
  games; `AGFProjectile::OnHit` applies damage on the server
  only, via `ApplyValidatedDamage`.
- UMG specs: `WBP_Lobby`, `WBP_Matchmaking`,
  `WBP_MultiplayerHUD` (full specs, `Docs/UI-SCREENS.md`);
  `WBP_Spectate` is spec-only, NOT IMPLEMENTED.
- Output: `GalaxyFrontier-Phase-06-Multiplayer-Battle-Royale.zip`
- Exit criteria NOT yet proven: the two-player test (dedicated
  server + two clients, one match end-to-end) has never run.
  Battle Royale at 60 players is a design target, not a tested
  number (BUILT: NONE, TESTED: NONE — BLOCKED).

## Full project status — all phases (2026-11-18)

| Phase | Status | Output |
|---|---|---|
| 00 Foundation | ✅ CREATED / REVIEWED | Project, Config, Docs |
| 01 Character | ✅ CREATED / REVIEWED | `AGFCharacter`, input, movement |
| 02 Combat & AI | ✅ CREATED / REVIEWED | Health, combat, weapons, enemy AI |
| 03 World & Missions | ✅ CREATED / REVIEWED | ZERON, "The Lost Power Core" |
| 04 Mobile UI & Optimization | ✅ CREATED / REVIEWED | UMG specs, settings, performance |
| 05 Expansion | ✅ CREATED / REVIEWED | Inventory, vehicles, planets, save |
| 06 Multiplayer / Battle Royale | ✅ CREATED / REVIEWED | Network base, sessions, zone, BR |

- **BUILT: NONE. TESTED: NONE.** No phase has ever been
  compiled or run — there is no UE, no Xcode and no macOS on
  this Linux VM. Every phase is CREATED + REVIEWED only.
- **IPA: BLOCKED.** The workflow
  (`.github/workflows/build-ios-ipa.yml`, on GitHub at commit
  `c62d890d`) needs a self-hosted Mac with UE 5.5 pre-installed
  (a standard GitHub runner cannot hold the engine).
- **Next step after the code:** open the project in the Unreal
  Editor on macOS, build the UMG widgets / levels / Data Assets
  from the Docs specs, run the two-player multiplayer test,
  then device testing on a real iPhone/iPad, then the IPA build
  for SideStore.

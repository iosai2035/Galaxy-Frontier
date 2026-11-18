# Roadmap — Galaxy Frontier

Single player first. Multiplayer and Battle Royale only after the
network base is built and tested (Phase 06).

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

## Phase 02 — Combat & AI (next)

- `UGFHealthComponent` (Health 100 / Shield 100, shield absorbs first),
  `UGFCombatComponent`, `AGFWeaponBase`, `AGFProjectile`,
  `UGFWeaponDataAsset` (Pistol 20 dmg / 3/s / 12, Assault 18 / 10/s / 30,
  Sniper 85 / 0.8/s / 5, Energy Rifle 24 / 5/s / 24 — starting balance),
  `AGFEnemyCharacter`, `AGFEnemyAIController`, Blackboard + Behavior
  Tree (Idle/Patrol/Investigate/Chase/Attack/Search/Retreat/Die),
  AI Perception, Navigation.
- Exit criteria: playable combat prototype, enemy detects / chases /
  attacks / dies, player can die and respawn.

## Phase 03 — World & Missions

- Planet ZERON, level `L_ZERON_Prototype` (small test area first),
  mission "The Lost Power Core" (Landing Zone → Outpost → enter building
  → find Power Core → activate → enemy encounter → Extraction Zone →
  complete / reward), `UGFMissionSubsystem`, `UGFObjectiveComponent`,
  `UGFMissionDataAsset`, `UGFInteractionComponent` (doors, crates,
  terminals, extraction points).
- Exit criteria: one mission completable start to finish.

## Phase 04 — Mobile UI & Optimization

- UMG: MainMenu, MobileHUD, Pause, Settings, Inventory, WeaponSelection,
  MissionPanel, CharacterCustomization, LoadingScreen, GameOver,
  MissionComplete. Touch settings, graphics settings, performance pass
  (LODs, texture compression, instancing, light/shadow budget).
- Exit criteria: tested on real iPhone + iPad, FPS/RAM/thermal recorded.

## Phase 05 — Expansion

- Character customisation, inventory/equipment, ground vehicle,
  spaceship, planets NEXUS / VELTRON / ASTRALIS, new missions,
  player progression / SaveGame (`UGFSaveGame`).

## Phase 06 — Multiplayer

- Dedicated server, lobby, sessions, matchmaking, replication,
  server-authoritative validation, anti-cheat design. Two-player test
  first, then scale. Battle Royale only after this base passes.

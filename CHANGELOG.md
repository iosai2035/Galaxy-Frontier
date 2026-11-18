# Changelog — Galaxy Frontier

All notable changes, per phase. Statuses: CREATED / REVIEWED / BUILT /
TESTED / BLOCKED / NOT IMPLEMENTED (see Docs/QUALITY.md).

## [Phase 06 — Multiplayer / Battle Royale] — 2026-11-18

### Added
- `Source/GalaxyFrontier/Multiplayer/AGFGameMode.h/.cpp` —
  server-authoritative GameMode: match flow Lobby (30 s) →
  Countdown (5 s) → Playing → Ended, MinPlayers 2,
  MaxPlayers 60 (design target, not tested), `NotifyElimination`
  (victim dead, killer credit, placement, match end at one
  player left), `ChoosePlayerStart` delegated to
  `UGFMatchManager`, disconnect mid-match counts as an
  elimination. `GalaxyFrontierServer.Target.cs` is the
  dedicated-server target (see `Docs/DEDICATED-SERVER.md`).
- `Source/GalaxyFrontier/Multiplayer/AGFGameState.h/.cpp` —
  replicated match state (`MatchState`, `MatchTimerRemaining`,
  `AlivePlayerCount`, `TotalPlayerCount`, `ZoneCenter`,
  `ZoneRadius`); server-only setters refuse on clients.
- `Source/GalaxyFrontier/Multiplayer/AGFPlayerState.h/.cpp` —
  replicated per-player state (`PlayerLevel` 1..20, clamped on
  the server; `Eliminations`; `bIsAlive`; `Placement`, 0 = not
  placed).
- `Source/GalaxyFrontier/Multiplayer/AGFPlayerController.h/.cpp`
  — Server RPCs (`ServerRequestStartFire/StopFire/Interact/
  Damage`, `ServerSetPlayerLevel`) validated via
  `UGFServerValidation` in `_Implementation`; `_Validate` stays
  permissive (null checks only — returning false there would
  disconnect the client). `ClientShowMatchResult` for the final
  result. The saved level is sent once at login (client only).
- `Source/GalaxyFrontier/Multiplayer/UGFServerValidation.h/.cpp`
  — ActorComponent on `AGFCharacter`: fire-rate check (weapon
  `FireRate`, 15% + 50 ms slack), movement check (SprintSpeed
  700 cm/s + 10%, +500 cm slack; gross jumps = teleport),
  damage check (exact weapon `Damage`, range, alive, Playing
  match), projectile-hit check (damage ≤ 85, range from shot
  origin), interact check (250 cm + `CanInteract`). All pass
  trivially in single-player (Standalone). Failed checks
  report violations to `UGFAntiCheatManager`.
- `Source/GalaxyFrontier/Multiplayer/UGFReplicationManager.h/.cpp`
  — replication policy: priority 1.0 / 0.6 / 0.3 by distance
  (< 1500 / < 4000 / further), update interval 0.05 / 0.10 /
  0.20 s, idle moves under 5 cm send nothing, heartbeat
  timeout 30 s.
- `Source/GalaxyFrontier/Multiplayer/UGFAntiCheatManager.h/.cpp`
  — GameInstance subsystem, server only: violation ledger
  keyed by UniqueId; 1–2 warn, 3 kick, 5 session ban
  (in-memory only — no persistent ban list, no backend).
- `Source/GalaxyFrontier/Multiplayer/UGFSessionManager.h/.cpp` —
  GameInstance subsystem on the Online Subsystem (NULL first):
  `CreateSession` (advertises `GF_Level` + `GF_Mode`),
  `FindSessions` (level ±3 filter, 60 s timeout — never
  invents players or bots), `JoinSession` (connect string from
  the session, no hard-coded IP), `LeaveSession`.
- `Source/GalaxyFrontier/Multiplayer/UGFZoneManager.h/.cpp` —
  ActorComponent on `AGFGameMode`: 8 zone phases (6000 → 0 cm,
  DPS 0 → 25), linear shrink, 1 s server damage tick outside
  the radius (shield-first), zone deaths credit nobody.
- `Source/GalaxyFrontier/Multiplayer/UGFMatchManager.h/.cpp` —
  ActorComponent on `AGFGameMode`: round-robin spawn over
  PlayerStarts (warning + GameMode fallback when none are
  placed), placements (death places at the current alive
  count), XP by placement (1st 500, 2–5 300, 6–10 200, 11–20
  100, 21+ 50) via `UGFProgressionComponent::AddXP`, result
  notification to all clients.
- `Source/GalaxyFrontier/Tests/GFPhase06Test.cpp` — automation
  test skeleton, spec values + failure cases (match flow,
  validation, zone, sessions, anti-cheat, replication).
  **NOT RUN** (no UE / server / second device here).
- `Docs/MULTIPLAYER.md`, `Docs/BATTLE-ROYALE.md`,
  `Docs/ANTI-CHEAT.md`, `Docs/DEDICATED-SERVER.md` —
  network base, BR design (map `L_ZERON_BR_Prototype` spec
  only), anti-cheat design + limits, server build/run + the
  two-player test procedure. `Docs/UI-SCREENS.md` gains
  `WBP_Lobby`, `WBP_Matchmaking`, `WBP_MultiplayerHUD`
  (specs) and `WBP_Spectate` (spec only, NOT IMPLEMENTED).
  `Docs/RELIABILITY.md` gains Phase 06 failure tables.
- `Source/GalaxyFrontierServer.Target.cs` — dedicated server
  target (`TargetType.Server`, same `GalaxyFrontier` module).

### Changed
- `Source/GalaxyFrontier/Characters/AGFCharacter.h/.cpp` —
  `UGFServerValidation` + `UGFReplicationManager` components
  (created in the constructor) + `IsNetworkedGame()`.
  `HandleFireStarted/Completed` and `HandleInteract` route
  through the controller's Server RPCs only in networked
  games; single-player calls the Combat / Interaction
  components directly, exactly as before.
- `Source/GalaxyFrontier/Weapons/AGFProjectile.cpp` — `OnHit`
  applies damage only on the server (`HasAuthority()`), via
  `UGFServerValidation::ApplyValidatedDamage` when the shot
  has a player instigator (elimination credit on kill); AI
  projectiles (server-spawned, no validation component) apply
  damage directly on the server as before.
- `Source/GalaxyFrontier/GalaxyFrontier.Build.cs` — adds the
  `OnlineSubsystem` module (Phase 06, `UGFSessionManager`).
- `Config/DefaultGame.ini` — `MaxPlayers` 1 → 60 (Battle
  Royale design target; single-player unaffected — Standalone
  games never create a session).
- `Config/DefaultEngine.ini` — `[OnlineSubsystem]
  DefaultPlatformService=Null` (local testing first; EOS /
  Steam are specs only).

### Not implemented (declared only)
- All UMG widgets as `.uasset` files (`WBP_Lobby`,
  `WBP_Matchmaking`, `WBP_MultiplayerHUD`, `WBP_Spectate`) —
  specs in `Docs/UI-SCREENS.md` only.
- The BR level `L_ZERON_BR_Prototype` (`.umap`) and its
  60 PlayerStarts — spec in `Docs/BATTLE-ROYALE.md` only.
- EOS / Steam integration, voice/text chat, friends, parties,
  teams/squads, spectating, reconnect/rejoin, ranked
  matchmaking, networked AI / co-op missions, kill feed,
  drop/parachute sequence, BR vehicles — see
  `Docs/MULTIPLAYER.md` §8 and `Docs/BATTLE-ROYALE.md` §7.

### Blocked
- UE build / IPA / signing / device tests — same reason as
  above. Plus: the two-player multiplayer test needs a
  dedicated server + two clients (see
  `Docs/DEDICATED-SERVER.md` §4); Battle Royale at 60 players
  is a design target, not a tested number.

## [Phase 05 — Expansion] — 2026-11-18

### Added
- `Source/GalaxyFrontier/Inventory/UGFItemDataAsset.h/.cpp` —
  data-only item definition: stable `ItemId`, type (Weapon /
  Ammo / HealthPack / ShieldCell / ShieldGenerator /
  MissionItem / Material), cosmetic-only rarity, `MaxStackSize`
  (weapons/mission items 1, Health/Shield 5, Ammo 200),
  consumable + use effect (+50 HP, +50 shield, +30 reserve
  rounds), `WeaponData` for weapon items, `ShieldBonus` for
  shield generators. `Validate()` rejects bad ids, bad stacks,
  consumable/effect mismatches and weapon items without data.
- `Source/GalaxyFrontier/Inventory/UGFInventoryComponent.h/.cpp` —
  ActorComponent on `AGFCharacter`: 24 slots, partial-stack
  filling, `AddItem` returns the added count (overflow fires
  `OnInventoryFull`, nothing is dropped), `UseItem` applies
  the effect first and consumes second (a failed effect never
  eats the item), `ClearInventory` for save loads.
- `Source/GalaxyFrontier/Inventory/UGFEquipmentComponent.h/.cpp` —
  3 slots (Primary / Secondary / Shield Generator). Weapons
  must be owned and in the combat loadout (no silent spawns);
  shield generator adds its bonus to `MaxShield` while
  equipped and clamps current shield on unequip. Equipping
  never removes the item from the inventory.
- `Source/GalaxyFrontier/Customization/UGFCharacterCustomizationData.h/.cpp`
  + `UGFCustomizationManager.h/.cpp` — cosmetic-only profiles
  (body Male/Female, hairstyle, armour set, helmet,
  accessories, 4 colours), GameInstance subsystem owning the
  active profile, lookup by id for save loads. No stat field
  exists on a profile.
- `Source/GalaxyFrontier/Vehicles/AGFGroundVehicle.h/.cpp` —
  Rover RX-7: `UFloatingPawnMovementComponent` (no Chaos
  dependency), 1800 cm/s, 250 HP own health component,
  possession-based enter/exit (exit refused above 50 cm/s,
  driver hidden with collision off while driving).
- `Source/GalaxyFrontier/Vehicles/AGFSpaceship.h/.cpp` —
  Starhopper: `TravelToPlanet` validates + unlock-checks, then
  a 5 s spool-up and a real `OpenLevel` of the planet's level.
  No space combat in Phase 05.
- `Source/GalaxyFrontier/World/UGFPlanetDataAsset.h/.cpp` —
  planet data (gravity 0.3–2.0, weather, level name,
  recommended level, unlock mission). `IsUnlocked` checks the
  save's completed missions.
- `Source/GalaxyFrontier/World/AGFPickupActor.h/.cpp` —
  `IGFInteractable` pickup: grants via the inventory only,
  partial pickup allowed, full inventory takes nothing and the
  pickup stays, respawning pickups return after `RespawnSeconds`
  (min 5 s).
- `Source/GalaxyFrontier/Progression/UGFProgressionComponent.h/.cpp` —
  level/XP on the character: XP to next level = 100 × level,
  cap 20, mission rewards granted exactly once per mission id
  (XP + one reward item; full-inventory fallback is a level
  pickup — see Docs/MISSIONS.md).
- `Source/GalaxyFrontier/Save/UGFSaveGame.h/.cpp` +
  `UGFPlayerSaveManager.h/.cpp` — full player save, slot
  `GF_PlayerSave`, version 1: level/XP, completed missions,
  inventory, equipment, profile id, planet id, language
  reference copy. Validation clamps/drops bad data; version-0
  saves migrate from the Phase 03 `GF_MissionSave` slot;
  newer-version saves are refused.
- `Source/GalaxyFrontier/Tests/GFPhase05Test.cpp` — Automation
  test skeleton, spec values + 25 failure cases (NOT RUN).
- `Docs/PLANETS.md` — ZERON recap + full NEXUS / VELTRON /
  ASTRALIS specs (environment, hazards, POIs, unlock chain,
  gravity, weather). `Docs/EXPANSION.md` — design doc (item
  table, XP curve, save format, vehicle numbers).
- `Docs/MISSIONS.md` — Missions 2 ("Signal in the Static",
  NEXUS, 6+2 stages, 250 XP + `NexusDataCore`) and 3
  ("The Frozen Vault", VELTRON, 7+2 stages, 400 XP +
  `VaultAlloy`), reward rules + inventory-full fallback.
- `Docs/UI-SCREENS.md` — full `WBP_Inventory` and
  `WBP_CharacterCustomization` specs (was brief/spec-only).
  `Docs/CHARACTERS.md` — Phase 05 customisation section.
  `Docs/RELIABILITY.md` — Phase 05 failure tables.

### Changed
- `AGFCharacter.h/.cpp` — `InventoryComponent`,
  `EquipmentComponent`, `ProgressionComponent` subobjects +
  getters; `HandleOpenInventory` is a real implementation now
  (delegates to `UGFHUDController::ToggleInventory()` — the
  last Phase 01 stub is gone); `FGFCharacterCustomization`
  gains `BodyType` + `AccentColour` (slot 2, applied only when
  the mesh has 3+ slots).
- `UGFHUDController.h/.cpp` — `InventoryWidgetClass` /
  `InventoryWidget`, `OpenInventory` / `CloseInventory` /
  `ToggleInventory` (pauses the game, stops firing, UIOnly
  input). Pause menu and Inventory never stack — opening one
  while the other is open is refused, so closing one never
  resumes a game the other paused.
- `UGFMissionDataAsset.h` — `CompletionXPReward` (default 100)
  + `CompletionRewardItem` (null = no item reward).

## [Phase 04 — Mobile UI & Optimization] — 2026-11-18

### Added
- `Source/GalaxyFrontier/UI/UGFHUDController.h/.cpp` — event-driven
  HUD bridge on `AGFCharacter`: binds health / shield / ammo /
  weapon-switched / mission / interaction delegates and
  re-broadcasts them for the widgets (no per-frame polling),
  creates the HUD widget (10× 0.5 s retry if the player
  controller is not ready), `TogglePause` / `OpenPauseMenu` /
  `ClosePauseMenu` (`SetGamePaused`, `StopFire()` on pause,
  input mode UIOnly/GameOnly), Game Over + Mission Complete
  widgets, `RefreshAll()` one-shot pull.
- `Source/GalaxyFrontier/UI/UGFMobileHUDWidget.h/.cpp` — C++
  base class for `WBP_MobileHUD` (BindWidgetOptional names in
  `Docs/MOBILE-HUD.md`): health/shield bars + "87 / 100" text,
  ammo "24 / 120", weapon name, objective text, interact prompt
  (button hidden when empty), diagnostics line, Fire/Aim/Jump/
  Sprint/Interact/Reload/SwitchWeapon/Pause button handlers.
- `Source/GalaxyFrontier/Settings/UGFSettingsSaveGame.h/.cpp` +
  `UGFSettingsManager.h/.cpp` — GameInstance subsystem, save
  slot `GF_SettingsSave`: camera sensitivity 0.1–5.0 (default
  1.0), invert pitch, button scale 0.5–2.0, joystick side +
  anchor 0.05–0.95, graphics Low/Medium/High (default Medium),
  auto quality (default on), volumes 0–1, language (20 codes,
  default "ar"), diagnostics toggle. Every value validated /
  clamped; non-finite values rejected; unsupported language
  rejected. Graphics + camera + language are applied in code;
  volumes are stored only (live mix needs a SoundMix asset —
  see `Docs/SETTINGS.md`).
- `Source/GalaxyFrontier/Performance/UGFPerformanceManager.h/.cpp` —
  GameInstance subsystem: 0.5 s sampling of FPS (EMA) / RAM,
  budgets High 150 draw calls / 500k polys / 512 MB / 2 lights,
  Medium 120 / 350k / 384 MB / 1, Low 80 / 200k / 256 MB / 1,
  hysteresis auto-quality (step down after 5 s below target−5,
  step up after 10 s above target+10, 10 s cooldown, dead band,
  never above the user's chosen quality), `OnQualityChanged` /
  `OnMetricsUpdated` delegates. Thermal state is `Unknown` in
  this build (iOS reader NOT IMPLEMENTED); draw calls / polygons
  are Editor measurements (`stat RHI`), not live values.
- `Source/GalaxyFrontier/Tests/GFPhase04Test.cpp` — Automation
  test skeleton, spec values + 15 failure cases listed. Status:
  NOT RUN.
- `Docs/MOBILE-HUD.md` (full Phase 04 spec: anchors, Safe Areas,
  joystick, camera drag, multi-touch, button table),
  `Docs/UI-SCREENS.md` (MainMenu, Pause, Settings, Inventory
  [Phase 05], WeaponSelection, MissionPanel,
  CharacterCustomization [brief, Phase 05], LoadingScreen,
  GameOver, MissionComplete), `Docs/SETTINGS.md`,
  `Docs/PERFORMANCE.md` (budgets, LOD0–3, ASTC, HISM, lighting
  budget, measurement protocol), `Docs/DEVICE_TEST_MATRIX.md`
  (Phase 04 measurement protocol, thermal recording),
  `Docs/RELIABILITY.md` (Phase 04 failure tables).
- `GalaxyFrontier.Build.cs` — added `UMG` + `Slate` +
  `SlateCore` (Public, actually used by the HUD widget /
  controller).
- `Config/DefaultScalability.ini` — streaming pool per tier
  (High 512 MB, Medium/Low 256 MB); `Config/DefaultDeviceProfiles.ini` —
  Phase 04 note (auto step-down may move the live tier below the
  starting profile).

### Changed
- `AGFCharacter::HandlePause` is now a real implementation
  (delegates to `UGFHUDController::TogglePause()`). The Phase 01
  stub is replaced. Character gained `HUDController`,
  `bInvertLookPitch`, and public `SetAiming()` / `SetSprinting()`
  (used by the HUD touch buttons; they update movement speed,
  so aim-walk stays WalkSpeed 300).
- `AGFCharacter::HandleLook` now honours `bInvertLookPitch`.

### Not implemented (declared / specced only)
- All UMG widgets as `.uasset` files — specs in `Docs/` only;
  built in the Editor on macOS.
- `WBP_Inventory` (Phase 05 backend) — `HandleOpenInventory`
  stays a stub that logs NOT IMPLEMENTED.
- Full character customisation (Phase 05) — only the 3 colour
  pickers on the Phase 02 hooks are specced.
- Live audio volumes (SoundMix asset), full localisation data,
  iOS thermal reader — see `Docs/SETTINGS.md` /
  `Docs/PERFORMANCE.md`.

### Blocked
- UE build, IPA build, signing, device testing — no UE / Xcode /
  macOS on this Linux VM (see Docs/ENVIRONMENT.md). FPS / RAM /
  thermal columns in `Docs/DEVICE_TEST_MATRIX.md` are empty
  until a real iPhone/iPad run.

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

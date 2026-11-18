# Phase 05 — Expansion — Report (2026-11-18)

- **Phase:** 05 Expansion
- **Systems created:** inventory + equipment, character
  customisation profiles, ground vehicle + spaceship, planets,
  pickups, two new missions, progression / XP, full player save.
- **Files created / updated:** see list below (all CREATED, REVIEWED).

## Files

| File | Status |
|---|---|
| `Source/GalaxyFrontier/Inventory/UGFItemDataAsset.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Inventory/UGFItemDataAsset.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Inventory/UGFInventoryComponent.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Inventory/UGFInventoryComponent.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Inventory/UGFEquipmentComponent.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Inventory/UGFEquipmentComponent.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Customization/UGFCharacterCustomizationData.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Customization/UGFCharacterCustomizationData.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Customization/UGFCustomizationManager.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Customization/UGFCustomizationManager.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Vehicles/AGFGroundVehicle.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Vehicles/AGFGroundVehicle.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Vehicles/AGFSpaceship.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Vehicles/AGFSpaceship.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/World/UGFPlanetDataAsset.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/World/UGFPlanetDataAsset.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/World/AGFPickupActor.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/World/AGFPickupActor.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Progression/UGFProgressionComponent.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Progression/UGFProgressionComponent.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Save/UGFSaveGame.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Save/UGFSaveGame.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Save/UGFPlayerSaveManager.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Save/UGFPlayerSaveManager.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Tests/GFPhase05Test.cpp` | CREATED, REVIEWED |
| `Docs/PLANETS.md` | CREATED, REVIEWED |
| `Docs/EXPANSION.md` | CREATED, REVIEWED |
| `Docs/PHASE-05-REPORT.md` (this file) | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Characters/AGFCharacter.h` (updated: 3 components, getters, BodyType + AccentColour) | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Characters/AGFCharacter.cpp` (updated: components created, HandleOpenInventory real, accent slot) | CREATED, REVIEWED |
| `Source/GalaxyFrontier/UI/UGFHUDController.h` (updated: Inventory widget + open/close API) | CREATED, REVIEWED |
| `Source/GalaxyFrontier/UI/UGFHUDController.cpp` (updated: Inventory open/close, Pause/Inventory mutual exclusion) | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Missions/UGFMissionDataAsset.h` (updated: CompletionXPReward + CompletionRewardItem) | CREATED, REVIEWED |
| `Docs/MISSIONS.md` (updated: Missions 2 + 3, reward rules) | CREATED, REVIEWED |
| `Docs/UI-SCREENS.md` (updated: full WBP_Inventory + WBP_CharacterCustomization specs) | CREATED, REVIEWED |
| `Docs/CHARACTERS.md` (updated: Phase 05 customisation section) | CREATED, REVIEWED |
| `Docs/RELIABILITY.md` (updated: Phase 05 failure tables) | CREATED, REVIEWED |
| `Docs/ROADMAP.md` (updated: Phase 05 done) | CREATED, REVIEWED |
| `README.md` (updated: Phase 05) | CREATED, REVIEWED |
| `CHANGELOG.md` (Phase 05 section) | CREATED, REVIEWED |

## Code implemented

- `UGFItemDataAsset` — data-only item: stable `ItemId`, type,
  cosmetic-only rarity, `MaxStackSize` (weapons/mission items
  1, Health/Shield 5, Ammo 200), consumable + use effect
  (+50 HP, +50 shield, +30 reserve rounds), `WeaponData` /
  `WeaponClass` for weapon items, `ShieldBonus` for shield
  generators. `Validate()` rejects bad ids, bad stacks,
  consumable/effect mismatches, weapon items without data.
- `UGFInventoryComponent` (on `AGFCharacter`, 24 slots) —
  partial-stack filling, `AddItem` returns the added count
  (overflow fires `OnInventoryFull`, nothing is dropped),
  `UseItem` applies the effect first and consumes second (a
  failed effect never eats the item), `ClearInventory` for
  save loads.
- `UGFEquipmentComponent` — 3 slots (Primary / Secondary /
  Shield Generator). Weapons must be owned *and* in the combat
  loadout (no silent spawns); the shield generator adds its
  bonus to `MaxShield` while equipped and clamps current shield
  on unequip. Equipping never removes the item from the
  inventory, so unequipping can never lose an item.
- `UGFCharacterCustomizationData` + `UGFCustomizationManager`
  (GameInstance subsystem) — cosmetic-only profiles (body
  Male/Female, hairstyle, armour set, helmet, accessories,
  4 colours). No stat field exists on a profile. The manager
  owns the active profile and looks it up by id for save loads;
  mesh swaps are Editor-side (Animation Blueprint reads the
  profile's ids).
- `AGFGroundVehicle` (Rover RX-7) — `UFloatingPawnMovementComponent`
  (no Chaos dependency), 1800 cm/s, 250 HP own health component,
  possession-based enter/exit; exit refused above 50 cm/s;
  the driver is hidden with collision off while driving and
  damage goes to the vehicle.
- `AGFSpaceship` (Starhopper) — `TravelToPlanet` validates the
  planet asset, checks the unlock against the save's completed
  missions, then a 5 s spool-up (real timer) and a real
  `OpenLevel` of the planet's level. No space combat in
  Phase 05 — the ship is transport only.
- `UGFPlanetDataAsset` — gravity 0.3–2.0, weather, level name,
  recommended level, unlock mission. `IsUnlocked` checks the
  completed-mission list. Unlock chain: ZERON (default) →
  NEXUS (The Lost Power Core) → VELTRON (Signal in the Static)
  → ASTRALIS (The Frozen Vault).
- `AGFPickupActor` (`IGFInteractable`) — grants via the
  inventory only; partial pickup allowed; a full inventory
  takes nothing and the pickup stays (prompt hidden);
  respawning pickups return after `RespawnSeconds` (min 5 s).
- `UGFProgressionComponent` — XP curve 100 × level
  (L1→2: 100, L2→3: 200, total to L5: 1000), cap 20, levels
  grant no stats. Mission rewards (XP + one item) granted
  exactly once per mission id on `OnMissionCompleted`; a full
  inventory still grants the XP, and the item is picked up
  from the mission's end-state pickup placed by the Level
  Blueprint (Docs/MISSIONS.md).
- `UGFSaveGame` + `UGFPlayerSaveManager` (slot `GF_PlayerSave`,
  version 1) — level/XP, completed missions, inventory,
  equipment, profile id, planet id, language reference copy.
  `Validate()` clamps bad level/XP, drops invalid inventory
  entries, de-duplicates missions; newer-version saves are
  refused; version-0 saves migrate once from the Phase 03
  `GF_MissionSave` slot. No passwords/secrets/tokens are ever
  stored. Settings stay in `GF_SettingsSave` (Phase 04).
- `AGFCharacter` — `InventoryComponent`, `EquipmentComponent`,
  `ProgressionComponent` subobjects + getters;
  `HandleOpenInventory` is a real implementation now
  (delegates to `UGFHUDController::ToggleInventory()`). **No
  Phase 01 stub remains.** `FGFCharacterCustomization` gains
  `BodyType` + `AccentColour` (metal/tech slot 2, applied only
  when the mesh has 3+ slots).
- `UGFHUDController` — `InventoryWidgetClass` / `InventoryWidget`,
  `OpenInventory` / `CloseInventory` / `ToggleInventory()`
  (pauses the game, stops firing, UIOnly input). The Pause menu
  and the Inventory screen never stack: opening one while the
  other is open is refused, so closing one never resumes a game
  the other paused.
- `UGFMissionDataAsset` — `CompletionXPReward` (default 100)
  + `CompletionRewardItem` (null = no item reward).

## Missions (Docs/MISSIONS.md)

- **Mission 2 — "Signal in the Static" (NEXUS):** 8 stages
  (LandNexus → ReachRelay → RestoreRelayPower → DefendRelay →
  RetrieveDataCore → ReachExtractionZone → CompleteMission →
  ShowResult). Reward: 250 XP + `NexusDataCore`. Completing it
  unlocks VELTRON.
- **Mission 3 — "The Frozen Vault" (VELTRON):** 9 stages
  (LandVeltron → CrossIceField → EnterVault → FindVaultKey →
  UnlockVault → DefeatGuardian → ReachExtractionZone →
  CompleteMission → ShowResult). Reward: 400 XP + `VaultAlloy`.
  Completing it unlocks ASTRALIS.
- ASTRALIS has no mission in Phase 05 (planet spec only).

## Planets (Docs/PLANETS.md)

- **NEXUS** (Relay City, IonStorm, gravity 1.0, level 3):
  neon ruins, relay spire, electrified puddles (10 HP/s in the
  Editor trigger volume), rover garage.
- **VELTRON** (Frozen Vault, Blizzard, gravity 0.9, level 5):
  ice field, buried vault, thin ice under the rover only.
- **ASTRALIS** (Living Jungle, AcidRain, gravity 1.1,
  level 8): bioluminescent jungle, HISM foliage, no mission yet.
- Hazards that would damage the player without a visible cause
  (VELTRON cold, ASTRALIS acid rain) are visual/audio only in
  Phase 05 — documented, not silently damaging.

## Tests

- `GFPhase05Test.cpp` — Automation test skeleton, spec values +
  25 failure cases listed (full inventory, use at full health,
  equip outside the loadout, double shield generator, rover
  exit while moving, locked-planet travel, XP carry-over,
  double mission reward, newer-version save refused, v0
  migration, pickup with a full inventory, inventory open/close
  pause behaviour). **NOT RUN** (no UE here). A real test needs
  a character in a world on macOS. Do not mark it PASSED.

## Errors found / fixed in review

- **Fixed:** `UGFEquipmentComponent::EquipShieldGenerator`
  logged `*ItemId.ToString()` where no `ItemId` variable exists
  (would not compile). Fixed to `*ItemData->ItemId.ToString()`.
- **Fixed:** `UGFEquipmentComponent::EquipWeapon` broadcast the
  slot name through a convoluted comparison of the slot ref
  against the primary id. Simplified to a direct slot check.
- **Fixed:** `UGFPlayerSaveManager` called
  `UGFSettingsManager::GetLanguageCode()`, which does not exist.
  Fixed to `GetSettings()->LanguageCode` with a null check
  (the settings slot stays authoritative).
- **Fixed:** `GFPhase05Test` case 3 said 12 Health Packs add 10
  (two stacks of 5). 12 = 5+5+2 in three stacks — comment fixed.
- **Fixed:** `UGFItemDataAsset::Validate` used
  `UEnum::GetValueAsString` for a log line. Replaced with a
  plain log line.
- **Fixed:** an edit to `UGFHUDController.cpp` accidentally
  deleted `ClosePauseMenu()` while inserting the Inventory
  functions. Restored, and added mutual-exclusion guards:
  `OpenPauseMenu` refuses while the inventory is open and
  `OpenInventory` refuses while paused, so exactly one screen
  owns the pause state at a time.
- **Fixed:** `AGFCharacter::ApplyCustomization` applied the
  accent colour to slot 2 unconditionally. Guarded with
  `GetNumMaterials() > 2` — a mesh with fewer slots gets no
  accent pass instead of touching a missing slot.
- **Fixed:** `UGFItemDataAsset::WeaponClass` was
  `TSubclassOf<AActor>`. Tightened to
  `TSubclassOf<AGFWeaponBase>`.
- **Fixed:** unused `#include "World/UGFPlanetDataAsset.h"` in
  `UGFPlayerSaveManager.cpp`. Removed.
- **Checked, no change:** every declared method in the 12 new
  headers has a definition in its `.cpp` (script-checked; the
  only hits were `FText::GetEmpty()` false positives); every
  project-relative `#include` resolves to a file in the repo;
  braces/parens balance in all 25 new files; `GetGameInstance()`
  on `UActorComponent` is null-safe in `EndPlay`.

## NOT IMPLEMENTED

- All UMG widgets as `.uasset` files (`WBP_Inventory`,
  `WBP_CharacterCustomization` included) — full specs in
  `Docs/UI-SCREENS.md` only; built in the Editor on macOS.
- All `DA_*` Data Asset instances (items, profiles, planets,
  missions 2/3) — created in the Editor from the specs in
  `Docs/EXPANSION.md` / `Docs/PLANETS.md` / `Docs/MISSIONS.md`.
- All `.umap` levels (`L_NEXUS_Prototype`, `L_VELTRON_Prototype`,
  `L_ASTRALIS_Prototype`) — specs in `Docs/PLANETS.md` only.
- All Skeletal/Static Mesh binaries (characters, rover,
  spaceship, pickups) — specs only, authored in the Editor.
- ASTRALIS mission (planet spec + travel target only).
- Enemy-kill XP, rover repair, Chaos vehicle physics, crafting
  (`VaultAlloy` / `ScrapMetal` have no use yet), autosave timer,
  branching / timed missions — see `Docs/EXPANSION.md`.
- Live audio volumes (SoundMix), full localisation data,
  iOS thermal reader — carried over from Phase 04.

## Changes to Phases 00–04 files (only linking changes)

- `AGFCharacter.h/.cpp` — 3 new components + getters,
  `HandleOpenInventory` real, `BodyType` + `AccentColour`.
  No movement, combat, AI, mission or interaction logic touched.
- `UGFHUDController.h/.cpp` — Inventory widget + open/close,
  Pause/Inventory mutual exclusion. No HUD binding logic touched.
- `UGFMissionDataAsset.h` — 2 new reward fields, defaults only.
- `Docs/MISSIONS.md`, `Docs/UI-SCREENS.md`, `Docs/CHARACTERS.md`,
  `Docs/RELIABILITY.md`, `Docs/ROADMAP.md`, `README.md`,
  `CHANGELOG.md` — Phase 05 sections / notes only.
- No Phase 00–04 gameplay code, weapon, AI, mission logic,
  interaction, settings or performance file was modified.

## Build / test status

- **BUILT: NONE — BLOCKED.** No Unreal Engine on this Linux VM.
- **TESTED: NONE — BLOCKED.** No UE, no device, no Editor run.
- **IPA: BLOCKED.** macOS + Xcode + iOS SDK + signing required.
  (Workflow `.github/workflows/build-ios-ipa.yml` is on GitHub
  at commit `c62d890d`; this agent did not touch `.github/`.)
- **Device testing: BLOCKED.** No iPhone/iPad here.
- **`.uproject` still parses as JSON** (verified live).

## Delivery

- ZIP: `GalaxyFrontier-Phase-05-Expansion.zip` (in
  `~/workspace/galaxy-frontier/`). Verified with `unzip -l` and
  `unzip -t` — see sizes/counts in the parent delivery report.
- Refreshed: `GalaxyFrontier-All-Phases.zip` and
  `GalaxyFrontier-All-Phases-Source.zip` (overwrite the old ones).
- GitHub push: NOT DONE by this agent.

## Next step

**Phase 06 — Multiplayer:** dedicated server, lobby, sessions,
matchmaking, replication, server-authoritative validation,
anti-cheat design. Two-player test first, then scale. Battle
Royale only after this base passes. Then the IPA build on a
self-hosted Mac + device testing (Phases 04–05 exit criteria).

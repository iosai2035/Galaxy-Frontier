# Planets — Galaxy Frontier (Phase 05)

Four planets. ZERON is playable since Phase 03; NEXUS, VELTRON
and ASTRALIS are defined here (data + level specs). **No `.umap`
exists in this repo** — each `L_<NAME>_Prototype` level is built
in the Unreal Editor on macOS (World Partition, ~400×400 m
prototype area first, same rule as `Docs/WORLD-ZERON.md`).
Planet data lives in `UGFPlanetDataAsset` instances created in
the Editor (e.g. `DA_Planet_NEXUS`); this doc is their spec.

## Unlock chain (one rule, no exceptions)

| Planet | Unlocked by | Recommended level |
|---|---|---|
| ZERON | Default (start planet) | 1 |
| NEXUS | Complete "The Lost Power Core" (ZERON) | 3 |
| VELTRON | Complete "Signal in the Static" (NEXUS) | 5 |
| ASTRALIS | Complete "The Frozen Vault" (VELTRON) | 8 |

Unlock is checked by `UGFPlanetDataAsset::IsUnlocked()` against
`UGFSaveGame::CompletedMissionIds`. Level is a recommendation
only — travel is never blocked by level, only by the unlock.
Travel itself is the spaceship (`AGFSpaceship::TravelToPlanet`),
which loads the planet's level by name.

## ZERON (recap — full spec in Docs/WORLD-ZERON.md)

- **Level:** `L_ZERON_Prototype`. Rocky desert, abandoned
  outpost, dust storms. Gravity 1.0, weather DustStorm.
- **Mission:** "The Lost Power Core" (9 stages, Phase 03).
- **Planet data:** `DA_Planet_ZERON` — `bUnlockedByDefault = true`,
  `RecommendedLevel = 1`, `AvailableMissionIds = ["TheLostPowerCore"]`.

## NEXUS — the Relay City

- **Level:** `L_NEXUS_Prototype` (~400×400 m prototype: relay
  district + plaza + two hab blocks).
- **Environment:** dead neon city — cracked ferroconcrete towers,
  dead holo-signs, overgrown transit rails, a central relay spire.
  Night palette with amber/cyan accent lights (2 dynamic lights
  max on High, per the Phase 04 budget). Rain-slick streets
  (roughness 0.25–0.4, no mirrors — no planar reflections on iOS).
- **Gravity:** 1.0. **Weather:** IonStorm (intermittent: sky
  flashes, HUD static flicker, no gameplay effect on the player
  — storms are visual + audio only in Phase 05).
- **Hazards:** electrified puddles near the relay (damage zones,
  10 HP/s while standing in them — reuse `UGFHealthComponent`
  via a trigger volume in the Editor, no new code in Phase 05);
  patrolling guards (Phase 02 AI, same stats).
- **Points of interest:** Relay Station (mission), Plaza Market
  (3 pickups: Health Pack, Ammo Box, Shield Cell), Hab Block A
  (rover garage — one `AGFGroundVehicle` spawn), Spire Base
  (extraction pad).
- **Mission:** "Signal in the Static" (6 stages, Docs/MISSIONS.md).
- **Planet data:** `DA_Planet_NEXUS` — `GravityScale = 1.0`,
  `Weather = IonStorm`, `RecommendedLevel = 3`,
  `UnlockMissionId = "TheLostPowerCore"`,
  `AvailableMissionIds = ["SignalInTheStatic"]`.

## VELTRON — the Frozen Vault

- **Level:** `L_VELTRON_Prototype` (~400×400 m prototype: ice
  field + vault entrance + glacier ridge).
- **Environment:** frozen ocean plain, blue-white ice with
  crevasses, a buried pre-collapse vault (the only interior in
  this prototype). Fog density high; draw distance short
  (fits the Low/Medium budgets — see Docs/PERFORMANCE.md).
- **Gravity:** 0.9 (slightly floaty jumps — applied by the Level
  Blueprint to the character's `JumpZVelocity` × 1.1 and gravity
  scale; the value is data in the planet asset).
- **Weather:** Blizzard (visibility pulses, wind audio).
- **Hazards:** thin ice (breaks under the rover — rover-only
  paths are marked; on foot the player is safe), cold zones
  outside the vault approach (no HP drain in Phase 05 — the
  cold is visual + a HUD warning line only, to avoid a hidden
  damage source the player cannot see).
- **Points of interest:** Ice Field crossing (rover recommended),
  Vault Entrance (mission), Glacier Ridge (sniper sightline —
  one Sniper pickup), Supply Cache (2 pickups).
- **Mission:** "The Frozen Vault" (7 stages, Docs/MISSIONS.md).
- **Planet data:** `DA_Planet_VELTRON` — `GravityScale = 0.9`,
  `Weather = Blizzard`, `RecommendedLevel = 5`,
  `UnlockMissionId = "SignalInTheStatic"`,
  `AvailableMissionIds = ["TheFrozenVault"]`.

## ASTRALIS — the Living Jungle

- **Level:** `L_ASTRALIS_Prototype` (~400×400 m prototype:
  jungle clearing + river + canopy walk).
- **Environment:** dense alien jungle — bioluminescent flora,
  a slow river, canopy platforms. The most expensive planet
  visually: foliage uses HISM instancing (Docs/PERFORMANCE.md)
  and the prototype area is kept small so the High budget
  (150 draw calls / 500k polys) still holds.
- **Gravity:** 1.1 (heavier jumps — `JumpZVelocity` × 0.95).
- **Weather:** AcidRain (visual + audio only in Phase 05;
  no HP drain — same rule as VELTRON's cold).
- **Hazards:** none that damage in Phase 05. Spore clouds are
  visual only.
- **Points of interest:** Jungle Clearing (landing pad),
  River Crossing, Canopy Walk (vertical sightlines).
- **Mission:** none in Phase 05. ASTRALIS is a travel target
  and an exploration area; its mission is Phase 05 follow-up
  work (listed in Docs/PHASE-05-REPORT.md, NOT IMPLEMENTED).
  Do not invent a mission for it in the Editor until it is
  written in Docs/MISSIONS.md.
- **Planet data:** `DA_Planet_ASTRALIS` — `GravityScale = 1.1`,
  `Weather = AcidRain`, `RecommendedLevel = 8`,
  `UnlockMissionId = "TheFrozenVault"`,
  `AvailableMissionIds = []` (empty until its mission exists).

## Editor setup (macOS, per planet)

1. Create the planet Data Asset (`UGFPlanetDataAsset`) with the
   values above.
2. Build `L_<NAME>_Prototype` (~400×400 m) with the points of
   interest listed above; place pickups (`AGFPickupActor`) and,
   on NEXUS, one rover (`AGFGroundVehicle`).
3. Level BeginPlay: start the planet's mission with
   `UGFMissionSubsystem::StartMission()` (same pattern as ZERON,
   Docs/MISSIONS.md), and apply `GravityScale` to the player
   character's movement (document the exact values in the level
   notes — the asset only stores the scale).
4. Register the planet in the spaceship's travel list (the ship
   reads planet assets, not hard-coded names).
5. Do not ship a planet whose level is not built: an asset
   without its `L_<NAME>_Prototype` level fails
   `UGFPlanetDataAsset::Validate()` only on the missing
   LevelName — the level asset itself is checked at travel time
   by the load failing loudly, never silently.

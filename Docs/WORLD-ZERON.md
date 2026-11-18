# Planet ZERON — Galaxy Frontier (Phase 03)

First playable world. **Spec only in this phase — no `.umap`
binary is in this repo.** The level `L_ZERON_Prototype` is created
in the Unreal Editor on macOS (see "Editor creation" below), as a
small test area first. Do not build a huge world before the systems
in Phases 00–03 are proven on it.

## Planet data

| Field | Value |
|---|---|
| Planet ID | `ZERON` |
| Name | ZERON |
| Environment | Rocky, alien terrain; thin atmosphere |
| Gravity | ~0.9 g (slightly lighter than Earth — jump feels a touch floaty; do not hard-code Earth gravity) |
| Atmosphere | Thin, dusty; long-distance haze |
| Weather | Dust storms (Niagara, Phase 04 tuning) |
| Mission set | The Lost Power Core (Phase 03) |
| Level | `L_ZERON_Prototype` |

Future planets (data only, not built): NEXUS, VELTRON, ASTRALIS
(Phase 05). No planet is loaded at the same time as another —
Level Streaming / World Partition per the level's size, decided
when the level exists (see "Streaming" below).

## Layout — L_ZERON_Prototype (limited test area, ~400 × 400 m)

```
                    [Extraction Zone]
                           |
  [Landing Zone] ——— [Outpost] ——— [Abandoned Station]
  (spawn)              (hub)          (enterable building)
                                          |
                                    [Power Core room]
                                          |
                                    [Guard enemies]
```

- **Landing Zone:** flat spawn pad, player spawns here facing the
  Outpost. Mission stage 1 completes on landing.
- **Outpost:** small hub, 2–3 enterable rooms, a terminal
  (`AGFInteractableActor`, type Device) and a crate.
- **Abandoned Station:** the main enterable building. Corridor →
  Power Core room. One guard enemy patrols inside, one outside.
  Doors are `AGFInteractableActor` (type Door, reusable).
- **Rocky areas:** instanced rock meshes around the edges (Instanced
  Static Meshes — never hundreds of individual actors), blocking
  the player from leaving the test area.
- **Tech installations:** 2–3 power stations / comm devices
  (`AGFInteractableActor`, type PowerStation / CommDevice) as
  scenery + interaction targets. The Power Core itself is a
  `MissionItem` interactable with a hold-to-interact duration.
- **Extraction Zone:** marked pad at the far end. Entering it with
  the core activated completes stage 7.
- **Collectibles:** health kits / ammo (Phase 05 inventory items)
  are NOT placed yet — the crates are interactable shells only.

## Performance budget for this level (targets, to measure)

- Draw calls: ≤ 150 on High, ≤ 80 on Low.
- Polygons on screen: ≤ 500k High, ≤ 200k Low.
- Texture memory: ≤ 512 MB High, ≤ 256 MB Low.
- Dynamic lights: ≤ 2 (1 directional sun + 1 interior point light).
  No per-room dynamic light spam.
- Shadows: 1 directional shadow only; interior shadows baked.
- Nanite: use for rocks / station hulls only where UE 5.5 on iOS
  supports it — otherwise traditional LODs (see below).
- Lumen: **not used on iOS** in this project. UE 5.5 Lumen is
  desktop-first; on iOS use baked lighting + a single dynamic
  directional light. Do not enable Lumen for the iOS build and
  expect it to work.

## Streaming

- `L_ZERON_Prototype` is one persistent level + streamed
  sub-levels only if it grows beyond the test area. World
  Partition is NOT enabled for the prototype — it adds
  complexity the test area does not need. Revisit in Phase 05
  when planets are added.

## Editor creation steps (macOS, Unreal Editor)

1. Create level `L_ZERON_Prototype` (File → New Level → Empty).
2. Landscape / terrain: sculpt a rocky heightmap, paint rock +
   dust materials. Keep the playable area ~400 × 400 m.
3. Place the Landing Zone pad, Outpost (modular wall/floor
   meshes), Abandoned Station (modular interior), Extraction pad.
4. Place a NavMesh Bounds Volume covering the whole playable
   area (AI cannot patrol/chase without it — see Docs/AI.md).
5. Place 2 guard enemies (`AGFEnemyCharacter`, AIControllerClass
   `AGFEnemyAIController`), 1 Power Core actor
   (`AGFInteractableActor` + `UGFObjectiveComponent`,
   ObjectiveId `FindPowerCore`), 1 extraction trigger
   (`UGFObjectiveComponent`, `ReachExtractionZone`).
6. Place the mission data asset `DA_TheLostPowerCore`
   (see Docs/MISSIONS.md) and set it as the level's mission
   (GameMode / Level Blueprint calls
   `UGFMissionSubsystem::StartMission` on BeginPlay).
7. Light: 1 directional light + SkyAtmosphere, bake lighting.
   Set the level as the project's default map in
   `Config/DefaultEngine.ini` only after it exists in the Editor.

## What is NOT done

- No `.umap`, no landscape, no meshes, no lighting in this repo.
- No dust-storm Niagara, no ambient audio (Phase 04).
- No second planet, no vehicles, no spaceship (Phase 05).

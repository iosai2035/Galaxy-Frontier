# Performance & Optimization — Galaxy Frontier (Phase 04)

Performance is a budget + measurement discipline, not a claim.
Every number below is a **target to measure** on a real device
(Phase 04 device testing, `Docs/DEVICE_TEST_MATRIX.md`). No
fps is promised for any device in this repo.

## Budgets (per quality tier)

From `Docs/QUALITY.md` rule 21 and `Docs/DEVICE_TEST_MATRIX.md`;
`UGFPerformanceManager::GetBudgetForQuality()` returns these
exact values in code, so the docs and the code cannot drift.

| Budget | High | Medium | Low |
|---|---|---|---|
| Draw calls | ≤ 150 | ≤ 120 | ≤ 80 |
| Polygons on screen | ≤ 500k | ≤ 350k | ≤ 200k |
| Texture memory | ≤ 512 MB | ≤ 384 MB | ≤ 256 MB |
| Dynamic lights | ≤ 2 | ≤ 1 | ≤ 1 |
| Shadow-casting lights | 1 directional | 1 directional | 0 |
| Character texture | 2048 px | 1024 px | 512 px |
| Streaming pool (`r.Streaming.PoolSize`) | 512 MB | 256 MB | 256 MB |

FPS targets: High 60, Medium 30, Low 30. "60 (target)" means
the profile *starts* at High and the test must measure whether
60 fps is actually held — if it is not, the profile drops.

## What is measured, and what is not

- **Measured live on device** by `UGFPerformanceManager`
  (0.5 s timer, never per-frame): average FPS (exponential
  moving average), lowest FPS since sampling started, used/total RAM
  (`FPlatformMemory::GetStats()`). Shown in the diagnostics
  overlay when Settings `bShowDiagnostics` is true.
- **Measured in the Editor only:** draw calls, polygons,
  texture memory, shader complexity, overdraw — with
  `stat RHI`, `stat SceneRendering`, `stat Streaming` and
  Unreal Insights, on the macOS build machine. The game
  code does not read these counters at runtime, and the
  diagnostics overlay deliberately shows none of them.
- **NOT measured in this build:** thermal state. The real
  iOS value comes from `NSProcessInfo.thermalState` and needs
  iOS platform code on macOS (NOT IMPLEMENTED). The metrics
  struct carries `EGFThermalState::Unknown` until that reader
  exists — a thermal reading must never be invented.
- **Never claimed:** 60 fps on all devices, battery drain
  figures, or load-time figures without a device run.

## Automatic quality (hysteresis)

`UGFPerformanceManager` steps the live quality down and back
up. The saved quality in `UGFSettingsSaveGame` is the player's
chosen **ceiling**: auto step-down never rewrites it and never
steps above it.

- Step down: avg FPS stays below (target − 5) for
  `DowngradeAfterSeconds` = 5 s → one tier down.
- Step up: avg FPS stays above (target + 10) for
  `UpgradeAfterSeconds` = 10 s → one tier up, max the ceiling.
- Dead band: between (target − 5) and (target + 10) neither
  timer accumulates — a borderline FPS cannot flip the
  quality back and forth.
- Cooldown: `ChangeCooldownSeconds` = 10 s minimum between
  any two changes, either direction.
- Disabled entirely when Settings `bAutoQualityEnabled` is
  false; `RestoreUserQuality()` returns to the ceiling at any
  time (Settings screen "Restore" line).
- Every change broadcasts `OnQualityChanged` (new quality +
  reason string) and is logged — the reason is visible in the
  diagnostics overlay as "(auto-lowered)".

Worked example: High, target 60. FPS 52 for 5 s → Medium
(target 30 now; 52 is above 40, so no further step-down).
FPS 45 for 10 s on Medium → stays Medium (45 > 40, inside
the dead band). FPS 25 for 5 s → Low. FPS 120 for 10 s on
Low → Medium (never above the High ceiling without the
player choosing it).

## LODs (LOD0–3)

- Characters: LOD0 full (face + hands + cloth), LOD1 ~50%
  triangles, LOD2 ~25%, LOD3 billboard/simple mesh. Screen
  sizes set in the Editor per mesh (see `Docs/CHARACTERS.md`);
  `AGFCharacter::ApplyForcedLOD()` exists for testing one LOD
  at a time — it is a test hook, not a shipping setting.
- Environment (ZERON rocks, station hulls): LOD0–3 with the
  same screen-size policy; distant rocks drop to LOD2/3 first
  because they are the cheapest win.
- Enemies: same LOD chain as the player character, one tier
  lower at the same distance (enemies are smaller on screen).
- Rule: no mesh ships with LOD0 only. A mesh without LODs is
  a review failure — it must be fixed in the Editor before
  the device test, not worked around in code.

## Texture compression (iOS)

- iOS uses ASTC (`ASTC_RGBAuto` / platform default in the
  Editor texture settings). Character body: 2048 px High /
  1024 Medium / 512 Low (per-quality cap applied by the
  scalability group, not by shipping three texture sets).
- No texture may exceed 2048 px on mobile. UI textures are
  kept small and atlased where possible; the HUD uses
  vector-ish flat colours + text, not large images.
- Streaming pool is budgeted per tier (table above) and set
  by both `DefaultDeviceProfiles.ini` and the performance
  manager — the two must agree; if they disagree, the
  manager's value wins at runtime and the mismatch is a bug
  to fix in the `.ini`, not in code.

## Instancing

- Repeated environment pieces (rocks, crates, station panels,
  foliage tufts) use Instanced Static Meshes / Hierarchical
  Instanced Static Meshes (HISM) — one draw call per instance
  batch, not one per rock. The ZERON level spec
  (`Docs/WORLD-ZERON.md`) places rocks/crates as HISM actors;
  placing 200 rocks as 200 individual Static Mesh actors is a
  review failure.
- Foliage (if any) uses the foliage tool with HISM under the
  hood; `r.FoliageQuality` is 0 / 1 / 2 per tier.
- Characters, weapons and interactables are NOT instanced —
  they are few, animated and individually stateful.

## Lighting / shadow budget

- **Lumen is NOT used on iOS** (desktop-first; see
  `DefaultScalability.ini`). iOS lighting is baked lightmaps
  + one dynamic directional light (sun) with CSM shadows.
- Dynamic lights ≤ 2 (High) / ≤ 1 (Medium/Low); only the
  directional light casts shadows. Muzzle flash, terminals
  and the Power Core glow are emissive materials / point
  lights without shadows — a second shadow-casting light is
  a budget breach.
- `r.ShadowQuality` 2 / 1 / 0 per tier; `r.MobileMSAA` 4 on
  High, 2 otherwise; `r.MobileContentScaleFactor` 1.0, except
  0.8 on Low (`DefaultScalability.ini`).

## Scalability per device

`Config/DefaultScalability.ini` holds the Low/Medium/High
groups; `Config/DefaultDeviceProfiles.ini` picks the starting
group per device class (iPhone → Low, iPhone Pro → High,
iPad → Medium, iPad Pro → High — all *starting* assignments,
revised from measurements). `UGFPerformanceManager` then
moves the live level within the ceiling. The three layers
must agree on what "High" means — the budget table above is
the single source of truth.

## Measurement protocol (device test, macOS + iPhone/iPad)

1. Build the Development IPA via the SideStore workflow,
   install, and play `L_ZERON_Prototype` for 15 minutes
   (sustained FPS counts, not a 30-second peak).
2. Record per device: avg FPS, 1% low FPS, RAM, temperature
   (by hand / Xcode Instruments — the in-game thermal field
   is Unknown until the iOS reader lands), load time, crashes,
   touch issues. Write them into
   `Docs/DEVICE_TEST_MATRIX.md` — the FPS/RAM/thermal
   columns are empty until this is done.
3. In the Editor on the same content: `stat RHI` (draw calls,
   polygons, texture memory), `stat Streaming`, Unreal Insights
   for CPU/GPU split. If draw calls exceed the tier budget,
   fix the content (HISM, LODs, merged meshes) — do not raise
   the budget to match the content.
4. If a High-profile device throttles (heat) during the
   15-minute session, its profile is downgraded — sustained
   FPS counts, not a peak.
5. **PASS needs a real run on that device, with numbers
   recorded.** Nothing in this repo is marked PASS before then.

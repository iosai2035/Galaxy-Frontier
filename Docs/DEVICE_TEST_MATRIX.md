# Device Test Matrix — Galaxy Frontier

No device is marked as tested until it has actually been tested.
All rows below are **NOT TESTED** (2026-11-18, Linux VM, no devices).

Record per device, per build: FPS (avg / 1% low), RAM, temperature,
load time, crashes, touch issues, build version, date, tester.

| Device | iOS | Target FPS | Quality | FPS | RAM | Temp | Load | Crashes | Touch | Status |
|---|---|---|---|---|---|---|---|---|---|---|
| iPhone (older, e.g. 12/13) | — | 30 | Low | — | — | — | — | — | — | NOT TESTED |
| iPhone (current) | — | 30–60 | Medium | — | — | — | — | — | — | NOT TESTED |
| iPhone Pro (current) | — | 60 (target) | High | — | — | — | — | — | — | NOT TESTED |
| iPad (base) | — | 30 | Medium | — | — | — | — | — | — | NOT TESTED |
| iPad Air | — | 30–60 | Medium | — | — | — | — | — | — | NOT TESTED |
| iPad Pro (M-series) | — | 60 (target) | High | — | — | — | — | — | — | NOT TESTED |

"60 (target)" means: the device profile starts at High and the
test must measure whether 60 fps is actually held. If it is not,
the profile drops to Medium — the target does not change the
measurement. No device is promised 60 fps.

## Performance budgets (targets, to measure in Phase 04)

| Budget | High | Medium | Low |
|---|---|---|---|
| Draw calls | ≤ 150 | ≤ 120 | ≤ 80 |
| Polygons on screen | ≤ 500k | ≤ 350k | ≤ 200k |
| Texture memory | ≤ 512 MB | ≤ 384 MB | ≤ 256 MB |
| Dynamic lights | ≤ 2 | ≤ 1 | ≤ 1 |
| Shadow-casting lights | 1 directional | 1 directional | 0 |
| Texture size (character body) | 2048 px | 1024 px | 512 px |
| Streaming pool (`r.Streaming.PoolSize`) | 512 MB | 256 MB | 256 MB |

## What to measure

- CPU, GPU, RAM, heat, battery drain, map load time, app size.
- Draw Calls, Polygons, Texture Memory, Shader Complexity, Overdraw.
- Touch: joystick, camera drag, multi-touch, Safe Areas, all screen sizes.
- Realistic characters: LOD transitions, skin/cloth material cost,
  facial rig cost — record per-LOD triangle counts in the Editor.

## Rules

- Do not claim 60 fps on all devices. Set quality / fps from measured
  data per device, via `DefaultDeviceProfiles.ini` + Scalability groups.
- Do not claim compatibility with an iOS version that was not tested.
- A "PASS" needs a real run on that device, with numbers recorded here.
- If a High-profile device throttles (heat) during a 15-minute
  session, its profile is downgraded — sustained FPS counts, not
  a 30-second peak.

## Phase 04 — measurement protocol (2026-11-18)

- In-game diagnostics: enable Settings `bShowDiagnostics` — the HUD
  shows measured avg FPS (EMA), window-min FPS, used RAM in MB and
  the live quality tier, with "(auto-lowered)" when
  `UGFPerformanceManager` stepped below the chosen quality. It shows
  **no** draw-call / polygon numbers — those come from the Editor
  (`stat RHI`, Unreal Insights) on the macOS build machine.
- Thermal state: the in-game field is `Unknown` in this build
  (the iOS `NSProcessInfo.thermalState` reader is NOT IMPLEMENTED).
  Record temperature by hand / Xcode Instruments during the
  15-minute session, using these states: Nominal / Fair / Serious /
  Critical. Never copy the in-game "Unknown" into this table as a
  measurement.
- Session: 15 minutes of play on `L_ZERON_Prototype` (combat +
  mission stages), Development build via the SideStore workflow.
  Record avg FPS, 1% low FPS, RAM, thermal state, load time,
  crashes, touch issues, build version, date, tester.
- Auto-quality check: on a device that cannot hold its tier, the
  live quality must step down exactly once per 5 s of low FPS,
  never oscillate, and never rise above the quality chosen in
  Settings — see `Docs/PERFORMANCE.md` (hysteresis) and the
  failure cases in `GFPhase04Test.cpp`.
- All FPS / RAM / Temp cells below are still empty — no device
  run has happened yet. Fill them only from a real run.

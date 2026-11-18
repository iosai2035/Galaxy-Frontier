# Device Test Matrix — Galaxy Frontier

No device is marked as tested until it has actually been tested.
All rows below are **NOT TESTED** (2026-11-18, Linux VM, no devices).

Record per device, per build: FPS (avg / 1% low), RAM, temperature,
load time, crashes, touch issues, build version, date, tester.

| Device | iOS | Target FPS | Quality | FPS | RAM | Temp | Load | Crashes | Touch | Status |
|---|---|---|---|---|---|---|---|---|---|---|
| iPhone (older, e.g. 12/13) | — | 30 | Low | — | — | — | — | — | — | NOT TESTED |
| iPhone (current) | — | 60 | Medium | — | — | — | — | — | — | NOT TESTED |
| iPhone Pro (current) | — | 60 | High | — | — | — | — | — | — | NOT TESTED |
| iPad (base) | — | 30 | Low | — | — | — | — | — | — | NOT TESTED |
| iPad Air / Pro | — | 60 | Medium/High | — | — | — | — | — | — | NOT TESTED |

## What to measure

- CPU, GPU, RAM, heat, battery drain, map load time, app size.
- Draw Calls, Polygons, Texture Memory, Shader Complexity, Overdraw.
- Touch: joystick, camera drag, multi-touch, Safe Areas, all screen sizes.

## Rules

- Do not claim 60 fps on all devices. Set quality / fps from measured
  data per device, via `DefaultDeviceProfiles.ini` + Scalability groups.
- Do not claim compatibility with an iOS version that was not tested.
- A "PASS" needs a real run on that device, with numbers recorded here.

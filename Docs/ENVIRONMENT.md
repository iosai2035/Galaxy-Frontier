# Environment — Galaxy Frontier (verified live 2026-11-18)

Checked on this Linux VM (`Linux htch-runtime 7.0.0-44-generic x86_64`,
Ubuntu-based). Commands run: `which <tool>`, `ls /opt`, `git --version`,
`python3 --version`, `unzip -v`, `uname -a`.

## Available

| Tool | Version found |
|---|---|
| git | 2.43.0 |
| Python 3 | 3.12.3 |
| g++ / gcc | /usr/bin/g++ / /usr/bin/gcc (version not pinned — not used for UE) |
| cmake | /usr/bin/cmake |
| unzip | UnZip 6.00 (20 April 2009) |
| Disk (workspace) | 140 GB free of 150 GB |

## Missing — BLOCKED

| Tool | Result | Impact |
|---|---|---|
| Unreal Engine / UnrealEditor / UE5 | NOT FOUND (`which unreal`, `which UnrealEditor`, `which UE5` all empty; no UE dir in `/opt`) | No UE build, no Editor, no Automation tests, no .uasset creation |
| Xcode / xcodebuild | NOT FOUND | No iOS build, no IPA, no signing |
| macOS | Not present (Linux only) | IPA can only be built on macOS |
| clang / clang++ | NOT FOUND | UE C++ toolchain absent |
| dotnet / mono | NOT FOUND | UBT dotnet host absent |
| ninja | NOT FOUND | — |
| Blender | NOT FOUND | No mesh authoring here; assets must be made on a workstation |
| Substance 3D / Audacity | NOT FOUND | — |

`/opt` contains only: `google`, `hatch`, `hatch-image`, `hatch-private`,
`meta-chromium`, `muse`. No Epic Games / Unreal directory.

## Consequences (do not work around by claiming otherwise)

- **UE build: BLOCKED.** No compile, no Editor open, no cook.
- **IPA build: BLOCKED.** Requires macOS + Xcode + iOS SDK + UE iOS support + Apple signing.
  A GitHub Actions path now exists (`.github/workflows/
  build-ios-ipa.yml`, macos-15, unsigned IPA for SideStore) but it
  has **never run** — and a standard GitHub-hosted runner cannot
  hold UE 5.5 (14 GB disk vs 40–60 GB engine). See
  `Docs/BUILD-IPA-SIDESTORE.md` for the full feasibility numbers.
- **Signing / provisioning: BLOCKED.** No certificates on this VM and none
  may be committed to the repo (see `.gitignore`). The SideStore
  path deliberately uses an **unsigned** IPA; SideStore applies the
  user's own free Apple ID signature on the phone (7 days, 3 apps).
- **Device testing: BLOCKED.** No iPhone / iPad available here.
- **Automation tests: NOT RUN.** `GFCharacterMovementTest.cpp` is a
  skeleton; it has never executed.
- **GitHub repo state (verified live 2026-11-18 via the connected
  account):** `iosai2035/Galaxy-Frontier`, branch `main`, HEAD
  `d1f8965f` ("Fix README: record Phase 00+01 push commit"), with
  `86202c1f` (Phases 00+01) before it. The repo was empty when
  first verified; Phases 00+01 have since been pushed. The
  SideStore workflow and `Docs/BUILD-IPA-SIDESTORE.md` in this
  working copy are **not yet pushed**.

## GitHub Actions IPA path (2026-11-18, NOT RUN)

- Workflow: `.github/workflows/build-ios-ipa.yml` — `macos-15`
  (NOT `macos-14`: retired 2026-11-02), Xcode 16.x, UE 5.5,
  `RunUAT BuildCookRun -platform=IOS -NoCodeSign`, unsigned `.ipa`
  uploaded as an Actions artifact (7-day retention).
- Standard runner limits verified against GitHub's official docs
  2026-11-18: macos-15 arm64 = 3 vCPU / 7 GB RAM / **14 GB SSD**;
  job cap 6 h; cache cap 10 GB/repo; Free artifact storage 500 MB.
  UE 5.5 is 40–60 GB installed, so a standard runner is **not
  feasible** — the workflow fails fast on a missing engine / low
  disk instead of attempting a doomed build.
- Realistic routes: self-hosted Mac with UE 5.5 pre-installed
  (recommended), or a paid runner whose disk is verified larger
  from a real `df -h` run. Full analysis:
  `Docs/BUILD-IPA-SIDESTORE.md`.

## What to do on the build machine (macOS)

1. Install UE 5.5 with iOS support, a compatible Xcode + iOS SDK.
2. Clone the repo, open `GalaxyFrontier.uproject`, generate Xcode files.
3. Create the Input Actions / Mapping Context in `Docs/INPUT.md` and the
   `WBP_MobileHUD` widget in `Docs/MOBILE-HUD.md` in the Editor.
4. Run the Automation test, record the real result in the Phase report.
5. Only then change any status from BLOCKED / NOT RUN to BUILT / TESTED.

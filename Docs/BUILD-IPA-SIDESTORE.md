# Build an IPA for SideStore — Galaxy Frontier

**Status: NOT BUILT / NOT TESTED — 2026-11-18.**
No IPA exists. The workflow in
`.github/workflows/build-ios-ipa.yml` has never run. Nothing in this
document claims a successful build, a working install, or a playable
game on a phone.

## Verdict, in one paragraph

**Do not attempt an IPA build on a standard GitHub-hosted runner
right now — it is expected to fail, on disk alone, before it ever
compiles anything.** A standard macOS runner has 14 GB of SSD in
total; Unreal Engine 5.5 needs 40–100 GB installed. The workflow is
written to fail fast with a clear error instead of burning a 6-hour
run. The realistic routes are a self-hosted Mac with UE 5.5
pre-installed, or a paid runner with a genuinely larger disk (verify
its free space first — see below, the published macOS larger-runner
table still lists 14 GB). And even if a build succeeded today, the
game is not playable yet: Phases 00–01 have a character and no map,
no mission, no weapons and no mobile HUD (see "What an IPA built
today would actually contain").

## The numbers

All figures below were checked on 2026-11-18; sources are listed at
the end. Re-check them before spending money — GitHub and Epic
change these numbers.

| Item | Figure | Source |
|---|---|---|
| Standard macOS runner, `macos-15` (arm64, M1) | 3 vCPU, 7 GB RAM, **14 GB SSD** | GitHub-hosted runners reference (official docs, read 2026-11-18) |
| Standard macOS runner, `macos-15-intel` | 4 vCPU, 14 GB RAM, **14 GB SSD** | Same official reference |
| GitHub-hosted job limit | **6 hours** per job; workflow 35 days | GitHub Actions limits (official docs, read 2026-11-18) |
| GitHub Actions cache | **10 GB per repository**, LRU-evicted, entries unused for 7 days deleted | GitHub Actions limits (official docs) |
| Artifact storage, Free plan | **500 MB** shared with Packages | GitHub Actions limits (official docs) |
| Included compute, Free plan | **$12/month**; standard runners free + unlimited on **public** repos | GitHub Actions limits (official docs) |
| macOS rate, paid / private repos | **$0.062/min** (~10.3× Linux $0.006/min); $12 ≈ **193 macOS minutes** | GitHub runner pricing, via billing references checked 2026-11-18 |
| Max concurrent macOS jobs, Free plan | **5** | GitHub Actions limits (official docs) |
| UE 5.5 download | ~**12–25 GB** depending on options; ~15 GB initial install | Download guides checked 2026-11-18 |
| UE 5.5 installed | **40–60 GB** typical; Core Components alone listed at ~64 GB in the installer; + ~5 GB for the iOS target platform; totals of 40–100 GB reported | Download/install guides checked 2026-11-18 |
| Unreal IPA size (typical) | Minimum shippable ~**150–200 MB**; a full game usually **500 MB–2 GB** | Mobile UE guides checked 2026-11-18 |
| Free Apple ID signature | **7 days**, then re-sign; **3 sideloaded apps** at a time; 10 App IDs per rolling 7 days | SideStore / sideloading guides checked 2026-11-18 |

Arithmetic that decides the verdict:

- 14 GB (runner disk, before Xcode/toolchain) < 40 GB (smallest
  realistic UE 5.5 install). The engine alone is ~3× the disk.
- 10 GB (cache cap) < 40 GB (engine). Caching the engine between
  runs is impossible; every run would re-download it, if it fit.
- 500 MB (Free artifact storage) < 500 MB–2 GB (a full Unreal IPA).
  A Shipping IPA may not even upload as an artifact on the Free
  plan. Retention is set to 7 days in the workflow to limit this.
- 6 hours (job cap) vs. a UE source build + cook on 3 vCPU / 7 GB
  RAM: a first source build of the engine alone can exceed 6 hours.
  A binary (Launcher) install is faster but cannot run headlessly
  on a runner (see below).

## Runner choice: why macos-15, not macos-14, not macos-26

- **`macos-14` is retired.** GitHub removed `macos-14`,
  `macos-14-large` and `macos-14-xlarge` on **2026-11-02**
  (actions/runner-images #13518). A job naming `macos-14` now
  fails instead of running. The earlier instruction to use
  `macos-14` is obsolete; this workflow deliberately does not use
  it. If you see `macos-14` in any old instruction, treat it as a
  bug.
- **`macos-15` (pinned) is the workflow's runner.** macOS Sequoia,
  Apple Silicon, Xcode 16.x. UE 5.5 needs Xcode 16.1+, iOS 15+,
  macOS Sequoia 15+ — macos-15 matches that set.
- **`macos-26` / `macos-latest` (now macOS 26) is not used.** UE
  5.5's `Engine/Config/Apple/Apple_SDK.json` caps the iOS SDK
  version it accepts; Xcode 26 / iOS 26 SDK is a known mismatch
  for UE 5.5 (forum reports of "Unreal SDK for iOS is not
  installed properly" and iOS 18 SDK upload errors on UE 5.5).
  Upgrading the engine to UE 5.6+ would be a separate, deliberate
  decision — do not change the runner to macOS 26 to "fix" it.

## How Unreal Engine is obtained — and why CI cannot just download it

There are two ways to get UE 5.5, and neither is a plain download
a headless runner can do on its own:

1. **Epic Games Launcher (binary install).** Needs an Epic account
   and an interactive login in the Launcher. There is no supported
   headless / command-line Launcher install for CI. The workflow
   does not attempt this.
2. **Source build from GitHub (`EpicGames/UnrealEngine`).** That
   repository is **private**: access requires an Epic Games account
   linked to a GitHub account, acceptance of the UE EULA, and
   accepting the @EpicGames organisation invitation within 7 days.
   Cloning without the link fails with a 404. A source build then
   needs `Setup.sh`, `GenerateProjectFiles.sh` and a full editor
   build — hours on a small runner, and far more disk than a binary
   install.

The workflow therefore **requires a pre-installed engine** at a
known path (default `/Users/Shared/Epic Games/UE_5.5`, overridable
per-run via the `engine_dir` input or the `UE_ENGINE_DIR`
repository variable) and fails fast with instructions if
`Engine/Build/BatchFiles/RunUAT.sh` is not executable there. No
Epic credentials, Apple ID, certificate or provisioning profile
appears anywhere in the workflow or the repo — and none may be
added.

## What the workflow does (11 steps)

File: `.github/workflows/build-ios-ipa.yml`
Triggers: `workflow_dispatch` (intended) + `push` to `main` only
when the workflow file itself changes. `runs-on: macos-15`,
`timeout-minutes: 330` (5.5 h, inside the 6 h cap),
`permissions: contents: read`, one job at a time
(`concurrency: ios-ipa`).

1. **Checkout** (`actions/checkout@v4`).
2. **Pre-flight** — prints macOS version, Xcode list, `df -h`,
   free GB, RAM and CPU. This is the log to read first when a run
   fails.
3. **Select Xcode 16.x** — picks the newest `Xcode_16*.app` under
   `/Applications`, `xcode-select`s it, fails clearly if none
   exists. UE 5.5 needs Xcode 16.1+.
4. **Cache DerivedDataCache only** (`actions/cache@v4`) — the
   project's small DerivedDataCache, keyed on the `.uproject` and
   `Config/*.ini`. The engine is deliberately NOT cached: 40–60 GB
   cannot fit in a 10 GB cache.
5. **Resolve UE 5.5 engine (fail fast)** — checks the engine path
   and `RunUAT.sh`; checks ≥ 20 GB free before cooking. Missing
   engine or low disk = immediate, explained failure.
6. **Generate Xcode project files** —
   `Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh -project=…
   -game`.
7. **Build, cook, stage and package iOS (unsigned)** —
   `RunUAT.sh BuildCookRun -project=… -noP4 -nocompileeditor
   -platform=IOS -clientconfig=Development|Shipping -cook -build
   -stage -pak -archive -archivedirectory=Build/IOS -package
   -NoCodeSign -nodebuginfo -utf8output`.
   `-NoCodeSign` is the documented UAT flag for an unsigned package
   (it appears in the UE source's own `BuildCookRunArgs`).
   `-package` + `-archive` produce the `.ipa` (or a staged
   `Payload/` folder).
8. **Locate or assemble the unsigned .ipa** — takes the `.ipa`
   UAT produced if there is one; otherwise, if a staged
   `Payload/<App>.app` exists, zips `Payload/` into
   `GalaxyFrontier-unsigned.ipa` (an `.ipa` is a zip with
   `Payload/` at the top level) and verifies it with `unzip -l`.
   If neither exists, the step fails and prints the build tree.
9. **Upload unsigned IPA artifact**
   (`actions/upload-artifact@v4`, name
   `galaxy-frontier-ios-unsigned-ipa`, retention 7 days,
   `if-no-files-found: error`).
10. **Upload build logs** (`if: always()`, retention 7 days).
11. **Summarise** — writes the run status, config, bundle ID and
    the unsigned/SideStore note to the run summary.

## How to run it (when a suitable machine/runner exists)

Prerequisites — do these first, on the machine that will run it:

1. A Mac (or a paid runner) with **≥ 100 GB free**, UE **5.5**
   installed with iOS support at a known path, and Xcode **16.x**.
2. In the repo, set the repository variable `UE_ENGINE_DIR` to
   that path (Settings → Secrets and variables → Actions →
   Variables), or pass `engine_dir` at dispatch time.
3. Confirm the bundle ID `com.iosai2035.GalaxyFrontier` is the one
   you want SideStore to register as an App ID (free accounts get
   10 App IDs per rolling 7 days; changing the bundle ID later
   burns another one).

Run it:

1. GitHub → repo `iosai2035/Galaxy-Frontier` → **Actions** →
   **Build iOS IPA (SideStore, unsigned)** → **Run workflow**.
2. Choose **Development** first (faster, debuggable). Use
   **Shipping** only after a Development IPA installs and runs.
3. Leave `engine_dir` empty unless the engine is somewhere else.
4. Watch the run. Read the **Pre-flight** step first: if it fails
   at "UE 5.5 engine NOT FOUND" or low disk, the run did what it
   was designed to do — do not re-run it hoping for a different
   result.
5. When (and only when) the run is green, download the artifact
   **galaxy-frontier-ios-unsigned-ipa** from the run page
   (Actions → the run → Artifacts). Artifacts expire after 7 days.
   You must be signed in to GitHub to download it.

## Installing the IPA with SideStore

SideStore is already set up on the user's phone (per the user:
SideStore with his own personal certificate). The IPA the
workflow produces is **unsigned**; SideStore applies the user's
own signature at install time. No paid Apple Developer account
is needed for this path.

1. Get the `.ipa` onto the phone (download it in Safari on the
   phone, or AirDrop / transfer it and save it to **Files**).
   The file must be the unsigned `.ipa` from the workflow
   artifact, not a zip of the whole project.
2. Make sure SideStore's **LocalDevVPN is connected** (SideStore
   needs it to re-sign on-device).
3. Open **SideStore → My Apps → +**, pick the `.ipa` from Files.
   SideStore signs it with your Apple ID and installs it.
4. First launch: **Settings → General → VPN & Device Management**,
   trust the developer certificate under your Apple ID, then open
   the app. Enable **Developer Mode** if iOS asks for it.
5. **Refresh before 7 days.** A free Apple ID signature lasts 7
   days; the app stops opening after that until re-signed. In
   SideStore, tap the countdown (e.g. **7 DAYS**) next to the app,
   or **Refresh All**, with LocalDevVPN connected and Wi-Fi on.
   SideStore can refresh in the background if it stays paired.
6. **App limit: 3 sideloaded apps at a time on a free Apple ID,
   including SideStore itself.** If you hit "Maximum number of
   apps reached", remove another sideloaded app first (or use
   LiveContainer, which is out of scope here).
7. A new build = a new `.ipa` = install it again through SideStore.
   Refreshing renews the signature; it does not update the code.

If SideStore itself is not installed yet, that is a separate
one-time setup (iloader on a computer, sign in with your Apple ID,
install SideStore, set up LocalDevVPN) — follow the current
SideStore docs for your iOS version; iOS 26.4+ changed the VPN /
refresh behaviour and older guides do not apply there.

## What an IPA built today would actually contain

Be honest about this before anyone spends a build on it:

- **Phase 00 (Foundation)** — project structure, `.uproject`,
  C++ module, Config. No content.
- **Phase 01 (Character)** — `AGFCharacter` (movement, camera,
  Enhanced Input wiring). No map, no mission, no weapons, no
  enemies, no mobile HUD.
- **Not implemented:** `Fire`, `Reload`, `Interact`,
  `SwitchWeapon`, `OpenInventory` (declared stubs that log a
  warning); the Input Action / Mapping Context `.uasset` files
  and `WBP_MobileHUD` exist only as specs in `Docs/` and must be
  created in the Editor; there is no default map set and no
  `L_ZERON_Prototype` level yet (Phase 03).
- So a successful IPA today would, at best, open to an empty /
  default level with a character that can move. It is **not a
  playable game** and should not be sent to anyone as one.

Recommendation: do not spend a build attempt until **Phase 03**
(World & Missions — ZERON + "The Lost Power Core") is complete
and the project opens and plays in the Editor on macOS. Phase 04
(Mobile UI & Optimisation) is the first point where an IPA is
worth installing on a real phone.

## Likely failure modes (read the log for these first)

| Failure | Cause | Fix |
|---|---|---|
| `UE 5.5 engine NOT FOUND` | No pre-installed engine at the path | Install UE 5.5 on the runner/Mac, or set `UE_ENGINE_DIR` |
| `Less than 20 GB free` / `No space left on device` | Runner disk (14 GB) vs UE + cook output | Larger disk / self-hosted Mac; free space; do not retry on a standard runner |
| `No Xcode 16.x found` | Runner image changed Xcode versions | Pin/select the Xcode UE 5.5 accepts; check `Apple_SDK.json` MaxVersion |
| `Unreal SDK for iOS is not installed properly` | Xcode / iOS SDK version vs UE 5.5's `Apple_SDK.json` | Use Xcode 16.x on macos-15; do not use macOS 26 / Xcode 26 with UE 5.5 |
| Cook errors, missing maps/assets | No `.uasset` content yet (Phases 00-01) | Complete the Editor-side assets (Phase 03) before packaging |
| Job killed at 6 h | Engine source build / cook too slow on 3 vCPU | Self-hosted Mac or larger runner; do not raise the timeout — 6 h is GitHub's cap |
| Artifact upload refused / missing | IPA > 500 MB Free artifact storage, or no `.ipa` produced | Shorter retention won't help; use a Release asset or external storage, and fix the packaging step first |
| App won't open after 7 days | Free Apple ID signature expired | Refresh in SideStore (LocalDevVPN on); this is Apple's limit, not a build bug |
| `Maximum number of apps reached` | Free Apple ID 3-app limit | Remove a sideloaded app first |

## Realistic routes, ranked

1. **Self-hosted Mac (recommended).** A Mac mini / MacBook with
   UE 5.5 + Xcode 16 pre-installed, registered as a self-hosted
   runner (or build locally with the same `RunUAT` command). No
   6-hour cap (self-hosted jobs run up to 5 days), no 14 GB disk,
   no per-minute charge, engine installed once. Change
   `runs-on:` to your self-hosted label; everything else in the
   workflow stays the same.
2. **Paid larger runner.** Only if its disk is genuinely larger —
   verify with the pre-flight `df -h` output in a cheap test run
   before committing to a full build. Note the published macOS
   larger-runner table still lists 14 GB SSD for macOS Large /
   XLarge sizes, so do not assume "larger" means "more disk" for
   macOS; confirm it from a real run, not from the name.
3. **Standard GitHub-hosted runner (`macos-15`).** Not viable for
   a full UE 5.5 iOS package, for the disk reasons above. Keep the
   workflow's fail-fast behaviour; do not remove the engine/disk
   checks to "try anyway" — the failure mode is a 6-hour run that
   dies on disk, and it teaches nothing a 2-minute pre-flight
   check does not.

## Sources checked 2026-11-18

- GitHub-hosted runners reference (official docs) —
  https://docs.github.com/en/actions/reference/runners/github-hosted-runners
  — macos-15 arm64 3 vCPU / 7 GB / 14 GB; macos-15-intel 4 vCPU /
  14 GB / 14 GB; macos-14 absent; public repos free + unlimited.
- GitHub Actions limits (official docs) —
  https://docs.github.com/en/actions/reference/limits
  — 6 h job limit, 35-day workflow limit, Free 500 MB artifact /
  $12 included compute / 10 GB cache per repo, 5 concurrent macOS
  jobs on Free.
- macOS 14 retirement —
  https://dev.to/kalemi/github-actions-deprecations-what-breaks-when-and-the-fix-3bj0
  and
  https://www.sevydevy.in/learn/github-november-2026-macos-14-runner-retirement-explained
  — macos-14 retired 2026-11-02, actions/runner-images #13518.
- UE 5.5 size —
  https://github.com/sandraschi/godot-mcp/blob/HEAD/docs/comparison.md
  (~40 GB total, engine ~15 GB + starter content ~5 GB + shader
  cache ~5–10 GB + ~10 GB per platform),
  https://practicetestgeeks.com/unreal/unreal-engine-download
  (12–25 GB download, 40–80 GB after install),
  https://www.cosmiclearn.com/ue5/installation-and-setup.php
  (Core Components 64.2 GB, iOS 5.2 GB).
- UE source access —
  https://www.unrealengine.com/ue-on-github?lang=en-US
  — private repo, Epic account linked to GitHub + EULA + org
  invitation within 7 days.
- UE 5.5 / Xcode —
  https://developers.hiveplatform.ai/en/v4.25.3.0/releases/unreal5/android-ios/
  (UE 5.5.3 iOS: Xcode 16.1+, iOS 15.0+, macOS Sequoia 15+) and
  https://forums.unrealengine.com/t/does-ue5-5-support-ios-sdk-18-required-for-appstore-connect/2494006
  (UE 5.5 / iOS SDK 18 / Apple_SDK.json issues).
- `BuildCookRun` / `-NoCodeSign` —
  https://gitea.cgnico.com/CGNICO/UnrealEngine/src/branch/ue5-main/Engine/Build/LowLevelTests.xml
  (`-NoCodeSign` in BuildCookRunArgs),
  https://forums.unrealengine.com/t/command-line-build-for-ios-on-mac/146578
  (RunUAT BuildCookRun iOS command line),
  https://kbve.com/application/ios/ (UE5 iOS BuildCookRun
  reference, `skip_signing`).
- SideStore / sideloading —
  https://github.com/darkpandawarrior/Gaddi/blob/HEAD/docs/SIDELOADING.md
  (unsigned .ipa + SideStore re-signs with your Apple ID, 7-day
  signature, 3-app limit),
  https://libshortcut.com/blog/install-ipa-with-sidestore/
  (SideStore setup, iloader, LocalDevVPN, My Apps → +, refresh),
  https://sidestore.io (SideStore project site).

## Status log

- 2026-11-18 — Workflow + this document created. **BUILT: NONE.
  TESTED: NONE.** No run attempted, no IPA produced, no SideStore
  install tested.

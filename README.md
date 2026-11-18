# Galaxy Frontier

Sci-fi third-person action/adventure for **iPhone and iPad**, built with **Unreal Engine 5**.

Original world, original characters, original story. No characters, maps,
logos or assets copied from other games.

## Status (2026-11-18, verified live on Linux)

- **Phase 00 — Foundation: CREATED / REVIEWED.** Project structure, `.uproject`,
  C++ module, Config, Docs and Git setup are written and reviewed.
- **Phase 01 — Character: CREATED / REVIEWED.** `AGFCharacter`, Enhanced Input
  wiring and movement values are written and reviewed.
- **Phase 02 — Combat & AI: CREATED / REVIEWED.** Health/shield, combat,
  weapons, projectiles, enemy + AI (perception, Blackboard, Behavior
  Tree spec + timer fallback). `Fire` / `Reload` / `SwitchWeapon`
  are real implementations now — the Phase 01 stubs are replaced.
- **Phase 03 — World & Missions: CREATED / REVIEWED.** Mission
  subsystem, objectives, data asset, save, interaction system,
  planet ZERON spec, mission "The Lost Power Core" (9 stages).
  `Interact` is a real implementation now.
- **BUILT: NONE. TESTED: NONE.** Unreal Engine is **not installed** on this
  Linux VM and there is no Xcode / macOS here, so no UE build, no IPA build,
  no signing and no device test has been run. See `Docs/ENVIRONMENT.md`.
  Those steps are **BLOCKED** until the project is opened on macOS with a
  compatible Xcode + iOS SDK + UE 5.5 with iOS support.

## Assumptions (recorded, not hidden)

1. **Unreal Engine 5.5** is the target version. The master prompt did not
   specify a version; 5.5 was chosen because it is a current, stable UE5
   line with Enhanced Input / Niagara / World Partition support.
2. **Bundle Identifier `com.iosai2035.GalaxyFrontier`** and **minimum iOS 16.0**
   are placeholders until the user confirms the Apple Developer account
   details. Do not ship with them unconfirmed.
3. **MIT License** is used until the user picks another licence.
4. The GitHub repo is `iosai2035/Galaxy-Frontier`, branch `main`.
   It was **empty** (no commits, no files) when verified on 2026-11-18.
   Phases 00 + 01 were pushed 2026-11-18 14:10 UTC in commit
   `86202c1f2727c436a03d7bfe7aab86f8fd1c6f5d`.

## How to open (macOS only)

1. Install Unreal Engine 5.5 (Epic Games Launcher) with iOS support.
2. Install a compatible Xcode + iOS SDK.
3. Clone `https://github.com/iosai2035/Galaxy-Frontier`, branch `main`.
4. Right-click `GalaxyFrontier.uproject` → Generate Xcode project files,
   or open the `.uproject` in the Unreal Editor.
5. In the Editor, create the Input Actions / Mapping Context listed in
   `Docs/INPUT.md` and the `WBP_MobileHUD` widget listed in
   `Docs/MOBILE-HUD.md` — these are Blueprint/UMG assets and cannot be
   authored as files on Linux (no `.uasset` binaries are faked in this repo).

## Project layout

```
GalaxyFrontier/
  GalaxyFrontier.uproject
  Config/            Engine / Game / Input / Scalability / DeviceProfiles
  Source/            C++ module GalaxyFrontier + GalaxyFrontier.Target.cs
    GalaxyFrontier/
      Characters/    AGFCharacter (Phases 01-03)
      Components/    UGFHealthComponent (Phase 02)
      Combat/        UGFCombatComponent (Phase 02)
      Weapons/       AGFWeaponBase, AGFProjectile, UGFWeaponDataAsset (Phase 02)
      AI/            AGFEnemyCharacter, AGFEnemyAIController, UGFEnemyHealthComponent (Phase 02)
      Missions/      UGFMissionSubsystem, UGFObjectiveComponent, UGFMissionDataAsset, UGFMissionSaveGame (Phase 03)
      World/         IGFInteractable, UGFInteractionComponent, AGFInteractableActor (Phase 03)
      Tests/         Automation test skeletons (NOT RUN — no UE here)
  Content/           README only in Phases 00-03; no fake .uasset files
  Docs/              Roadmap, quality, device matrix, environment, reports
```

## Phases

- **00 Foundation** — project, structure, modules, Config, Git, Docs.
- **01 Character** — character, camera, movement, touch input, HUD spec.
- **02 Combat & AI** — health/shield, weapons, projectiles, enemies, AI.
  Done (CREATED / REVIEWED, not built/tested — see above).
- **03 World & Missions** — planet ZERON, `L_ZERON_Prototype` spec,
  mission "The Lost Power Core" (9 stages), interaction system.
  Done (CREATED / REVIEWED, not built/tested). The level itself is
  an Editor asset, not a file in this repo.
- **04 Mobile UI & Optimization** — full UMG, settings, performance.
- **05 Expansion** — customisation, inventory, vehicles, ships, planets.
- **06 Multiplayer** — dedicated server, lobby, sessions, Battle Royale
  (only after the network base is built and tested).

See `Docs/ROADMAP.md` for the full plan and `Docs/QUALITY.md` for the
rules that apply to every file: no fake code, no invented test results,
no claim of BUILT / TESTED without a real run.

## iOS / IPA — hard requirement

A real IPA requires **macOS + compatible Xcode + iOS SDK + UE with iOS
support + Apple signing (certificate + provisioning profile)**.
It cannot be produced from Linux alone. Any claim otherwise in this repo
would be false; none is made.

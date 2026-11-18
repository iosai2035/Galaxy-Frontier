# Phase 02 — Combat & AI — Report (2026-11-18)

- **Phase:** 02 Combat & AI
- **Systems created:** Health/shield, combat, weapons, projectiles,
  enemy character, enemy AI (perception + Blackboard + Behavior Tree
  spec + timer fallback), combat Automation test skeleton.
- **Files created:** see list below (all CREATED, REVIEWED).

## Files

| File | Status |
|---|---|
| `Source/GalaxyFrontier/Components/UGFHealthComponent.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Components/UGFHealthComponent.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Combat/UGFCombatComponent.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Combat/UGFCombatComponent.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Weapons/UGFWeaponDataAsset.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Weapons/UGFWeaponDataAsset.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Weapons/AGFWeaponBase.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Weapons/AGFWeaponBase.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Weapons/AGFProjectile.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Weapons/AGFProjectile.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/AI/UGFEnemyHealthComponent.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/AI/UGFEnemyHealthComponent.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/AI/AGFEnemyCharacter.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/AI/AGFEnemyCharacter.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/AI/AGFEnemyAIController.h` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/AI/AGFEnemyAIController.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Tests/GFCombatTest.cpp` | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Characters/AGFCharacter.h` (updated) | CREATED, REVIEWED |
| `Source/GalaxyFrontier/Characters/AGFCharacter.cpp` (updated) | CREATED, REVIEWED |
| `Source/GalaxyFrontier/GalaxyFrontier.Build.cs` (updated) | CREATED, REVIEWED |
| `Docs/WEAPONS.md` | CREATED, REVIEWED |
| `Docs/AI.md` | CREATED, REVIEWED |
| `Docs/CHARACTERS.md` | CREATED, REVIEWED |
| `Docs/RELIABILITY.md` | CREATED, REVIEWED |
| `Config/DefaultScalability.ini` (updated) | CREATED, REVIEWED |
| `Config/DefaultDeviceProfiles.ini` (updated) | CREATED, REVIEWED |
| `Docs/QUALITY.md` (updated) | CREATED, REVIEWED |
| `Docs/DEVICE_TEST_MATRIX.md` (updated) | CREATED, REVIEWED |
| `CHANGELOG.md` (Phase 02 section) | CREATED, REVIEWED |

## Code implemented

- `UGFHealthComponent`: MaxHealth 100, MaxShield 100, shield
  absorbs first, remainder to health, clamp 0..max, no damage
  after death, `Die()` fires `OnDeath` exactly once, `Revive()` /
  `ResetHealth()`, regen via 0.5 s timer with a post-damage delay,
  `OnHealthChanged` / `OnShieldChanged` delegates. Death logic is
  separated from animation (owner handles visuals).
- `UGFWeaponDataAsset`: Pistol 20 dmg / 3/s / 12, Assault 18 / 10/s /
  30, Sniper 85 / 0.8/s / 5, Energy Rifle 24 / 5/s / 24 (starting
  balance, Editor instances `DA_Pistol` etc. — no `.uasset` faked).
- `AGFWeaponBase`: `Fire` / `Reload` / `CanFire` / `CanReload` /
  `Equip` / `Unequip` / `SetAmmo` / `GetAmmo` / `ApplyRecoil` /
  `SpawnProjectile`. No fire while reloading or at 0 ammo, reload
  cancelled on unequip, muzzle socket with a logged fallback.
- `AGFProjectile`: sphere collision, `UProjectileMovementComponent`,
  owner/instigator ignored, no damage beyond `MaxRange`, 3 s
  lifetime cap, damage only via `UGFHealthComponent`.
- `UGFCombatComponent`: weapon spawn/equip/switch, fire-rate timer
  (no Tick), automatic fire, `StartFire` / `StopFire` / `Reload` /
  `SwitchToNextWeapon`, dead owners cannot fire.
- `AGFEnemyCharacter`: Health 100, Detection 1500 cm, Attack 800 cm,
  Patrol 1000 cm, Attack cooldown 1.5 s, Speed 350 cm/s, Attack
  damage 15, Retreat at 25% health, states Idle…Die, material
  slots + forced LOD hooks.
- `UGFEnemyHealthComponent`: subclass of `UGFHealthComponent`
  (choice documented in its header — reuse, not a copy). Enemy
  tuning only: Health 100, Shield 0, no regen; `OnEnemyDied`
  plus a logged reward intent (inventory is Phase 05).
- `AGFEnemyAIController`: `UAIPerceptionComponent` sight (FOV 90°),
  Blackboard keys `TargetActor` / `TargetLocation` /
  `HomeLocation` / `State`, Behavior Tree when assigned, 0.25 s
  timer fallback state machine when it is not. Patrol via
  NavMesh random reachable points, Chase, Attack (range +
  cooldown re-checked), Search 6 s then Patrol, Retreat.
- `AGFCharacter`: `HandleFireStarted/Completed` → combat
  `StartFire`/`StopFire`, `HandleReload` → `Reload`,
  `HandleSwitchWeapon` → `SwitchToNextWeapon`. Fire/Reload
  stubs from Phase 01 are **replaced — no longer stubs**.
  Added `HealthComponent`, `CombatComponent`,
  `FGFCharacterCustomization` (`SkinTone` / `HairColour` /
  `ArmourTint`), `ApplyCustomization()` (dynamic material
  instances), `ApplyMaterialToSlot()`, `ApplyForcedLOD()`.
- `GalaxyFrontier.Build.cs`: added `AIModule` + `NavigationSystem`
  (Private) — both actually used by the AI controller.
  UMG/Slate/GameplayTasks still NOT added (no code uses them).

## Realistic characters / high graphics (refinement)

- `Docs/CHARACTERS.md`: original realistic humans only (no
  real-person likeness, no copied characters), proportion/anatomy,
  PBR skin/cloth/metal, facial/Control Rig notes, LOD0–3,
  customisation hooks. **No mesh binary exists** — meshes must be
  authored in Blender / MetaHuman-style tooling on macOS/Windows.
- Config: High scalability group + iPhone Pro / iPad Pro profiles
  (60 fps is a *target to measure*, never a promise), budgets in
  `Docs/DEVICE_TEST_MATRIX.md` and `Docs/QUALITY.md` (draw calls,
  polygons, texture memory, dynamic lights ≤ 2, Lumen not used
  on iOS, Nanite only where supported).
- `Docs/RELIABILITY.md`: per-system failure-mode checklist
  (damage after death, reload with 0 ammo, fire while reloading,
  projectile self-hit, BT missing, no NavMesh, etc.) and the
  degradation for each. Defensive coding in every new component:
  null checks, clamps, timer/event-driven (no Tick) logic.

## Tests

- `GFCombatTest.cpp` — Automation test skeleton, spec values +
  10 failure cases listed. **NOT RUN** (no UE here). It spawns
  nothing and ticks no world; a real test needs the Phase 03
  test map. Do not mark it PASSED.

## Errors found / fixed in review

- **Fixed:** `AGFWeaponBase.cpp` originally contained a stray
  `AGFProjectile::SpawnProjectile()` definition (a member that
  does not exist on `AGFProjectile`). Removed — only
  `AGFWeaponBase::SpawnProjectile()` remains.
- **Fixed:** `AGFProjectile::OnHit` originally applied damage
  *before* the MaxRange check, so a beyond-range hit still
  damaged. Reordered: range guard first, damage only inside range.
- **Fixed:** `UGFEnemyHealthComponent.cpp` originally referenced
  `OnEnemyDiedHandler` without declaring it. Declared it as a
  `UFUNCTION` in the header and implemented it properly.
- **Fixed:** `IGFInteractable` is a C++-only interface — the
  interaction component originally called `Execute_*` wrappers,
  which are only generated for `UFUNCTION` interface methods.
  Changed to `Cast<IGFInteractable>(Actor)` + direct calls
  (this fix landed with the Phase 03 interaction code, reviewed
  together with Phase 02).

## NOT IMPLEMENTED

- Weapon meshes, muzzle-flash Niagara, firing SFX — Editor assets.
- Laser / Melee weapon data assets (type exists, no asset yet).
- Weapon pickups / inventory (Phase 05).
- BT / Blackboard `.uasset` files — spec in `Docs/AI.md` only.
- EQS, hearing sense, squad AI, ranged enemies.
- `HandleOpenInventory` (Phase 05), `HandlePause` (Phase 04)
  remain stubs in `AGFCharacter`.

## Build / test status

- **BUILT: NONE — BLOCKED.** No Unreal Engine on this Linux VM.
- **TESTED: NONE — BLOCKED.** No UE, no device, no Editor run.
- **IPA: BLOCKED.** macOS + Xcode + iOS SDK + signing required.
- **Device testing: BLOCKED.** No iPhone/iPad here.

## Delivery

- ZIP: `GalaxyFrontier-Phase-02-Combat-AI.zip` (in
  `~/workspace/galaxy-frontier/`). Verified with `unzip -l` and
  `unzip -t` — see sizes/counts in the parent delivery report.
- GitHub push: NOT DONE by this agent.

## Next step

**Phase 03 — World & Missions:** ZERON / `L_ZERON_Prototype`
spec, "The Lost Power Core" mission, interaction system.

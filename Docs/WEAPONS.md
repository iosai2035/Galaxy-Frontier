# Weapons — Galaxy Frontier (Phase 02)

All weapon numbers live in `UGFWeaponDataAsset` instances. Logic lives
in `AGFWeaponBase` / `UGFCombatComponent` — never hard-code a weapon's
numbers in a Character or in the HUD.

## Starting balance (master prompt §10 — NOT final)

| Weapon | Damage | Fire rate | Magazine | Reload | Automatic | Projectile speed |
|---|---|---|---|---|---|---|
| Pistol | 20 | 3 /s | 12 | 1.5 s | No | 4000 cm/s |
| Assault Rifle | 18 | 10 /s | 30 | 1.5 s | Yes | 4000 cm/s |
| Sniper | 85 | 0.8 /s | 5 | 1.5 s | No | 4000 cm/s |
| Energy Rifle | 24 | 5 /s | 24 | 1.5 s | Yes | 4000 cm/s |

Reload time, spread, recoil and range are per-asset values in the
Editor (defaults: Reload 1.5 s, Spread 1.0°, Recoil 0.4, Range 8000 cm).
Laser / Melee types exist in `EGFWeaponType` but have no data asset
yet — they are Phase 05, NOT IMPLEMENTED here.

## Editor creation steps (macOS, Unreal Editor)

1. Content → Right-click → Miscellaneous → Data Asset →
   `UGFWeaponDataAsset`. Create one per weapon:
   `DA_Pistol`, `DA_AssaultRifle`, `DA_Sniper`, `DA_EnergyRifle`.
2. Fill the table above into each asset. Set `ProjectileClass` to the
   `AGFProjectile` Blueprint/class (create a Blueprint child of
   `AGFProjectile` if a mesh / Niagara trail is needed).
3. Create a Blueprint child of `AGFWeaponBase` per weapon, assign its
   `WeaponData` and its `WeaponMesh` (mesh authored in Blender —
   no mesh binary is in this repo), and set the `Muzzle` socket.
4. In `AGFCharacter` (or the player Blueprint), add the weapon classes
   to `UGFCombatComponent.WeaponClasses` in loadout order.
5. `WBP_MobileHUD` reads ammo via `UGFCombatComponent.GetCurrentAmmo()`
   / `GetReserveAmmo()` — it must listen to `OnAmmoChanged`, never
   poll every frame.

## Rules (enforced in code)

- No fire while reloading, no fire at 0 ammo (dry fire logs, does not
  crash). `CanFire()` is the single authority; the HUD and input both
  go through it.
- No self-hit: the projectile ignores its owner/instigator.
- No damage beyond `MaxRange`: the projectile destroys itself
  without applying damage.
- Reload is cancelled if the weapon is unequipped mid-reload.
- A weapon with no data asset is never spawned by the combat
  component (destroyed at spawn with a warning).
- Muzzle VFX (Niagara) and firing SFX (Sound Cue / MetaSound) are
  Editor assets — no VFX/SFX is faked in code. If they are missing,
  firing still works and logs a warning.

## What is NOT done

- No weapon meshes, no muzzle-flash Niagara, no firing sounds in
  this repo (Editor assets, macOS only).
- No weapon pickups / inventory (Phase 05).
- No network replication of fire/reload (Phase 06).

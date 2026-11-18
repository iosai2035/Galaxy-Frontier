# WBP_MobileHUD — Galaxy Frontier (Phase 01, spec only)

UMG widget. **Not a file in this repo** — UMG widgets are `.uasset`
Blueprints and must be built in the Unreal Editor on macOS. This doc
is the build spec; do not claim the widget exists until it is created
and tested in the Editor.

## Layout (Landscape, Safe Areas mandatory)

- Left: virtual joystick (move). Fixed base, floating thumb.
- Right: camera-drag area (empty space, not a button).
- Right cluster: Fire (large, bottom-right), Aim (above Fire),
  Jump (right of Fire), Sprint, Interact, Reload, Switch Weapon.
- Top-left: Health bar + Shield bar (numbers, not just bars).
- Top-right: Ammo counter, current objective text.
- Top-centre: Pause button.

## Requirements

1. Multi-touch: joystick + drag + Fire at the same time, no conflicts.
2. Buttons must not block the camera-drag area or the play view.
3. Safe Areas: respect iPhone notch / Dynamic Island and iPad edges.
4. All screen sizes: anchor-based layout, no fixed pixel positions
   that break on a different aspect ratio.
5. Button position / size adjustable by the player (Phase 04).
6. Camera sensitivity adjustable; vertical invert option (Phase 04).
7. HUD updates via Events / Delegates, not per-frame polling.
8. Health/shield/ammo are driven by `UGFHealthComponent` /
   `UGFCombatComponent` (Phase 02) — in Phase 01 the HUD shows
   placeholder bindings only, with no fake values.

## Widgets (full list, Phases 01-04)

`WBP_MainMenu`, `WBP_MobileHUD`, `WBP_PauseMenu`, `WBP_Settings`,
`WBP_Inventory`, `WBP_WeaponSelection`, `WBP_MissionPanel`,
`WBP_CharacterCustomization`, `WBP_LoadingScreen`, `WBP_GameOver`,
`WBP_MissionComplete`. Only `WBP_MobileHUD` is specced in Phase 01;
the rest are Phase 04/05.

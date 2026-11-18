# WBP_MobileHUD — Galaxy Frontier (Phase 04, build spec)

UMG widget. **Not a file in this repo** — UMG widgets are `.uasset`
Blueprints and must be built in the Unreal Editor on macOS. This doc
is the exact build spec; do not claim the widget exists until it is
created and tested in the Editor. The C++ base class is
`UGFMobileHUDWidget` (`Source/GalaxyFrontier/UI/`): the Blueprint
must be parented to it, and its widgets must be named exactly as
in the table below (all bindings are `BindWidgetOptional` — a
missing name logs a warning, it never crashes).

## Parent class and data flow

- Parent: `UGFMobileHUDWidget` (C++). It binds in `NativeConstruct`
  to `UGFHUDController` (component on `AGFCharacter`) and to
  `UGFPerformanceManager`. It never polls: health / shield / ammo /
  weapon / objective / prompt / diagnostics update only when the
  matching delegate fires; `RefreshAll()` pulls the current values
  once at construct time.
- Health 100 / Shield 100 come from `UGFHealthComponent`; ammo and
  weapon name from `UGFCombatComponent` / `AGFWeaponBase`
  (Pistol 12, Assault 30, Sniper 5, Energy 24 per magazine);
  objective text from `UGFMissionSubsystem::GetCurrentObjectiveTitle()`
  ("The Lost Power Core", 9 stages); prompt from
  `UGFInteractionComponent::GetCurrentPrompt()` (250 cm range).
- The widget must NOT read health/ammo on Tick, must NOT call
  `FindComponentByClass` per frame, and must NOT invent values
  when a component is missing — empty fields stay empty.

## Layout (Landscape only, anchor-based, no fixed pixels)

The canvas is a full-screen `Canvas Panel`. Every element is
positioned with anchors + alignment, sized as a fraction of the
screen (or with a `Size Box` driven by the settings
`ButtonScale`, 0.5–2.0). **No absolute pixel position may be the
only positioning** — a layout that works on one phone and breaks
on an iPad is a fail.

| Element (Blueprint name) | Type | Anchor | Size / position |
|---|---|---|---|
| `HealthBar` | Progress Bar | top-left | 28% screen width, bar + `HealthText` "87 / 100" beside it |
| `ShieldBar` | Progress Bar | top-left, under HealthBar | same width, `ShieldText` "100 / 100" |
| `AmmoText` | Text Block | top-right | "24 / 120" (magazine / reserve), large, right-aligned |
| `WeaponNameText` | Text Block | top-right, under AmmoText | current weapon name, e.g. "Assault Rifle" |
| `ObjectiveText` | Text Block | top-right, under WeaponNameText | current objective title, wraps to 2 lines max |
| `PauseButton` | Button | top-centre | 44 dp touch target minimum |
| `DiagnosticsText` | Text Block | top-centre, under PauseButton | only when Settings `bShowDiagnostics` is true; "FPS 58 (min 52) \| RAM 412 MB \| High" |
| Joystick base | Canvas / custom touch area | bottom-left | see Joystick below |
| `FireButton` | Button | bottom-right | largest button, ~96 dp at ButtonScale 1.0 |
| `AimButton` | Button | bottom-right, above Fire | ~72 dp |
| `JumpButton` | Button | bottom-right, right of Fire | ~72 dp |
| `SprintButton` | Button | bottom-right, above Jump | ~64 dp |
| `InteractButton` | Button | bottom-right, left of Fire | ~72 dp; **Hidden when the prompt is empty** |
| `ReloadButton` | Button | bottom-right, above Interact | ~64 dp |
| `SwitchWeaponButton` | Button | bottom-right, above Reload | ~64 dp |

Button sizes are in dp (density-independent): at `ButtonScale`
1.0 the Fire button is ~96 dp, others as listed. Every button's
hit area is at least 44 × 44 dp — smaller is not shippable.

## Virtual joystick (left, Blueprint touch logic)

- Fixed base, floating thumb: the thumb follows the finger
  within the base radius; releasing the finger returns the thumb
  to centre and stops movement.
- Base position: anchor fraction from Settings
  (`JoystickAnchorX` 0.12 / `JoystickAnchorY` 0.75 by default,
  range 0.05–0.95) and side from `JoystickSide` (Left default,
  Right flips the whole left cluster to the right and the Fire
  cluster to the left).
- Touch handling: `OnTouchStarted` / `OnTouchMoved` /
  `OnTouchEnded` on the joystick canvas. Movement is applied
  with the pawn's `AddMovementInput` (forward = stick Y, right
  = stick X), exactly like `AGFCharacter::HandleMove`.
  Dead zone 0.15 — inside it, no input is sent.
- The joystick area must not overlap any button, and buttons
  must not overlap it.

## Camera drag (right, empty space)

- The empty right-hand area (not covered by buttons) is the
  camera-drag zone: drag rotates the camera via
  `AddControllerYawInput` / `AddControllerPitchInput`, scaled by
  `LookSensitivity` and `bInvertLookPitch` (Settings).
- Drag must work while the joystick is held and while Fire is
  held — three simultaneous touches minimum (see Multi-touch).

## Buttons (behaviour — C++ handlers in UGFMobileHUDWidget)

| Button | Press | Release |
|---|---|---|
| Fire | `UGFCombatComponent::StartFire()` | `StopFire()` |
| Aim | `AGFCharacter::SetAiming(true)` | `SetAiming(false)` (aim-walk = WalkSpeed 300) |
| Jump | `ACharacter::Jump()` on click | — |
| Sprint | `SetSprinting(true)` | `SetSprinting(false)` (SprintSpeed 700) |
| Interact | `UGFInteractionComponent::BeginInteraction()` on click | — |
| Reload | `UGFCombatComponent::Reload()` on click | — |
| SwitchWeapon | `SwitchToNextWeapon()` on click | — |
| Pause | `UGFHUDController::TogglePause()` on click | — |

## Multi-touch (mandatory)

- Minimum simultaneous touches: joystick + Fire + camera drag.
- Each touch is tracked by its finger index; a button press
  must never steal the joystick's finger or the drag's finger.
- Test: hold the joystick forward with the left thumb, hold
  Fire with the right thumb, drag the camera with a third
  finger — the character must keep moving, keep firing and keep
  turning. If any one of the three stops, the layout fails.

## Safe Areas (mandatory)

- The root canvas is wrapped in a `Safe Zone` widget (or the
  layout uses the platform Safe Area insets — either way, the
  result is what is tested, not the method).
- iPhone notch / Dynamic Island: top-left health bars and
  top-right ammo must clear the island; the top-centre Pause
  button must not sit under it.
- iPad: all edges clear the rounded corners and the home
  indicator; the HUD must not assume a phone aspect ratio.
- Landscape only — portrait is not supported and the game must
  not be tested in portrait.

## Diagnostics overlay

- `DiagnosticsText` is `Collapsed` unless Settings
  `bShowDiagnostics` is true. When shown it displays measured
  FPS (avg + lowest since sampling started), used RAM in MB and the live quality
  tier, with "(auto-lowered)" when the performance manager
  stepped below the user's chosen quality. It shows **no**
  draw-call / polygon numbers — those are measured in the
  Editor with `stat RHI`, not on the device (see
  Docs/PERFORMANCE.md).

## Editor creation checklist (macOS)

1. Create Widget Blueprint `WBP_MobileHUD`, parent class
   `UGFMobileHUDWidget`, in `Content/UI/`.
2. Build the canvas per the table above; name every widget
   exactly as listed; set anchors, never fixed pixels.
3. Set the joystick + camera-drag touch handlers per this doc.
4. In the `AGFCharacter` Blueprint (or its defaults), set
   `HUDController → HUDWidgetClass = WBP_MobileHUD`.
5. Play in Editor (PIE) on the `L_ZERON_Prototype` map: health
   / shield / ammo / objective must update on events, buttons
   must fire / reload / switch, pause must open `WBP_PauseMenu`.
6. Test on a real iPhone + iPad: multi-touch, Safe Areas, all
   screen sizes — record results in `Docs/DEVICE_TEST_MATRIX.md`.
   **Until step 6 is done, the HUD is CREATED / REVIEWED only.**

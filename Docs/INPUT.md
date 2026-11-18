# Input — Galaxy Frontier (Phase 01)

Enhanced Input only. No legacy input.

## Assets to create in the Unreal Editor (macOS)

These are Blueprint assets and cannot be authored as files on Linux.
Create them in the Editor under `Content/Input/`:

| Asset | Type | Value type |
|---|---|---|
| `IMC_Player` | Input Mapping Context | — |
| `IA_Move` | Input Action | Vector2D (Axis2D) |
| `IA_Look` | Input Action | Vector2D (Axis2D) |
| `IA_Jump` | Input Action | Bool |
| `IA_Sprint` | Input Action | Bool |
| `IA_Crouch` | Input Action | Bool |
| `IA_Aim` | Input Action | Bool |
| `IA_Fire` | Input Action | Bool |
| `IA_Reload` | Input Action | Bool |
| `IA_Interact` | Input Action | Bool |
| `IA_SwitchWeapon` | Input Action | Bool |
| `IA_OpenInventory` | Input Action | Bool |
| `IA_Pause` | Input Action | Bool |

## Default bindings (IMC_Player, priority 0)

| Action | Keyboard / mouse | Touch (via WBP_MobileHUD) |
|---|---|---|
| IA_Move | W/A/S/D | Left virtual joystick |
| IA_Look | Mouse | Right-side touch drag |
| IA_Jump | Space | Jump button |
| IA_Sprint | Left Shift (hold) | Sprint button (hold) |
| IA_Crouch | C | Crouch button |
| IA_Aim | Right Mouse (hold) | Aim button (hold) |
| IA_Fire | Left Mouse (hold) | Fire button (hold) |
| IA_Reload | R | Reload button |
| IA_Interact | E | Interact button |
| IA_SwitchWeapon | Q / 1 / 2 | Switch button |
| IA_OpenInventory | Tab / I | Inventory button |
| IA_Pause | Esc / P | Pause button |

## Wiring in C++

- `AGFCharacter::BeginPlay` adds `IMC_Player` via
  `UEnhancedInputLocalPlayerSubsystem::AddMappingContext(..., 0)`.
  If the context is unassigned, a warning is logged and input does
  nothing — it is not assumed to exist.
- `AGFCharacter::SetupPlayerInputComponent` binds each action only
  if its `UInputAction*` property is assigned in the Editor.
- `HandleMove` / `HandleLook` use the controller yaw; `HandleJump`
  calls `ACharacter::Jump()`; sprint/aim change `MaxWalkSpeed`
  (see `AGFCharacter.cpp`, `UpdateMovementSpeed`).

## Touch requirements

- Multi-touch: joystick + camera drag + fire must work simultaneously.
- No mouse-emulation-only design; test on real devices.
- Camera sensitivity editable (`LookSensitivity`, default 1.0),
  vertical-axis invert option to be added in Phase 04 settings.
- Control layout / button size / position must be user-adjustable
  (Phase 04); settings are saved via SaveGame.

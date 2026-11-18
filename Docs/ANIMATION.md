# Animation — Galaxy Frontier (Phase 01, spec only)

No animation assets are included in Phases 00-01. Animation Blueprints,
Blend Spaces, Control Rig / IK Rig and retargeting are Editor work on
macOS. This doc is the spec.

## Animation Blueprint (to create in the Editor)

- Name: `ABP_GFCharacter` (under `Content/Characters/`).
- Parent class: `AGFCharacter`.
- State Machine states: Idle, Walk, Run, Sprint, Crouch, Jump / Fall,
  Land, Aim, Fire (Phase 02), Death (Phase 02).
- Variables driven from `AGFCharacter`: Speed (from velocity),
  IsInAir, IsCrouching, IsSprinting, IsAiming, AimPitch/Yaw.

## Blend Spaces

- `BS_Locomotion`: Idle → Walk (300 cm/s) → Run (550) → Sprint (700),
  direction -180° to +180°.
- `BS_Aim`: aim-walk at WalkSpeed (300) with upper-body aim pose.
- `BS_Crouch`: crouch idle / crouch walk at 180 cm/s.

## Requirements

- Use the UE Manny/Quinn skeleton or a fully original rig — never a
  rig/character copied from another game.
- IK Rig / IK Retargeter for foot placement; Control Rig only if
  needed for procedural aim.
- No animation is marked as done until it plays in the Editor on the
  real character mesh.
- Character mesh itself is not in this repo yet (Phase 01 is code +
  specs only). A placeholder capsule / Manny mesh may be used in the
  Editor for testing movement, and must be recorded as a placeholder.

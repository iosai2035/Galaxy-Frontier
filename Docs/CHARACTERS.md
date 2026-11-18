# Realistic Characters — Galaxy Frontier (Phases 02–03)

Original realistic human characters only. **No real-person
likenesses, no copied game characters, no copied assets.** Every
character in this game is an original design, authored from scratch.

## What "realistic" means here (and what it does not)

- Realistic human proportion and anatomy, PBR skin / cloth / metal,
  facial rig for expression, and LODs that hold up at third-person
  camera distance (320 cm) on iPhone/iPad.
- It does NOT mean photogrammetry of a real person, and it does NOT
  mean a mesh copied from another game. If a mesh cannot be proven
  original and licensed, it is not used.

## Proportion / anatomy (authoring spec, Blender / MetaHuman-style)

- Height ~1.75–1.85 m, ~7.5 heads tall, realistic limb ratios;
  no stylised / chibi proportions.
- Clean edge flow around eyes, mouth, shoulders, hips and knees
  (deformation zones). No triangles in deformation areas.
- Hands: full fingers, posed neutral; feet: boots with a sole.
- Two base bodies (male / female, Phase 05 customisation) share
  the same skeleton so one Animation Blueprint retargets to both.

## PBR material spec

| Surface | Base colour | Roughness | Metallic | Notes |
|---|---|---|---|---|
| Skin | PBR skin tone (see below) | 0.45–0.65, oily zones lower | 0.0 | Subsurface scattering (SSS) on, wrap lighting |
| Cloth / armour fabric | Desaturated sci-fi palette | 0.7–0.9 | 0.0 | Normal-mapped weave, no gloss |
| Metal / tech | Dark gunmetal, accent colour | 0.25–0.45 | 0.8–1.0 | Edge wear in roughness, not in colour |
| Hair | Dark, low saturation | 0.5 | 0.0 | Anisotropic highlight if supported |

- Skin tone is a material parameter (`SkinTone`, set by
  `AGFCharacter::ApplyCustomization`), not a new texture per
  character. Armour tint is `ArmourTint` on the armour slot.
- Textures: 2048 px max for the body on High, 1024 px on Medium,
  512 px on Low (see `DefaultScalability.ini` / device profiles).
  BC7 / ASTC compression, mipmaps on, streaming pool budgeted in
  `Docs/DEVICE_TEST_MATRIX.md`.
- Material slot convention (must match the mesh in the Editor):
  slot 0 = skin, slot 1 = cloth/armour, slot 2 = metal/tech.
  `AGFCharacter` and `AGFEnemyCharacter` both expose
  `ApplyMaterialToSlot()` / `ApplyForcedLOD()` for these slots;
  a missing slot or null material is logged, never a crash.

## Facial rig / Control Rig

- Face: jaw, eyes (look-at), brows, eyelids, mouth (phoneme-ish
  shapes for barks, not full lip-sync in Phase 02–03).
- Control Rig in the Editor drives aim / recoil / hit reactions;
  the C++ side only broadcasts events (`OnHealthChanged`,
  `OnDeath`, fire/recoil) — the rig listens, it does not poll.
- No facial capture of a real person is used.

## LODs

- LOD0: full mesh (High, close camera). LOD1: ~50% triangles.
  LOD2: ~25%. LOD3: silhouette only (distant enemies).
- `ForcedLOD = -1` (automatic) is the default in code; a forced
  LOD is only set per-platform in the Editor after measurement.
- Enemies use the same LOD chain — distant guards must not cost
  the same as the player character.

## Customisation hooks (code, Phases 02/05)

- `FGFCharacterCustomization` (`AGFCharacter.h`): `BodyType`
  (Male/Female), `SkinTone`, `HairColour`, `ArmourTint`,
  `AccentColour` (Phase 05, metal/tech slot 2), skin/armour
  slot indices.
- `AGFCharacter::ApplyCustomization()` creates dynamic material
  instances per slot and sets `SkinTone` / `ArmourTint` /
  `AccentColour` (slot 2 only when the mesh has 3+ slots).
- **Phase 05 — full customisation:** profiles are
  `UGFCharacterCustomizationData` assets (body, hairstyle,
  armour set, helmet, accessories, colours) owned by
  `UGFCustomizationManager`; the active profile id is stored
  in `UGFSaveGame`. The screen is `WBP_CharacterCustomization`
  (full spec in `Docs/UI-SCREENS.md`). Mesh swaps (hair /
  armour / helmet / accessories) are Editor-side — the
  Animation Blueprint reads the profile's ids and swaps the
  assigned meshes. **Cosmetic only:** no profile field affects
  any stat (see `Docs/EXPANSION.md`).

## What is BLOCKED (honest)

- **No Skeletal Mesh binary exists in this repo.** High-poly
  meshes must be authored in Blender / MetaHuman-style tooling on
  macOS/Windows. They cannot be authored as files on this Linux
  VM, and no mesh is faked as a `.uasset` here.
- No in-Editor material / rig tuning has been done (no UE here).
- No device FPS / memory measurement of the characters exists
  (no device here). The budgets above are targets to measure
  against in Phase 04, not measured results.

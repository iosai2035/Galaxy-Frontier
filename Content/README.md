# Content — Galaxy Frontier

No `.uasset` / `.umap` files are included in Phases 00-01.

Reason: Content assets (Blueprints, maps, meshes, materials, Niagara
systems, UMG widgets, Input Actions) are binary Editor assets. They
cannot be authored truthfully on this Linux VM, and faking them would
violate the project's quality rules (Docs/QUALITY.md §13).

## What goes here, and when

- `Content/Input/` — `IMC_Player`, `IA_*` (Phase 01, Editor — see Docs/INPUT.md).
- `Content/UI/` — `WBP_MobileHUD` etc. (Phases 01/04 — see Docs/MOBILE-HUD.md).
- `Content/Characters/` — `ABP_GFCharacter`, Blend Spaces (Phase 01 — see Docs/ANIMATION.md).
- `Content/Maps/` — `L_ZERON_Prototype` (Phase 03).
- `Content/Weapons/`, `Content/Enemies/`, `Content/Materials/`,
  `Content/Textures/`, `Content/Meshes/`, `Content/VFX/`,
  `Content/Audio/`, `Content/Data/`, `Content/Cinematics/` — as their
  phases land.

All assets must be original or properly licensed, with the licence
recorded. Large binaries go through Git LFS (see `.gitattributes`).

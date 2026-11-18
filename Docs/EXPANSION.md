# Expansion — Galaxy Frontier (Phase 05)

Design document for the Phase 05 systems: inventory, equipment,
character customisation, vehicles, planets, missions, progression
and the player save. Code references are the real classes; asset
names (`DA_*`) are Editor Data Assets — no `.uasset` is faked here.

## Inventory (`UGFInventoryComponent`, `UGFItemDataAsset`)

- **Cap:** 24 slots (`MaxSlots`), fixed grid in `WBP_Inventory`.
  No weight system in Phase 05 — slots are the only limit.
- **Stacks:** per-item `MaxStackSize` (weapons / mission items 1,
  Health Pack 5, Shield Cell 5, Ammo Box 200, Material 99).
  Adding fills partial stacks first, then empty slots.
- **Overflow:** `AddItem` returns how many were added; the rest
  is never dropped and never silently lost — `OnInventoryFull`
  fires and the caller keeps the remainder. Pickups stay in the
  world when the inventory is full.
- **Use:** consumables only, one unit per use, effect first /
  consume second — a failed effect (full health, no weapon for
  ammo) never eats the item.

### Starter item table (Editor Data Assets)

| ItemId | Type | Stack | Effect | Notes |
|---|---|---|---|---|
| `HealthPack` | HealthPack | 5 | +50 HP | Consumable |
| `ShieldCell` | ShieldCell | 5 | +50 shield | Consumable |
| `AmmoBox` | Ammo | 200 | +30 reserve rounds, current weapon | Consumable |
| `Pistol`, `AssaultRifle`, `SniperRifle`, `EnergyRifle` | Weapon | 1 | — | Weapon items; `WeaponData` = the Phase 02 asset |
| `ShieldGenerator_Mk1` | ShieldGenerator | 1 | +25 MaxShield while equipped | Not consumable |
| `PowerCell` | MissionItem | 1 | — | "The Lost Power Core" reward |
| `NexusDataCore` | MissionItem | 1 | — | "Signal in the Static" reward |
| `VaultAlloy` | Material | 99 | — | "The Frozen Vault" reward (crafting is Phase 06+) |
| `ScrapMetal` | Material | 99 | — | World pickup, no use yet |

- **Rarity is cosmetic only.** Common / Uncommon / Rare / Epic
  change the item's display colour in the inventory grid and
  nothing else. No rarity ever changes a number, and no item is
  sold for real money. There are no paid stats anywhere in the
  game.
- **Item ids are permanent.** A save stores item ids; renaming
  an id orphans every save that holds it. Add new ids, never
  rename old ones.

## Equipment (`UGFEquipmentComponent`)

- **Slots (3):** Primary Weapon, Secondary Weapon, Shield
  Generator.
- **Weapons:** the player must own the weapon item, and the
  weapon's class must be in the combat loadout
  (`UGFCombatComponent::WeaponClasses`). Equipping switches the
  combat component to that weapon. Equipping never spawns a
  weapon and never creates an item.
- **Shield generator:** equipping adds its `ShieldBonus` to
  `MaxShield` (Mk1: +25). Unequipping removes the bonus and
  clamps current shield to the new maximum — current shield is
  never healed by equipping. Only one generator at a time.
- **Equipping never removes the item from the inventory**, so
  unequipping can never lose an item. The save stores equipped
  ids; on load, an equipped id missing from the loaded inventory
  leaves the slot empty with a warning.

## Character customisation

- **Profiles:** `UGFCharacterCustomizationData` assets
  (`DA_Profile_Scout`, `DA_Profile_Vanguard`, `DA_Profile_Ranger`
  as starters). One profile = body + hairstyle + armour set +
  helmet + accessories + 4 colours (skin / hair / armour /
  accent). `UGFCustomizationManager` (GameInstance subsystem)
  owns the active profile; the save stores its id.
- **Bodies:** Male / Female — two base bodies sharing one
  skeleton, so one Animation Blueprint retargets to both
  (Docs/CHARACTERS.md).
- **Hairstyles (ids):** `Buzz`, `Short`, `Tied`, `Mohawk`.
  **Armour sets (ids):** `Scout`, `Vanguard`, `Ranger`.
  **Helmets (ids):** `None`, `Visor`, `FullHelm`.
  **Accessories (ids):** `Visor`, `Backpack`, `Scarf`.
  All are ids of meshes authored in the Editor; the code only
  applies colours, the Animation Blueprint swaps meshes.
- **Cosmetic only.** A profile changes no stat: no damage, no
  health, no speed, no shield. This is a design rule, not a
  tuning choice — it is enforced by the data (there is no stat
  field on a profile to tune).

## Vehicles

### Ground vehicle — Rover RX-7 (`AGFGroundVehicle`)

- `UFloatingPawnMovementComponent` (Engine module). No Chaos
  vehicle dependency is added in Phase 05: Chaos Vehicles would
  need the ChaosVehicles module, wheel setups and a physics
  asset, none of which is authored yet. If a later phase wants
  real wheel physics, that is a deliberate migration, not a
  silent swap.
- **Numbers:** MaxSpeed 1800 cm/s (~65 km/h), Acceleration 900,
  Brake 2400, TurnRate 75°/s, HP 250 (own `UGFHealthComponent`,
  no shield). Steering authority scales with speed — the rover
  cannot spin on the spot.
- **Enter/exit:** possession-based. While driving, the character
  is hidden with collision off; damage goes to the vehicle.
  Exiting above 50 cm/s is refused. Exit puts the character at
  the vehicle's right side (+250 cm, +50 cm up).
- **Destroyed rover:** `HealthComponent::IsDead()` blocks
  entering. Repair is not implemented in Phase 05.

### Spaceship — Starhopper (`AGFSpaceship`)

- Transport only. **No space combat in Phase 05** — the ship
  has no weapons and no combat code.
- `TravelToPlanet(Planet, CompletedMissionIds)`: validates the
  planet asset, checks the unlock, then a 5 s spool-up (real
  timer; the HUD shows the real countdown) and a real level
  load (`UGameplayStatics::OpenLevel(Planet->LevelName)`).
  Travel in progress blocks a second call; `CancelTravel()`
  cancels the spool-up only.
- The ship's pad speed is 600 cm/s (hover manoeuvres only).

## Planets

Full specs in `Docs/PLANETS.md`. Unlock chain: ZERON (default)
→ NEXUS (complete "The Lost Power Core") → VELTRON (complete
"Signal in the Static") → ASTRALIS (complete "The Frozen
Vault"). Recommended levels 1 / 3 / 5 / 8. Gravity scales:
ZERON 1.0, NEXUS 1.0, VELTRON 0.9, ASTRALIS 1.1 — applied by
each Level Blueprint on spawn (the asset only stores the scale).

## Progression (`UGFProgressionComponent`)

- **Curve:** XP to go from level L to L+1 is `100 * L`.
  Total XP to reach level N from level 1 is `50 * N * (N-1)`.

  | Level | XP to next | Total XP |
  |---|---|---|
  | 1 | 100 | 0 |
  | 2 | 200 | 100 |
  | 3 | 300 | 300 |
  | 4 | 400 | 600 |
  | 5 | 500 | 1000 |
  | 10 | 1000 | 4500 |
  | 20 | — (cap) | 19000 |

- **Max level:** 20. XP stops at the cap.
- **XP sources in Phase 05:** mission completion only
  (ZERON 150, NEXUS 250, VELTRON 400). Enemy kills grant no XP
  yet — that is Phase 06 design, not an oversight to patch in
  quietly.
- **Levels grant no stats.** A level-20 player deals the same
  damage per shot as a level-1 player. Levels gate planet
  recommendations and are shown in the HUD — nothing more.
- **Mission rewards** (`UGFMissionDataAsset::CompletionXPReward`
  + `CompletionRewardItem`) are granted exactly once per mission
  id, on `OnMissionCompleted`. If the inventory is full when the
  reward item is granted, the XP is still granted and the item
  is placed as an `AGFPickupActor` at the mission's end state
  by the Level Blueprint (see Docs/MISSIONS.md) — the item is
  never silently lost.

## Save system (`UGFSaveGame`, `UGFPlayerSaveManager`)

- **Slot:** `GF_PlayerSave`. One slot, user index 0.
- **Contents:** save version, level/XP, completed mission ids,
  inventory entries, equipped ids, customisation profile id,
  active planet id, reference copy of the language code.
- **Not stored:** passwords, secrets, tokens, account data —
  none exists in the game. Settings (graphics/camera/volumes)
  stay in `GF_SettingsSave` (Phase 04); the language copy here
  is a reference only. Mission-only progress stays in
  `GF_MissionSave` (Phase 03).
- **Versioning:** current version 1. A newer-version save is
  refused (never silently dropped). A version-0 save is
  migrated: the completed mission is imported once from the
  Phase 03 `GF_MissionSave` slot, then the version is set to 1.
- **Load rules:** a loaded save is validated
  (`UGFSaveGame::Validate`) before anything is applied — bad
  level/XP is clamped, invalid inventory entries are dropped,
  duplicate mission ids are removed. Applying a save to a
  character with no pawn yet returns false; the caller retries
  after spawn. Equipped ids missing from the loaded inventory
  leave their slots empty with a warning.
- **Save timing:** the game saves on mission completion and
  from the Pause menu (Level Blueprint / UI call
  `UGFPlayerSaveManager::SaveGame()`). There is no autosave
  timer in Phase 05 — a timer that saves mid-combat could
  capture a dead player.

## Pickups (`AGFPickupActor`)

- Interact through the normal interaction system (0.15 s scan,
  250 cm range). Grants via the inventory only.
- Full inventory: nothing is taken, the pickup stays, the
  prompt hides (the player is told by `OnInventoryFull`).
- Partial pickup allowed: what fits is taken, the rest stays
  with the updated quantity in the prompt.
- `bRespawns` pickups (arena health/ammo) return after
  `RespawnSeconds` (min 5 s). One-shot pickups (mission items)
  never respawn.

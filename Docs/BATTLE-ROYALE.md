# Battle Royale — Galaxy Frontier (Phase 06)

Design + implementation notes for the Battle Royale mode.
**Status: CREATED / REVIEWED — NOT BUILT, NOT TESTED.** The
mode has never run: there is no server, no second device and
no UE on this Linux VM. "60 players" is a design target, not a
tested number — the two-player test comes first
(`Docs/MULTIPLAYER.md` §7).

## 1. The mode in one paragraph

Up to 60 players land on an expanded ZERON island
(`L_ZERON_BR_Prototype`, spec only — no `.umap` in this repo).
No teams, no respawn, no revive. A safe zone shrinks in fixed
phases; outside it you take damage every second. The last
player alive wins. Placement XP is awarded by
`UGFMatchManager` through `UGFProgressionComponent::AddXP`
on the server pawn.

## 2. Map — L_ZERON_BR_Prototype (spec only)

- Expanded ZERON island, ~1200 × 1200 m of playable ground
  (the Phase 03 prototype `L_ZERON_Prototype` is ~400 × 400 m
  — the BR map is a new, larger level, not a rescale of the
  prototype's content).
- Points of interest (Editor placement, names only — no assets
  here): Landing Fields (open ground, 12 PlayerStarts),
  Outpost Ruins (the Phase 03 outpost, expanded), Relay Ridge,
  The Foundry (high loot), The Docks, The Crater.
- PlayerStarts: at least 60, spread across the island, never
  closer than 1500 cm to each other. `UGFMatchManager::
  ChooseSpawnPoint` assigns them round-robin; if a level has
  fewer starts than players it logs a warning and the GameMode
  falls back to its default spawn (there is no grid of fake
  actors — a spawn point that does not exist is not invented).
- Loot: `AGFPickupActor` placements (weapons, Health Packs,
  Shield Cells, Ammo Boxes — the Phase 05 item table,
  `Docs/EXPANSION.md`). Respawn pickups return after
  `RespawnSeconds` (min 5 s); mission items never respawn.
- The zone centre is the map centre (0, 0) for the prototype.
  A random centre per match is a later phase, not silently
  added here.

## 3. Zone phases (UGFZoneManager)

| Phase | Radius (cm) | Wait | Shrink | DPS outside |
|---|---|---|---|---|
| 0 | 6000 | 30 s | — | 0 |
| 1 | 4000 | 20 s | 60 s | 1 |
| 2 | 2500 | 15 s | 45 s | 2 |
| 3 | 1500 | 15 s | 40 s | 5 |
| 4 | 800 | 10 s | 35 s | 10 |
| 5 | 300 | 10 s | 30 s | 15 |
| 6 | 100 | 5 s | 25 s | 20 |
| 7 | 0 | 60 s | 20 s | 25 |

- Radius is the 2D (ground-plane) distance from the zone
  centre. The shrink is a linear interpolation over
  `ShrinkSeconds`; the radius never jumps.
- Zone damage is applied by the server every second to every
  alive player outside the radius. It uses the normal
  shield-first rule (`UGFHealthComponent::ApplyDamage`).
- A player killed by the zone gets no killer credit and
  places at the current alive count
  (`AGFGameMode::NotifyElimination(victim, nullptr)`).
- The current centre/radius are written to `AGFGameState`
  (replicated) so clients can draw the zone on the HUD.
- The final phase (radius 0, 25 DPS) ends the match: nobody
  can survive outside a zero-radius zone, so the last
  elimination arrives on its own. Its 60 s wait is a backstop
  only — at 25 DPS the match ends in seconds.

## 4. Landing / drop

- Players spawn on the ground at their assigned PlayerStart
  (no parachute / drop sequence in Phase 06 — a drop is a
  later phase, not a silent cut: the match starts with all
  players on the ground, weapons unspawned until they pick
  them up from loot points).
- The match starts (`Playing`) only after the 5 s countdown,
  so nobody is shot during spawn.

## 5. Eliminations, placements, XP

- Placement: a player who dies places at the current alive
  count (10 alive, one dies → 10th). The winner is the single
  remaining alive player (placement 1), assigned in
  `UGFMatchManager::AwardPlacementXP` at match end.
- Killer credit: only validated player damage counts
  (`UGFServerValidation::ApplyValidatedDamage`). Zone deaths
  credit nobody. A disconnect counts as an elimination with
  no killer credit.
- XP by placement (granted via `UGFProgressionComponent::
  AddXP` on the server pawn at match end):

  | Placement | XP |
  |---|---|
  | 1st | 500 |
  | 2nd–5th | 300 |
  | 6th–10th | 200 |
  | 11th–20th | 100 |
  | 21st+ | 50 |

- The XP is saved when the player returns to the lobby and
  the client saves (`UGFPlayerSaveManager::SaveGame`). The
  server never writes a client's save file, and a disconnected
  player gets no XP (there is no pawn left to award it to).
- Levels grant no combat stats (Phase 05 rule) — a level-20
  player deals the same damage per shot as a level-1 player.
  Placement XP is progression, not power.

## 6. Rules (explicit)

- No respawn, no revive, no rejoin after death or disconnect.
- Free-for-all only: no teams, no squads, no friendly fire
  rule needed (every other player is hostile).
- Spectating after death is a spec only (`WBP_Spectate`,
  `Docs/UI-SCREENS.md`) — NOT IMPLEMENTED in Phase 06. A dead
  player sees the match-result screen at match end
  (`ClientShowMatchResult`).
- The match ends when one player is left. There is no draw:
  if the final two players kill each other in the same server
  tick, the first processed elimination wins and the second
  player places 2nd (documented, not hidden).

## 7. What is NOT in Phase 06 (Battle Royale)

- Drop / parachute sequence, glider, redeploy.
- Random zone centres, moving zone centres.
- Loot tiers / rarity drops in BR (the Phase 05 item table
  is used as-is; rarity is cosmetic only).
- Kill feed (spec only — the HUD has no kill-feed widget
  yet; eliminations are visible in the end screen).
- Vehicles in BR (the Rover exists in Phase 05; BR vehicle
  spawns and vehicle damage rules are not designed yet).
- Ranked BR, seasons, battle pass, paid items. There are no
  paid stats anywhere in the game.

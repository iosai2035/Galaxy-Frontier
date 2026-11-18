# Multiplayer — Galaxy Frontier (Phase 06)

Network base for multiplayer matches. Code references are the
real classes in `Source/GalaxyFrontier/Multiplayer/`; widget
names (`WBP_*`) are Editor UMG assets — no `.uasset` is faked
here. **Status: CREATED / REVIEWED — NOT BUILT, NOT TESTED.**
There is no server, no second device and no UE on this Linux
VM, so nothing in this document has ever run. The two-player
test is the first exit criterion (see §7).

## 1. Architecture (one paragraph)

The server is authoritative for everything that matters. The
client only *asks* (Server RPCs on `AGFPlayerController`) and
the server *decides* (`UGFServerValidation` on the server pawn).
Game state that every client must see lives replicated on
`AGFGameState` (match state, timers, player counts, zone) and
per-player on `AGFPlayerState` (level, eliminations, alive,
placement). The client never writes replicated state — the
server-only setters log a warning and refuse on clients.

## 2. Classes

| Class | Where | Role |
|---|---|---|
| `AGFGameMode` | `Multiplayer/` | Match flow (Lobby → Countdown → Playing → Ended), spawn via `UGFMatchManager`, elimination routing, match timers. Server only. |
| `AGFGameState` | `Multiplayer/` | Replicated: `MatchState`, `MatchTimerRemaining`, `AlivePlayerCount`, `TotalPlayerCount`, `ZoneCenter`, `ZoneRadius`. |
| `AGFPlayerState` | `Multiplayer/` | Replicated: `PlayerLevel` (1..20), `Eliminations`, `bIsAlive`, `Placement` (0 = not placed). |
| `AGFPlayerController` | `Multiplayer/` | Server RPCs (fire / stop / interact / damage / set level) + `ClientShowMatchResult`. |
| `UGFServerValidation` | `Multiplayer/` | All server-side checks (see §4). ActorComponent on `AGFCharacter`. |
| `UGFReplicationManager` | `Multiplayer/` | Replication policy: priority + update interval by distance + idle suppression + heartbeat timeout. |
| `UGFAntiCheatManager` | `Multiplayer/` | Violation ledger + escalation (see `Docs/ANTI-CHEAT.md`). GameInstance subsystem. |
| `UGFSessionManager` | `Multiplayer/` | Sessions + matchmaking on the Online Subsystem (NULL first). GameInstance subsystem. |
| `UGFZoneManager` | `Multiplayer/` | Safe-zone phases (Battle Royale). Component on `AGFGameMode`. |
| `UGFMatchManager` | `Multiplayer/` | Spawn distribution, placements, XP rewards, match results. Component on `AGFGameMode`. |

## 3. Match flow

```
Lobby (30 s, or until 60 players)
  -> Countdown (5 s)
  -> Playing (zone drives the end)
  -> Ended (one player alive; XP + results sent)
```

- The lobby never starts below `MinPlayersToStart` (2): the
  timer extends by 15 s and keeps waiting.
- `EndMatch` is called by the GameMode when one player is left
  (or by the zone when the final phase ends the match through
  eliminations). It stops the zone, awards placement XP and
  notifies every client of its result.
- A player who disconnects mid-match counts as eliminated
  (`AGFGameMode::Logout` → `NotifyElimination`, no killer
  credit). In the lobby a disconnect just removes the player
  from the counts.

## 4. Server-authoritative validation (UGFServerValidation)

Every request is checked on the server before it is applied.
A failed check returns false, logs, and (in a session) reports
a violation to `UGFAntiCheatManager`. It never applies partial
state and never crashes.

| Request | Checked against | Tolerance |
|---|---|---|
| Fire | Current weapon's `FireRate` (Pistol 3/s, Assault 10/s, Sniper 0.8/s, Energy 5/s) | 15% early + 50 ms network slack |
| Movement | `AGFCharacter::SprintSpeed` (700 cm/s) | +10% speed, +500 cm displacement slack; gross jumps (> 2× speed) = teleport |
| Damage (direct) | Weapon `Damage` (exact), `MaxRange`, attacker/target alive, match Playing, attacker ≠ target | Exact damage match (±0.01) |
| Projectile hit | Damage ≤ 85 (Sniper, the highest in the game), range from shot origin, target alive, match Playing | Range measured from `SpawnLocation` |
| Interact / pickup | Target within 250 cm, `IGFInteractable::CanInteract` true, match Playing | None — range is the rule |

Notes:
- Damage is applied only by the server. `AGFProjectile::OnHit`
  returns without damage when `!HasAuthority()`; the server's
  own projectile is the one that hits. In single-player the
  local projectile IS the authority, so behaviour is unchanged.
- Fire and Interact input in `AGFCharacter` routes through the
  controller's Server RPCs only when `IsNetworkedGame()` is
  true (`World->GetNetMode() != NM_Standalone`). In
  single-player the handlers call the Combat / Interaction
  components directly, exactly as in Phases 02-03.
- Zone damage is server-originated (not a client request), so
  it needs no validation round-trip — it is applied directly
  by `UGFZoneManager` on the server.
- AI enemies (Phases 02-03) exist only in single-player
  missions. Networked AI is NOT IMPLEMENTED: an AI projectile
  has no validation component and applies damage directly on
  the server by construction. Do not put AI enemies in a
  networked match until that is designed.

## 5. Replication policy (UGFReplicationManager)

| Viewer distance | Update interval | Priority |
|---|---|---|
| < 1500 cm (near) | 0.05 s (20 Hz) | 1.0 |
| < 4000 cm (mid) | 0.10 s (10 Hz) | 0.6 |
| further | 0.20 s (5 Hz) | 0.3 |

- Idle players (velocity ≈ 0) get half priority — a stale copy
  of a stationary player is still correct.
- A move under 5 cm sends no position update at all.
- Heartbeat: the server records the last validated position
  time; silence longer than 30 s marks the player timed out
  (fallback — the engine's own connection timeout normally
  fires first).

What is replicated (and what is not):
- Replicated: match state + timers + counts + zone
  (`AGFGameState`), level / eliminations / alive / placement
  (`AGFPlayerState`), character movement (engine), health /
  shield changes (via the existing delegates — the HUD reads
  them, it does not poll).
- NOT replicated: inventory contents, settings, save data,
  mission progress. Those are per-player local state; the
  server never writes a client's save file.

## 6. Sessions & matchmaking (UGFSessionManager)

- Subsystem: Online Subsystem, **NULL first**
  (`DefaultEngine.ini`: `[OnlineSubsystem]
  DefaultPlatformService=Null`). LAN / two clients on one
  machine. EOS / Steam are specs only — no plugin, no app id,
  no account exists in Phase 06.
- A session advertises `GF_Level` (host's progression level,
  1..20) and `GF_Mode` (`BattleRoyale` / `Multiplayer`).
- Search filters: only sessions whose host level is within
  ±3 of the searching player's level are shown. Sessions that
  do not advertise a level are hidden.
- A search that finds nothing stops after 60 s and reports
  failure. **It never invents players and never adds bots.**
  Bots are NOT used to fill a match in Phase 06.
- Join: the connect string comes from the session
  (`GetResolvedConnectString`) — there is no hard-coded IP
  anywhere in the repo.
- The client's saved level reaches the server once, at login,
  via `AGFPlayerController::ServerSetPlayerLevel` (clamped
  1..20 on the server). A client sending 99 does not become
  level 99.

## 7. Exit criteria (NOT proven)

1. **Two-player test first:** dedicated server + two clients
   (NULL subsystem, LAN) complete one match: lobby → play →
   one elimination → winner + XP + result screens on both
   clients. Until this runs, the whole phase is CREATED only.
2. Then scale: 10, then 30, then 60 players, watching server
   CPU / bandwidth per the budgets in `Docs/PERFORMANCE.md`.
3. Battle Royale only after (1) passes — see
   `Docs/BATTLE-ROYALE.md`.

## 8. What is NOT in Phase 06

- Voice chat, text chat, friends, parties, cross-play.
- Teams / squads (free-for-all only).
- Spectating after death (spec only, `WBP_Spectate`).
- Reconnect / rejoin after a disconnect (a disconnect is an
  elimination).
- Ranked matchmaking (level ±3 is a filter, not a ranking).
- EOS / Steam / any live backend integration.
- Networked AI enemies, networked missions, co-op missions
  (missions are single-player in Phases 03/05).

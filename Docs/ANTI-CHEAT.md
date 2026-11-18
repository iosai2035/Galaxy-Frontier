# Anti-Cheat — Galaxy Frontier (Phase 06)

Design + implementation notes for the server-side anti-cheat.
**Status: CREATED / REVIEWED — NOT BUILT, NOT TESTED.** This
is detection of crude client tampering by a server that does
not trust the client. It is NOT a kernel anti-cheat, NOT a
client-side scanner, and it does not claim to stop a
determined cheater — the limits are stated in §5 and must not
be softened in any report.

## 1. Principle

The server is authoritative (see `Docs/MULTIPLAYER.md` §4).
Every gameplay request is validated on the server before it
is applied. A request that fails validation is rejected AND
reported: `UGFServerValidation` reports the violation to
`UGFAntiCheatManager`, which counts it per player and
escalates. Detection and enforcement are separate classes on
purpose — validation proves the fact, the manager decides the
consequence.

## 2. What is detected (UGFServerValidation)

| Violation | How it is proven on the server |
|---|---|
| Fire rate | Shots arriving faster than the weapon's `FireRate` allows (15% tolerance + 50 ms slack). Pistol 3/s, Assault 10/s, Sniper 0.8/s, Energy 5/s. |
| Speed hack | Movement faster than `SprintSpeed` (700 cm/s) + 10% over the elapsed time. |
| Teleport | A position jump beyond 2× top speed × time + 500 cm slack. |
| Impossible damage | Damage that is not exactly the weapon's `Damage`, or above 85 (Sniper, the highest in the game). |
| Invalid interact | Interaction with a target beyond 250 cm or not usable (`IGFInteractable::CanInteract` false). |
| Invalid pickup | A pickup request the player cannot reach — same range rule as interact (pickups go through the interaction system, Phase 05). |

What is NOT detected in Phase 06: aimbots (no aim analysis),
wallhacks (no line-of-sight audit log), lag switches (no
packet-timing analysis), modified game files (no integrity
check), or any client-side memory tampering. Those need a
client-side anti-cheat product, which is out of scope here.

## 3. Escalation (UGFAntiCheatManager)

| Violations in this session | Action |
|---|---|
| 1–2 | Warning (logged + counted, player stays) |
| 3 | Kick (`ClientReturnToMainMenu` with a reason) |
| 5 | Session ban + kick |

- Counts are per player, keyed by the PlayerState's UniqueId
  (stable in a session; player names are not used — names can
  be changed or duplicated).
- A "ban" is a **session-local, in-memory record only**. It is
  empty again when the server restarts. There is no persistent
  ban list, no backend, no account system in Phase 06.
- `ClearViolations` resets one player's count (human admin
  action, e.g. after an appeal). There is no automatic unban.
- Reports on a client (non-server) return `None` and change
  nothing. A null PlayerState report returns `None`, no crash.

## 4. False positives (designed tolerances)

- Fire rate: 15% early + 50 ms slack absorbs normal network
  jitter. A legitimate player on a laggy connection does not
  trip it; a modified client firing 2× the rate does.
- Movement: +10% speed and +500 cm displacement slack absorb
  replication jitter. A teleport (kilometres in one tick) is
  still caught.
- The `_Validate` step of every Server RPC stays permissive
  (null checks only) — returning false there disconnects the
  client, so the real rules live in `_Implementation` via
  `UGFServerValidation`, where a failure only rejects the
  request.
- If a tolerance ever proves too tight in real testing, the
  fix is to widen the documented tolerance — never to silently
  disable a check.

## 5. Limits (stated plainly)

- This system catches crude tampering (modified fire rates,
  speed/teleport hacks, forged damage numbers). It does not
  catch aimbots, wallhacks, or client-memory cheats.
- There is no client-side component: a cheater who modifies
  their own game files is only caught when the server sees an
  impossible request.
- There is no human review queue, no replay system, and no
  report-a-player flow in Phase 06. Kicks and session bans are
  automatic, by count, with the thresholds above.
- The ban does not survive a server restart and does not
  follow the player to another server — there is no shared
  backend to carry it.

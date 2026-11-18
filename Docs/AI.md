# AI — Galaxy Frontier (Phase 02)

## Classes

- `AGFEnemyCharacter` (`AI/AGFEnemyCharacter.h/.cpp`) — movement,
  health, attack, death. Decisions live in the controller, never here.
- `AGFEnemyAIController` (`AI/AGFEnemyAIController.h/.cpp`) — perception,
  Blackboard, Behavior Tree, fallback state machine.
- `UGFEnemyHealthComponent` — subclass of `UGFHealthComponent`, enemy
  tuning only (Health 100, Shield 0, no regen). Damage rules are
  inherited verbatim, not copied.

## States

`Idle → Patrol → Investigate → DetectPlayer → Chase → Attack →
TakeDamage → Search → Retreat → Die`

- **Idle / Patrol:** patrol points are random reachable points in
  `PatrolRadius` (1000 cm) from the enemy, via the NavMesh
  (`UNavigationSystemV1::GetRandomReachablePointInRadius`). No
  reachable point = stay put and retry next tick — never teleport.
- **DetectPlayer / Chase:** AI Perception (sight) fires
  `OnPerceptionUpdated`. Only the player pawn is a valid target;
  other sensed actors are ignored (no friendly fire).
- **Attack:** only inside `AttackRange` (800 cm), only when the
  attack cooldown (1.5 s) has expired. `Attack()` re-checks range
  and cooldown itself — the controller cannot force an out-of-range hit.
- **TakeDamage:** entered via `OnHealthChanged` in the Editor /
  Animation Blueprint (the C++ state is set by the controller when
  damage is observed; no Tick polling of health).
- **Search:** lost sight → move to `LastKnownPlayerLocation` for
  6 s, then give up and return to Patrol. Never hunts forever.
- **Retreat:** health fraction ≤ `RetreatHealthFraction` (0.25) →
  move away from the player towards home. Holds if already near home.
- **Die:** entered once via `OnDeath`. Movement disabled, collision
  disabled, AI timer stopped. Death animation is Animation-Blueprint
  owned, not code-owned.

## Starting values (master prompt §11, all editable UPROPERTY)

| Value | Default |
|---|---|
| Enemy Health | 100 |
| Detection Range | 1500 cm (LoseSight 1800 cm, FOV 90°) |
| Attack Range | 800 cm |
| Patrol Radius | 1000 cm |
| Attack Cooldown | 1.5 s |
| Movement Speed | 350 cm/s |
| Attack Damage | 15 |
| Retreat Health Fraction | 0.25 |

## Blackboard keys (must match the BT asset)

| Key | Type | Purpose |
|---|---|---|
| `TargetActor` | Object | Player pawn when sensed |
| `TargetLocation` | Vector | Last known player location |
| `HomeLocation` | Vector | Spawn location (patrol centre) |
| `State` | Enum (`EGFEnemyState`) | Current state, for BT decorators |

## Behavior Tree — Editor creation (macOS only, no fake .uasset)

1. Create Blackboard `BB_Enemy` with the four keys above.
2. Create Behavior Tree `BT_Enemy`, set its Blackboard to `BB_Enemy`.
3. Root → Selector:
   - Sequence (Decorator: Blackboard `State` == Die) → Task: Die
     (stop movement — the C++ death already disabled it).
   - Sequence (Decorator: `State` == Attack) → Task: Attack player
     (calls `AGFEnemyCharacter::Attack`, waits `AttackCooldownSeconds`).
   - Sequence (Decorator: `TargetActor` is set) → Task: MoveTo
     `TargetActor` (acceptance = `AttackRange * 0.5`).
   - Sequence (Decorator: `State` == Search) → Task: MoveTo
     `TargetLocation`, wait 6 s.
   - Task: Patrol (MoveTo random reachable point in `PatrolRadius`).
4. Assign `BT_Enemy` to `AGFEnemyAIController.BehaviorTreeAsset` and
   set the enemy's `AIControllerClass` to `AGFEnemyAIController`.

**Fallback:** if no BT asset is assigned, or `RunBehaviorTree`
fails, the controller runs a 0.25 s timer state machine with the
same states. The enemy is never inert because of a missing asset.
Perception is event-driven in both cases; there is no Tick.

## What is NOT done

- No BT / Blackboard `.uasset` files in this repo (Editor assets).
- No EQS, no hearing sense, no squad/coordinated AI (future).
- No ranged enemy weapons — enemies attack in melee range only
  (Phase 02). Ranged enemies are a Phase 05 expansion.
- No AI performance test on device — BLOCKED (no UE / no device).

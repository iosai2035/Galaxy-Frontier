# Reliability — Galaxy Frontier (Phases 02–03)

Goal: no crashes, no silent failures, no lag from polling, no
state that can get stuck. Every system below lists its failure
modes and how it degrades. "Works without errors, lag or gaps"
is a **target to verify on device in Phase 04** — it is NOT
claimed as tested here (no UE, no device on this Linux VM).

## Global rules (all systems)

- Null-check every component / actor / asset before use; log a
  `UE_LOG(LogTemp, Warning, ...)` and return safely. Never crash
  on a missing Editor assignment.
- Clamp / validate every numeric input (damage, ammo, health,
  stage index, slot index, LOD). Non-finite values are rejected.
- No Tick-heavy logic: timers and events only. The only Tick in
  the project is `AGFCharacter::Tick` (camera FOV/arm smoothing)
  and `AGFEnemyCharacter` / components have Tick disabled.
- State machines are explicit: a state can only be entered from
  a valid previous state, and death is terminal unless
  `Revive()` is called explicitly.
- Every public function that can fail returns `bool` / a count,
  so callers can react instead of assuming success.

## Per-system checklist

### Health (`UGFHealthComponent`)
| Failure mode | Behaviour |
|---|---|
| Damage after death | Returns 0, health stays 0 |
| Damage ≤ 0 / NaN / infinite | Rejected, warning logged, nothing applied |
| Damage > shield+health | Clamped at 0, `Die()` fires exactly once |
| Restore beyond max | Clamped at MaxHealth / MaxShield |
| Revive on a living owner | Warning, no-op |
| Regen with no damage delay | Regen waits `RegenDelaySeconds` after damage |

### Combat (`UGFCombatComponent` / `AGFWeaponBase`)
| Failure mode | Behaviour |
|---|---|
| Fire with 0 ammo | `CanFire()` false, dry-fire log, no projectile |
| Fire while reloading | Blocked; reload timer is the only path to ready |
| Reload with 0 reserve | `CanReload()` false, no reload started |
| Weapon with no data asset | Destroyed at spawn, warning logged |
| Weapon unequipped mid-reload | Reload timer cleared, `bIsReloading` reset |
| Fire with no weapon equipped | Returns false, no crash |
| Fire after owner death | `CanFire()` false (health component is authority) |
| Projectile with no world / no class | `SpawnProjectile()` returns null, caller handles it |

### Projectile (`AGFProjectile`)
| Failure mode | Behaviour |
|---|---|
| Hit on owner / instigator | Ignored (collision ignore list) |
| Hit beyond MaxRange | No damage, projectile destroyed |
| Missed shot | Destroyed by the 3 s lifetime cap — never flies forever |
| Hit a wall / prop (no health) | No damage, projectile destroyed |

### AI (`AGFEnemyCharacter` / `AGFEnemyAIController`)
| Failure mode | Behaviour |
|---|---|
| No Behavior Tree asset | Fallback 0.25 s timer state machine runs |
| BT fails to start | Warning + fallback state machine |
| No NavMesh in the level | `PickPatrolPoint` returns false, enemy stays put, warning logged |
| Player sensed then lost | Search last known location 6 s, then Patrol — never hunts forever |
| Enemy health ≤ 25% | Retreat, not Chase |
| Dead enemy | No further AI decisions, movement/collision disabled |
| Attack out of range | `Attack()` returns without damage |
| Attack on cooldown | Returns without damage |

### Missions (`UGFMissionSubsystem` / `UGFObjectiveComponent`)
| Failure mode | Behaviour |
|---|---|
| Complete wrong stage (out of order) | Rejected, warning, stage unchanged |
| Complete same stage twice | Second call returns false |
| Start mission with 0 objectives | Returns false |
| Start while a mission is active | Returns false |
| Save with no mission | Returns false |
| Load save for a different mission | Not applied, warning logged |
| Fail after completion | No-op, mission stays complete |
| Objective with no ObjectiveId | Warning, returns false |

### Interaction (`UGFInteractionComponent`)
| Failure mode | Behaviour |
|---|---|
| No interactable in range | Press is a no-op, no prompt |
| Target beyond 250 cm at completion | Interaction cancelled, warning logged |
| Target became unusable mid-hold | Does not complete |
| Cancel mid-hold | Timer cleared, actor stays unused |
| Used-once actor, second use | `CanInteract()` false |

### Character (`AGFCharacter`)
| Failure mode | Behaviour |
|---|---|
| No mapping context / action assigned | Warning in BeginPlay, input does nothing (no crash) |
| No Skeletal Mesh for customisation | Warning, customisation skipped |
| Material slot out of range | Warning, material not applied |
| Forced LOD out of range | Clamped to the mesh's LOD count |
| No Combat / Interaction component | Input handler logs a warning and returns |

## Performance / lag guards

- No per-frame overlap, no per-frame `FindComponentByClass` in
  hot paths (health is cached by the owner at BeginPlay where
  possible; the combat component caches its weapon pointer per call).
- Fire rate is a timer, AI is a 0.25 s timer, interaction scan is
  a 0.15 s timer, regen is a 0.5 s timer. Nothing in Phases 02–03
  polls every frame except camera smoothing.
- Projectiles self-destruct; timers are cleared in `EndPlay` /
  on death, so no timer keeps a dead actor alive.
- Draw-call / texture / polygon budgets are in
  `Docs/DEVICE_TEST_MATRIX.md` — they are targets, not measurements.

## What is still unproven

- All of the above is **code review only**. No test in this repo
  has ever run (see `GFCombatTest.cpp` / `GFMissionTest.cpp` —
  skeletons, NOT RUN). The failure cases listed in those tests
  must be executed in the Editor on macOS before any of them is
  marked PASSED.
- No device FPS / thermal / memory measurement exists. "No lag"
  cannot be claimed until Phase 04 device testing records it.

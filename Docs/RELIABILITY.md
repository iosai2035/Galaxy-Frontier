# Reliability — Galaxy Frontier (Phases 02–05)

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
| No HUDController | `HandlePause` logs a warning and returns; the game is not paused (a missing component is a setup error, visible in the log) |

### HUD (`UGFHUDController` / `UGFMobileHUDWidget`) — Phase 04
| Failure mode | Behaviour |
|---|---|
| No `HUDWidgetClass` assigned | `CreateHUD` returns false, retries 10× at 0.5 s, then warns and stops; game keeps running |
| No player controller at BeginPlay | Retry timer (0.5 s, max 10) instead of a crash |
| No Health / Combat / Mission component | That HUD section stays empty, warning logged; other sections still work |
| Weapon switched mid-magazine | Ammo delegates rebind to the new weapon; the old weapon never updates the HUD again |
| Fire held, then pause | `StopFire()` is called on pause; no shot fires while paused |
| Player dies | `OnDeath` stops firing, shows Game Over widget when assigned |
| Pause with no `PauseWidgetClass` | Game still pauses (input UIOnly, `SetGamePaused(true)`); no menu shown, warning logged |
| Widget created late / rebound | `RefreshAll()` pulls every value once — no waiting for the next change event |
| HUD widget name mismatch in Blueprint | `BindWidgetOptional` leaves it null; the missing element is logged, the rest works |

### Settings (`UGFSettingsManager` / `UGFSettingsSaveGame`) — Phase 04
| Failure mode | Behaviour |
|---|---|
| No save on first run | Defaults load, applied, saved on first change |
| Save slot holds a non-settings object | Warning, defaults used |
| Save fails (disk full) | `SaveSettings()` returns false; in-memory settings stay valid |
| Sensitivity NaN / 99 | NaN rejected (returns false); 99 clamped to 5.0 |
| Language "xx" | Setter returns false, nothing changes |
| Corrupt / out-of-range stored value | `ValidateSettings()` clamps / resets to "ar" / Medium |
| No player character yet | Camera apply is a no-op; applied when the character spawns |

### Performance (`UGFPerformanceManager`) — Phase 04
| Failure mode | Behaviour |
|---|---|
| No world at subsystem Initialize | Sampling starts lazily via `StartSampling()` from the HUD controller |
| FPS below target for < 5 s | No step-down (hysteresis threshold not reached) |
| FPS oscillating around the target | No change — dead band + 10 s cooldown hold the tier |
| FPS high for 10 s | Steps up one tier, never above the user's chosen quality |
| Auto quality disabled in Settings | No automatic change at all; manual `SetQuality` still works |
| Thermal state | Always `Unknown` in this build — never invented; record by hand (see `Docs/DEVICE_TEST_MATRIX.md`) |
| Draw calls / polygons over budget | Not fixed by this manager — fix the content (HISM, LODs); do not raise the budget |

### Inventory (`UGFInventoryComponent` / `UGFItemDataAsset`) — Phase 05
| Failure mode | Behaviour |
|---|---|
| AddItem with a null item, 0 or negative quantity | Returns 0, nothing changes |
| Item asset fails `Validate()` | Refused, warning logged, nothing added |
| Inventory full (24 slots) | Adds 0, fires `OnInventoryFull`, nothing is dropped |
| Stack over `MaxStackSize` | Split across stacks/slots; never exceeds the cap |
| UseItem at full health/shield, or with no weapon (ammo) | Returns false, the unit is NOT consumed |
| UseItem on a non-consumable or unowned item | Returns false, nothing changes |
| Save with an invalid inventory entry (null item, quantity ≤ 0) | Entry dropped on load with a warning |

### Equipment (`UGFEquipmentComponent`) — Phase 05
| Failure mode | Behaviour |
|---|---|
| Equip a weapon the player does not own | Returns false, loadout unchanged |
| Equip a weapon whose class is not in `WeaponClasses` | Returns false with a setup warning; nothing is spawned |
| Equip a second shield generator | Returns false; MaxShield raised only once |
| Unequip shield generator | Bonus removed, current shield clamped to the new max |
| Saved equipped id missing from the loaded inventory | Slot left empty with a warning, never a crash |

### Progression (`UGFProgressionComponent`) — Phase 05
| Failure mode | Behaviour |
|---|---|
| AddXP ≤ 0 | Returns 0, nothing changes |
| AddXP at level 20 (cap) | Returns 0, XP stays 0 |
| Mission reward broadcast twice | Granted once — the second call returns false |
| Mission completed is not the subsystem's current mission | Reward not granted, warning logged |
| `SetProgress` with level > 20 or XP ≥ needed | Returns false, progress unchanged |

### Save (`UGFSaveGame` / `UGFPlayerSaveManager`) — Phase 05
| Failure mode | Behaviour |
|---|---|
| Save version newer than this build | `Validate()` returns false, load refused, current progress kept |
| Save version 0 (pre-Phase 05) | Migrated: completed mission imported once from `GF_MissionSave` |
| Corrupt level/XP in the save | Clamped to level 1 / XP 0 with a warning |
| LoadGame with no player pawn yet | Returns false; the caller retries after spawn |
| No save slot at all | `LoadGame` returns false, `HasSaveGame` is false — not an error |

### Vehicles (`AGFGroundVehicle` / `AGFSpaceship`) — Phase 05
| Failure mode | Behaviour |
|---|---|
| Enter an occupied or destroyed rover | Returns false, possession unchanged |
| Exit while moving (> 50 cm/s) | Returns false, driver stays hidden in the vehicle |
| Travel to a locked planet | Returns false, no spool-up timer starts |
| Travel while already travelling | Returns false |
| Travel to a planet with no level name | Asset fails `Validate()`, travel refused |

### Pickups (`AGFPickupActor`) — Phase 05
| Failure mode | Behaviour |
|---|---|
| Interact with a full inventory | Nothing taken, pickup stays, prompt hidden |
| Pickup with no ItemData | Never available, warning at BeginPlay |
| EndPlay while a respawn timer runs | Timer cleared, no callback after destruction |

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

### Network — GameMode / GameState / PlayerState (Phase 06)
| Failure mode | Behaviour |
|---|---|
| Client calls a server-only setter (SetMatchState, SetAlive, AddElimination) | Warning logged, nothing changes — the server is authoritative |
| Lobby ends with < 2 players | Lobby extends by 15 s and keeps waiting; a match never starts below MinPlayersToStart |
| Player disconnects mid-match | Counts as an elimination (no killer credit); counts refreshed |
| Player disconnects in the lobby | Removed from the counts only |
| NotifyElimination called twice for the same victim | Second call is a no-op (victim already dead) |
| EndMatch called twice | Second call is a no-op (state already Ended) |
| No PlayerStarts in the level | Warning logged, GameMode falls back to its default spawn — no fake spawn point is invented |
| MatchManager asked for XP with no pawn (disconnected player) | Placement kept, no XP awarded — the server never writes a client's save |

### Sessions & matchmaking (UGFSessionManager) — Phase 06
| Failure mode | Behaviour |
|---|---|
| No Online Subsystem | Create/Find/Join return false, warning logged, no session invented |
| Search finds no session | Failure reported after the 60 s timeout — never invents players or bots |
| Session does not advertise a level | Hidden from search results (unfiltered mixing is refused) |
| Join with an out-of-range index | Returns false, warning logged |
| Join succeeds but no connect string resolves | Reported as failure, `bIsInSession` reset to false |
| Leave mid-match | Session destroyed; the server counts it as an elimination |

### Server validation (UGFServerValidation) — Phase 06
| Failure mode | Behaviour |
|---|---|
| Fire faster than the weapon rate | Request rejected, FireRate violation reported |
| Fire with no weapon / no weapon data | Returns false, no shot |
| Movement jump beyond speed × time + 500 cm | Rejected; gross jumps report Teleport, moderate over-speed reports SpeedHack |
| Damage != weapon Damage (e.g. 999 from a Pistol) | Rejected, ImpossibleDamage violation reported, no health removed |
| Damage to a dead target / self / out of range / in Lobby | Rejected, no damage |
| Projectile hit on a client (`!HasAuthority()`) | Destroyed with no damage — only the server's projectile hits |
| Interact beyond 250 cm or unusable target | Rejected, InvalidInteract violation reported |
| Any check in single-player (Standalone) | Passes — local play is never blocked by validation |

### Anti-cheat (UGFAntiCheatManager) — Phase 06
| Failure mode | Behaviour |
|---|---|
| Violations 1-2 | Warn only, player stays in the match |
| Violation 3 | Kick via `ClientReturnToMainMenu` with a reason |
| Violation 5 | Session ban (in-memory only) + kick |
| Report on a client / null PlayerState | Returns None, nothing changes |
| Banned player reports again | Returns Ban without re-kicking |
| Server restart | Ban record is empty again (session-local by design) |

### Zone (UGFZoneManager) — Phase 06
| Failure mode | Behaviour |
|---|---|
| Zone started on a client | Ignored (server only) |
| Player outside the radius | DPS every second (shield-first rule); inside takes none |
| Zone damage kills a player | No killer credit, placed at the current alive count |
| Final phase (radius 0) | 25 DPS — nobody survives outside a zero-radius zone |
| EndMatch while the zone runs | `StopZone()` clears the damage timer immediately |
| No phases configured | `StartZone` refuses (empty phase table) |

### Replication (UGFReplicationManager) — Phase 06
| Failure mode | Behaviour |
|---|---|
| No owner actor | Priority 0.0, no crash |
| Player idle (< 5 cm move) | No position update sent |
| Heartbeat silent > 30 s | `HasTimedOut()` true (fallback — the engine timeout normally fires first) |

## What is still unproven

- All of the above is **code review only**. No test in this repo
  has ever run (see `GFCombatTest.cpp` / `GFMissionTest.cpp` —
  skeletons, NOT RUN). The failure cases listed in those tests
  must be executed in the Editor on macOS before any of them is
  marked PASSED.
- No device FPS / thermal / memory measurement exists. "No lag"
  cannot be claimed until Phase 04 device testing records it.

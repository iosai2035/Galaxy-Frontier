# Missions — Galaxy Frontier (Phase 03)

## System

- `UGFMissionSubsystem` (GameInstance subsystem) — one active
  mission, stages in strict order, `SaveMissionProgress()` to slot
  `GF_MissionSave` (`UGFMissionSaveGame`).
- `UGFObjectiveComponent` — attach to the actor that represents
  a stage (trigger volume, terminal, Power Core, Extraction Zone).
  It forwards to the subsystem; the subsystem is the authority.
- `UGFMissionDataAsset` — the stage list. A new mission is data,
  not a code change. Instance `DA_TheLostPowerCore` is created in
  the Editor (no `.uasset` faked here).

## Rules

- Stages complete **in order only**. `CompleteObjective()` with
  the wrong id is rejected with a warning and changes nothing.
- Completing the same stage twice is a no-op.
- `StartMission` with 0 objectives, or while a mission is active,
  returns false.
- `FailMission` after completion is a no-op.
- `LoadMissionProgress` only applies if the save's mission id
  matches the currently loaded mission.
- Full player saving (level, health, inventory, customisation,
  settings) is `UGFSaveGame` in Phase 05 — mission progress alone
  is saved here, nothing else is faked as saved.

## Mission 1 — The Lost Power Core

| # | Stage / ObjectiveId | Where | How it completes |
|---|---|---|---|
| 1 | `Land` — Land in the Landing Zone | Landing Zone | Player spawns / lands (Level Blueprint calls `CompleteObjective("Land")` on BeginPlay) |
| 2 | `ReachOutpost` — Reach the Outpost | Path Landing Zone → Outpost | Enter Outpost trigger volume |
| 3 | `EnterBuilding` — Enter the building | Abandoned Station door | Interact with the station door |
| 4 | `FindPowerCore` — Find the Power Core | Power Core room | Interact with the Power Core (`MissionItem`) |
| 5 | `ActivateCore` — Activate the core | Power Core room | Hold-to-interact on the Power Core (duration set in the Editor) |
| 6 | `DefeatEnemy` — Defeat the guard enemy | Station interior | Enemy health reaches 0 (`OnEnemyDied` → objective) |
| 7 | `ReachExtractionZone` — Reach the Extraction Zone | Extraction pad | Enter Extraction Zone trigger volume |
| 8 | `CompleteMission` — Complete the mission | — | Automatic when stage 7 completes (subsystem advances) |
| 9 | `ShowResult` — Result and reward | Mission Complete UI | `WBP_MissionComplete` (Phase 04) shows the reward text from the data asset |

Stage 8 is the subsystem's completion step (not a player action);
stage 9 is the UI presentation step. The data asset lists all 9
stages so the HUD / Mission Panel can show the full chain.

## Editor setup (macOS)

1. Create Data Asset `DA_TheLostPowerCore` (`UGFMissionDataAsset`)
   with the 9 stages above (ids, titles, descriptions, locations).
2. In the level, attach `UGFObjectiveComponent` to each stage's
   actor/trigger and set its `ObjectiveId` to match the table.
3. The Power Core actor is `AGFInteractableActor` (type
   `MissionItem`) + `UGFObjectiveComponent` (`FindPowerCore` /
   `ActivateCore` — two components on the same actor, or one
   component whose completion is called twice by the interaction
   handler; do not reuse one ObjectiveId for two stages).
4. The guard enemy's `UGFEnemyHealthComponent.OnEnemyDied` is
   bound (Level Blueprint / a small handler) to complete
   `DefeatEnemy` — the enemy class itself does not know about
   missions (separation of concerns).
5. Level BeginPlay: `GetGameInstance()->GetSubsystem<UGFMissionSubsystem>()->StartMission(DA_TheLostPowerCore)`.

## What is NOT done

- No mission UI (`WBP_MissionPanel`, `WBP_MissionComplete`) —
  Phase 04.
- No mission rewards as inventory items — Phase 05.
- No second mission, no branching, no timed missions.

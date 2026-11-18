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

- **Reward (Phase 05):** 150 XP + item `PowerCell` (MissionItem).
  Asset: `DA_TheLostPowerCore` — `CompletionXPReward = 150`,
  `CompletionRewardItem = DA_PowerCell`.

## Mission 2 — Signal in the Static (NEXUS, Phase 05)

Unlocks NEXUS travel is the other way round: this mission is
*on* NEXUS, and completing it unlocks VELTRON. The player
reaches NEXUS by completing Mission 1.

| # | Stage / ObjectiveId | Where | How it completes |
|---|---|---|---|
| 1 | `LandNexus` — Land at the Relay District | Landing pad | Level BeginPlay calls `CompleteObjective("LandNexus")` |
| 2 | `ReachRelay` — Reach the Relay Station | Plaza → Relay Station | Enter Relay Station trigger volume |
| 3 | `RestoreRelayPower` — Restore relay power | Relay Station interior | Hold-to-interact on the relay console |
| 4 | `DefendRelay` — Defend the relay | Relay Station | Defeat 3 guard enemies (`OnEnemyDied` × 3 — the Level Blueprint counts them; the subsystem stage completes on the third) |
| 5 | `RetrieveDataCore` — Retrieve the Data Core | Relay core room | Interact with the Data Core (`MissionItem`) |
| 6 | `ReachExtractionZone` — Reach the extraction pad | Spire Base | Enter Extraction Zone trigger volume |
| 7 | `CompleteMission` — Complete the mission | — | Automatic when stage 6 completes |
| 8 | `ShowResult` — Result and reward | Mission Complete UI | `WBP_MissionComplete` shows the reward text |

- **Reward (Phase 05):** 250 XP + item `NexusDataCore` (MissionItem).
  Asset: `DA_SignalInTheStatic` — `MissionId = "SignalInTheStatic"`,
  `CompletionXPReward = 250`, `CompletionRewardItem = DA_NexusDataCore`.
- **Unlocks:** completing this mission adds `SignalInTheStatic`
  to `UGFSaveGame::CompletedMissionIds`, which unlocks VELTRON
  (`DA_Planet_VELTRON.UnlockMissionId`).

## Mission 3 — The Frozen Vault (VELTRON, Phase 05)

| # | Stage / ObjectiveId | Where | How it completes |
|---|---|---|---|
| 1 | `LandVeltron` — Land on the Ice Field | Landing pad | Level BeginPlay calls `CompleteObjective("LandVeltron")` |
| 2 | `CrossIceField` — Cross the Ice Field | Ice Field | Enter Vault Approach trigger volume (rover recommended, on foot allowed) |
| 3 | `EnterVault` — Enter the vault | Vault Entrance door | Interact with the vault door |
| 4 | `FindVaultKey` — Find the Vault Key | Vault antechamber | Interact with the Vault Key (`MissionItem`) |
| 5 | `UnlockVault` — Unlock the vault | Vault inner door | Hold-to-interact on the vault lock (requires `FindVaultKey` — the stage order enforces it) |
| 6 | `DefeatGuardian` — Defeat the Vault Guardian | Vault chamber | Guardian enemy health reaches 0 |
| 7 | `ReachExtractionZone` — Reach the extraction pad | Glacier Ridge | Enter Extraction Zone trigger volume |
| 8 | `CompleteMission` — Complete the mission | — | Automatic when stage 7 completes |
| 9 | `ShowResult` — Result and reward | Mission Complete UI | `WBP_MissionComplete` shows the reward text |

- **Reward (Phase 05):** 400 XP + item `VaultAlloy` (Material).
  Asset: `DA_TheFrozenVault` — `MissionId = "TheFrozenVault"`,
  `CompletionXPReward = 400`, `CompletionRewardItem = DA_VaultAlloy`.
- **Unlocks:** completing this mission unlocks ASTRALIS
  (`DA_Planet_ASTRALIS.UnlockMissionId = "TheFrozenVault"`).

## Mission rewards (Phase 05)

- Rewards are granted by `UGFProgressionComponent` on
  `UGFMissionSubsystem::OnMissionCompleted`, exactly once per
  mission id. XP is always granted; the item is added to the
  inventory (1 unit).
- **Inventory-full fallback:** if the inventory is full when a
  reward item is granted, the XP is still granted and the item
  is NOT lost — the Level Blueprint places the reward item as
  an `AGFPickupActor` at the mission's end state (extraction
  pad), so the player picks it up there. Every mission level
  must place that pickup; a mission without it loses the item
  on a full inventory (logged as a warning).

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
  Phase 04. (Done in Phase 04 — specs in Docs/UI-SCREENS.md.)
- No mission rewards as inventory items — Phase 05.
  (Done in Phase 05 — see "Mission rewards" above.)
- No second mission, no branching, no timed missions.
  (Two more missions added in Phase 05 — Missions 2 and 3
  above. Branching and timed missions are still NOT done.)

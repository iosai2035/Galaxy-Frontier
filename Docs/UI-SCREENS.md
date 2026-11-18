# UI Screens — Galaxy Frontier (Phase 04, build specs)

Every screen below is a UMG Widget Blueprint (`.uasset`) built in
the Unreal Editor on macOS. **None of them is a file in this repo**
— no `.uasset` is faked. Each section is the exact build spec for
one widget, with its parent class, its bindings and its Editor
creation steps. Status for all of them: **CREATED (spec) /
REVIEWED — NOT BUILT, NOT TESTED** until created and played in
the Editor on macOS and on a real device.

Common rules for all screens:
- Landscape only. Anchor-based layout, no fixed pixel positions.
- Safe Areas mandatory (notch / Dynamic Island / iPad edges).
- Text in `FText` (localisable), never hard-coded `FString`
  display text in Blueprint nodes that the player reads.
- Buttons: minimum 44 × 44 dp hit area.
- No screen polls per frame. Every value comes from a delegate
  event or a single pull on construct / on show.
- Navigation: every screen has a Back button that returns to the
  previous screen, and Pause has Resume / Settings / Main Menu.
- Language: all player-facing text goes through the localisation
  pipeline (Docs/SETTINGS.md) — ar/en first, the other 18
  languages when loc data exists.

## WBP_MainMenu

- Parent: `UUserWidget`. Shown on game start (GameMode / level
  Blueprint creates it; the character HUD is not created yet).
- Elements: game title "Galaxy Frontier", buttons Continue
  (only when a mission save exists in slot `GF_MissionSave`),
  New Game (starts "The Lost Power Core" via
  `UGFMissionSubsystem::StartMission` with `DA_TheLostPowerCore`),
  Settings (opens `WBP_Settings`), and a version line
  ("0.1.0 — UE 5.5", from `DefaultEngine.ini` ProjectVersion).
- Continue with no save: the button is disabled, not hidden —
  the player must see why it cannot be pressed.
- Background: the ZERON vista (Editor level / image asset —
  no image is faked in this repo; a plain gradient is the
  fallback until the art exists).
- Back: none (it is the root screen); quitting the game on iOS
  is done with the OS gesture, there is no Quit button.

## WBP_PauseMenu

- Parent: `UUserWidget`. Created by `UGFHUDController` when
  `PauseWidgetClass` is assigned; shown/hidden by
  `OpenPauseMenu()` / `ClosePauseMenu()`.
- Elements: Resume (closes the menu, `SetGamePaused(false)`,
  input mode GameOnly), Settings (opens `WBP_Settings` on top,
  pausing stays on), Restart Mission (re-starts the current
  mission from stage 0 via the mission subsystem — only when a
  mission is active), Main Menu (unpauses, opens
  `WBP_MainMenu`; the mission progress stays saved in
  `GF_MissionSave`), and a line showing the current objective
  title + the live quality tier.
- While paused, `UGFHUDController` has already called
  `CombatComponent::StopFire()` — holding Fire into the pause
  must not resume firing on unpause.
- The game is paused with `UGameplayStatics::SetGamePaused`;
  the menu must not try to pause by any other mechanism.

## WBP_Settings

- Parent: `UUserWidget`. Reads/writes through
  `UGFSettingsManager` (GameInstance subsystem) — never edits
  the save object directly.
- Sections and controls (every control maps to a field in
  `UGFSettingsSaveGame`, ranges in Docs/SETTINGS.md):
  - Camera: sensitivity slider 0.1–5.0 (default 1.0), invert
    pitch toggle (default off).
  - Touch: button scale slider 0.5–2.0 (default 1.0), joystick
    side Left/Right (default Left), "Move joystick" mode: drag
    the joystick base on a preview canvas, stored as
    `JoystickAnchorX/Y` (0.05–0.95).
  - Graphics: quality choice Low/Medium/High (default Medium),
    auto-quality toggle (default on), a note line when the live
    quality is auto-lowered ("Lowered automatically to Medium
    — tap Restore", calls
    `UGFPerformanceManager::RestoreUserQuality()`).
  - Audio: Master / Music / SFX sliders 0–1 (defaults 1.0 /
    0.7 / 0.8). Stored + validated now; live mix needs the
    SoundMix asset (Editor step, Docs/SETTINGS.md).
  - Language: dropdown of the 20 supported codes, default "ar".
  - Diagnostics: toggle for the FPS/RAM overlay (default off).
  - Reset button: `ResetToDefaults()` + a confirmation line.
- Every change saves immediately (`SaveSettings()` in the
  manager's setters) — there is no separate "Apply" button
  that can be forgotten, and no change is lost if the player
  leaves the screen.
- Back button returns to the caller (Pause or Main Menu).

## WBP_Inventory

- **Status: full spec (Phase 05). The backend exists now
  (`UGFInventoryComponent` + `UGFEquipmentComponent` on
  `AGFCharacter`); the `.uasset` widget itself is still built
  in the Editor on macOS.**
- Parent: `UUserWidget`. Opened by `AGFCharacter::HandleOpenInventory`
  (OpenInventory action) via `UGFHUDController::ToggleInventory()`.
  Opening pauses the game (same rule as the Pause menu) and
  stops firing; closing resumes it. The Pause menu and the
  Inventory screen never stack — opening one while the other
  is open is refused (see `UGFHUDController`).
- Elements: a 24-slot grid (4 rows × 6 columns, the
  `MaxSlots` cap), one cell per inventory entry showing the
  item icon (Editor-assigned), name, quantity for stacks, and
  a rarity-coloured border (rarity is display-only). Below the
  grid: the selected item's description + Use / Equip buttons.
  Use is shown only for consumables the player owns; Equip is
  shown only for weapon / shield-generator items.
- Data comes from `UGFInventoryComponent::Entries` only — read
  on construct and on `OnInventoryChanged`. The widget never
  keeps its own item list and never hard-codes items.
- Actions: Use calls `UGFInventoryComponent::UseItem(ItemId)`
  (one unit, effect first); Equip (weapon) calls
  `UGFEquipmentComponent::EquipWeapon(ItemData, Slot)`; Equip
  (shield generator) calls `EquipShieldGenerator(ItemData)`.
  A failed call (full health, wrong loadout) leaves the
  inventory unchanged and shows the warning line.
- A 3-row equipment strip at the top shows the equipped
  Primary / Secondary / Shield Generator with an Unequip
  button per slot (`UnequipSlot`).
- The "inventory full" state is shown when `AddItem` returns
  less than requested (`OnInventoryFull`): the widget shows
  "Inventory full" and pickups stay in the world.

## WBP_WeaponSelection

- Parent: `UUserWidget`. The in-game weapon switcher (also
  reachable from the HUD `SwitchWeaponButton`, which cycles).
- Elements: one row per owned weapon (the four from Phase 02:
  Pistol, Assault Rifle, Sniper Rifle, Energy Rifle), each
  showing the weapon name, damage / fire rate / magazine from
  its `UGFWeaponDataAsset`, current ammo / reserve, and a
  "current" marker on the equipped one.
- Selecting a row calls `UGFCombatComponent::SwitchWeapon(index)`
  and closes the screen. An index with no weapon is never shown.
- The screen pauses the game while open (same mechanism as the
  Pause menu, via `UGFHUDController`), and closing it resumes.
- Laser / Melee rows do not exist yet — the types exist in
  `EGFWeaponType` but no data assets do (Phase 02 NOT
  IMPLEMENTED list). Do not add rows for them.

## WBP_MissionPanel

- Parent: `UUserWidget`. The mission journal, opened from the
  HUD (objective text tap) and from the Pause menu.
- Elements: mission name "The Lost Power Core", the 9 stages
  in order (Land → ReachOutpost → EnterBuilding → FindPowerCore
  → ActivateCore → DefeatEnemy → ReachExtractionZone →
  CompleteMission → ShowResult) with a done / current / locked
  marker per stage, and the current objective title + its
  description from `UGFMissionDataAsset` / `FGFObjectiveData`.
- Data comes from `UGFMissionSubsystem` only
  (`CurrentMission`, `CurrentStageIndex`, `bMissionCompleted`) —
  the panel never keeps its own copy of the stage list, so it
  cannot drift from the subsystem. It refreshes on
  `OnObjectiveCompleted` / `OnMissionCompleted`, plus one pull
  on show.
- With no active mission the panel shows "No active mission"
  and the New Game button (same action as Main Menu).

## WBP_CharacterCustomization

- **Status: full spec (Phase 05). The backend exists now
  (`UGFCharacterCustomizationData` + `UGFCustomizationManager`);
  the `.uasset` widget itself is still built in the Editor.**
- Parent: `UUserWidget`. Opened from the Main Menu (and from
  the Pause menu). Live preview on the character mesh
  (Editor-assigned Skeletal Mesh) — changing a control applies
  it immediately through `UGFCustomizationManager`.
- Controls: body-type selector (Male / Female — two base bodies,
  one skeleton); hairstyle list (`Buzz`, `Short`, `Tied`,
  `Mohawk`); armour-set list (`Scout`, `Vanguard`, `Ranger`);
  helmet list (`None`, `Visor`, `FullHelm`); accessory toggles
  (`Visor`, `Backpack`, `Scarf`); four colour pickers (skin /
  hair / armour / accent). A profile name field + Save button
  writes the profile as a `UGFCharacterCustomizationData`
  asset choice; Load lists `AvailableProfiles`.
- Data comes from `UGFCustomizationManager` only. Selecting a
  profile calls `SetActiveProfile()`; saving stores the
  profile id in `UGFSaveGame` (via `UGFPlayerSaveManager`).
- **Cosmetic only, stated on the screen:** a line under the
  preview reads "Looks only — customisation never changes
  combat stats." There is no control that grants a stat.
- What is NOT in this screen: new meshes (authored in the
  Editor), purchases, or any real-money control. Do not add
  them.

## WBP_LoadingScreen

- Parent: `UUserWidget`. Shown during level load
  (`L_ZERON_Prototype`) and during the first mission start.
- Elements: progress bar (driven by the async load progress —
  a real value from the streaming/level load, never a fake
  timer), the current tip line (rotating gameplay tips in
  `FText`), and the mission name being loaded.
- The bar must reflect real load progress. If the load system
  cannot report progress, the screen shows an indeterminate
  spinner and the text "Loading…" — a bar that fills on a timer
  while nothing loads is fake progress and is forbidden.

## WBP_GameOver

- Parent: `UUserWidget`. Created by `UGFHUDController` when
  `GameOverWidgetClass` is assigned, on
  `UGFHealthComponent::OnDeath` (player death only — enemy
  death uses `UGFEnemyHealthComponent::OnEnemyDied` and never
  opens this screen).
- Elements: "Mission Failed" + the cause line (last damage
  source is not tracked in Phase 02 — the line reads the
  mission name + stage reached, from the mission subsystem),
  buttons Retry (restart the current mission from stage 0)
  and Main Menu.
- Firing is stopped by the controller before the widget shows;
  input mode is UIOnly while it is open.

## WBP_MissionComplete

- Parent: `UUserWidget`. Created by `UGFHUDController` when
  `MissionCompleteWidgetClass` is assigned, on
  `UGFMissionSubsystem::OnMissionCompleted`.
- Elements: "Mission Complete", the mission name, the 9 stages
  all marked done, a reward line (text only in Phase 04 —
  rewards as inventory items are Phase 05, so the line reads
  "Reward: mission complete" and nothing more), buttons
  Continue (back to the world / free roam on ZERON) and Main
  Menu.
- The mission subsystem marks the mission complete before
  this widget opens; completing twice is impossible
  (`CompleteMission` is a no-op after completion — Phase 03).

## WBP_Lobby (Phase 06, spec only)

- Parent: `UUserWidget`. Shown after joining a session, before
  the match starts (GameMode state `Lobby`, `AGFGameState::
  MatchState`). Created by the lobby flow, not by
  `UGFHUDController` (the HUD controller owns in-match screens
  only).
- Elements: session name + mode line ("Battle Royale — up to 60
  players"), player list (name + level + alive state, from
  `AGFGameState::PlayerArray` / `AGFPlayerState`, read on
  construct and on the `OnMatchStateChanged` event — never
  polled), a countdown line (`MatchTimerRemaining`, "Match
  starts in 23 s"), a Ready toggle (visual only in Phase 06 —
  readiness does not gate the start; the timer does), and a
  Leave button (calls `UGFSessionManager::LeaveSession`; in
  the lobby this just removes the player).
- The list shows at most 60 rows; an empty lobby shows "Waiting
  for players…" instead of an empty list.
- Back: Leave returns to `WBP_MainMenu`.

## WBP_Matchmaking (Phase 06, spec only)

- Parent: `UUserWidget`. Opened from `WBP_MainMenu` ("Play
  Online" button). Drives `UGFSessionManager::FindSessions`
  with the player's saved level.
- Elements: a level line ("Your level: 7 — searching sessions
  within ±3 levels"), a searching spinner, a results list
  (host level + mode + player count per session, from
  `GetFoundSessionCount()` / `GetFoundSessionHostLevel()`),
  Join (calls `JoinSession(index)`), Host (calls
  `CreateSession(level, bIsBattleRoyale)` then opens
  `WBP_Lobby`), and Cancel.
- No results: the list shows "No sessions found — host one or
  try again" after the 60 s search timeout. No fake sessions
  and no bots are ever shown — an empty list is an empty list.
- Back: Cancel returns to `WBP_MainMenu`; leaving mid-search
  cancels the search (`CancelFindSessions`).

## WBP_MultiplayerHUD (Phase 06, spec only)

- Parent: `UGFMobileHUDWidget` (the Phase 04 HUD base — same
  joystick, buttons and safe-area rules as `WBP_MobileHUD`).
- Extra elements over the single-player HUD: alive-players
  counter (`AGFGameState::AlivePlayerCount` / `TotalPlayerCount`),
  eliminations counter (own `AGFPlayerState::Eliminations`),
  zone status line (current radius + DPS outside, from
  `AGFGameState::ZoneRadius` — "Outside the zone!" warning
  when the player is outside it), placement line at match end
  ("#1 — Victory" / "#7", from `ClientShowMatchResult`), and
  a match-result panel (placement, eliminations, XP earned).
- The zone is drawn as a circle on the mini-map area when the
  map asset exists; until then the status line is the only zone
  display — no fake map is drawn.
- Back: none in-match (Pause handles leaving); the result panel
  has Main Menu and Play Again (Play Again re-opens
  `WBP_Matchmaking`).

## WBP_Spectate (Phase 06, spec only — NOT IMPLEMENTED)

- Parent: `UUserWidget`. Intended screen for watching a match
  after elimination. **Not implemented in Phase 06**: there is
  no spectate camera, no player switching, and no code that
  opens this screen. A dead player sees the match-result panel
  on `WBP_MultiplayerHUD` at match end instead.
- Spec kept so the screen is not built ad-hoc later: when it
  is implemented it must show the alive players list, a
  "watching <name>" line, and a Leave button — and it must
  never let a spectator send gameplay input.

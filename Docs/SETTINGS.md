# Settings — Galaxy Frontier (Phase 04)

Settings are owned by `UGFSettingsManager` (GameInstance
subsystem) and stored in `UGFSettingsSaveGame`, save slot
`GF_SettingsSave` (slot index 0). The Settings screen is
`WBP_Settings` (Docs/UI-SCREENS.md). Every value is validated
by `UGFSettingsManager::ValidateSettings()` before it is used;
a corrupt or hand-edited save can never drive a setting out of
range, and a non-finite number is rejected, never silently
clamped into a plausible value.

## Defaults (safe on every device)

| Setting | Field | Default | Range |
|---|---|---|---|
| Camera sensitivity | `CameraSensitivity` | 1.0 | 0.1–5.0 |
| Invert camera pitch | `bInvertCameraPitch` | false | — |
| Button scale | `ButtonScale` | 1.0 | 0.5–2.0 |
| Joystick side | `JoystickSide` | Left | Left / Right |
| Joystick anchor X | `JoystickAnchorX` | 0.12 | 0.05–0.95 |
| Joystick anchor Y | `JoystickAnchorY` | 0.75 | 0.05–0.95 |
| Graphics quality | `GraphicsQuality` | Medium | Low / Medium / High |
| Auto quality | `bAutoQualityEnabled` | true | — |
| Master volume | `MasterVolume` | 1.0 | 0.0–1.0 |
| Music volume | `MusicVolume` | 0.7 | 0.0–1.0 |
| SFX volume | `SFXVolume` | 0.8 | 0.0–1.0 |
| Language | `LanguageCode` | "ar" | one of the 20 below |
| Diagnostics overlay | `bShowDiagnostics` | false | — |

Defaults are deliberately middle-of-the-road: Medium graphics
(30 fps target), sensitivity 1.0, volumes audible but not full
for music/SFX. A first-run player on an old iPhone gets a
playable game, not a High preset that throttles.

## What ApplySettings() actually applies

- **Graphics:** `UGameUserSettings::SetOverallScalabilityLevel`
  (0 Low / 1 Medium / 2 High — Epic is never used on mobile)
  + `ApplySettings(false)`, and the streaming pool CVar
  (`r.Streaming.PoolSize` 512 MB on High, 256 MB otherwise).
  This is the same path `UGFPerformanceManager` uses for its
  auto step-down, so manual and automatic changes cannot
  disagree about what "Medium" means.
- **Camera:** `AGFCharacter::LookSensitivity` and
  `bInvertLookPitch` (used in `HandleLook`). Applied to the
  current player character; if there is no character yet (e.g.
  the Main Menu), the values are applied when the character
  spawns — the settings object is the source of truth, not a
  one-time push.
- **Language:** `FInternationalization::SetCurrentCulture(code)`.
  Switching the culture is real, but full translations need
  the Editor localisation targets (see below) — without loc
  data the UI falls back to the source text. No translation is
  invented in code.
- **Touch layout:** read by `WBP_MobileHUD` from
  `GetSettings()` at construct time (button scale, joystick
  side/anchor). A live HUD re-reads the layout when the
  Settings screen closes.
- **Audio volumes:** validated + saved only. The live mix
  needs a SoundMix asset authored in the Editor (see below).
  `ApplySettings()` does not claim to change any volume.

## Languages (20)

`ar en ru es fr de tr fa pt id zh ja ko hi ur bn th vi ms ta`
— the same set as the download bot's i18n list. Default "ar".
`SetLanguage()` returns false and changes nothing for any
other code; `ValidateSettings()` resets an unsupported stored
code to "ar".

## Editor steps (macOS, NOT done here)

1. **Localisation:** create a Localization Target in the
   Editor, gather the UI text (`FText` in widgets and
   `UGFWeaponDataAsset::WeaponName` etc.), translate ar/en
   first, export/import the other 18 languages when ready.
   Until a language has loc data, its UI shows the source
   text — that is the documented fallback, not a bug to hide.
2. **SoundMix:** create a Sound Mix asset with Master / Music /
   SFX buses, and a Blueprint (or C++ call in the manager)
   that pushes `MasterVolume` / `MusicVolume` / `SFXVolume`
   into it on `ApplySettings()`. Until then, volumes are
   stored values only.
3. **WBP_Settings:** build the screen per Docs/UI-SCREENS.md;
   every control calls the matching `UGFSettingsManager`
   setter (each setter validates, saves and applies).
4. Test: change a setting, quit to the OS, relaunch — the
   setting must persist (slot `GF_SettingsSave`). Corrupt the
   slot (delete it mid-write in a test) — the game must start
   on defaults with a warning, never crash.

## Failure modes

| Failure | Behaviour |
|---|---|
| No save on first run | Defaults load, applied, save written on first change |
| Save of the wrong class in the slot | Warning logged, defaults used |
| Save fails (disk full) | `SaveSettings()` returns false, in-memory settings stay valid |
| Sensitivity NaN / 99 | Setter rejects NaN; 99 clamps to 5.0 |
| Language "xx" | Setter returns false, nothing changes |
| No player character yet | Camera apply is a no-op; settings apply on spawn |
| Reset | `ResetToDefaults()` restores the table above, saves + applies |

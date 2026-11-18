# Dedicated Server — Galaxy Frontier (Phase 06)

How to build and run the dedicated server for multiplayer
testing. **Status: CREATED / REVIEWED — NOT BUILT, NOT
TESTED.** No server has ever been built or run: there is no UE
on this Linux VM, no macOS host here, and no server address
exists. There are no IP addresses, hostnames, passwords or
credentials in this document — a server operator supplies
their own machine and network.

## 1. What the server is

- Target: `GalaxyFrontierServer` (`Source/
  GalaxyFrontierServer.Target.cs`, `TargetType.Server`) — the
  same `GalaxyFrontier` module, built headless (no rendering,
  no audio, no input).
- GameMode: `AGFGameMode` (Lobby → Countdown → Playing →
  Ended). GameState: `AGFGameState`. PlayerState:
  `AGFPlayerState`. PlayerController: `AGFPlayerController`.
- The server is authoritative: all damage, fire, movement,
  interaction and pickup requests are validated on the server
  (`UGFServerValidation`). See `Docs/MULTIPLAYER.md` §4.

## 2. Build (on a machine with UE 5.5)

Prerequisites (not present here): UE 5.5 with the Linux
server platform support installed, on a Linux build machine
(or a Windows machine cross-compiling for Linux — either way,
the build machine needs the engine, which this VM does not
have).

```sh
# From the engine's Build/BatchFiles directory, with the
# project checked out (replace <EngineDir> and <ProjectDir>
# with the real paths on the build machine):
<EngineDir>/Engine/Build/BatchFiles/RunUAT.sh BuildCookRun \
  -project=<ProjectDir>/GalaxyFrontier.uproject \
  -noP4 -nocompileeditor \
  -platform=Linux -server -serverconfig=Development \
  -cook -build -stage -pak -archive \
  -archivedirectory=<ProjectDir>/Build/LinuxServer
```

- `-server` builds the `GalaxyFrontierServer` target.
- Do NOT build the server on the iOS workflow
  (`.github/workflows/build-ios-ipa.yml` builds the iOS client
  only, on macOS).
- Expected size: the server build is the cooked content plus
  the server binary (several GB — measure on the build
  machine; no size is claimed here).

## 3. Run

```sh
# <BuildDir> is the staged server output from step 2.
# <MapName> is the match level, e.g. L_ZERON_BR_Prototype
# (spec only in Phase 06 — the level is built in the Editor).
<BuildDir>/GalaxyFrontier/Binaries/Linux/GalaxyFrontierServer \
  <MapName> -log -port=7777
```

- Port `7777` is the default UE game port (UDP). The exact
  port is the operator's choice — open it (UDP) on the
  server's firewall, or run clients on the same LAN.
- `-log` keeps the server log on stdout (match flow,
  eliminations, anti-cheat warnings are all `UE_LOG` lines).
- The server needs no Epic account, no certificate and no
  Apple ID to run. It needs no player credentials either —
  there is no login in Phase 06 (NULL subsystem, LAN).
- One server process = one match. Do not run two matches in
  one process.

## 4. Two-player test (the first exit criterion)

1. Build the server (step 2) and the iOS clients (the IPA
   workflow, on a self-hosted Mac — see
   `Docs/BUILD-IPA-SIDESTORE.md`).
2. Run the server on the LAN with `L_ZERON_BR_Prototype`
   (or the Phase 03 `L_ZERON_Prototype` for a first smoke test
   — the zone still works, the map is just smaller).
3. Connect two iOS clients (or two Editor PIE clients on the
   Mac) to the server's LAN address via the session browser
   (`WBP_Matchmaking` — the host creates the session,
   `UGFSessionManager::CreateSession`).
4. Accept the match when: both players see each other in the
   lobby, the countdown starts, both can move and shoot, one
   elimination places the loser 2nd and ends the match, the
   winner sees placement 1 + 500 XP, and both clients show a
   result screen. See `GFPhase06Test.cpp` for the full list.
5. Until this runs, Phase 06 is CREATED only — do not mark it
   TESTED, and do not claim multiplayer "works".

## 5. Operating notes

- Logs: match flow, eliminations and anti-cheat warnings are
  `UE_LOG(LogTemp, ...)` lines. Keep the server log — it is
  the only record of a match (there is no replay system in
  Phase 06).
- Bans are session-local and in-memory
  (`UGFAntiCheatManager`): restarting the server clears them.
- The server does not write any player's save file. XP is
  awarded on the server pawn and saved by the client on
  return to the lobby (`UGFPlayerSaveManager::SaveGame`).
- Scaling: 60 players is the design target, not a tested
  number. Scale in steps (2 → 10 → 30 → 60) and watch server
  CPU / bandwidth at each step before raising the player cap.

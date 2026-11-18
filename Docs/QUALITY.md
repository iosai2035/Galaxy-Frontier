# Quality — Galaxy Frontier

## Statuses (use exactly these)

- **CREATED** — file written.
- **REVIEWED** — file read back and checked against the spec (includes,
  UPROPERTY/UFUNCTION, Build.cs dependencies, no undefined symbols).
- **BUILT** — compiled / packaged successfully in a real run. Never
  claimed without a run.
- **TESTED** — passed a real executed test. Never claimed without a run.
- **BLOCKED** — cannot be completed here, with the reason stated.
- **NOT IMPLEMENTED** — declared / specced only, no behaviour.

## Rules

1. No fake code described as complete. No placeholder classes.
2. No class/function names that do not exist in the code.
3. No missing `#include`s. Every symbol used must be included or
   forward-declared correctly.
4. `Build.cs` dependencies must match the code actually used.
5. All gameplay values are `UPROPERTY` — no hard-coded magic numbers.
6. `TObjectPtr<>` for UObject references; no unnecessary raw pointers.
7. No circular dependencies; Delegates / Events / Interfaces for
   cross-system communication.
8. Separate data (Data Assets / Data Tables) from gameplay logic.
9. Handle failure: null checks on PlayerController, subsystems,
   unassigned Input Actions (log a warning, do not crash).
10. No copied assets, no stolen content, no unlicensed assets in ZIPs.
11. No secrets in the repo or in ZIPs (certificates, keys, passwords,
    provisioning profiles, API keys).
12. No empty files to look complete. Every file has a purpose, stated
    at its top or in its Doc.
13. No fake `.uasset` binaries. Blueprint/UMG assets are documented
    in `Docs/` with creation steps and created in the Editor on macOS.
14. Before any commit: review the diff, check for secrets, run every
    test that can run, write a clear commit message.
15. Do not mix CREATED with BUILT/TESTED in a report. A file can be
    CREATED + REVIEWED and still be BLOCKED for build/test.

## Reliability rules (Phases 02–03, see Docs/RELIABILITY.md)

16. Every public function that can fail returns `bool` or a count.
17. No Tick-heavy logic: timers and events only. The only Tick in
    the project is `AGFCharacter::Tick` (camera smoothing).
18. Validate and clamp every numeric input; reject non-finite
    values with a warning. A bad caller must never drive health,
    ammo or a stage index out of range.
19. State machines are explicit and terminal states (Die,
    MissionComplete) cannot be left except by an explicit
    `Revive()` / new mission start.
20. No silent failures: a missing asset, a failed spawn, a
    rejected mission stage or an out-of-range interaction logs
    a warning with the actor and the reason.

## Graphics budgets (targets, to measure — Phase 04)

21. Draw calls ≤ 150 (High) / ≤ 80 (Low); polygons ≤ 500k (High) /
    ≤ 200k (Low); texture memory ≤ 512 MB (High) / ≤ 256 MB (Low).
22. Dynamic lights ≤ 2; shadow-casting lights: 1 directional only.
23. Lumen is NOT used on iOS. Nanite only where UE 5.5 on iOS
    supports it; otherwise traditional LODs carry the quality.
24. Character textures: 2048 px (High) / 1024 px (Medium) /
    512 px (Low). PBR skin/cloth/metal per Docs/CHARACTERS.md.
25. No fixed 60 fps is promised on any device. Per-device
    targets and measurements live in Docs/DEVICE_TEST_MATRIX.md.

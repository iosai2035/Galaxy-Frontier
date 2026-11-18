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

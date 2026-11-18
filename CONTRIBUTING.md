# Contributing to Galaxy Frontier

## Ground rules

1. Do not commit fake code, placeholder classes, or invented test results.
   A file is either real and reviewed, or it is marked NOT IMPLEMENTED.
2. Do not copy characters, maps, logos, music or any asset from another
   game. All content must be original or properly licensed, with the
   licence recorded in `Docs/` and in the asset's source comment.
3. Do not commit secrets: no Apple certificates, provisioning profiles,
   API keys, passwords or signing material. Ever.
4. Do not commit `DerivedDataCache/`, `Intermediate/`, `Saved/` (except
   intentional config), `Binaries/` or `.uproject` user files. See
   `.gitignore`.
5. Use Git LFS for large binaries (see `.gitattributes`).

## Before a commit

1. Check `git status` / `git diff` and review every changed file.
2. Confirm no secret is in the diff (`git diff | grep -i -E "password|secret|api[_-]?key|private|provisioning|\.p12|\.mobileprovision"` must be empty).
3. Run every test that can actually run in the current environment and
   record the result honestly (PASS only after a real run).
4. Write a clear commit message: what changed, why, and what is still
   BLOCKED / NOT IMPLEMENTED.

## Branching

- `main` is the only branch in use in Phases 00-01.
- Do not rewrite history on `main` without the owner's explicit approval.
- One logical change per commit; keep generated files out of commits.

## Code style (Unreal C++)

- Class prefix `AGF` for Actors, `UGF` for UObjects/Components.
- All gameplay-tunable values are `UPROPERTY(EditAnywhere, BlueprintReadWrite,
  Category=...)`, never hard-coded magic numbers in gameplay code.
- `TObjectPtr<>` for UObject references; no unnecessary raw pointers.
- Every public function that Blueprint may call is `UFUNCTION`.
- Dependencies in `GalaxyFrontier.Build.cs` must match the code actually
  used — add a module only when a `#include` from it is in the code.
- No circular dependencies between systems; communicate via Delegates,
  Events and Interfaces, not direct cross-system includes.

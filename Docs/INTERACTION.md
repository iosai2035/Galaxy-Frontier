# Interaction — Galaxy Frontier (Phase 03)

## System

- `IGFInteractable` (`World/IGFInteractable.h`) — interface:
  `Interact(Interactor)`, `GetInteractionPrompt()`,
  `CanInteract(Interactor)`, `GetInteractionDuration()`.
  C++-only interface: call it via `Cast<IGFInteractable>(Actor)`,
  not via `Execute_*` wrappers (those are only generated for
  `UFUNCTION` interface methods).
- `UGFInteractionComponent` (`World/UGFInteractionComponent.h/.cpp`)
  — attached to the player character. Scans on a 0.15 s timer
  (no Tick), picks the nearest usable interactable in range,
  broadcasts `OnInteractableChanged` (HUD prompt), runs
  Begin / Complete / Cancel.
- `AGFInteractableActor` (`World/AGFInteractableActor.h/.cpp`) —
  base actor implementing the interface. Mesh assigned in the
  Editor; behaviour is generic here.

## Types (`EGFInteractableType`)

Door, Crate, Device, PowerStation, CommDevice, ExtractionPoint,
MissionItem. A door toggles `bIsDoorOpen` on interact; a crate /
mission item is single-use by default (`bCanBeUsedMultipleTimes`
false); terminals / doors are reusable.

## Rules (enforced in code)

- **Range:** `InteractionRange` = 250 cm. Actors beyond it are
  never offered, never prompted, never completed — even if the
  player was in range when the interaction started (re-checked
  at completion time).
- **Usability:** `CanInteract()` is checked at scan time, at
  begin time, and again at completion time. A used-once actor
  that was already used cannot be interacted with.
- **Hold-to-interact:** duration > 0 starts a timer; `CancelInteraction()`
  (e.g. player walks away, releases the touch button) clears the
  timer and the interaction never completes. No partial credit.
- **No Tick:** the scan is a 0.15 s timer; while an interaction is
  running the scan is paused so the target cannot change under
  the player's finger.
- **Touch:** the HUD Interact button (see `Docs/MOBILE-HUD.md`)
  calls `BeginInteraction()` on press and `CancelInteraction()`
  on release for hold interactions. Multi-touch: the interact
  button is independent of the joystick / camera drag.
- **Events, not polling:** the HUD listens to
  `OnInteractableChanged` / `OnInteractionCompleted`. It must
  not query `FindInteractable()` every frame.

## Editor setup (macOS)

1. Place `AGFInteractableActor` instances in `L_ZERON_Prototype`
   (see `Docs/WORLD-ZERON.md`): station door, Outpost terminal,
   crates, Power Core, Extraction Zone marker.
2. Set `InteractableType`, `PromptText` (e.g. "Open Door",
   "Activate Power Core"), `InteractionDurationSeconds`
   (Power Core: 2–3 s hold), `bCanBeUsedMultipleTimes`.
3. Assign a Static Mesh to `MeshComponent` in the Editor.
4. For mission stages, attach `UGFObjectiveComponent` to the
   same actor and bind `OnInteracted` to complete the objective
   (Level Blueprint) — the interactable actor itself does not
   know about missions.

## What is NOT done

- No interaction animations (door swing, terminal typing) —
  Animation / Control Rig work in the Editor, Phase 04.
- No interaction SFX — Phase 04.
- No lock / key / hacking minigame — future expansion.

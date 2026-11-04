The first Matrix milestone should implement a **single mono path through five ordered effect slots**, with saved configurations and independent controls for each slot. It can reuse Blocks’ existing Faust catalogue and browser audio infrastructure. The hex grid is the presentation and future placement model; this milestone needs only the five co-linear cells.

The scope matches the pedal-chain capabilities of one Houston virtual loop: choose, edit, bypass, reorder and remove pedals. It excludes Houston’s amp stack and does not claim preset-file or DSP equivalence with Houston.

## Current starting point

- [audio.ts](/home/booky/Trinity%20Labs/guitarix/blocks/src/audio.ts) owns one active Faust processor. It already provides input selection, trim, output routing, whole-effect bypass, mute, metering, output protection and a 25 ms effect replacement crossfade.
- [App.tsx](/home/booky/Trinity%20Labs/guitarix/blocks/src/App.tsx) stores parameter values by **effect ID**, so two instances of the same effect cannot currently have different settings.
- [Pedal.tsx](/home/booky/Trinity%20Labs/guitarix/blocks/src/Pedal.tsx) renders controls from catalogue metadata and can be reused for the selected matrix instance.
- [types.ts](/home/booky/Trinity%20Labs/guitarix/blocks/src/types.ts) exposes each effect’s input and output counts. The initial Matrix catalogue can therefore be limited to effects with exactly one input and one output.
- [launcher.ts](/home/booky/Trinity%20Labs/guitarix/blocks/src/launcher.ts) describes the session passed through the desktop launcher’s buffer-size restart. Matrix state must be included in that restoration path.
- Houston’s local [VirtualLoopEditor.tsx](/home/booky/Trinity%20Labs/houston/ui/src/components/views/VirtualLoopEditor.tsx) supports opening, bypassing, reordering and removing pedals. Those interactions are the useful parity target; Blocks’ catalogue and audio host remain its own.

## Target behavior

```mermaid
flowchart LR
    I[Selected mono input] --> T[Input trim]
    T --> A[Slot 1]
    A --> B[Slot 2]
    B --> C[Slot 3]
    C --> D[Slot 4]
    D --> E[Slot 5]
    E --> O[Output gain and protection]
```

Each slot is empty or contains one mono effect instance. Empty and bypassed slots pass their input through. Duplicate effects are allowed, with separate parameter values and bypass states. The user can select a slot, choose or replace its effect, edit its controls, bypass it, remove it, and move it left or right. A chain can be named, saved and recalled.

The existing single-effect workbench remains available. Navigation between it and Matrix must have a clear audio behavior: **only the visible mode owns the active processing path**, and changing mode installs the destination configuration without opening a second capture stream.

## Implementation plan

### 1. Define the matrix model

Add Matrix-specific types and pure state operations, preferably in new files under `blocks/src/`:

```ts
interface MatrixInstance {
  instanceId: string;
  effectId: string;
  enabled: boolean;
  values: Record<string, number>;
}

interface MatrixChain {
  slots: [
    MatrixInstance | null,
    MatrixInstance | null,
    MatrixInstance | null,
    MatrixInstance | null,
    MatrixInstance | null
  ];
}
```

- Generate a new `instanceId` when adding an effect; retain it when reordering. Key runtime nodes and parameter edits by this ID, never by catalogue ID or slot index.
- Store only finite, validated parameter values by the catalogue’s exact control addresses. Defaults come from metadata when values are absent.
- Define operations for insert/replace, remove, bypass, update parameter and move. Moving an instance moves its settings and enabled state with it.
- Treat a replacement as a new instance. Do not silently carry parameter values from the old effect.
- Validate exactly five slots and require `inputs === 1 && outputs === 1` for every occupied slot. Make that eligibility rule visible in the picker.
- Keep the ordered five-slot model independent of screen coordinates. Map it to the five co-linear hexes in the UI; this avoids making the first audio implementation depend on future stereo-routing rules.

### 2. Extend the audio engine for ordered instances

Refactor [audio.ts](/home/booky/Trinity%20Labs/guitarix/blocks/src/audio.ts) so the existing device and protection graph is shared while its processing section can host a chain.

- Retain one `AudioContext`, input source, trim, global dry/wet bypass, master gain, compressor, peak guard, meters and output route.
- Introduce a chain runtime that creates a separate Faust node for every occupied instance, including duplicate effect IDs. Connect the nodes in slot order, with pass-through for empty or bypassed slots.
- Set all instance parameters before a new node becomes audible. Route updates using `(instanceId, address)`, and validate the instance still exists before applying an asynchronous update.
- Give each active processor an error handler identifying its slot and effect. On `processorerror`, stop audio and report the failing instance, consistent with the current engine’s failure behavior.
- Preserve whole-chain bypass and add **per-slot bypass**. Use short gain ramps for bypass transitions. Define tail behavior explicitly: bypass removes the effect’s output from the audible path; re-enabling may reveal its continuing internal state if the processor remains running.
- Build and validate replacements before changing the audible graph. If loading fails, leave the previously committed chain active and show an actionable error.
- Serialize or version chain changes so rapid edits, mode changes and Stop cannot attach stale nodes. Destroy superseded nodes, disconnect all temporary graph branches, and invalidate pending work on Stop.
- Avoid transient doubled signal or silence when changing topology. A practical first implementation is to prepare a complete replacement chain and crossfade old and new chain outputs, then retire the old nodes. Apply this to topology changes; apply parameter updates directly to the current instance.
- Account for the cost and behavior of that choice: crossfading complete graphs temporarily runs both chains and recreating processors resets their state. Measure worst-case transitions; if they exceed the available audio budget, refine the transition strategy before release. Do not promise uninterrupted delay or reverb tails across a reorder or replacement.
- Update diagnostics to report active instance count and chain/transition state instead of assuming one selected effect.

The existing Faust “mono generator” is a runtime class, not a channel eligibility test. Use the catalogue’s actual input/output metadata for the Matrix restriction.

### 3. Build the Matrix page

Add a Matrix view and navigation in [App.tsx](/home/booky/Trinity%20Labs/guitarix/blocks/src/App.tsx), extracting components rather than growing the existing single component indefinitely.

- Render the five usable positions as a horizontal sequence of **dashed-outline hexagons**, following [BLOCKS.md](/home/booky/Trinity%20Labs/guitarix/BLOCKS.md). Label input, output and signal direction clearly.
- Show slot number, effect name and enabled/bypassed state. Empty cells invite the user to add an effect.
- Selecting a cell opens a searchable, category-filtered picker limited to eligible mono effects, or opens that instance’s controls using [Pedal.tsx](/home/booky/Trinity%20Labs/guitarix/blocks/src/Pedal.tsx).
- Provide explicit bypass, replace, remove, move-left and move-right controls. Drag-to-reorder can be added, but buttons and keyboard operations must provide the same functionality.
- Preserve selection by instance ID when a pedal moves; select a sensible neighboring slot after removal.
- Make controls usable on touch and narrow screens. Give each cell an accessible name and state, visible focus, and keyboard access. Do not rely on the hexagon shape or color alone to communicate bypass.
- Reuse the existing setup, Start/Stop, trim, output, mute, global bypass and meter controls. Matrix changes made while stopped become the configuration installed on Start.

The full grid and inactive future cells do not need routing behavior in this milestone. If displayed for visual context, distinguish unavailable cells from the five usable ones and do not make them interactive.

### 4. Save and recall chains

Add a versioned, Matrix-specific local storage format and a small preset service.

- Save a name, schema version and the ordered five slots, including effect IDs, instance IDs, enabled state and parameter values. Do not persist runtime nodes or device IDs in a chain preset.
- Offer save new, overwrite, rename, load and delete. Keep editing state separate from a saved snapshot so changes do not silently overwrite it.
- On load, validate the schema, effect availability, mono eligibility, parameter addresses, finite values and ranges. Report missing or incompatible effects by slot; do not silently substitute an effect or change the audible chain.
- Prepare a loaded chain before committing it during playback. If preparation fails, retain the current chain and the saved data.
- Handle unavailable storage and malformed data with a visible error and retain the in-memory chain. Version the format now so later grid features can migrate it deliberately.
- Extend the launcher `AudioSession` and restoration code to include the active page and unsaved Matrix state. Keep compatibility with existing sessions that contain only `selected` and `values`; do not automatically start capture merely because a saved chain was loaded.

### 5. Verify the implementation

Add focused tests around the new behavior and run the existing suite.

- **Model and persistence:** five-slot validation; insert, remove and reorder; duplicate effect IDs with independent values; bypass retention; save/load round trips; invalid, missing and incompatible entries; storage failures.
- **Audio correctness:** use deterministic mono processors or offline fixtures to verify serial order, pass-through of empty and bypassed slots, independent duplicate instances, parameter targeting and output protection. Include a noncommutative pair of effects so reversing the order produces a detectable difference.
- **Lifecycle and races:** failed node creation leaves the old chain active; rapid replacements cannot install stale results; Stop during loading releases resources; repeated start/stop, mode switches and preset loads do not accumulate nodes or capture streams.
- **Browser interaction:** add and edit five pedals, duplicate one, bypass and reorder it, save and recall the chain, navigate between pages, and exercise keyboard and narrow-screen layouts. Extend the existing Playwright coverage in `blocks/tests/browser/`.
- **Build and regression:** run `npm run build`, `npm test` and `npm run test:browser` from `blocks/`. Confirm the current single-effect workflow, setup, launcher buffer restart, input/output routing, global bypass and mute still work.
- **Real audio acceptance:** test a representative five-effect chain and repeated topology changes on the intended machine and audio interface. Record CPU, audible dropouts, transition behavior and measured round-trip latency; set an explicit product threshold before declaring performance acceptable.

## Delivery order and acceptance

1. **Model and tests:** five independently addressable instances and validated state operations.
2. **Audio chain:** ordered mono processing, bypass, safe replacement and cleanup, verified without relying on the UI.
3. **Matrix page:** five hex cells and complete pedal editing and reordering.
4. **Persistence and launcher restore:** named recall and recovery of unsaved state across a buffer restart.
5. **Regression and hardware validation:** existing workbench remains functional; a saved five-effect chain can be recalled and played reliably.

The milestone is complete when a user can build a five-effect mono chain, including two copies of the same effect with different settings, hear the effects in the displayed order, bypass or move any instance, and save and recall the result. No stereo routing, branching, mixing or amp-stack implementation is required for this first template.
# Web Faust port: Milestone 1 implementation

The audition app is in the **blocks Git submodule**, with its own npm project.
See [blocks/README.md](blocks/README.md) for setup, XR18 routing, architecture,
source provenance, licences and validation commands. The original plan and
RESEARCH.md are unchanged.

- Milestone 0: merged `origin/nam-a2-support` at `c804468b` into this branch
  (merge `2f355a7b`). All six new native effects are present.
- Milestone 1: 12 mono effects compiled from the existing Faust sources to WASM,
  hosted by AudioWorklet in a React/TypeScript app. Seven use the attached Artemis
  pedal artwork and controls. The new reverbs and Phase 90 have generic controls.
- Live capture and test tone, device/channel selection, input/output trim, meters,
  bypass, mute, output protection, crossfaded switching and resource cleanup are
  implemented. Each effect is auditioned separately.
- [Portability inventory](blocks/docs/EFFECT_INVENTORY.md): 504 Faust sources,
  native implementations and subsystems. Static candidates are distinguished
  from compiled and tested effects. Klon is retained upstream but deferred from
  the first browser subset due to its heavier circuit compilation/oversampling.

Run from this checkout:

```sh
cd blocks
npm ci
npm run live
```

The live runner builds and opens the production app at <http://127.0.0.1:4173>,
with named routes from XR18 hardware input 1 to the browser and browser playback
to XR18 USB 1/2. It requests 256-frame Chromium buffers to match the user's current
QjackCtl setting, without changing system settings. See the README for overrides
and routing-only commands. Device choices persist and remain stable when devices
reorder or disappear; capture loss stops audio with a useful error.

Validation on 2026-10-29:

- Production build and TypeScript checks pass.
- 18 WASM tests cover all 12 effects at 44.1/48/96 kHz, parameter extremes,
  silence/sine/impulse processing, gain and dry/wet behavior; an additional test
  verifies the final output guard's clamp, silence and non-finite handling.
- Ten Chromium integration tests cover all AudioWorklets, controls, switching,
  bypass/mute, restart, capture release, denied/late permissions, channel isolation,
  failed effect loads, Tremolo's third waveform, mobile layout, stable device
  selection and input disconnection.
- The upstream native effect audio regression test passes. Its Houston parameter
  comparison is skipped because that sibling checkout is unavailable.
- Actual XR18 bass capture/playback through the Volume effect was confirmed by
  the user after reconnecting the USB interface (the kernel had reported USB
  protocol errors). The route now uses input 1, following the user's cable move.
- The live graph shows 256-frame Chromium playback and XR18 playback at 48 kHz.
  Browser latency diagnostics are estimates; round-trip latency was not measured.

Graph editing, accounts, publishing and device deployment from RESEARCH.md
are outside this milestone. App changes are uncommitted within `blocks`; the
upstream merge is committed in the parent repository. Nothing was pushed.

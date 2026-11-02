# Web Faust port: Milestones 1 and 2

The audition app is in the **blocks Git submodule**, with its own npm project.
See [blocks/README.md](blocks/README.md) for setup, interface configuration, optional XR18 routing, architecture,
source provenance, licences and validation commands. The milestone plan now includes the complete candidate catalogue; RESEARCH.md
is unchanged.

Milestone 2 is implemented and validated (2026-11-02). It compiles all
**361 Candidate rows** found in this checkout
(the request named 341), alongside the original 12: **373 source entries**.
See [Milestone 2 implementation and validation](blocks/docs/MILESTONE_2.md).

- The candidate baseline is frozen in `blocks/scripts/milestone2-candidates.json`.
  Canonical Faust sources, LV2 copies, plugins, helpers and generators have distinct
  IDs and names; a successful build requires every entry to compile.
- The UI offers 13 effect types plus combined type/search filtering. **74 entries**
  use **50 matching Artemis designs** from asset revision `1f2975e`, including the
  five newer effects. Unmatched processors retain generic controls.
- Mono/stereo, zero-input and four-input channel contracts are supported. The
  selected mono source feeds all processor inputs; stereo output is preserved on
  playback pairs and downmixed when one output channel is selected.
- Recorded browser adaptations cover legacy syntax, native tube/circuit lookup
  tables, beat conversion and numerical repairs. Original source files remain
  intact; original and exact adapted sources accompany the static build.
- Production build and TypeScript checks pass. All **394 DSP/catalogue/native
  lookup/output-guard tests** pass, including every entry at 44.1/48/96 kHz and
  direct comparisons against the original C++ lookup implementations.
- All **29 Chromium tests** pass, including every AudioWorklet, all 1,760 named
  parameters in the desktop sweep, seven viewport sizes, category/search behavior,
  enum/default preservation, device/capture lifecycle and stereo/mono routing.
  Formatting and Git whitespace checks pass.
- Physical interface listening and comparative tonal/performance checks remain
  manual acceptance work. Native/host-dependent inventory rows outside the frozen
  candidate set remain separate work; this does not add NAM or convolution.

The following records the earlier Milestone 1 implementation and hardware checks.

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

The live runner builds and opens the production app at <http://127.0.0.1:4173>
in a dedicated Chromium window. It works with browser-visible audio interfaces;
XR18 hardware, `pactl` and predefined system routes are **not required**. It
requests 256-frame Chromium buffers without changing system audio settings.
Set `CHROMIUM_PATH` for another Chromium executable. To use your regular browser,
run `npm run dev` instead, then open <http://127.0.0.1:5173>.

The initial **Audio configuration** page lets you enable device access, select
input/output devices, **Check channels**, and choose input and output channels
before starting playback. Click **Start audio**, then **Continue to effects**.
Use **Test tone** to check playback without an input. The header's **Audio setup**
button reopens configuration at any time; stop audio to change devices/source.
Choices persist, missing devices stay marked unavailable, and invalid saved
channels produce a clear error. Channel numbers reflect what the browser exposes;
use your interface mixer or system patchbay for physical ports hidden by the
browser. The XR18 routing commands remain available as an explicit optional helper;
see the README.

Validation of general audio configuration on 2026-11-02:

- Production build, TypeScript checks and formatting checks pass.
- All 19 DSP/output-guard tests pass.
- 17 Chromium tests cover the effects and audio lifecycle plus initial setup,
  reopening setup during playback, generic device discovery, channel probing and
  capture release, saved selections, unavailable channels, output channel signal
  isolation, system-output fallback, output disconnection and mobile layout.
- The real `npm run live` launcher opens Audio configuration in Chromium without
  invoking XR18 setup. Verified in a virtual display without starting playback.
- Physical interface playback and listening remain manual acceptance checks.

Earlier XR18 validation on 2026-10-29:

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

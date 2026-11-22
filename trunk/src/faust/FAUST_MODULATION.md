# Houston modulation refresh

Deploy Guitarix and Houston together. Harmonic Tremolo is a new mono rack
processor; the other effects retain their processor IDs, parameter IDs and
native units. Saved parameter values and expression ranges are retained. Their
expanded definitions gain the new limits/defaults on load. New defaults apply
to newly added effects and knob resets, not to saved Mix or Depth values.
The revised chorus, flanger and tremolo intentionally change the sound at the
same settings; this is not a bit-identical preservation of their old algorithms.

## Research and model choices

Primary sources reviewed for this change:

- [Faust virtual-analogue effects](https://faustlibraries.grame.fr/libs/vaeffects/#vemxrphase90):
  the existing Phase 90 already uses David Braun's four-JFET circuit model,
  including signal-dependent channel resistance. Keep that model and its
  polarity/level compensation; improve the rate control and reset behaviour.
- [Faust phasers/flangers](https://faustlibraries.grame.fr/libs/phaflangers/):
  standard delay-comb and all-pass topologies are useful building blocks, but
  are not calibrated BBD pedal circuits.
- [Faust fractional delays](https://faustlibraries.grame.fr/libs/delays/):
  third-order Lagrange interpolation offers a higher-order alternative to the
  old linear delay interpolation. Its minimum delay and tap bounds are enforced.
- [Raffel and Smith, Practical Modeling of Bucket-Brigade Device Circuits](https://colinraffel.com/software/bbdmodeling/bbdmodeling.pdf):
  a convincing BBD model involves the clocked delay and its surrounding
  filtering/companding. This is a research foundation, not a verified drop-in
  Faust implementation. No new model here is advertised as a CE-2, BF-2 or
  transistor-by-transistor BBD clone.

The existing Univibe also has a component-oriented native implementation;
this change restores its native Depth, Feedback and Mix defaults rather than
replacing it with a generic all-pass chain. Harmonic Tremolo is an original
complementary band-split effect, not a calibrated Fender valve-circuit model.

## Audible and control changes

| Effect | Change |
| --- | --- |
| Harmonic Tremolo | New opposing low/high-band modulation; Depth, BPM, 200–1600 Hz Crossover, Mix and output Level. Complementary one-pole bands sum to the original input at zero Depth. Five controls on one touchscreen panel. |
| Tremolo | Depth now blends the normalised optical gain, instead of reducing the LED excursion before the nonlinear resistance curve. Zero Depth is unity. Keep the existing optical time constants and use a continuous-phase triangle LFO. New defaults: 75% Depth, 100% Mix, 240 BPM. |
| Mono Chorus | Increase excursion from ±0.2 ms to ±2 ms around 10 ms, with cubic fractional delay. Level now reaches 2× delayed-voice gain (+6 dB relative to the old maximum); the parallel dry path is preserved. New defaults: Level 0.7, Mix 100%. |
| Mono Flanger | Shared regenerative/feed-forward comb, cubic delay, a full-strength delayed voice and a 0.5–10.5 ms maximum sweep. Feedback input compensation bounds steady gain. |
| Mono Flanger Plus | Keep the two-stage, bipolar-feedback topology; cubic delays, smoothed controls, actual feedback limited to ±0.95 even for old saved ±1 endpoints. All 15 ms of delay remain available at 192 kHz. |
| Phase 90 | Keep 0.1–10 Hz native values, with logarithmic knob travel: midpoint is 1 Hz instead of 5.05 Hz. Seed smoothers from saved controls on reset. |
| Mono Phaser | Smooth Depth/Mix/rate; permit slower sweeps; restore the full native Mix default. |
| Mono Univibe | Native defaults restored: Depth 0.37, Feedback −0.6, Mix 1, Width 0.5. Logarithmic rate gesture. |
| Ring Modulator | Smooth live frequency/Mix changes; correct the catalogue to allow true dry at Mix 0 and restore the 240 Hz native default. Logarithmic frequency gesture. |

Both flangers now cover 3–360 BPM (0.05–6 Hz), with a logarithmic midpoint
of approximately 32.9 BPM (0.55 Hz), rather than 192 BPM (3.2 Hz). Mono defaults
to 12 BPM; Plus defaults to 6 BPM. The upper endpoint remains accessible so old
fast presets are not truncated. Chorus, Phaser and Tremolo now reach 6 BPM.
Rate knob motion and artwork use the same curve. RPCs, saved values and
expression assignments still use actual BPM/Hz, not a hidden rate multiplier.

MIDI/manual tempo now supports 16-beat, 8-beat, whole-note and half-note cycles
as well as existing subdivisions. Default quarter-note assignments remain
unchanged. Fractional multipliers survive backend serialisation, and tap history
permits intervals long enough for the slower ranges. BPM readouts show tenths
for these controls. Existing saved expression endpoints stay in native units.

## Realtime and switching behaviour

The common control smoother has a 10 ms time constant for live edits and seeds
itself from the saved value on its first sample after reset. Audio paths have no
new lookahead, resampling delay, warm-up wait, JACK routing or resource loading.
No scene ramp length or runtime switching code was changed. Different-model
switches use the existing scene transaction; changing logical owners of the
same stateful processor retains the existing off/on reset barrier, preventing
history from a previous scene from leaking into the next one.

Harmonic Tremolo has under 2 KiB of instance state and no activation allocation.
The mono chorus delay storage falls from 65536 floats to 4096. Mono Flanger
uses 4096 doubles; Plus uses two 2048-double lines and one 4096-double line,
the same allocation sizes as before, while removing high-sample-rate clamps.
Callbacks and resets perform no C++ allocations in the fixture.

An `-O2` AArch64 build-host measurement at 48 kHz/32 frames (not the Artemis
hardware) gave the following illustrative means. These are not hard realtime
or switch-latency guarantees; shared-host scheduling makes small differences noisy.

| Processor | Callback before / after (µs) | Reset before / after (µs) |
| --- | ---: | ---: |
| Mono Chorus | 0.29 / 0.38 | 3.72 / 0.11 |
| Mono Flanger | 0.16 / 0.41 | 0.24 / 0.20 |
| Mono Flanger Plus | 0.25 / 0.79 | 0.48 / 0.42 |
| Tremolo | 1.61 / 1.82 | <0.01 / <0.01 |
| Harmonic Tremolo (new) | — / 0.86 | — / <0.01 |

Higher-order interpolation costs more CPU per sample. It does not introduce an
extra processing block or wait, and delay-reset work is unchanged or reduced.
On-device listening, XRUN stress tests and measured before/after scene switching
are still needed: this workspace has no Artemis audio interface or JACK server.

## Validation

- `tests/test_faust_modulation.py` runs the actual generated plugin callbacks
  under ASan/UBSan at 44.1, 48, 96 and 192 kHz: in-place/block-size equivalence,
  control extremes, feedback corners, silence after reset, exact dry endpoints,
  allocation checks, edit boundaries, chorus gain range and harmonic-band
  opposition. It also compares every exposed control with Houston's catalogue.
- Full-depth optical tremolo at 48 kHz measured approximately 0.002–0.996 gain
  at 1 Hz, 0.029–0.976 at 4 Hz, and 0.064–0.944 at 6 Hz. Half-depth waveforms
  are verified to be halfway between unity and full depth. Measured cycle count
  matches BPM.
- Native scene regression tests and Houston tests cover resident ownership,
  bypass/re-entry, preset reload, clock assignment and retained parameter values.
- UI tests cover logarithmic drag/rollback, artwork position, defaults and slow
  tempo subdivisions. Playwright verifies all five harmonic controls and both
  flangers, Phase 90 and chorus on the 800×480 touchscreen, plus tempo entry.
- Native Guitarix and production Houston UI builds pass. There is no separate
  TypeScript check: this UI checkout has neither a TypeScript compiler nor a
  project tsconfig; an optional `npx` attempt could not fetch one in the sandbox.
  The wider artwork suite has a pre-existing Astrofuzz category expectation
  mismatch (Drive versus Distortion), unrelated to these changes. The older
  `test_faust_effects.py` audio checks pass, but its catalogue test still expects
  the removed `Synth.json` and four retired audition effects. The new modulation
  metadata test covers all eight changed/new Faust processors, including Phase 90.

Example commands (from the respective project directories):

```sh
ASAN_OPTIONS=detect_leaks=0 python3 -m unittest discover -s tests -p test_faust_modulation.py -v
ASAN_OPTIONS=detect_leaks=0 python3 -m unittest discover -s tests -p 'test_scene*.py' -v
python3 -m unittest tests.test_modulation_controls tests.test_tempo_controls tests.test_expression_assignments -v
npm test -- --run tests/unit/modulationControls.test.tsx tests/unit/tempoSubdivisions.test.ts
npm run test:browser -- tests/browser/modulation-controls.spec.ts tests/browser/virtual-loop-tempo.spec.ts
```

LeakSanitizer is disabled only because the build sandbox cannot support its
process inspection; AddressSanitizer and UndefinedBehaviorSanitizer stay enabled.
Generated sources use Faust 2.81.10 and the installed libraries 2.59.0, using
`tools/dsp2cc` with the project's 40000-byte allocation threshold. Chorus/Phaser
retain float precision; the other updated processors use double precision.

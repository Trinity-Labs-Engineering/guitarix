# Artemis drive and fuzz audition set

All eleven effects have mono callbacks and independent per-instance state. Houston
lists them in **Audition**, with generic artwork. Existing effect IDs and presets
are retained. These are candidates for listening comparisons, not claims of
hardware-calibrated pedal emulations.

| Houston name | Runtime prefix | Origin / controls |
| --- | --- | --- |
| Tone Machine | `audition_tone_machine` | GxToneMachine preamp/tone networks and clipping table; Sustain, Tone, Level, Octave |
| Super-Fuzz | `audition_super_fuzz` | GxSuperFuzz networks/table; Expander, Level, Full/Scooped Tone, Trim |
| Muff Laboratory | `audition_muff` | BYOD Muff stages + tone networks; Sustain, Tone, Mids, Stages, Voicing, Harmonics (Hz), Smoothing, Level (dB) |
| Fuzz Machine | `audition_fuzz_machine` | BYOD custom-pedal LSTM models; Fuzz, Mode 1.5/2, Level |
| Harmonic Percolator | `audition_percolator` | New nodal model; Harmonics, Balance, Germanium/Open/Silicon Clipping, Stock/Alternate component set |
| Bias Fuzz | `audition_bias_fuzz` | New Fuzz Factory-inspired experimental circuit; Drive, Gate, Compression, Stability, Level |
| Mouse Drive | `audition_mouse` | BYOD RAT-style WDF core; Distortion, Filter (clockwise darker), Level (dB) |
| Tone King | `audition_tone_king` | BYOD King of Tone drive stages; Drive, Tone, Level (dB), Distortion/Overdrive/Both |
| Flapjack | `audition_flapjack` | BYOD Hot Cake model; Drive, Presence, Low Cut (Hz), Level (dB), Fizzy/Bluesberry/Peachy |
| Yen Drive | `audition_yen` | BYOD Zendrive WDF core; Gain, Voice, Tone, Level (dB) |
| OCD Drive | `audition_ocd` | Faust Fulltone OCD v2 idealised circuit; Drive, Tone, Level (dB), LP/HP |

Muff's eight controls use two pages in Houston's generic focus view. Its ten
voicings select **tone networks**, not complete models of ten different pedals.
Mode labels live beside their numerical values in Houston's `Audition.json`.
All parameter defaults/ranges/steps are checked against the actual C++ callbacks.

## Pinned upstream sources and licensing

No network access is needed to build or run these effects. Full commit IDs are
also recorded in `sources.json`.

- [Chowdhury-DSP/BYOD](https://github.com/Chowdhury-DSP/BYOD/tree/1cf22b6ac802b9dc33cfc9f8dd6af5b3c3e40bc9),
  Jatin Chowdhury, GPL-3.0. WDF circuit headers are in `vendor/byod`; its full
  licence is retained there. `muff.h` adapts `BigMuffClippingStage.cpp`,
  `BigMuffDrive.cpp`, `BigMuffTone.cpp/.h`. `byod_drives.h` adapts the processor
  signal chains in `mouse_drive`, `king_of_tone`, `flapjack` and `zen_drive`.
  Four original Fuzz Machine model files are retained in `models` under the
  upstream repository licence. `models/generate.py` deterministically embeds
  them; runtime inference uses their original PyTorch LSTM gate order, summed
  biases, conditioning input, dense output and residual input.
- [chowdsp_wdf](https://github.com/Chowdhury-DSP/chowdsp_wdf/tree/7a3c85e47346af7765c23c8855b032f6c0bcdedb),
  Copyright (c) 2022 Chowdhury-DSP, BSD-3-Clause; full notice in
  `vendor/chowdsp_wdf/LICENSE`. The R-type adapter adds explicit cached-wave
  save/restore methods for scene reuse. Series/parallel RC one-ports also expose
  DC-voltage reset methods used by Mouse Drive's analytical operating-point reset.
  The series RC voltage source exposes the same operation for Yen Drive's
  bounded DC feedback solve. Tone King's DC filters use its saved overdrive bias.
- [GxToneMachine](https://github.com/brummer10/GxToneMachine.lv2/tree/2eba371285d130eb82a116db0bd2a6f8e6a72eab)
  and [GxSuperFuzz](https://github.com/brummer10/GxSuperFuzz.lv2/tree/eda1d498e71605d398781153c71ef2b373d72814),
  Guitarix/MOD project (Hermann Meyer and contributors). Upstream repositories
  include GPL-3.0 licence text, retained in `vendor/gx/LICENSE`; the legacy
  plugin interface retains its GPL-2.0-or-later notice. Generated Faust cores
  are kept with the adapter rather than passed through the current Faust compiler.
- [Faust libraries](https://github.com/grame-cncm/faustlibraries/tree/4b8961b03ff90f5457724d987d14703d960ec30b),
  `vaeffects.lib:fulltoneOCD`, David Braun, MIT. The function and its full modelling
  notes are retained in `../../faust/artemis_ocd.lib`. The existing Faust
  `FAUST_EFFECTS_LICENSES.txt` contains the MIT/STK helper notices. The clipping
  data is derived from Giampiccolo et al., DAFx-26, pp. 193–200 (CC BY 4.0), as
  attributed in the upstream function. See its embedded paper/repository links.

The new native wrappers and circuit models are GPL-3.0-or-later. They are linked
with Guitarix's GPL-2.0-or-later code under GPLv3-compatible terms. The native
inference kernel uses the existing Eigen dependency. It does not add JUCE,
runtime model downloads, external plugins, model file selection or MIDI.

## Adaptation and modelling limits

- WDF math uses double precision; JUCE parameter/buffer infrastructure is removed.
  Flapjack's R-type nonlinear transform uses equivalent scalar math. Its complete
  scattering matrix is retained. A bounded Newton solve replaces BYOD's fixed
  four-iteration Muff solve. Output trims, smoothing and DC removal are integrated.
  Mouse uses a logarithmic distortion taper; Tone King and Yen gain additional
  audition tone filters, not asserted models of their hardware tone stages.
- The older Tone Machine and Super-Fuzz generated graphs contain linear input
  networks and symmetric table clipping. They cannot produce the requested strong
  octave from a sine input alone. Tone Machine therefore adds an explicit smooth
  full-wave rectifier in its octave branch; Super-Fuzz rectifies ahead of its
  clipping table with a small fundamental component. These are **modified** Gx
  models, not unchanged ports or newly solved full transistor circuits. Harmonic
  tests verify the octave and Tone Machine's octave-off behaviour. Their high-order
  legacy filters run at a host-related rate no greater than 48 kHz, avoiding the
  numerical sensitivity of those direct-form filters at very high rates. The
  legacy table/rectification nonlinearities still warrant aliasing auditions.
- Fuzz Machine is a **neural model of a custom pedal**, not a Fuzz Factory model.
  Both modes remain warm and crossfade. The core runs at its actual 88.2/96 kHz
  training rate; 44.1/88.2/176.4 kHz hosts select the 88.2 kHz weights. Other host
  rates use the 96 kHz weights with streaming sample-rate conversion. Its LSTM
  is independent of the optional Guitarix neural model loader.
- Percolator follows the signal topology and stock/alternate values in the
  [Aion Particle schematic](https://aionfx.com/app/files/docs/particle_documentation_v1.0.pdf),
  page 5: a PNP germanium-like transistor and NPN silicon transistor, shared
  emitters, input/output coupling, feedback/bypass capacitors and selectable
  diode branches. New Ebers-Moll device parameters are representative estimates,
  not measured 2N404A/2N3565 devices. Silicon mode uses the equivalent voltage
  law of two series silicon diodes per branch. Backward-Euler capacitor companions
  and damped bounded Newton iterations favour stable audition behaviour.
- Bias Fuzz uses a new two-transistor circuit with variable second-base loading,
  emitter degeneration, supply voltage and capacitive positive feedback. It
  explores the bias/sputter/feedback behaviour suggested by the
  [ZVEX Fuzz Factory](https://www.zvex.com/guitar-pedals/fuzz-factory-guitar-effects-pedal).
  It is **not** a traced or calibrated model of that pedal. The controls interact;
  extremes can choke the signal or produce sustained oscillation.
- OCD preserves the upstream **ideal op-amp** assumption, including the absence
  of supply-rail clipping. Its four-times-oversampled clipping node adds 11 samples
  of latency. A 0.035 output multiplier brings its gain into an audition range;
  it is not a model of the missing rail saturation.

## Runtime and validation

Native WDF, Muff and new transistor circuits run at twice the host rate. Their
33-tap Blackman-windowed sinc converters use fixed storage and persistent sample
clocks, including across 32-sample and uneven callbacks. A 2x round trip has
25 host samples of delay. Conversion at other ratios has 16 samples of delay at
each converter's input rate plus one host sample; matching rates bypass conversion.
There is no added audio-block buffer and no dry parallel blend to misalign.

Sample-rate setup preconditions circuit/neural state and saves a baseline.
Scene clearing restores state without allocation or a long silent preroll.
Percolator and Bias Fuzz recompute their bounded DC operating-point solve at
reset using the current circuit controls, so a scene cannot restore capacitor
voltages from the factory settings after changing its component values.
WDF snapshots restore capacitor state and cached scattering waves without copying
topology references. Instances do not share mutable histories. Native continuous
controls use a 15 ms ramp; mode changes in Tone Machine, Muff, Tone King and Fuzz
Machine crossfade. A 5 ms activation fade suppresses reset transients.

From Guitarix `trunk`:

```sh
python3 -m unittest discover -s tests -p test_audition_effects.py -v
./waf build -j4
```

The fixture exercises all eleven production callbacks at 44.1/48/96/192 kHz,
32-sample and uneven/in-place blocks, independent control extremes, scene reset,
mode/conditioning changes, upper-octave generation, and allocation-free native
processing/clearing under ASan/UBSan. Fuzz Machine additionally exercises 32/88.2
kHz hosts. Houston tests check catalogue routing, storage restoration, labelled
modes, generic rendering, category filtering and both pages of Muff controls.
These checks do not replace listening tests or CM5 realtime measurements.

OCD regeneration, from `trunk/build` (Faust 2.81.10 used here):

```sh
../tools/dsp2cc --double -s 40000 --init-type=plugin-instance --no-version-header \
  --add='-t 600' -o ../src/faust-generated/audition_ocd.cc ../src/faust/audition_ocd.dsp
```

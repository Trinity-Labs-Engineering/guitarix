# TONE3000 Pitch Shift DSP port

`src/headers/gx_poly_pitch_shifter.h` adapts `plugin/include/PitchShift.h`
and `plugin/src/PitchShift.cpp` from TONE3000 commit
`01a842a6064300e90a61e59505b4bfd52b58047d`. The original MIT copyright and
permission notice are included in the header.

The core follows the upstream correlation-spliced delay line: four-point
Hermite interpolation, a mirrored correlation buffer, incremental coarse/fine
lag searches scored by damage per splice, correlation-dependent crossfade
lengths and gains, and onset detection that brings attacks toward the front
of the buffer. Pitch changes use the upstream search-preservation rules.
The previous Guitarix ratio smoothing, fixed short crossfades, input filter,
and forced recentering near unison have been removed.

## Guitarix integration

The `smbPitchShift` plugin and parameter IDs and Pitch/Wet/Dry control ranges
remain unchanged. Wet and Dry are independent linear gain trims: 0% is exact
silence, 50% is unity, and 100% is 2x amplitude (about +6 dB). Both at zero
produce silence; Dry alone passes the input at its selected level, Wet alone
passes only shifted audio, and their intermediate settings add the two
independently scaled signals. Wet defaults to 50% (unity) and Dry to off.
The existing octave control still adds ±12 semitones; the core accepts the
resulting full ±24-semitone range. Neutral legacy band controls leave the
shifted signal unchanged; existing band settings still apply after shifting.

The legacy quality parameter selects the base window lengths:

| `smbPitchShift.latency` | Window | Nominal mean delay |
| --- | --- | --- |
| 0 (high quality) | 60 ms | 31 ms |
| 1 (Houston's selection) | 30 ms, TONE3000 default | 16 ms |
| 2 (realtime) | 20 ms | 11 ms |

All pitch effects expose `bass_mode`: **Guitar** (0, default) uses the selected
base window with bounded chord adaptation; **Bass** (1) allows extra range for low notes. Houston places the
Guitar/Bass toggle in the pedal focus footer and saves it with the preset.
Existing presets without this setting use Guitar, including expanded presets
whose saved control definitions predate the switch. Select Bass for bass parts
that need the extended range. The control changes delay geometry, not note
quantisation or the harmoniser's musical key/interval rules.

In Bass mode, a low-pass cycle estimator requires two consistent periods, then
requests 2.4 periods of delay range (up to 100 ms). Correlation now uses 45 ms of
past input in both modes. Startup and uncertain periods keep the base window or briefly
retain the last confirmed range; uncertainty never requests maximum latency.
A 5% resizing margin suppresses crossing jitter. A confirmed higher note
restores the base window immediately; without further confirmed periods, the
request expires after 120 ms. Geometry changes only between searches/fades;
increasing the range preserves the current upshift fade/search safety margins. No sample-rate
reset, memory allocation or hard tap jump is needed. The extra buffer is
allocated by `prepare()`. The estimator stays warm in Guitar mode, and live
mode changes use the same safe boundaries without resetting audio history.

Bass wet delay is consequently longer and variable. This fixes the constrained
lag search that put bass drop D almost a semitone sharp at +12 in the harmonic
regression signal. Low B, drop D and low E are checked at 44.1/48/96 kHz with
sines and harmonic-rich notes, shifts from -12 through +24, and varying blocks.
Fixed Pitch and Dual Pitch still do not classify notes or quantise to a key.

## Sustained chords

The shared engine used by Pitch Shift, Dual Pitch and both harmonisers now
matches 45 ms of input history, independently of the moving audio-delay window.
A mild 800 Hz shelf in the **control signal only** gives upper strings and
harmonics more influence on splice selection. The crossfade gain still uses
the correlation of the original audio, so weighting the search does not alter
the wet spectrum or introduce a gain boost.

After startup, two consecutive drift searches below 0.96 correlation request
one-third more landing range, capped at 40 ms. Houston's 30 ms setting can
therefore extend to 40 ms on a sustained, poorly matching chord; well-matching
notes keep the short window. The legacy 20 ms mode extends only to 26.7 ms,
and the 60 ms mode needs no further chord extension. The request stays latched
until a new attack or silence, avoiding repeated expansion/contraction as a
better match becomes available. Geometry changes still occur only between
searches and fades. Bass mode can separately earn its longer low-note range.

This is a bounded quality/latency trade: the nominal mean for a 40 ms window
is 21 ms, versus the 30 ms base window's 16 ms. Onset re-sync remains enabled,
and the previous quiet-articulation and muted-tail regressions still pass.
It does not restore the previous unconditional 100 ms uncertainty fallback.
The 45 ms correlation history is not an additional audio delay.

On the repository's four recorded guitar chords, sustained energy outside the
shifted input partials fell by an average 2.6–5.6 dB across shifts of ±2, ±7 and
±12 semitones, compared with commit `565735f3`. The measurement uses the dry
take's spectral peaks and allows 3 Hz or 0.3% around each shifted partial;
these are offline measurements, not a listening assessment. Synthetic tests
also cover harmonic-rich close dyads and six-string minor chords in both modes
at 44.1/48/96 kHz, with level preservation as well as unwanted-frequency energy.

## Virtual Capo

`virtual_capo` is a separate, fully wet mono effect. Its `semitone` control
transposes every string together from −12 to +12 in whole-semitone steps
(default +2); `bass_mode` selects Guitar/Bass. It does not identify a key or
generate per-note harmonies. Put it before drive/amp processing for a virtual
retuning/capo effect.

It uses the shared improvements above with a 60 ms base landing range, giving
complex chords more opportunity to find a compatible splice. The nominal mean
delay is 31 ms, with variable wet delay and attack re-sync as in the other
effects; Bass can extend further for low notes. This deliberately favours
sustained chord quality over the faster pitch effects. The synthetic minor
chord regression requires lower unwanted-frequency energy than the fast mode
at down/up tuning, fifth and octave shifts. Very close or dissonant chords at
large shifts can still compromise individual partials in this time-domain
engine; the Capo is not a claim of artifact-free polyphony.

Houston lists it in **Pitch**, with a signed semitone readout and the same
Guitar/Bass footer toggle, preset persistence and expression assignment support.
It has no dry blend that could mix the original tuning into the transposition.
All allocation remains in sample-rate setup; reset, instrument changes and
in-place processing are covered by the native plugin fixture.

## Mixing and lifecycle

The dry path is immediate unless the legacy latency-compensation option is
enabled, in which case it uses the fixed base-window nominal delay above. This
approximation is retained to avoid moving the dry tap as bass adaptation changes. The wet tap moves
through the buffer: nominal latency does not imply a fixed wet delay. A
fresh unity-ratio tap starts at the 2 ms floor; returning to unity holds its
current position, following upstream behavior.

The port retains upstream priming and the 25 ms startup duration, but fades
the wet path in from silence rather than blending from raw input. Only the
Dry control contributes unshifted input, including while the wet engine is
unprepared or temporarily unready. Guitarix manages effect activation/bypass.
TONE3000's optional tonality crossover, stereo interface, and power toggle are
not needed for this mono effect. JUCE buffer access, clamping, and linear
startup smoothing use standard C++ equivalents; Guitarix gains no JUCE dependency.

Only `prepare()` allocates memory. Audio processing supports arbitrary block
sizes and in-place buffers. Resident scene reset clears the rings, search,
onset history, startup blend, and legacy band history without allocating.
It also resets the sample counter and pitch ratio, so re-entry behaves like
a fresh instance regardless of the preceding scene.

## Dual Pitch

The additional `dual_pitch` plugin runs two independent instances of the same
engine at the 30 ms base setting, with a shared Guitar/Bass selection.
`voice1` and `voice2` each span −12 to +12 semitones,
in whole-semitone steps. `level1`, `level2`, and `dry` are independent 0–100 trims
with the same off/unity/+6 dB mapping as the original plugin. Defaults are −12
and +12 semitones, both voices at unity, and dry off. The original `smbPitchShift`
plugin remains available with its own Guitar/Bass selection.

Both voices consume the original input; neither processes the other voice.
The audio callback uses bounded stack scratch buffers for arbitrary block sizes,
including in-place calls. Sample-rate setup prepares both engines, and scene
reset clears both without allocating. Houston exposes five matching knobs,
including expression assignments, plus the instrument toggle in the footer.

## Regression checks

From `trunk`:

```sh
python3 -m unittest discover -s tests -v
```

The pitch fixture exercises chord transposition at ±2 semitones, single-note
shifts including fractional and two-octave settings, sample rates and quality
modes, Wet/Dry mixing and dry compensation, unity delay, automation, attack
timing, block partitioning, in-place processing, deterministic scene reset,
and allocation-free processing/reset. As in upstream, large shifts of
dissonant chords can compromise individual note pitches. Scene contract tests
also cover the existing `PluginDef::clear_state` callback registration.

The instrument-mode regressions also check a sustained open-E chord, a beating
G3/G#3 dyad, quiet articulation markers and muted tails at 44.1/48/96 kHz,
stale bass estimates, and live mode changes at several callback sizes. The
previous uncertainty fallback repeatedly expanded the open-E window from
30 to 100 ms and could replay a muted dyad almost 100 ms later. Both modes now
keep that chord at the base window; the tested dyad responds to quiet markers
within 30 ms and has no audible tail after 40 ms. These are synthetic DSP
measurements, not end-to-end device latency or a hardware listening assessment.

`tests/test_pitch_chords.py` adds spectral and level checks for sustained
harmonic-rich chords and the production Virtual Capo plugin. It also exercises
the Capo's bass tuning, absence of dry leakage, arbitrary block sizes, in-place
processing, scene reset and allocation-free callbacks.

During the initial port, the original TONE3000 source compiled with JUCE and this
adapter produced bit-identical output for 1,116 scenarios / 65,007,360 samples:
all three windows, 44.1/48/96 kHz, sine/chords/attacks/noise plus bass, shifts
through ±24 semitones and fractional changes, and fixed/varying blocks with
in-place and separate output. This comparison used full Wet, zero Dry, neutral
legacy bands, and upstream tonality disabled.
An additional 372 scenarios / 18,662,400 samples at 30/54/78 kHz also matched
bit-for-bit, covering JUCE's ties-to-even correlation-step rounding.

The later independent-level correction replaces the startup dry-to-wet blend
with a wet fade from silence and scales unready fallback audio by Dry. That correction alone left pitch search and interpolation unchanged. The later
bass and chord adaptations described above intentionally change search geometry
and correlation, so the initial port's bit-identical comparison is historical,
not a claim for current output. Mix regressions cover both zero, each path alone, intermediate
levels, startup, reset, and an unprepared engine, including in-place buffers.

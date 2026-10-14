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

The bass correction adapts these windows instead of forcing bass into the
short guitar splice range. A low-pass cycle estimator requires two consistent
periods, then requests at least 2.4 periods of delay range (up to 100 ms) and
1.5 periods of correlation (up to 45 ms). Until the period is known, or changes
substantially, it requests the larger range. Higher notes return to the chosen
base window. Geometry changes only between searches/fades; increasing the
range preserves the current upshift fade/search safety margins. No sample-rate
reset, memory allocation or hard tap jump is needed. The extra buffer is
allocated by `prepare()`. Both Pitch variants and both harmonisers use this
same implementation.

Bass wet delay is consequently longer and variable. This fixes the constrained
lag search that put bass drop D almost a semitone sharp at +12 in the harmonic
regression signal. Low B, drop D and low E are checked at 44.1/48/96 kHz with
sines and harmonic-rich notes, shifts from -12 through +24, and varying blocks.
Fixed Pitch and Dual Pitch still do not classify notes or quantise to a key.

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
engine at the adaptive 30 ms base setting. `voice1` and `voice2` each span −12 to +12 semitones,
in whole-semitone steps. `level1`, `level2`, and `dry` are independent 0–100 trims
with the same off/unity/+6 dB mapping as the original plugin. Defaults are −12
and +12 semitones, both voices at unity, and dry off. The original `smbPitchShift`
plugin remains available unchanged.

Both voices consume the original input; neither processes the other voice.
The audio callback uses bounded stack scratch buffers for arbitrary block sizes,
including in-place calls. Sample-rate setup prepares both engines, and scene
reset clears both without allocating. Houston exposes five matching controls,
including expression assignments, on the Dual Pitch pedal artwork.

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
bass adaptation described above intentionally changes search geometry, so the
initial port's bit-identical comparison is historical, not a claim for current
bass output. Mix regressions cover both zero, each path alone, intermediate
levels, startup, reset, and an unprepared engine, including in-place buffers.

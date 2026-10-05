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

The `smbPitchShift` plugin and parameter IDs, Pitch/Wet/Dry controls, ranges,
and defaults remain unchanged. Wet and Dry are independent linear gains from
0% (silence) to 100% (unity). Both at zero produce silence; Dry alone passes
the input at its selected level, Wet alone passes only shifted audio, and
their intermediate settings add the two independently scaled signals.
The existing octave control still adds ±12 semitones; the core accepts the
resulting full ±24-semitone range. Neutral legacy band controls leave the
shifted signal unchanged; existing band settings still apply after shifting.

The legacy quality parameter selects upstream window lengths:

| `smbPitchShift.latency` | Window | Nominal mean delay |
| --- | --- | --- |
| 0 (high quality) | 60 ms | 31 ms |
| 1 (Houston's selection) | 30 ms, TONE3000 default | 16 ms |
| 2 (realtime) | 20 ms | 11 ms |

The dry path is immediate unless the legacy latency-compensation option is
enabled, in which case it uses the nominal mean delay. The wet tap moves
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
with a wet fade from silence and scales unready fallback audio by Dry. The
pitch search, tap interpolation, attack detection, and splice crossfades are
unchanged. Mix regressions cover both zero, each path alone, intermediate
levels, startup, reset, and an unprepared engine, including in-place buffers.

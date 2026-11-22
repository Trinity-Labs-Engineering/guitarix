# Houston compressor models

Houston's Compression drawer now includes **Studio Compressor**, **FET 76** and
**Opto Leveler**, alongside the existing Compressor. They are native mono Guitarix
processors with saved controls, expression/modifier support and dedicated Houston
panels. Existing compressor IDs, parameters, defaults and audio code are preserved.
Deploy the matching Guitarix and Houston builds together.

The answer to “transparent or warm like a 76?” is **both**: Studio supplies clean
gain control; FET 76 adds fast feedback compression and adjustable harmonic colour;
Opto provides gentler, programme-dependent recovery and colour. The latter two are
behavioural designs, not calibrated component models of an 1176 or LA-2A.

## Research and selection

Sources reviewed on 22 November 2026:

| Candidate | Finding and decision |
| --- | --- |
| [Faust compressors.lib](https://faustlibraries.grame.fr/libs/compressors/) / [Bart Brouns' faustCompressors](https://github.com/magnetophon/faustCompressors) | Established feed-forward/feedback building blocks with adjustable knee and detector topology. The quadratic gain computer is the common foundation for these models |
| [Faust limiter_1176_R4_mono](https://faustlibraries.grame.fr/libs/compressors/#colimiter_1176_r4_mono) | Calls the generic compressor at fixed settings; documentation identifies it as an example with omitted EQ coloration. It is not a component-modelled 1176 |
| [UA 2-1176 manual](https://media.uaudio.com/assetlibrary/2/-/2-1176_manual.pdf) | Documents input/output operation, 4/8/12/20 ratios, 20–800 µs attack and 50–1100 ms release. FET 76 uses those familiar controls with an approximate response |
| [UA LA-2A manual](https://media.uaudio.com/assetlibrary/l/a/la-2a_manual.pdf) | Describes optical-cell memory and release dependent on compression duration/amount. Opto models a fast envelope plus slower charge storage |
| [Noob CompressorLab](https://github.com/Noob-Audio-Engineering/noob-compressorlab) | Advertises more detailed voltage-domain FET and optical grey-box models in a separate C++ plugin. A possible future port/validation project; not a ready Faust integration, and not vendored here |

No vetted, ready-to-integrate Faust 1176/LA-2A component model was established from
these sources. This integration does not claim transistor, transformer, tube or
electroluminescent-panel circuit accuracy, or matching to physical hardware.

The adapted gain computer comes from the installed `compressors.lib` 1.6.0, with
Faust libraries 2.59.0, checked against the current
[upstream source](https://github.com/grame-cncm/faustlibraries/blob/master/compressors.lib).
The source file's SHA-256 is
`e89e97a117d9159ac7c2139758b971b2e5260ff78cce320c400e67baf036eccf`.
Only the required computation is retained in `artemis_compressors.lib`;
there is no runtime or build-time download. That file and the three wrappers use
GPL-3.0-only, compatible with Guitarix's distribution. See
`FAUST_EFFECTS_LICENSES.txt` and `../../COPYING`.

## Controls and response

| Model / runtime prefix | Controls | Response |
| --- | --- | --- |
| Studio Compressor / `studio_compressor` | Threshold −60–0 dB; Ratio 1–20; Knee 0–24 dB; Attack 0.1–100 ms; Release 20–2000 ms; Makeup −12–24 dB; Mix | Feed-forward peak gain computer with gain-domain attack/release. Ratio 1 with 0 dB makeup and Mix 0 are transparent |
| FET 76 / `fet_compressor` | Input −12–24 dB; Ratio 4/8/12/20; Attack 20–800 µs; Release 50–1100 ms; Colour; Output −24–12 dB; Mix | Feedback sidechain before colour/output/mix, fixed −18 dB internal threshold. Detector times scale with ratio for stability; knee scales to retain its input-referred width |
| Opto Leveler / `opto_compressor` | Threshold −48–0 dB; Ratio 2–10; Recovery 0.5–2×; Colour; Makeup −12–24 dB; Mix | 10 ms fast envelope and 60 ms fast recovery, plus storage charging over 350 ms and releasing over 2.5 s at Recovery 1. The slow branch sustains up to half the reduction after long compression |

FET ratio is stored as positions **0, 1, 2, 3**, displayed as **4:1, 8:1, 12:1,
20:1**. There is no all-buttons mode. Time ranges are nominal model controls, not
measured hardware timing. Output/makeup is manual and can raise peaks above full
scale; these are not brick-wall limiters. Colour and Mix use 0–100%.

Colour adds a unity-small-signal-gain saturator, `x / sqrt(1+x*x)`, with first-order
antiderivative antialiasing. A rationalised divided difference stays well behaved
at identical samples. It adds odd harmonics without a DC bias. It reduces aliasing
but is not an oversampled hardware circuit. FET/Opto's linear paths match the
half-sample FIR inherent in that antialiasing, including at Colour 0 and Mix 0.
Their dry endpoint is therefore half-sample filtered rather than a bit-identical
wire; at 48 kHz this is 10.4 µs of group delay with some upper-treble rolloff.
There is no lookahead buffer, whole-sample onset delay or additional switching wait.

## Scene integration

- Each processor has less than 2 KiB of inline state and no activation allocator,
  file load, worker, lock, service or JACK graph change. Audio callbacks allocate
  nothing. Source and generated C++ use the existing Faust/Guitarix build paths.
- Editable values have a 10 ms smoothing time constant, including FET ratio
  changes. Sample one seeds controls directly from their stored targets, avoiding
  a startup fade from zero or a wait for controls to settle.
- `PGNI_RESIDENT_TRACK_BYPASS` makes resident bypassed models observe live input
  in the engine's existing scratch pass. Re-entry preserves their envelopes.
  Tail processors retain their existing reset rules.
- Houston skips the redundant ownership-reset transaction for these models and
  the existing tracking Compressor/Expander. Prepared model changes, new logical
  owners of shared models, bypass and re-entry retain a single scene RPC. Existing
  software/firmware ramps and wait budgets are unchanged. Full switching time
  still depends on routing, resource preparation, preset and target CPU load.
- Panels resolve controls and units by DSP ID, including renamed presets. All
  six/seven controls are visible and use the existing touch, footswitch, expression
  and modifier paths. The legacy `compressor.*` transfer graph/meter does not
  describe these models and is not shown. No extra meter process starts.

## Verification

Validated in this workspace on 22 November 2026:

- Native Guitarix build (`./waf build -j2`) and Houston production UI build pass.
- Both dedicated DSP/contract tests pass with ASan and UBSan at 44.1, 48, 96 and
  192 kHz: silence/control extremes, steady-state ratios, soft knee, effective
  attack/release, control edges, colour harmonics, zero DC, optical memory,
  sample-one control seeding, impulse onset, in-place/irregular-block equivalence,
  allocation-free callbacks, and warm bypass re-entry.
- All 37 native scene tests pass. The expanded backend scene selection ran 132
  tests: 131 pass; one pre-existing catalogue-identity assertion fails (below).
- All 48 focused UI/formatting tests and three 800×480 browser tests pass. Browser
  checks include visible, non-overlapping targets, ratio labels and changing Mix.
  Screenshots were visually inspected.
- Two older UI assertions fail: the legacy compressor expects an outdated bypass
  colour, and a fuzz test expects Astrofuzz in Drive instead of Distortion. Both
  were reproduced against unchanged Houston code in a temporary checkout. The
  backend catalogue test also fails there: 79 physical identities for 83 entries;
  the three unique additions make this 82/86. These unrelated failures are unchanged.

An optimized 48 kHz benchmark on this **ARM64 development host**, over 150,000
32-frame callbacks per processor, measured:

| Processor | Mean callback | Approximate share of one core |
| --- | ---: | ---: |
| Existing Compressor | 1.13 µs | 0.17% |
| Studio Compressor | 1.14 µs | 0.17% |
| FET 76 | 2.21 µs | 0.33% |
| Opto Leveler | 1.80 µs | 0.27% |

These are mean offline DSP costs, not worst-case deadlines, scene wall times or
CM5 measurements. Resident bypass incurs the same DSP cost for prepared tracking
models. No Artemis audio device was available: listening, loaded two-loop XRUN
tests and before/after switch-latency measurements on CM5 remain release checks.
These fixtures cannot guarantee the absence of audible clicks or timing regressions
on target hardware.

## Reproduce

From `guitarix/trunk/build`, using Faust 2.81.10:

```sh
for effect in studio_compressor fet_compressor opto_compressor; do
    ../tools/dsp2cc --double -s 40000 --init-type=plugin-instance --no-version-header \
        -o ../src/faust-generated/$effect.cc ../src/faust/$effect.dsp
done
```

From `guitarix/trunk`:

```sh
python3 -m unittest discover -s tests -p test_faust_compressors.py -v
python3 -m unittest discover -s tests -p 'test_scene*.py' -v
c++ -std=c++20 -O3 '-D_(x)=x' -Isrc/headers -Isrc \
    tests/faust_compressors_fixture.cpp -o /tmp/houston-compressors-bench
/tmp/houston-compressors-bench --benchmark
```

This sandbox cannot run LeakSanitizer's process tracer, so sanitizer runs used
`ASAN_OPTIONS=detect_leaks=0`; AddressSanitizer and UndefinedBehaviorSanitizer stayed
enabled. Normal hosts can omit that override.

From `houston`:

```sh
python3 -m unittest tests.test_compressor_models tests.test_faust_effect_library tests.test_resident_scene_history -v
cd ui
npm test -- --run tests/unit/CompressorModels.test.tsx tests/unit/potValueDisplay.test.ts
npm run test:browser -- tests/browser/compressor-models.spec.ts
npm run build
```

On Artemis, compare existing Compressor scenes against the additions using the
same route and input. Include sustained notes, silence, repeated bypass, different
settings on shared resident models and both virtual loops at 48 kHz/32 frames.
Record existing scene timing telemetry and run `python3 bin/xrun_test.py` in
Houston. Audition level-matched clean/coloured settings, including full Colour and
low guitar/bass fundamentals, before release.

# Mono Faust audition effects

These six processors are registered as Guitarix mono rack plugins. Houston exposes
them in Reverb, Modulation and Drive using its existing generic pedal renderer.
Existing effects and their identities remain available. This is an audition set;
listening tests and CM5 realtime load measurements are still needed before release.

| Effect | Source and reason for inclusion | Controls |
| --- | --- | --- |
| Spring Reverb | Daniel Leonov / Faust `springreverb`: metallic multi-spring-inspired delay network, fills the spring gap | Dwell, Tone, Tension, Character, Mix |
| Dattorro Plate | Jakob Zerbian / Faust `dattorro_rev`: dense plate-style diffusion, distinct from Freeverb | Decay, Damping, Predelay (ms), Mix |
| Zita Hall | Julius O. Smith's Faust adaptation of Fons Adriaensen's Zita topology: eight-line FDN and separate bass decay | Decay (s), Bass Decay multiplier, HF Damping (Hz), Predelay (ms), Mix |
| JP Ambient | Julian Parker / Till Bovermann `jpverb`: modulated diffuse tails for ambient playing | Decay (s), Damping, Size, Mod Depth, Mod Rate (Hz), Mix |
| Phase 90 | David Braun's four-stage JFET circuit model of the 1974 script-logo pedal, signal-dependent rather than a generic phaser | Rate (Hz), Level (dB) |
| Klon Centaur | David Braun's port of Jatin Chowdhury's ChowCentaur Traditional circuit model, with WDF gain stages and 2x diode-stage oversampling | Gain, Treble, Level |

## Sources and candidate review

Research snapshot: 2026-10-29. The newly vendored spring and circuit sections are
from [faustlibraries revision 4b8961b](https://github.com/grame-cncm/faustlibraries/tree/4b8961b03ff90f5457724d987d14703d960ec30b),
including the Wright omega helper needed by the Klon diode stage,
with attribution and implementation notes retained in `artemis_reverbs.lib` and
`artemis_circuits.lib`. Hall and JP Ambient use the established `reverbs.lib`
functions supplied with the build's Faust installation (1.4.0 with Faust 2.81.10
for the checked-in C++). No library download occurs during builds.

- [Faust reverb reference](https://faustlibraries.grame.fr/libs/reverbs/)
- [Faust virtual analog effects reference](https://faustlibraries.grame.fr/libs/vaeffects/)
- [Original spring implementation](https://github.com/daleonov/SpringReverb)
- [Phase 90 reference model and DAFx-24 work](https://github.com/polimi-ispl/mxrphase90)
- [ChowCentaur reference implementation](https://github.com/jatinchowdhury18/KlonCentaur)

The spring is an algorithmic spring-inspired network, not a component or physical
spring simulation. Phase 90 and Klon are circuit models; neither has been compared
to physical hardware here. Faust's Fulltone OCD is an interesting later candidate,
but its documented ideal-op-amp model omits supply-rail clipping, has substantial
level boost, and adds 11 samples of latency. It needs a separate gain/latency review.
Greyhole is another useful ambient-delay candidate, but overlaps more with the
existing delay category. Vital's reverb and Supermassive-style algorithms are
possible follow-ups after evaluating this smaller set's CPU and sound.

## Integration details

- All exposed callbacks are **one input / one output**, with no stereo callback.
  Plate, Hall and Ambient average the two tank outputs. This preserves mono
  compatibility but does not halve the cost of the internal stereo tank.
- Reverbs use a single dry/wet mix, 0–100%, with true dry and wet endpoints once
  control smoothing settles. Default mix is 25–30%. All new DSP runs in double
  precision, including the Klon's wave digital filters.
- Spring's vendored core is wet-only. The upstream diffuser reversed the arguments
  of `de.delay(maximum, delay)`; the local copy fixes all four calls and increases
  the maximum to 50 ms to cover the longest tap. Tone is capped below Nyquist.
  Character 0/1/2 means Tight/Wide/Balanced spacing, not spring count.
- Plate scales the reference delay lengths from 29761 Hz to the current sample
  rate, uses the distinct output delays 3720/3163, and has a crossfaded external
  pre-delay. Decay is a feedback coefficient, not a claimed T60 in seconds.
- Hall pre-delay is outside the legacy Zita input buffer, allowing the full 100 ms
  range at 192 kHz. JP Ambient retains the upstream prime-table/interpolation
  design; its Size changes should be auditioned while audio is playing. JPverb's
  nominal decay/size are not guaranteed acoustical measurements at every rate.
- Phase 90's output polarity is restored for parallel chains. The default -5 dB
  Level roughly compensates the circuit's 1.788x output gain. Rate is deliberately
  in Hz (0.1–10), preserving the slow sweep range. The circuit already includes
  its dry/all-pass sum, so no second mix control is added.
- Generated plugins follow Guitarix's 40000-byte lazy-allocation threshold. Large
  histories allocate on activation and release on deactivation, and state clearing
  supports resident-scene reuse. Houston defaults, ranges, steps and IDs are tested
  against the actual generated C++ registrations.

## Regeneration and validation

Checked-in C++ was generated with Faust 2.81.10. From `trunk/build`:

```sh
for effect in spring_reverb plate_reverb hall_reverb ambient_reverb phase90 klon_centaur; do
    ../tools/dsp2cc --double -s 40000 --init-type=plugin-instance --no-version-header \
        --add='-t 600' -o ../src/faust-generated/$effect.cc ../src/faust/$effect.dsp
done
```

The Klon wave-digital tree takes several minutes to expand on this compiler;
its Waf generation task allows 600 seconds (user Faust options still apply).
Normal generated-source builds do not need Faust. An older Faust compiler is not
claimed tested for regenerating these additions. The generated files are the
reviewable source used by production builds. `waftools/faust.py` also tracks
`library(...)` dependencies so editing the vendored cores triggers regeneration.

```sh
# From Guitarix trunk:
python3 -m unittest discover -s tests -p test_faust_effects.py -v
# From Houston:
python3 -m unittest discover -s tests -p test_faust_effect_library.py -v
# From Houston/ui:
npm test -- --run tests/unit/VirtualPedalArtwork.test.tsx tests/unit/virtualPedalCategories.test.ts
npm run build
```

The audio fixture uses ASan/UBSan and exercises 44.1, 48, 96 and 192 kHz, in-place
processing, uneven block sizes, dry/wet endpoints, decaying impulse responses,
control extremes, effective decay controls, and clearing history. The existing
`test_lazy_dsp.py` also exercises allocation failures and resident reuse for all
four new reverbs. It also compares Houston's catalog to
registered DSP controls. This does not replace listening or a CM5 xrun test.

## Licences

Faust's spring demo and Phase 90 identify MIT; JPverb identifies MIT. The original
spring repository has no separate licence file; this integration uses the Faust
library distribution and its MIT-labelled spring demo. Dattorro/Zita and the
underlying WDF helpers use the MIT-style STK-4.3 licence. The Klon port identifies
BSD-3-Clause and Chowdhury-DSP copyright. See `FAUST_EFFECTS_LICENSES.txt`, retained
source declarations, and Guitarix's existing `COPYING` for notices. Generated DSP
and wrappers remain part of Guitarix's GPL distribution.

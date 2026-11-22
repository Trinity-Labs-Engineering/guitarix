# Houston delay time and slapback

The previous 360 BPM ceiling allowed a shortest quarter-note delay of about
167 ms. Delay (`delay`), Digital Delay (`dide`), Duck Delay (`duckDelay`), Echo
(`echo`) and Reverse Delay (`reversedelay`) now accept 24–6,000 BPM, corresponding
to 2,500–10 ms. All five register a 120 BPM default and a 0.1 BPM step.

## Houston controls and compatibility

The primary control displays milliseconds and uses logarithmic travel, with
1 ms adjustments and clockwise/up movement increasing time. Artwork, footer,
keyboard, footswitch adjustments and live previews use the same conversion.
Selecting a clock-synchronised knob without moving it does not round its time.

BPM entry, tap tempo and clock subdivisions live in the Expression panel.
Subdivision availability uses each effect's native range, allowing, for example,
120 BPM × 4 = 480 BPM = 125 ms and 120 BPM × 8 = 960 BPM = 62.5 ms.
The global clock generator and modulation effect ranges are unchanged.

Native parameter names, saved values, expression/modifier bounds and transport
messages remain BPM. Loading an expanded preset refreshes the definition's
range and presentation without changing its value or assignments. Compact
presets obtain the new definitions from the catalogue. Legacy Duck/Reverse
`time` values and expression bounds still convert from milliseconds exactly
once, with their assignment direction preserved by the existing migration.
Digital Delay uses its native default quarter-note setting; Houston's clock
subdivision is applied to the BPM value before sending it to the engine.

Deploy Houston and Guitarix together: the older native engine would clamp the
new short settings. Existing saved times in the supported range remain intact.

## DSP and scene switching

The four Faust algorithms retain their delay buffers and interpolation/smoothing:
100 ms delay-tap crossfades for Delay, Digital Delay and Echo; the existing
100 ms time smoother for Duck Delay. No additional lookahead, block delay,
resource loading, scene wait or JACK routing operation is added.

Reverse Delay is a reversed-chunk processor, so its time is the chunk length,
not a conventional forward first-repeat time. Its BPM clamp is expanded and
its overlap window now enforces a two-sample minimum fade. Previously, short
chunks with small nonzero Window values could divide by zero. The processor's
existing parameter-change/reset behaviour otherwise remains intact.

Generated code uses the repository's current `dsp2cc` lifecycle handling,
including deferred initialisation, safe clearing and array deletion. The
regenerated Echo also adopts the compiler's current expression ordering.
Buffer sizes, DSP precision and the resident scene ownership/reset protocol
are unchanged. Same-processor owner changes retain the existing two-transaction
off/on barrier; different-model switches retain one transaction.

## Validation

- `tests/test_tempo_effects.py` compiles and runs the actual plugin callbacks
  with AddressSanitizer, UndefinedBehaviorSanitizer and float-divide-by-zero
  checks. At 44.1, 48, 96 and 192 kHz, it measures forward repeats at 10, 40,
  80, 100, 120 and 2,500 ms within three samples, verifies reverse chunk periods
  and finite output at Window 0/0.5/100%, and checks silence after reset.
  It also tests feedback decay, digital filter defaults and native/catalogue
  range agreement. LeakSanitizer is disabled for sandbox process restrictions.
- Houston's 19 focused backend tests pass: native and legacy preset reload,
  retained expression bounds, short clock subdivisions, modulation limits,
  resident model/owner switches, bypass and re-entry.
- 82 focused UI unit tests pass, including logarithmic adjustment, native BPM
  previews/commits, no-motion selection, tap tempo and preset migration.
- Seven Playwright tests pass, including each delay on the 800×480 touchscreen,
  manual 750 BPM entry, clock subdivisions and the existing touchscreen numpad.
- Native Guitarix and production Houston UI builds pass.

These checks do not substitute for on-device listening, XRUN stress tests or
before/after switch-latency measurements. This workspace has no Artemis audio
interface or running JACK audio server.

```sh
# From guitarix/trunk:
ASAN_OPTIONS=detect_leaks=0 python3 -m unittest discover -s tests -p test_tempo_effects.py -v
./waf build -j2
# From houston:
python3 -m unittest tests.test_delay_controls tests.test_tempo_controls tests.test_expression_assignments tests.test_modulation_controls -v
# From houston/ui:
npm test -- --run tests/unit/delayControls.test.tsx tests/unit/tempoSubdivisions.test.ts tests/unit/useBoundedVerticalControl.test.tsx tests/unit/KnobControl.test.tsx tests/unit/ExpressionAssignmentPanel.test.tsx tests/unit/potValueDisplay.test.ts
npm run test:browser -- tests/browser/delay-controls.spec.ts tests/browser/virtual-loop-tempo.spec.ts
npm run build
```

Faust generation uses 2.81.10 with libraries 2.59.0, the project's 40,000-byte
allocation threshold, plugin instances, float precision for Delay/Digital
Delay/Echo and double precision for Duck Delay.

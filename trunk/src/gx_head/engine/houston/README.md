# Houston native analysis

Built into standalone Guitarix by `src/gx_head/wscript`; no JUCE libraries,
headers, or runtime initialization. Disabled unless
`GUITARIX_HOUSTON_ANALYSIS=1`. Artemis enables it only in VL1.

The owner opens one input-only JACK client, `<instance>_analysis`. Its five
inputs are `signal_in`, `pre_in_l`, `pre_in_r`, `post_in_l`, and `post_in_r`.
Houston owns connections. There are no outputs and no changes to Guitarix's
mono/stereo DSP callbacks. The observer stays active when Guitarix's amp/FX
clients are gated off.

The JACK callback (`gx-analysis-rt`) only copies bounded packets into a
preallocated 1 MiB SPSC JACK ringbuffer. It never waits for a consumer. Full
queues or blocks above 8192 frames increment `dropped_process_blocks`.
Frames retain their sample clock, rate, and tuner session generation. Gaps
reset pitch history; stale sessions cannot reappear after closing the tuner.

One SCHED_OTHER analysis worker drains meter blocks and evaluates the newest
complete tuner hop every 5 ms. Meter peak/RMS release remains 0.35/0.45 s,
clipping is latched at 0.999, and dB floors at -120. The original pitch detector,
gate, smoothing, search range, hop and window are preserved. Tuner work is
disabled while closed; reference calibration survives UI visits.

HTTP remains loopback-only:

| Port environment variable | Default | Service |
|---|---:|---|
| `HOUSTON_TUNER_PORT` | 7714 | Tuner |
| `HOUSTON_METER_PORT_PRE` | 7712 | Pre meter |
| `HOUSTON_METER_PORT_POST` | 7713 | Post meter |

Each serves `/health`, `/state`, `/ports`, and `/api`; meters also serve
`/levels`. Health identifies `implementation: "guitarix-native"` and the actual
JACK client. Meter port maps contain input ports only. Existing tuner reference
and meter reset actions remain available. `tracking.set` acknowledges the
fixed original 80 ms smoothing used by Houston.

`analysis.start` and `analysis.stop` control tuner work. The legacy tuner
`service.shutdown` action also stops only tuner analysis: it can never kill
Guitarix or meters. Meter `service.shutdown` is unsupported. HTTP snapshots
never enumerate JACK connections; the analysis worker caches these at 250 ms.
Each listener has one HTTP worker and at most eight queued requests.

Bind/open failures clean up the optional observer and are reported by Guitarix
without preventing its DSP startup. Worker exceptions mark health unavailable.
The containing process remains the failure boundary for unrecoverable native
faults; moving code into a process cannot retain process-level crash isolation.

## Source provenance

`yin/{PitchTracker,YinTracker}.{cpp,h}` are byte-for-byte copies of
`houston/plugins/yin-tuner/src/yin` at Houston commit
`f85fe708ea9de3e6df095904596eeb2f0bbb1b07`. Keep detector changes separate from
hosting changes. `tests/test_houston_analysis.py` checks parity when the Houston
sibling checkout is present.

`third_party/httplib.h` is copied from the workspace's cpp-httplib commit
`34b7c846d43e30598f3b748ef5ce23591077cb7e`; its MIT license is included.
JSON uses Guitarix's existing NAM nlohmann header.

## Tests

From `trunk`:

```sh
python3 -m unittest discover -s tests -p test_houston_analysis.py -v
GUITARIX_ANALYSIS_JACK_TEST=1 \
  python3 -m unittest discover -s tests -p test_houston_service.py -v
```

The second command needs JACK development files and `jackd`; it starts and
cleans up a uniquely named dummy server. To exercise the compiled Guitarix
host, including amp/FX deactivation and reactivation:

```sh
GUITARIX_ANALYSIS_JACK_TEST=1 \
GUITARIX_ANALYSIS_HOST="$PWD/build/src/gx_head/guitarix" \
LD_LIBRARY_PATH="$PWD/build/libgxwmm/gxwmm:$PWD/build/libgxw/gxw" \
  python3 -m unittest discover -s tests -p test_houston_service.py -v
```

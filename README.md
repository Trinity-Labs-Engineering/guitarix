# guitarix

![guitarix](https://a.fsdn.com/con/app/proj/guitarix/screenshots/Guitarix_Dark.png/max/max/1)

Guitarix is a modular, virtual amplifier for Linux.
With Guitarix you can choose different preamp and amp models
and /or could load [*.nam files](https://tonehunt.org/all) with the [Neural Amp Modeler](https://github.com/sdatkinson/NeuralAmpModelerCore) modules,
or load [*.json or .aidax files](https://cloud.aida-x.cc/all) with the [RTNeural](https://github.com/jatinchowdhury18/RTNeural) modules, 
to simulate a specific hardware unit.
Combine them with various effects and speaker cabinet emulations
and/or load your own Impulse Response files
to come up with your very own tones.
Guitarix comes as a standalone application or as [vst3 plugin](https://github.com/brummer10/guitarix.vst).
Its modules are also available in the LV2 plugin format, 
which you can incorporate into your DAW of choice. 
Furthermore, it can even be run headless, so you can turn a Raspberry Pi, 
or any other such devices, into a dedicated amp modeler. 
You can even control Guitarix via a MIDI controller or foot-board.

Guitarix comes with an extensive list of effects 
including compression, distortion, modulation, reverb, delay, EQ, etc. 
Some of the effects modules that are included in Guitarix 
are influenced by some popular hardware units, 
for example the Tube Screamer is, not surprisingly, based off of the Ibanez Tube Screamer.
Guitarix can also load up LADSPA and LV2 plugins to comply the effect chain.

## Documentation
[Standalone app](/documentation/standalone-app-overview.md)

[LV2 plugins](/documentation/lv2-plugins-overview.md)


## Building guitarix from source code

- git clone https://github.com/brummer10/guitarix.git
- cd guitarix
- git submodule update --init --recursive
- cd trunk
- ./waf configure --prefix=/usr  --includeresampler --includeconvolver --optimization 
- ./waf build
- sudo ./waf install

## (build) Dependencies

- gperf,
- intltool,
- libavahi-gobject-dev,
- libbluetooth-dev 
- libboost-dev,
- libboost-iostreams-dev,
- libboost-thread-dev,
- libeigen3-dev,
- libgtk-3-dev,
- libgtkmm-3.0-dev,
- libjack-dev | libjack-jackd2-dev,
- liblilv-dev,
- liblrdf0-dev,
- libsndfile1-dev,
- libfftw3-dev,
- lv2-dev,
- python3,
- sassc,

## Hardware Optimization and Containerized Deployment

Guitarix can be easily built with hardware optimizations and deployed in a containerized environment using GuitarixContainer, a Docker-based solution. This approach ensures the code is automatically built for your hardware, enabling low-latency performance, especially when running Neural Amp Modeler (NAM) module. Perfect for devices like Raspberry Pi or laptop/workstation setups. Learn more at the [GuitarixContainer](https://github.com/fjammes/GuitarixContainer) GitHub repository.



## Prepared resident snapshots (Trinity Phase 1)

The acknowledged `set_scene` / `set_scene_muted` API remains the compatibility
path for structural and asset changes. Resident bypass signals have their own
RT dirty class, so ordinary bypass does not rerun module selectors or rebuild
processing lists. Topology and asset changes invalidate prepared snapshot handles.

Phase 2 additionally supports prepared matrix recipes as described below.

For an already committed matrix, the compact JSON-RPC API is:

- `snapshot_prepare(matrixSignature, snapshotId, parameter, value, ...)` resolves
  control pointers and returns `handle`, `generation` and `controlCount`.
- `snapshot_clock()` returns the unsigned 32-bit JACK `currentFrame`,
  `sampleRate`, `bufferSize`, `generation` and `snapshotSupported`.
- `snapshot_activate(handle, targetJackFrame, transitionMs)` schedules the
  resident controls. `transitionMs` is 2–5 ms (default 4 ms, total down and up).

The host prepares both Guitarix processes first, reads fresh clocks, then sends
both handles with the same future, buffer-aligned frame. The requested frame
must leave at least half the envelope plus one audio buffer of preparation
margin and be no more than 200 ms ahead. Arithmetic handles JACK frame wrap.
Both mono and stereo callbacks apply their own controls at that exact frame,
including in the production multi-client mode. The final stereo output carries
one sample-linear envelope. The reply follows completion of the envelope and
reports `scheduledFrame`, `appliedFrame`, `completedFrame`, `structureChanged`
and `rtStateChanged`; missed frames fail instead of applying late.

Snapshot controls include resident effect bypass and directly bound continuous
DSP controls without internal parameter-change callbacks. DSP state is retained;
activation does not clear, allocate, prewarm, activate or deactivate plugins.
A plugin requiring a reset must declare `PGNI_SNAPSHOT_RESET_REQUIRED`, which
excludes its changed bypass from this path. Split boundaries, stereo-source
selection, position, pre/post placement, file selectors and NAM size/profile
changes are rejected. Unchanged unsupported numeric controls can be retained as
guards; changing a guard requires the general scene path. This conservative
contract also covers third-party plugins whose control callbacks do more than
write a DSP scalar. No second matrix bank is introduced in this phase.

Run the native fixtures without building the application:

```sh
python3 -m unittest discover -s trunk/tests -p 'test_*.py'
```

The snapshot fixture covers 1,000 directed transitions, opposite callback order
in two instances, allocation-free activation, the 4 ms DC transition envelope,
frame wrap, stale handles, late frames, constant guards and callback ownership
after cancellation. Device recordings and end-to-end latency/XRUN measurement
are still required to establish the performance acceptance targets.

## Prepared matrix recipes (Trinity Phase 2)

Song preparation now has a server-owned, generation-scoped recipe pool:

- `matrix_begin()` retires previous handles and inactive prepared assets, and
  returns protocol version 3, the generation and admission limits. Version 2
  must not be used: its IR worker warm-up can deadlock the control thread.
- `matrix_prepare(generation, matrixId, snapshotId, sceneJson, namJson)` validates
  the whole graph, resolves typed parameter setters and prepares its assets.
  `sceneJson` is an encoded alternating parameter/value array. `namJson` is an
  encoded sequence of slot/path/profile triples; slots are `nam`, `nam__2`,
  `snam`, `A` and `B`. Empty paths explicitly clear a slot. Numeric directory
  selectors, NAM profile setters and unhandled file controls are rejected.
- `matrix_activate(handle, externallyMuted)` applies the retained recipe through
  the existing scene commit (`externallyMuted` must be integer 1 in Phase 2). The reply
  includes `matrixHandle`, `matrixGeneration`, `matrixActivated`, normal scene
  readiness fields and timings. Error -32003 rejects stale handles/assets before
  applying anything; other activation errors must be treated as uncertain.
- `matrix_release(handle)` releases that recipe and invalidates snapshots.
  Shared assets are retained until the next `matrix_begin()`.

Each recipe contains its initial snapshot controls. Recipes sharing `matrixId`
may differ only in nonstructural controls. Guitarix completes omitted dynamic
rack state from deterministic defaults rather than the live graph. Once active,
its handle can replace the legacy signature argument to `snapshot_prepare`.
Matrix and snapshot handles include an engine-instance identity; pool epochs,
JACK activity, runtime reconfiguration and sample/block dimensions fence reuse.

Admission allows 128 recipes, eight NAM slot/file/profile identities and eight
IR slot/configuration identities, including active assets. A 256 MiB estimated
asset budget is enforced before loading. Replies expose `namCount`, `irCount`,
`estimatedAssetBytes`, `assetBudgetBytes` and `memoryAccounting=estimated-assets`.
These estimates cover source/model and convolution storage, not measured process
RSS or every plugin/worker/allocator allocation. NAM models are loaded/prewarmed
through their existing DSP loader. IR preparation configures separate stopped
convolvers, including FFT plans and transformed IR partitions, without starting
worker threads. Guarded activation transfers ownership of cached processors and
starts them with normal asynchronous processing.

`ready`/`assetsReady` means a validated recipe with prepared assets.
`requiresGuard=true` and `pointerSwitchReady=false` are deliberate: module-list
publication, plugin lifecycle work, resampler setup and the singular amp chain
still use the general guarded commit. Phase 3's independently running Current/
Next chains and Phase 4's short physical cutover are not implemented here.

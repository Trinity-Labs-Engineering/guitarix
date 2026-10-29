BLOCKS Studio: Technical Specification
Project: Trinity Labs BLOCKS (software side) Component: BLOCKS Studio, a browser-based DSP effect editor Status: Draft v0.1 Stack: React, TypeScript, Faust, WebAssembly, Web Audio (AudioWorklet)

1. Background and Purpose
BLOCKS began as a concept for a small, affordable (£99–£149), reprogrammable digital effects pedal. The agreed strategy is to build the software ecosystem first:
Release BLOCKS Studio, a free browser-based editor in which users create DSP effects and audition them in the browser.
Let users deploy those effects to Artemis as virtual effects.
Build a community around creating and sharing effects.
Only if demand is proven, manufacture the standalone BLOCKS hardware, launching with an established catalogue of effects.
This document specifies Phase 1 and the foundations of Phase 2. The hardware pedal is out of scope except where it constrains software design (Section 4).
1.1 Goals
Let a guitarist with no programming experience create a working effect through a drag-and-drop interface.
Let advanced users write Faust code directly, with the visual and code views staying in sync where practical.
Compile Faust to WebAssembly in the browser and run it live on audio input for instant trialling.
Store effects in a portable DSP format, not as Artemis-specific binaries, so the same effect can target Artemis (Raspberry Pi CM5), a future BLOCKS embedded processor, and the browser.
Validate effects for CPU load, memory use and real-time safety before they can be deployed to a device.
Work without owning Artemis, to reach the widest possible audience.
1.2 Non-Goals for Phase 1
Subscription billing, paid plugins, or a full marketplace (Section 12 describes hooks only).
Porting arbitrary desktop VST plugins. Effects must be expressed in Faust.
Native mobile apps.
The BLOCKS hardware firmware.

2. Users and Core User Stories
Persona
Need
Guitarist-tinkerer
Build a custom drive or delay by connecting blocks, with no code.
DSP hobbyist / developer
Write Faust directly, publish reusable blocks.
Artemis owner
Send an effect to Artemis and recall it via presets and MIDI.
Visitor without hardware
Play with effects in the browser using a guitar input, a sample loop or a DI file.
Trinity Labs
Measure whether people create, share and return to the editor, and who wants hardware.

Key stories:
As a user, I can drag blocks onto a canvas, wire them, tweak knobs and hear the result immediately.
As a user, I can switch to a code view, edit Faust, and see the graph update (or be told why it cannot).
As a user, I can audition an effect on my live guitar input, an uploaded audio file, or built-in test signals.
As a user, I can save, version, publish and share an effect via a link.
As an Artemis owner, I can deploy a published effect to my device.
As a user, I can tell before deploying whether my effect is safe for the target device's CPU and memory budget.

3. High-Level Architecture
┌──────────────────────────── Browser ────────────────────────────┐
│  React App (BLOCKS Studio)                                      │
│  ┌──────────────┐  ┌──────────────┐  ┌───────────────────────┐  │
│  │ Graph Editor │  │ Code Editor  │  │ Inspector / Controls  │  │
│  │ (node canvas)│  │ (Monaco)     │  │ Meters / Scope / Spec │  │
│  └──────┬───────┘  └──────┬───────┘  └───────────┬───────────┘  │
│         └────────┬────────┘                      │              │
│            Effect Document (canonical model) ◄───┘              │
│                  │                                              │
│         Faust Code Generator                                    │
│                  │ Faust source                                 │
│         ┌────────▼─────────┐                                    │
│         │ Compile Worker   │  libfaust (WASM) → DSP wasm + JSON │
│         └────────┬─────────┘                                    │
│         ┌────────▼─────────┐                                    │
│         │ AudioWorklet     │  runs DSP, exposes params          │
│         └──────────────────┘                                    │
└───────────────┬─────────────────────────────────────────────────┘
                │ HTTPS (REST/JSON)
┌───────────────▼─────────────────────────────────────────────────┐
│  BLOCKS Backend                                                 │
│  API · Auth · Effect storage · Publishing/sharing · Analytics   │
│  Validation service (sandboxed): compile → C++ → bench → report │
│  Build artefact store (Artemis packages)                        │
└───────────────┬─────────────────────────────────────────────────┘
                │ Artemis deploy channel (to be defined with Houston)
┌───────────────▼─────────────────────────────────────────────────┐
│  Artemis (Raspberry Pi CM5, Houston)                            │
└─────────────────────────────────────────────────────────────────┘
Principles:
Faust source is the canonical executable form. The visual graph is a structured representation that generates Faust.
Browser compile for trialling, server compile for deployment. The browser result is never trusted as a deployment artefact.
Everything portable. Effects are stored as source plus metadata; target binaries are derived and disposable.

4. Targets and Constraints
Target
Purpose
Notes
Browser (WASM + AudioWorklet)
Trialling in Studio
Phase 1
Artemis (Raspberry Pi CM5)
Deployment as a virtual effect
Phase 1. Packaging route (LV2 or native C++ module) must be verified against Houston's actual processing architecture.
BLOCKS hardware (embedded MCU/DSP, TBD)
Standalone pedal
Phase 3. Not built yet, but the effect format and resource model must not preclude it.

Design implications:
Do not rely on Faust libraries or features that are impractical on constrained targets without flagging them in the validator (e.g. large delay lines, large tables, heavy FFT use).
The validator reports resource usage against per-target budgets. Budgets are configuration, not code.
Sample rate and block size are parameters at compile and run time. Do not hard-code 44.1 or 48 kHz.
Assumption to verify early: that Artemis can host Faust-generated code (via LV2 from faust2lv2 or an equivalent C++ module) without destabilising Houston. Treat this as the first technical spike (Section 15).

5. Frontend Specification
5.1 Technology
React with TypeScript, built with Vite.
State management: a single store (e.g. Zustand or Redux Toolkit) holding the Effect Document with undo/redo history.
Node-graph canvas: React Flow (@xyflow/react) or equivalent.
Code editor: Monaco Editor with a Faust language definition (syntax highlighting, bracket matching, error markers).
Faust in the browser: @grame/faustwasm (libfaust compiled to WASM, with AudioWorklet/FaustAudioWorkletNode helpers). Verify the current API and licence terms before committing to a version.
Styling: CSS modules or Tailwind. Responsive, with desktop as the primary experience.
Audio: Web Audio API with AudioWorklet.
5.2 Layout
Top bar: effect name, save status, undo/redo, audio on/off, input source selector, Publish, Deploy.
Left panel: block library (search, categories, favourites).
Centre: tabbed canvas. Tabs: Graph, Code, Split.
Right panel: inspector for the selected block (parameters, description, CPU cost estimate), plus effect-level controls.
Bottom panel: level meters, oscilloscope, spectrum, compile log and resource report.
5.3 Graph Editor (Visual)
Behaviour:
Drag blocks from the library onto the canvas; connect output ports to input ports.
Signal flow is left to right by default. Fixed ports: Audio In and Audio Out (mono in Phase 1, with a design that does not preclude stereo).
Supported topologies: serial chains, parallel splits with a mixer, feedback loops via an explicit Feedback block (Faust's ~ operator), and a wet/dry mix.
Port typing: audio and control (control signals drive parameters, e.g. an LFO to a filter cutoff). Connecting incompatible ports is rejected with a visible reason.
Cycles are permitted only through Feedback blocks, which introduce the required one-sample delay. Any other cycle is a validation error.
Parameter exposure: any block parameter can be marked "exposed" to appear as a top-level control on the effect (this becomes the effect's UI and its MIDI-mappable parameter set).
Context menu: duplicate, delete, group into a Sub-effect (a reusable composite), bypass.
Auto-layout, minimap, zoom/pan, keyboard shortcuts, multi-select.
5.4 Block Library (Initial Set)
Each block maps to a Faust snippet or a function from the standard Faust libraries (stdfaust.lib).
Category
Blocks
Dynamics
Compressor, noise gate, limiter
Distortion
Overdrive, fuzz, waveshaper (selectable curves), bit crusher
Filters
Low/high/band pass, shelf, parametric EQ, wah
Modulation
Chorus, flanger, phaser, tremolo, vibrato, LFO (sine/tri/square/random)
Time
Delay (with feedback, tap-tempo parameter), reverb, pitch shift
Utility
Gain, pan, mixer, dry/wet, DC blocker, polarity, mute
Control
LFO, envelope follower, constant, expression mapping
Custom
User-authored Faust blocks (Section 5.6)

Each block definition includes: id, version, display name, category, ports, parameters (name, type, range, default, unit, taper), Faust template, documentation text, and a static CPU/memory cost hint.
5.5 Code Editor
Full Faust editing with live (debounced) compilation, inline error markers and a compile log.
Graph ↔ code sync:
Graph → code is always available (the generator is authoritative).
Code → graph is best effort. If the code cannot be represented as a graph, the effect is marked Code-only and the Graph tab shows a single opaque "Custom Faust" block with its exposed parameters. Never silently discard code.
Faust hslider/vslider/nentry/checkbox/button declarations define controls; the UI is generated from the compiler's JSON interface description.
5.6 Custom Blocks
Users can wrap Faust code as a reusable block with declared ports and parameters.
Custom blocks can be published to the community library (Section 8) and imported into other effects by reference to a specific version.
5.7 Audio Trialling
Input sources:
Live input (getUserMedia, with echo cancellation, noise suppression and auto-gain disabled; low-latency constraints requested).
Uploaded audio file (DI guitar recordings), looped.
Built-in test material: DI guitar loops (clean, chugging, lead), test tone, noise burst.
Controls:
Input/output gain and master volume with a limiter on the output to protect users' ears and speakers.
A/B compare against bypass.
Feedback warning: if live input is used with speakers, show a prominent warning and default to a safe output level.
Show measured round-trip latency estimate where the browser exposes it, and the worklet's own CPU load.
5.8 Accessibility and Platform Support
Keyboard-navigable canvas and inspector; visible focus states; ARIA labels for controls.
Target evergreen Chromium, Firefox and Safari. AudioWorklet and WebAssembly are required; show a clear unsupported message otherwise. Low-latency live input varies by browser and OS; document known limitations.

6. Compilation Pipeline (Browser)
Document → Faust: the generator walks the graph and emits Faust. Output must be deterministic (stable ordering and naming) so diffs and hashes are meaningful.
Compile Worker: a dedicated Web Worker hosts libfaust so compilation never blocks the UI thread. Compilation is debounced and cancellable; only the latest request's result is applied.
Output: a DSP WebAssembly module plus the JSON interface description (controls, ranges, metadata).
Instantiate: create an AudioWorkletNode running the DSP; map generated parameters to AudioParams or message-based setters.
Hot swap: when a recompile completes, crossfade (short, e.g. 20–50 ms) from the old DSP instance to the new one to avoid clicks. Preserve current parameter values by name where names and ranges are compatible.
Errors: compile errors are shown inline in the code view and mapped back to the originating graph block where possible. The previous working DSP keeps running.
Performance notes:
Cache compiled modules by content hash (source + compile options + libfaust version) in memory and IndexedDB.
Pre-warm the compiler at app load so the first compile is not slow.
Large Faust libraries make compile times noticeable; measure and display compile time in the log.
Cross-origin isolation (COOP/COEP headers) is only required if SharedArrayBuffer is used. Prefer a design that does not require it unless measurement shows a need.

7. Effect Document Format (Portable DSP)
The Effect Document is the single source of truth. It is JSON, versioned, and contains no device-specific binaries.
```jsonc
{
  "schema": "blocks.effect/1",
  "id": "uuid",
  "name": "Warm Drive",
  "description": "Soft-clipping overdrive with tone control",
  "author": { "id": "user-uuid", "displayName": "..." },
  "version": "1.2.0",
  "license": "CC-BY-4.0",              // chosen from an allowed list
  "tags": ["overdrive", "amp-like"],
  "mode": "graph",                     // "graph" | "code"
  "graph": {
    "nodes": [ { "id": "n1", "block": "core.gain@1", "params": { "gain": 6 }, "position": { "x": 0, "y": 0 } } ],
    "edges": [ { "from": "in:0", "to": "n1:in" } ],
    "exposed": [ { "node": "n1", "param": "gain", "label": "Drive", "order": 0 } ]
  },
  "faust": {
    "source": "import(\"stdfaust.lib\"); ...",   // generated, or authored in code mode
    "libraryVersion": "x.y.z",
    "compileOptions": { "double": false }
  },
  "interface": { /* controls extracted from the compiler: id, label, min, max, init, step, unit */ },
  "io": { "inputs": 1, "outputs": 1 },
  "requirements": { "sampleRates": [44100, 48000, 96000], "targets": ["browser", "artemis"] },
  "assets": [ { "type": "nam", "ref": "asset-uuid" } ],   // see Section 7.2
  "validation": { "lastRun": "...", "reports": { /* per target */ } },
  "createdAt": "...", "updatedAt": "..."
}
```

Rules:
Semantic versioning. Published versions are immutable. Edits create new versions.
Migrations: each schema version has an upgrade function. Never break old documents.
Hashing: each published version has a content hash over canonical JSON.
Parameter identity: exposed parameters carry stable IDs so presets and MIDI mappings survive edits.
7.1 Presets
A preset is a named set of exposed parameter values for a specific effect version, stored separately from the effect. Used for Artemis preset recall and MIDI control.
7.2 NAM Assets (Future-Facing)
BLOCKS is envisaged as running NAM amp models alongside effects. NAM inference is not expressed in Faust. Treat NAM models as separate assets referenced by effects or presets, handled by the host (Artemis/Houston). Phase 1 only needs the asset reference field in the schema and a decision on whether Studio can preview them (likely a later phase, e.g. a WASM NAM runtime). Do not block Phase 1 on it.

8. Backend Specification
8.1 Services
API service: REST/JSON, authenticated. Handles users, effects, versions, presets, ratings, downloads, reports, hardware interest list.
Validation service: sandboxed workers (containers with no network, strict CPU/time/memory limits) that compile and benchmark effects.
Artefact store: object storage for effect JSON, derived build artefacts, and test audio.
Analytics pipeline: privacy-respecting event collection (Section 11).
Technology is left open (e.g. Node/TypeScript with PostgreSQL and S3-compatible storage). The spec constrains interfaces, not frameworks.
8.2 Data Model (Core Entities)
User (id, displayName, email, auth provider, role, createdAt)
Effect (id, ownerId, slug, visibility: private | unlisted | public, currentVersionId)
EffectVersion (id, effectId, semver, document JSON, contentHash, status: draft | validating | published | rejected, createdAt)
Preset (id, effectVersionId, ownerId, name, values)
ValidationReport (id, effectVersionId, target, result, metrics, log, createdAt)
Rating, Download, Report (abuse/moderation), Collection
InterestSignup (email, interest: blocks-hardware, source, createdAt)
8.3 API Outline
POST   /auth/...                          sign up / sign in
GET    /effects?query&tag&sort            browse / search
POST   /effects                           create
GET    /effects/{id}                      read
PUT    /effects/{id}/draft                save draft
POST   /effects/{id}/publish              request validation and publish a version
GET    /effects/{id}/versions/{v}         immutable version
POST   /effects/{id}/versions/{v}/deploy  create an Artemis deployment package
GET    /effects/{id}/versions/{v}/reports validation reports
POST   /effects/{id}/rate                 rating
POST   /effects/{id}/report               flag content
GET    /users/{id}                        creator profile
POST   /interest/blocks-hardware          join hardware interest list
8.4 Persistence Without an Account
Anonymous users can create and trial effects locally (IndexedDB). Prompt to sign in on publish, share or deploy. Offer export/import of effect files (.blocks.json) so nothing is locked in.

9. Validation and Safety
An effect cannot be deployed to a device until it passes validation for that target. Validation runs server-side; the browser provides early feedback only.
9.1 Checks
Check
Description
Compiles
Faust compiles cleanly with the pinned compiler and library version.
Interface sanity
Parameter ranges are finite, min < max, defaults inside range, names unique.
I/O contract
Channel counts match the target's expectations.
Output safety
Run the DSP against a test corpus (silence, noise, sines, DI guitar, full-scale square waves, parameter sweeps). Fail on NaN/Inf output, sustained DC, runaway feedback or output above a threshold.
Real-time safety
No dynamic allocation or blocking in the generated audio path. Faust-generated C++ is generally allocation-free in compute; verify for the chosen backend and flag exceptions.
CPU load
Benchmark on representative hardware (a Raspberry Pi CM5 in CI) and report worst-case and average DSP time as a percentage of the buffer period at specific sample rates and buffer sizes. Fail above the target's budget.
Memory
Static state size and delay-line/table allocation compared with the target budget.
Determinism
Same input and parameters produce the same output across runs.
Parameter-sweep stability
Rapid parameter changes do not cause instability, large clicks or excessive zipper noise.
Static policy
Blocklist for disallowed constructs (if any), and source size limits.

9.2 Results
Each effect version gets a per-target report with a pass/warn/fail result and metrics, shown in the Studio UI as a simple traffic light plus detail.
Published effects must carry a passing report for every target they advertise.
The hard requirement from the product vision: a community effect must not be able to crash Artemis mid-performance. Validation, resource limits and process isolation on the device (Section 10) all exist to uphold that.
9.3 Sandbox Requirements
The compile and benchmark workers run untrusted code. Use containers or microVMs, no network access, read-only root filesystem, CPU/time/memory quotas, and a fresh environment per job.
Faust source is treated as hostile: limit include paths to approved libraries, cap compile time, and cap generated code size.

10. Deployment to Artemis
This section defines what Studio produces. The device-side contract must be agreed with the Artemis/Houston software and is the biggest open dependency.
10.1 Package
A deployment package contains:
Effect metadata and interface description (from the document).
A compiled artefact for the Artemis target (LV2 bundle via the Faust toolchain, or a native module in Houston's format, to be confirmed).
The validation report and content hash.
A signature (Section 12.2).
10.2 Delivery
Two options to evaluate, in order of preference:
Cloud sync: the user's Artemis account pulls effects from their library, installing them in the background.
Direct transfer: download a package file and install via USB, local network or Artemis's own UI.
10.3 Device Requirements (to be agreed with Artemis)
Effects run isolated from the core audio engine where feasible, or under a watchdog that bypasses a misbehaving effect rather than failing the signal path.
Parameter changes are smoothed on-device.
Presets and MIDI: exposed parameters appear in Artemis's preset system and are MIDI-mappable, using the stable parameter IDs from the effect document.
Installed effects work fully offline.

11. Analytics and Validation Metrics
The point of Phase 1 is to learn whether BLOCKS is worth manufacturing. Instrument the questions Trinity Labs needs answered:
Question
Signal
Are people creating effects?
Effects created, effects compiled, effects that reach audible playback
Are they completing the loop?
Funnel: open editor → first block placed → audio started → saved → published
Do users return?
Weekly active creators, return rate at 7 and 30 days
Are others downloading effects?
Downloads and installs per effect; unique users
Do creators attract followers?
Follows, repeat downloads by creator
Is there genuine hardware demand?
Hardware interest sign-ups, split by Artemis owners vs. non-owners, with an optional price-willingness question
Do non-Artemis owners engage?
Studio usage segmented by device ownership

Privacy requirements: collect the minimum, honour consent in line with UK GDPR / PECR, avoid recording audio, never upload microphone/line input, and document what is collected. Demand for free software does not automatically translate into hardware sales, so keep the hardware interest list distinct from general sign-ups.

12. Community and Future Marketplace Hooks
Phase 1 is free and community-only. The following are designed for, not built:
12.1 Licensing and Entitlements
Each effect version carries a license and an optional pricing block, initially always free.
An entitlement model (user, effect or collection, source: free | purchase | subscription, expiry) is reserved in the schema.
Principle recorded from product planning: purchased effects must work offline, and any future subscription must enhance rather than gate basic use. Entitlement checks must therefore support long offline grace periods.
12.2 Signing and Provenance
Published packages are signed by Trinity Labs' build pipeline after validation. Devices install only signed packages. This is the foundation for later commercial code distribution without allowing arbitrary uploads to compromise devices.
Partner (commercial) effects may be distributed in compiled form only, with source held privately. The format must allow faust.source to be absent and replaced by a compiled artefact.
12.3 Creator Tooling (Later)
Creator profiles, signature collections, revenue reporting and royalty splits (starting negotiation reference from planning: 70/30 in the creator's favour on eligible one-time sales).
A desktop VST cannot be uploaded directly. Partners need a Faust or otherwise portable implementation, so provide partner documentation and a dedicated review path.

13. Non-Functional Requirements
Area
Requirement
Audio latency
Studio should add no more than the browser's inherent latency; target worklet buffer of 128 frames.
Glitch-free
No audible clicks on recompile, parameter change or bypass.
Compile time
Typical simple effect recompiles in under ~1 s on a mid-range laptop (to be measured).
Startup
Interactive editor within ~3 s on broadband; lazy-load compiler and heavy assets.
Reliability
Autosave drafts every few seconds; recover after tab crash.
Security
Content Security Policy; sanitise all user text; sandbox validation; rate limiting; no secrets in the client.
Moderation
Report flow, takedown process, and a human review queue for flagged effects.
Testing
Unit tests for the generator and schema migrations; golden-file tests for generated Faust; audio regression tests (render offline and compare); Playwright end-to-end tests for core flows; CPU benchmark tracking in CI.
Observability
Structured logs, error tracking, compile and validation duration metrics.
Internationalisation
Strings externalised from day one, with UK English as the default.


14. Risks and Open Questions
#
Risk / Question
Mitigation
1
Artemis/Houston compatibility. Whether LV2 (or another route) integrates with Houston is unverified.
First spike: run a Faust-generated effect on a CM5 inside Houston.
2
Real-time safety of community code. A bad effect could glitch or crash a live performance.
Validation gates, device-side isolation and watchdog.
3
Graph ↔ code round-tripping is hard in general.
Graph is authoritative; code mode is best effort with a Code-only fallback.
4
Browser audio quirks. Latency and input processing differ across browsers and OSes.
Clear input constraints, documented limitations, latency display.
5
Faust licensing. Faust-generated code licensing and the libfaust licence need review for both browser distribution and commercial/partner use.
Legal review before launch.
6
Embedded target limits may differ widely from the CM5.
Per-target budgets and block cost hints from the start.
7
Authoring UX for non-programmers. A drag-and-drop editor can still be intimidating.
Starter templates, guided tutorial, forkable community effects.
8
Community cold start. An empty library is unappealing.
Seed with 20–30 first-party effects and templates; invite a few creators early.
9
Demand ≠ hardware sales.
Separate hardware interest list; track non-Artemis owners.
10
NAM in the browser. Previewing NAM models is outside Faust.
Defer; reference as assets only in Phase 1.


15. Phased Delivery Plan
Phase 0: Technical Spikes (2–4 weeks)
Faust to WASM running in an AudioWorklet in a React app with live parameter control.
Faust to LV2 (or alternative) running on a Raspberry Pi CM5, ideally inside Houston.
Benchmark harness for CPU/memory on the CM5.
Prototype the graph → Faust generator on five or six blocks.
Exit criteria: an effect built in a browser makes sound live, and the same effect runs on Artemis hardware.
Phase 1: BLOCKS Studio Alpha
Full editor (graph + code + split), initial block library, audio trialling, local save, export/import.
Backend with accounts, draft saving and private effects.
Validation service for browser-side checks, then server-side.
Deploy to Artemis for a small group of testers.
Phase 2: Public Beta and Community
Publishing, sharing links, browsing, search, ratings, downloads, creator profiles.
Custom reusable blocks, starter templates and tutorials.
Moderation tooling, analytics dashboards and the hardware interest list.
Seed library of first-party effects.
Phase 3: Decision Point
Review the metrics in Section 11. If creation, retention, downloads and hardware demand are strong, proceed to the BLOCKS hardware pedal and the partner/marketplace programme (Section 12). If not, BLOCKS Studio continues as an Artemis feature that adds value to the flagship.

16. Glossary
BLOCKS Studio: the browser-based editor specified here.
Artemis: Trinity Labs' flagship digital guitar modelling product (Raspberry Pi CM5-based).
Houston: Artemis's processing/software platform.
Faust: a functional DSP programming language that compiles to WebAssembly, C++ and other targets.
Effect Document: the portable JSON representation of an effect.
NAM: Neural Amp Modeler, a neural network amp-capture format.
LV2: an open audio plugin standard, one candidate route onto Artemis.
Validation: automated server-side checks that gate deployment.

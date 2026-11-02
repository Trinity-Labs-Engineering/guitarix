# BLOCKS web app style guide

This is the living design guide for the Faust web app in `blocks/`. Extend it as
the interface develops. Record reusable decisions here; keep implementation
progress and milestone history in the existing port documents.

## Design direction

BLOCKS should feel like an instrument: immediately usable, readable while playing,
and comfortable to operate with fingers. The effect and its controls take priority.
Use the available screen for interaction, with compact branding and short guidance.

- Fit the normal effects and audio configuration views within the browser viewport.
- Prefer horizontal browsing when a collection or group of controls needs more room.
- Keep playback controls reachable while editing effects.
- Preserve comfortable touch targets as the viewport shrinks. Reduce decoration,
  rearrange controls, or introduce horizontal pages before shrinking interactions.
- Retain the dark green surfaces, warm pedal artwork, and lime accents.

## App shell

Use the current viewport height (`100dvh`, with a `100vh` fallback). Lay out the
header, workspace, and small diagnostic footer as a bounded app shell. Account for
device safe areas. Workspace children must be able to shrink with `min-height: 0`
and `min-width: 0`.

The top bar is 48–56 px high. Place the small BLOCKS wordmark at the left edge and
**Audio setup** at the right edge, with audio status immediately before it. The
wordmark returns to effects without reloading the app or interrupting playback.
The lab label is secondary and can disappear on narrow screens.

Start the workspace directly below this bar. The current view supplies its own
heading; avoid a separate promotional introduction or large empty top margin.
Keep technical diagnostics and source notices in the compact footer.

## Effects workspace

The effects view has three areas, in this order:

1. **Workbench:** selected pedal, effect name, short description, named parameters,
   bypass, reset, and source link.
2. **Collection:** type filter and search beside a single horizontal strip of effects.
3. **Playback:** input trim, output level, input/output meters, Start/Stop, and Mute.

On wide screens, place artwork beside the named parameters. Artwork scales to the
space available. On portrait phones, place the artwork above the parameters and
use a horizontal parameter strip. Short landscape layouts keep artwork and
parameters side by side and use one horizontal parameter row.

The collection stays horizontal at every size. Keep search stationary beside it,
show the effect count and a directional cue, and retain a visible scrollbar when
the platform provides one. Selection uses a border, surface change, and pressed
state. Search matches names, categories and source paths, combines with the type filter,
and shows an explicit empty result. Filtering preserves the current effect and its
settings. On phones the type selector and search stack in the stationary tools
column; desktop places them side by side. Long collections remain horizontal.

The playback area remains visible independently of horizontal browsing. Routing
selectors belong in Audio setup. Effects retain their values when moving between
the two views.

Long source names can occupy two lines in collection cards. At constrained
heights, keep them to one line and retain the full name in the accessible label
and title. Use one parameter row on short desktop screens. Size parameter columns
to their content so source group names remain readable without covering sliders.
Selected headings truncate on phones and short landscape screens, with the full
name retained in their accessible text and title.

## Audio configuration

Group settings into three ordered cards:

| Card           | Contents                                                                    |
| -------------- | --------------------------------------------------------------------------- |
| Input          | Source, device access, input device, input channel, channel guidance        |
| Output         | Output device, output channels, channel check, capture status, routing help |
| Monitor & play | Input trim, output level, meters, Start/Stop, Mute, monitoring guidance     |

Display cards side by side when space permits. On phones, show one card at a time
and swipe horizontally between them. Provide Input, Output, and Monitor buttons as
an alternative to swiping. In short landscape windows, arrange fields horizontally
inside wider cards. Keep **Continue to effects** outside the scrolling cards so it
is always reachable.

Keep normal configuration controls visible without vertical scrolling. Shorten
repeated prose and put extended explanations in expandable help. Local vertical
scrolling is a fallback for expanded help, unusually long device names or errors,
and constrained accessibility layouts; it must not become the ordinary setup flow.
Do not hide required controls to make a view appear to fit.

Audio begins only after an explicit Start action. Entering or leaving configuration
preserves playback. Device/source changes require stopping audio; channels remain
adjustable while playing. Preserve saved choices and clear unavailable-device
feedback. Channel probing must release temporary capture without playing audio.

## Touch, keyboard, and accessibility

- Use a minimum 44 px height for buttons, selectors, search, and named sliders.
- Keep a full-size named slider or selector available for every artwork control.
  Artwork knobs can become smaller as the pedal scales.
- Use native buttons, range inputs, selects, labels, outputs, and meters. Display
  parameter values and units beside their names.
- Give horizontal regions keyboard focus and use native scrolling. Apply scroll
  snapping only where it helps browsing and leaves controls fully reachable.
- Leave space between parameters for swiping. A horizontal slider gesture changes
  its value; a gesture in the surrounding strip browses parameters.
- Provide visible focus outlines. Focus the view heading when changing views and
  expose selection, mute, bypass, and setup state with the appropriate ARIA state.
- Respect reduced motion for scripted scrolling. Keep audio state readable as text
  as well as a colored indicator.
- Keep errors dismissible and within the app shell. A short error must not push
  Start/Stop or the setup exit below the viewport.

## Visual foundations

| Role                            | Current value            |
| ------------------------------- | ------------------------ |
| App background                  | `#111614`                |
| Panel surface                   | `#19201a`                |
| Main text                       | `#e5e8df`                |
| Secondary text                  | `#a4b19a`                |
| Panel border                    | `#354035`                |
| Accent / focus / live indicator | `#c2f27b`                |
| Selected effect surface         | `#2b3828`                |
| Primary playback action         | `#bcec7e` with dark text |
| Error surface / text            | `#512e24` / `#ffd0b7`    |

Use the system sans-serif stack without remote fonts. Main headings are typically
21–30 px; interactive labels and values are 11–14 px. Reserve smaller text for
secondary metadata. Use tabular numerals for values and meters. Panel radii are
8–10 px, control radii 5–6 px, and usual spacing increments are 4, 8, 12, 16, and
24 px. Keep decorative letter spacing confined to short metadata labels.

## Validation and future changes

Use `blocks/tests/browser/layout.spec.ts` to check actual content geometry, not
just the absence of document scrollbars. Current viewport coverage is 1360×630,
1280×720, 1024×768, 768×1024, 390×844, 360×640, and 844×390 CSS pixels.
Check every shipped effect and named parameter on desktop. Across all seven
viewports, also cover each artwork, type and channel contract, the longest names
and largest parameter panels, initial and checked audio configuration, playback
diagnostics, navigation and horizontal browsing.

When adding UI, identify which area owns it and how it behaves on portrait and
short landscape screens. Extend the horizontal layout before adding vertical
stacks. Recheck touch targets, keyboard reachability, playback access, long text,
and permission errors. Update this guide when a reusable design decision changes;
keep it a description of the app's current conventions.

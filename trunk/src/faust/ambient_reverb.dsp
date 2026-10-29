declare id "ambient_reverb";
declare name "JP Ambient";
declare category "Reverb";
declare author "Julian Parker; Till Bovermann; Artemis integration";
declare license "MIT";

import("stdfaust.lib");

decay = hslider("decay[name:Decay][unit:s]", 4, 0.2, 20, 0.1) : si.smoo : max(0.2);
damping = hslider("damping[name:Damping]", 0.3, 0, 0.95, 0.01) : si.smoo;
size = hslider("size[name:Size]", 1.5, 0.5, 3, 0.05);
depth = hslider("depth[name:Mod Depth]", 0.3, 0, 1, 0.01) : si.smoo;
rate = hslider("rate[name:Mod Rate][unit:Hz]", 0.4, 0.1, 3, 0.05) : si.smoo;
mix = hslider("mix[name:Mix][unit:%]", 30, 0, 100, 1) : *(0.01) : si.smoo;
// JPverb has its own interpolation for size; smoothing its prime-table index is unsafe.
wet = _ <: _, _ : re.jpverb(decay, damping, size, 0.8, depth, rate, 1, 1, 1, 500, 4000)
    : + : *(0.5);
process = _ <: *(1-mix), (wet : *(mix)) : +;

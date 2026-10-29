declare id "greyhole";
declare name "Greyhole";
declare category "Delay";
declare author "Julian Parker; Till Bovermann; Artemis integration";
declare license "MIT";

import("stdfaust.lib");
gh = library("artemis_greyhole.lib");

delay = hslider("delay[name:Delay][unit:ms]", 400, 20, 1500, 1) * 0.001;
feedback = hslider("feedback[name:Feedback]", 0.55, 0, 0.92, 0.01) : si.smoo;
size = hslider("size[name:Size]", 1, 0.5, 3, 0.05);
damping = hslider("damping[name:Damping]", 0.35, 0, 0.95, 0.01) : si.smoo;
modulation = hslider("modulation[name:Modulation]", 0.3, 0, 1, 0.01) : si.smoo;
mix = hslider("mix[name:Mix][unit:%]", 30, 0, 100, 1) * 0.01 : si.smoo;
// Size indexes the upstream prime table; its delay interpolation is internal.
// Preserve both diffusion channels internally and average to a mono wet output.
wet = _ <: _, _ : gh.greyhole(delay, damping, size, 0.65, feedback, modulation, 0.3)
    : + : *(0.5);
process = _ <: *(1-mix), (wet : *(mix)) : +;

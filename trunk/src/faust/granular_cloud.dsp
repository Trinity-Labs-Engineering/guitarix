declare id "granular_cloud";
declare name "Granular Cloud";
declare category "Delay";
declare author "Artemis contributors";
declare license "MIT";

import("stdfaust.lib");
gr = library("artemis_granular.lib");

size = hslider("size[name:Grain Size][unit:ms]", 120, 40, 250, 1) * 0.001 : si.smoo : max(0.04);
pitch = hslider("pitch[name:Pitch][unit:semitones]", 0, -12, 12, 1) : si.smoo;
position = hslider("position[name:Position][unit:ms]", 100, 0, 500, 1) * 0.001 : si.smoo;
scatter = hslider("scatter[name:Scatter][unit:ms]", 40, 0, 200, 1) * 0.001 : si.smoo;
freeze = hslider("freeze[name:Freeze]", 0, 0, 1, 1);
mix = hslider("mix[name:Mix][unit:%]", 50, 0, 100, 1) * 0.01 : si.smoo;
wet = gr.granular(size, pow(2, pitch/12), position, scatter, freeze) : fi.dcblocker;
process = _ <: *(1-mix), (wet : *(mix)) : +;

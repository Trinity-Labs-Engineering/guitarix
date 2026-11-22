declare id   "ring_modulator";
declare name "Ring Modulator Mono";
declare shortname "Ring Modulator";
declare category "Modulation";

import("stdfaust.lib");
mod = library("artemis_modulation.lib");

freq = hslider("freq[name:Freq]", 240, 120, 1600, 0.5) : mod.smooth_control;
wet_dry = hslider("dry/wet[name:Mix]", 0.5, 0, 1, 0.05) : mod.smooth_control;

process = _<:((1 - wet_dry)*_,(_*os.osc(freq))*wet_dry):>_;

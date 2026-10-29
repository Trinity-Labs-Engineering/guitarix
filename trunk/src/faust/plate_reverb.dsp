declare id "plate_reverb";
declare name "Dattorro Plate";
declare category "Reverb";
declare author "Jakob Zerbian; Artemis integration";
declare license "STK-4.3";

import("stdfaust.lib");
plate = library("artemis_reverbs.lib");

decay = hslider("decay[name:Decay]", 0.65, 0.1, 0.95, 0.01) : si.smoo;
damping = hslider("damping[name:Damping]", 0.3, 0, 0.95, 0.01) : si.smoo;
predelay = hslider("predelay[name:Predelay][unit:ms]", 15, 0, 100, 1);
mix = hslider("mix[name:Mix][unit:%]", 25, 0, 100, 1) : *(0.01) : si.smoo;
wet = de.sdelay(32768, 0.02 * ma.SR, predelay * ma.SR * 0.001) : plate.plate_wet(decay, damping);
process = _ <: *(1-mix), (wet : *(mix)) : +;

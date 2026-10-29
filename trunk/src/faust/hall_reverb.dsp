declare id "hall_reverb";
declare name "Zita Hall";
declare category "Reverb";
declare author "Julius O. Smith; Fons Adriaensen; Artemis integration";
declare license "STK-4.3";

import("stdfaust.lib");

decay = hslider("decay[name:Decay][unit:s]", 2.5, 0.2, 10, 0.1) : si.smoo : max(0.2);
bass = hslider("bass[name:Bass Decay]", 1.25, 0.5, 2, 0.05) : si.smoo : max(0.5);
damping = hslider("damping[name:HF Damping][unit:Hz]", 6000, 1500, 16000, 100) : si.smoo : max(1500);
predelay = hslider("predelay[name:Predelay][unit:ms]", 20, 0, 100, 1);
mix = hslider("mix[name:Mix][unit:%]", 25, 0, 100, 1) : *(0.01) : si.smoo;
// Keep pre-delay outside zita's legacy 8192-sample input buffer (100 ms at 192 kHz).
wet = de.sdelay(32768, 0.02 * ma.SR, predelay * ma.SR * 0.001)
    <: _, _ : re.zita_rev1_stereo(0, 200, min(damping, 0.45 * ma.SR), decay*bass, decay, 192000)
    : + : *(0.5);
process = _ <: *(1-mix), (wet : *(mix)) : +;

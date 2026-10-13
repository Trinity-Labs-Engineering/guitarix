declare name "Delay";
declare category "Echo / Delay";

import("stdfaust.lib");
import("guitarix.lib");
msec	= ma.SR/1000.0;
interp  = 100*msec;
N       = int( 2^19);
gain    = vslider("gain[name:Gain]", 0, -20, 20, 0.1) : ba.db2linear : smoothi(0.999);
d       = ba.tempo(hslider("bpm[name:BPM][tooltip:Delay in Beats per Minute]",120,24,360,1));
//d       = vslider("de.delay", 0, 0, 5000, 10)*msec;
// Map the existing gain range to stable feedback (0 dB = 50%).
feedback = gain / (1 + gain);
// The recursive connection contributes one sample to the delay.
repeat = (+ : de.sdelay(N, interp, max(1, d - 1))) ~ *(feedback);
process = _ <: _, (repeat : *(feedback)) :> _;

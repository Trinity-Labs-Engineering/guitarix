declare id "wavefolder_synth";
declare name "Wavefolder Synth";
declare category "Synth";
declare author "David Braun; Dario Sanfilippo; Artemis integration";
declare license "MIT";

import("stdfaust.lib");
wf = library("artemis_wavefold.lib");

fold = hslider("fold[name:Fold][unit:dB]", 18, 0, 36, 0.1) : ba.db2linear : si.smoo;
tone = hslider("tone[name:Cutoff][unit:Hz]", 1200, 100, 8000, 10) : si.smoo;
resonance = hslider("resonance[name:Resonance]", 1, 0.5, 4, 0.05) : si.smoo : max(0.5);
envelope = hslider("envelope[name:Envelope]", 1.5, -3, 3, 0.05) : si.smoo;
mix = hslider("mix[name:Mix][unit:%]", 100, 0, 100, 1) * 0.01 : si.smoo;
level = hslider("level[name:Level][unit:dB]", -6, -24, 6, 0.1) : ba.db2linear : si.smoo;

wet(x) = wf.triangleFold1(0.5, 1, x*fold)
    : fi.SVFTPT.LP2(cutoff, resonance) : fi.dcblocker : *(level)
with {
    env = an.amp_follower_ar(0.005, 0.15, x) : *(4) : min(1);
    cutoff = max(40, min(0.4*ma.SR, tone*pow(2, envelope*env)));
};
// ADAA1 has half a sample of group delay; match the parallel dry path.
process = _ <: ((_,mem) : + : *(0.5*(1-mix))), (wet : *(mix)) : +;

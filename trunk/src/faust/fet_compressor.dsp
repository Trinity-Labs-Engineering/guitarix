declare id "fet_compressor";
declare name "FET 76";
declare category "Guitar Effects";
declare author "Bart Brouns; Artemis integration";
declare license "GPL-3.0-only";
declare description "76-inspired feedback compression and adjustable harmonic colour; behavioural model.";
import("stdfaust.lib");
c = library("artemis_compressors.lib");

input = hslider("input[name:Input][unit:dB]",0,-12,24,0.1) : c.smooth_control : ba.db2linear;
choice = hslider("ratio[name:Ratio]",0,0,3,1) : int : max(0) : min(3);
ratio = ba.selectn(4,choice,4,8,12,20) : c.smooth_control;
attack = hslider("attack[name:Attack][unit:us]",400,20,800,1) : c.smooth_control : *(0.000001);
release = hslider("release[name:Release][unit:ms]",200,50,1100,1) : c.smooth_control : *(0.001);
colour = hslider("colour[name:Colour][unit:%]",35,0,100,1) : c.smooth_control : *(0.01);
output = hslider("output[name:Output][unit:dB]",0,-24,12,0.1) : c.smooth_control : ba.db2linear;
mix = hslider("mix[name:Mix][unit:%]",100,0,100,1) : c.smooth_control : *(0.01);
process(x) = c.blend(mix,c.half_sample(x),wet*output)
with { wet = x*input : c.fet(ratio,attack,release) : c.colour(colour); };

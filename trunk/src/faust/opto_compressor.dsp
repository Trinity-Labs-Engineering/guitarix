declare id "opto_compressor";
declare name "Opto Leveler";
declare category "Guitar Effects";
declare author "Bart Brouns; Artemis integration";
declare license "GPL-3.0-only";
declare description "Optical-inspired leveling with programme-dependent recovery and harmonic colour; behavioural model.";
import("stdfaust.lib");
c = library("artemis_compressors.lib");

threshold = hslider("threshold[name:Threshold][unit:dB]",-24,-48,0,0.1) : c.smooth_control;
ratio = hslider("ratio[name:Ratio]",3,2,10,0.1) : c.smooth_control;
recovery = hslider("recovery[name:Recovery]",1,0.5,2,0.01) : c.smooth_control;
colour = hslider("colour[name:Colour][unit:%]",25,0,100,1) : c.smooth_control : *(0.01);
makeup = hslider("makeup[name:Makeup][unit:dB]",0,-12,24,0.1) : c.smooth_control : ba.db2linear;
mix = hslider("mix[name:Mix][unit:%]",100,0,100,1) : c.smooth_control : *(0.01);
process(x) = c.blend(mix,c.half_sample(x),wet*makeup)
with { wet = c.opto(ratio,threshold,recovery,x) : c.colour(colour); };

declare id "studio_compressor";
declare name "Studio Compressor";
declare category "Guitar Effects";
declare author "Bart Brouns; Artemis integration";
declare license "GPL-3.0-only";
declare description "Transparent feed-forward compression with a soft knee and parallel mix.";
import("stdfaust.lib");
c = library("artemis_compressors.lib");

threshold = hslider("threshold[name:Threshold][unit:dB]",-24,-60,0,0.1) : c.smooth_control;
ratio = hslider("ratio[name:Ratio]",3,1,20,0.1) : c.smooth_control;
knee = hslider("knee[name:Knee][unit:dB]",6,0,24,0.1) : c.smooth_control;
attack = hslider("attack[name:Attack][unit:ms]",10,0.1,100,0.1) : c.smooth_control : *(0.001);
release = hslider("release[name:Release][unit:ms]",150,20,2000,1) : c.smooth_control : *(0.001);
makeup = hslider("makeup[name:Makeup][unit:dB]",0,-12,24,0.1) : c.smooth_control : ba.db2linear;
mix = hslider("mix[name:Mix][unit:%]",100,0,100,1) : c.smooth_control : *(0.01);
process(x) = c.blend(mix,x,c.studio(ratio,threshold,knee,attack,release,x)*makeup);

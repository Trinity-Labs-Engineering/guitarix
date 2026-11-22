declare id "flanger_mono";
declare name "Flanger Mono";
declare category "Modulation";
declare license "BSD";
import("stdfaust.lib");
mod = library("artemis_modulation.lib");
freq = hslider("lfobpm[name:BPM][unit:bpm]", 12, 3, 360, 0.1) : mod.smooth_control : /(60);
depth = hslider("depth[name:Depth]", 0.5, 0, 1, 0.01) : mod.smooth_control;
wet = vslider("wet_dry[name:Dry/Wet]", 100, 0, 100, 1) : /(100) : mod.smooth_control;

// Classic feed-forward comb with a shared, regenerative delay. Depth controls
// excursion rather than also diluting the delayed voice with a second dry mix.
// Feedback has unity DC gain and ample margin below self-oscillation.
curdel = ma.SR * (0.0005 + 0.005 * depth * (1 + os.oscs(freq)));
voice = *(0.5) : (+ : mod.delay(4092, curdel)) ~ *(0.5);
process(x) = mod.blend(wet, x, (x + voice(x)) * 0.5);

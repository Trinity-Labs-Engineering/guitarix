declare id "harmonic_tremolo";
declare name "Harmonic Tremolo";
declare category "Modulation";
declare author "Artemis contributors";
declare license "GPL-3.0-or-later";
import("stdfaust.lib");
mod = library("artemis_modulation.lib");
depth = hslider("depth[name:Depth]", 0.75, 0, 1, 0.01) : mod.smooth_control;
freq = hslider("bpm[name:BPM][unit:bpm]", 240, 6, 360, 0.1) : mod.smooth_control : /(60);
crossover = hslider("crossover[name:Crossover][unit:Hz]", 700, 200, 1600, 1) : mod.smooth_control;
wet = hslider("wet_dry[name:Mix][unit:%%]", 100, 0, 100, 1) : /(100) : mod.smooth_control;
level = hslider("level[name:Level][unit:dB]", 0, -12, 6, 0.1) : mod.smooth_control : ba.db2linear;

// Complementary one-pole bands sum exactly to dry at zero depth. Opposite LFO
// polarities alternate bass and treble. This is a band-split model, not a
// calibrated valve circuit.
harmonic(x) = low * (1-depth*lfo) + (x-low) * (1-depth*(1-lfo))
with {
    low = x : fi.lowpass(1, crossover);
    lfo = (1 + os.oscs(freq)) * 0.5;
};
process(x) = mod.blend(wet, x, harmonic(x)) * level;

declare name "Chorus Mono";
declare category "Modulation";
declare author "Albert Graef; Artemis integration";
declare version "1.1";
import("stdfaust.lib");
mod = library("artemis_modulation.lib");
level = hslider("level[name:Level]", 0.7, 0, 2, 0.01) : mod.smooth_control;
freq = hslider("bpm[name:BPM][unit:bpm]", 120, 6, 360, 0.1) : mod.smooth_control : /(60);
wet = vslider("wet_dry[name:Dry/Wet][tooltip:percentage of processed signal in output signal]", 100, 0, 100, 1) : /(100) : mod.smooth_control;

// Preserve Level = delayed-voice gain and the original parallel dry path.
// The former +/-0.2 ms excursion was barely audible; +/-2 ms gives a useful
// pitch sweep. The smaller delay buffer still covers 12 ms at 192 kHz.
process(x) = x + wet * level * mod.delay(4092, ma.SR * (0.010 + 0.002 * os.oscs(freq)), x);

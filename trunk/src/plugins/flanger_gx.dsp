declare id 		"flanger_mono_gx";
declare name            "Flanger GX";
declare category        "Modulation";

import("stdfaust.lib");
mod = library("../faust/artemis_modulation.lib");

h_flanger(dmax, curdel, fb, wet) = _ <: _, (dly : (+:dly)~*(fb)) : mix with {
    dly = mod.delay(dmax/2, curdel/2);
    mix = *(2-wet), _*(wet) : +;
};

flanger(dmax, curdel, fb, wet) = _ <: _, (+:dly)~*(fb) : mix with {
    dly = mod.delay(dmax, curdel);
    mix = *(2-wet), _*(wet) : +;
};

process = h_flanger(dmax, curdel,  fb, wet1) :
          flanger(  dmax, curdel, -fb, wet2) :
          *(0.25) with {
   // ma.SR = component("math.lib").ma.SR;
    dflange = 0.001 * ma.SR * (hslider("width[name:Width]", 5.0, 0, 10, 0.01) : mod.smooth_control);
    odflange = 0.001 * ma.SR * (hslider("depth[name:Depth]", 0.5, 0, 5, 0.01) : mod.smooth_control);
    dmax = 4092; // includes 15 ms at 192 kHz and interpolation taps
    freq = hslider("bpm[name:BPM][unit:bpm]", 6, 3, 360, 0.1) : mod.smooth_control : /(60);
    // Preserve saved +/-1 endpoints, but bound the actual loop gain to 0.95.
    fb = (hslider("feedback[name:Feedback]", -0.707, -1, 1, 0.01) : max(-0.95) : min(0.95) : mod.smooth_control) * (2+abs(mix))/3;
    lfo = os.oscs;
    curdel = odflange+dflange*(1+lfo(freq))/2;
    //curdel = hslider("de.delay", 20, 0, 400, 0.1);
    wet = hslider("wet[name:Wet]", 100, 0, 100, 1) : /(100) : mod.smooth_control;
    mix = hslider("mix[name:Mix]", 0, -1, 1, 0.1) : mod.smooth_control;
    wet1 = wet*min(1,1+mix);
    wet2 = wet*min(1,1-mix);
};

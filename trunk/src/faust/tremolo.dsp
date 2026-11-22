declare name "Tremolo";
declare category "Modulation";
declare gladefile "tremolo_ui.glade";

/*
** Model of a vactrol tremolo unit by "transmogrify"
** c.f. http://sourceforge.net/apps/phpbb/guitarix/viewtopic.php?f=7&t=44&p=233&hilit=transmogrifox#p233
** http://transmogrifox.webs.com/vactrol.m
*/

import("stdfaust.lib");
mod = library("artemis_modulation.lib");

/* vactrol model */

/* transcendental functions in fast signal path:
** cds: 2 exp() (2 alpha containing each 1 exp())
** vactrol: 1 exp(), 1 pow(), 1 log()
*/

R1 = 2700;
Ra = 1e6;
Rb = 300;
b = exp(log(Ra)/log(Rb)) - exp(1);
dTC = 0.06;
minTC = log(0.005/dTC);

cds = ((_ <: _,_),_ : _+(1-alpha)*_) ~ (_<:*(alpha)) with {
    iSR = 1/ma.SR;
    dRC = dTC * exp(*(minTC));
    alpha = 1 - iSR / (dRC + iSR);
};

vactrol = pow(_,1.9) : cds : *(b) + exp(1) : exp(log(Ra)/log) : R1/(_ + R1);

/* tremolo unit, using triangle or sine oscillator as lfo */

tremolo(freq, depth) = 1-depth + depth*gain with {
    sine(freq) = (os.oscs(freq) + 1) / 2 : max(0); // max(0) because of numerical inaccuracy
    SINE=checkbox("SINE[enum:triangle|sine|square]");
    lfo = select3(SINE, os.lf_trianglepos(freq), sine(freq), os.lf_squarewavepos(freq));
    // Depth belongs after the nonlinear LDR, so half-depth means half the
    // gain excursion. Normalise the physical divider's dark/light gains.
    dark = R1/(Ra+R1);
    light = R1/(Rb+R1);
    gain = (vactrol(lfo)-dark)/(light-dark) : max(0) : min(1);
};

wet = vslider("wet_dry[name:Dry/Wet][tooltip:percentage of processed signal in output signal]",  100, 0, 100, 1) : /(100) : mod.smooth_control;
dry = 1 - wet;

rate = vslider("bpm[name:BPM][unit:bpm]", 240, 6, 360, 0.1) : mod.smooth_control : /(60);
depth = vslider("depth[name:Depth]", 0.75, 0, 1, 0.01) : mod.smooth_control;
process = *(dry + wet * tremolo(rate, depth));

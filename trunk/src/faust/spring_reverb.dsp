declare id "spring_reverb";
declare name "Spring Reverb";
declare category "Reverb";
declare author "Daniel Leonov; Artemis integration";
declare license "MIT";

import("stdfaust.lib");
spring = library("artemis_reverbs.lib");

dwell = hslider("dwell[name:Dwell]", 0.5, 0, 1, 0.01) : si.smoo;
tone = hslider("tone[name:Tone]", 0.5, 0, 1, 0.01) : si.smoo;
tension = hslider("tension[name:Tension]", 0.5, 0, 1, 0.01) : si.smoo;
// These are delay-spacing characters, not a physical number of springs.
character = hslider("character[name:Character][enum:Tight|Wide|Balanced]", 2, 0, 2, 1);
mix = hslider("mix[name:Mix][unit:%]", 30, 0, 100, 1) : *(0.01) : si.smoo;
process = _ <: *(1-mix), (spring.spring_wet(dwell, 1, tone, tension, character) : *(mix)) : +;

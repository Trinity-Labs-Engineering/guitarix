declare id "klon_centaur";
declare name "Klon Centaur";
declare category "Distortion";
declare author "David Braun; Jatin Chowdhury; Artemis integration";
declare license "BSD-3-Clause";

import("stdfaust.lib");
circuit = library("artemis_circuits.lib");

gain = hslider("gain[name:Gain]", 0.35, 0, 1, 0.01);
treble = hslider("treble[name:Treble]", 0.5, 0, 1, 0.01);
level = hslider("level[name:Level]", 0.5, 0, 1, 0.01);
// The model contains its own 50 ms control ramps and 2x diode-stage oversampling.
process = circuit.klonCentaur(gain, treble, level);

declare id "tape_saturation";
declare name "Tape Colour";
declare category "Distortion";
declare author "Thomas Mandolini; Bart Brouns; Artemis integration";
declare license "LGPL-2.1 with Faust exception";

import("stdfaust.lib");
hy = library("artemis_hysteresis.lib");

drive = hslider("drive[name:Drive][unit:dB]", 12, 0, 30, 0.1) : ba.db2linear : si.smoo : max(1);
tone = hslider("tone[name:Tone][unit:Hz]", 6500, 1000, 16000, 10) : si.smoo;
wow = hslider("wow[name:Wow][unit:%]", 10, 0, 100, 1) * 0.01 : si.smoo;
flutter = hslider("flutter[name:Flutter][unit:%]", 5, 0, 100, 1) * 0.01 : si.smoo;
level = hslider("level[name:Level][unit:dB]", -3, -24, 6, 0.1) : ba.db2linear : si.smoo;

// Normalized magnetic core with bounded input and fixed, stable material
// parameters. Drive reaches the nonlinear region without the library UI's
// -50 dB calibration. This models tape colour, not a particular tape machine.
magnetic = *(drive/8) : ma.tanh : *(8)
    : hy.ja_hysteresis(380, 720, 0.015, 380, 0.6)
    : *(720/(0.6*380)/sqrt(drive)) : fi.SVFTPT.HP2(20, 0.7071);
// Fixed 2 ms centre delay, with at most 1.65 ms excursion (never negative).
time = ma.SR*(0.002 + 0.0015*wow*os.osc(0.55) + 0.00015*flutter*os.osc(8.3));
process = magnetic : fi.SVFTPT.LP2(min(0.4*ma.SR, max(1000, tone)), 0.7071)
    : de.fdelay(1024, time) : *(level);

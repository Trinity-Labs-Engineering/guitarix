declare id 		"eqs";
declare name            "Scaleable EQ";
declare category        "Tone Control";
declare license 	"BSD";
declare copyright 	"(c)GRAME 2006";

import("stdfaust.lib");
import("guitarix.lib");

//------------------------- Process --------------------------------

// lower bands (up to 125 Hz) suffer from numerical cancellation
// when using single precision.
// vectorization makes code slower (tested on ARM NEON).

// Map the historic 1..100 bandwidth control onto a useful shelf Q range.
// Q=50 (the existing default) is approximately Butterworth; larger values
// make the terminal transition sharper and smaller values make it gentler.
shelfQ(control) = 0.25 * pow(8.0, (control - 1.0) / 99.0);

lowShelf(level, frequency, control) = fi.TF2(b0/a0, b1/a0, b2/a0, a1/a0, a2/a0)
with {
    A = pow(10.0, level / 40.0);
    w = 2.0 * ma.PI * frequency / ma.SR;
    alpha = sin(w) / (2.0 * shelfQ(control));
    c = cos(w);
    r = 2.0 * sqrt(A) * alpha;
    b0 = A * ((A + 1.0) - (A - 1.0) * c + r);
    b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * c);
    b2 = A * ((A + 1.0) - (A - 1.0) * c - r);
    a0 = (A + 1.0) + (A - 1.0) * c + r;
    a1 = -2.0 * ((A - 1.0) + (A + 1.0) * c);
    a2 = (A + 1.0) + (A - 1.0) * c - r;
};

highShelf(level, frequency, control) = fi.TF2(b0/a0, b1/a0, b2/a0, a1/a0, a2/a0)
with {
    A = pow(10.0, level / 40.0);
    w = 2.0 * ma.PI * frequency / ma.SR;
    alpha = sin(w) / (2.0 * shelfQ(control));
    c = cos(w);
    r = 2.0 * sqrt(A) * alpha;
    b0 = A * ((A + 1.0) + (A - 1.0) * c + r);
    b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * c);
    b2 = A * ((A + 1.0) + (A - 1.0) * c - r);
    a0 = (A + 1.0) - (A - 1.0) * c + r;
    a1 = 2.0 * ((A - 1.0) - (A + 1.0) * c);
    a2 = (A + 1.0) - (A - 1.0) * c - r;
};

process =   lowShelf(vslider("fs31_25[tooltip:gain below the corner (dB)]", 0, -40, 30, 0.1)
                       : smoothi(0.999),
                     vslider("freq31_25 [tooltip:corner frequency (Hz)]",31.25, 20, 20000, 1),
                     vslider("Qs31_25 [tooltip:transition Q]", 50, 1, 100, 1))

          : ifilter(vslider("Qs62_5 [tooltip:bandwidth]", 50, 1, 100, 1),
                    vslider("freq62_5 [tooltip:Hz]",62.5, 20, 20000, 1),
                    vslider("fs62_5 [tooltip:gain (dB) at 62.5 Hz]", 0, -40, 30, 0.1)
                      : ba.db2linear : smoothi(0.999))

          : ifilter(vslider("Qs125 [tooltip:bandwidth]", 50, 1, 100, 1),
                    vslider("freq125 [tooltip:Hz]",125., 20, 20000, 1),
                    vslider("fs125  [tooltip:gain (dB) at 125 Hz]", 0, -40, 30, 0.1)
                      : ba.db2linear : smoothi(0.999))

          : ifilter(vslider("Qs250 [tooltip:bandwidth]", 50, 1, 100, 1),
                    vslider("freq250 [tooltip:Hz]",250., 20, 20000, 1),
                    vslider("fs250  [tooltip:gain (dB) at 250 Hz]", 0, -40, 30, 0.1)
                      : ba.db2linear : smoothi(0.999))

          : ifilter(vslider("Qs500 [tooltip:bandwidth]", 50, 1, 100, 1),
                    vslider("freq500 [tooltip:Hz]",500., 20, 20000, 1),
                    vslider("fs500  [tooltip:gain (dB) at 500 Hz]", 0, -40, 30, 0.1)
                      : ba.db2linear : smoothi(0.999))

          : ifilter(vslider("Qs1k [tooltip:bandwidth]", 50, 1, 100, 1),
                    vslider("freq1k [tooltip:Hz]",1000., 20, 20000, 1),
                    vslider("fs1k   [tooltip:gain (dB) at 1 kHz]", 0, -40, 30, 0.1)
                      : ba.db2linear : smoothi(0.999))

          : ifilter(vslider("Qs2k [tooltip:bandwidth]", 50, 1, 100, 1),
                    vslider("freq2k [tooltip:Hz]",2000., 20, 20000, 1),
                    vslider("fs2k   [tooltip:gain (dB) at 2 kHz]", 0, -40, 30, 0.1)
                      : ba.db2linear : smoothi(0.999))

          : ifilter(vslider("Qs4k [tooltip:bandwidth", 50, 1, 100, 1),
                    vslider("freq4k [tooltip:Hz]",4000., 20, 20000, 1),
                    vslider("fs4k   [tooltip:gain (dB) at 4 kHz]", 0, -40, 30, 0.1)
                      : ba.db2linear : smoothi(0.999))

          : ifilter(vslider("Qs8k [tooltip:bandwidth", 50, 1, 100, 1),
                    vslider("freq8k [tooltip:Hz]",8000., 20, 20000, 1),
                    vslider("fs8k   [tooltip:gain (dB) at 8 kHz]", 0, -40, 30, 0.1)
                      : ba.db2linear : smoothi(0.999))

          : highShelf(vslider("fs16k  [tooltip:gain above the corner (dB)]", 0, -40, 30, 0.1)
                        : smoothi(0.999),
                      vslider("freq16k [tooltip:corner frequency (Hz)]",16000., 20, 20000, 1),
                      vslider("Qs16k [tooltip:transition Q]", 50, 1, 100, 1))
          ;

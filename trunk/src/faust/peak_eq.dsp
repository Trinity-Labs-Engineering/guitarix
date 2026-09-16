declare id 		"eq";
declare name            "Peak EQ";
declare category        "Tone Control";
declare license 	"BSD";
declare copyright 	"(c)GRAME 2006";

import("stdfaust.lib");
import("guitarix.lib");

//------------------------- Process --------------------------------
// The terminal bands are shelves rather than bells.  This keeps 0 dB flat
// (and therefore preserves existing presets) while allowing the end controls
// to attenuate everything beyond their corner frequency.  The existing
// bandwidth control sets the shelf transition Q.

shelfQFromBandwidth(frequency, bandwidth) =
    min(2.0, max(0.25, sqrt(frequency / max(5.0, bandwidth)) / 2.0));

lowShelf(level, frequency, bandwidth) = fi.TF2(b0/a0, b1/a0, b2/a0, a1/a0, a2/a0)
with {
    A = pow(10.0, level / 40.0);
    w = 2.0 * ma.PI * frequency / ma.SR;
    alpha = sin(w) / (2.0 * shelfQFromBandwidth(frequency, bandwidth));
    c = cos(w);
    r = 2.0 * sqrt(A) * alpha;
    b0 = A * ((A + 1.0) - (A - 1.0) * c + r);
    b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * c);
    b2 = A * ((A + 1.0) - (A - 1.0) * c - r);
    a0 = (A + 1.0) + (A - 1.0) * c + r;
    a1 = -2.0 * ((A - 1.0) + (A + 1.0) * c);
    a2 = (A + 1.0) + (A - 1.0) * c - r;
};

highShelf(level, frequency, bandwidth) = fi.TF2(b0/a0, b1/a0, b2/a0, a1/a0, a2/a0)
with {
    A = pow(10.0, level / 40.0);
    w = 2.0 * ma.PI * frequency / ma.SR;
    alpha = sin(w) / (2.0 * shelfQFromBandwidth(frequency, bandwidth));
    c = cos(w);
    r = 2.0 * sqrt(A) * alpha;
    b0 = A * ((A + 1.0) + (A - 1.0) * c + r);
    b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * c);
    b2 = A * ((A + 1.0) + (A - 1.0) * c - r);
    a0 = (A + 1.0) - (A - 1.0) * c + r;
    a1 = 2.0 * ((A - 1.0) - (A + 1.0) * c);
    a2 = (A + 1.0) - (A - 1.0) * c - r;
};

process =   lowShelf(vslider("level1 [name:Low cut][tooltip:gain below the corner (dB)]", 0, -50, 50, 0.1),vslider("peak1 [tooltip:corner frequency (hz)][log]", 110, 20, 22000, 1.01),vslider("bandwidth1 [name:Q][tooltip:transition bandwidth (hz)][log]", 41, 5, 20000, 1.01))
          : fi.peak_eq(vslider("level2 [name:Low][tooltip:gain (dB)]", 0, -50, 50, 0.1),vslider("peak2 [tooltip:frequency (hz)][log]", 440, 20, 22000, 1.01),vslider("bandwidth2 [name:Q][tooltip:bandwidth (hz)][log]", 220, 5, 20000, 1.01))
          : fi.peak_eq(vslider("level3 [name:Mid][tooltip:gain (dB)]", 0, -50, 50, 0.1),vslider("peak3 [tooltip:frequency (hz)][log]", 1760, 20, 22000, 1.01),vslider("bandwidth3 [name:Q][tooltip:bandwidth (hz)][log]", 880, 5, 20000, 1.01))
          : highShelf(vslider("level4 [name:High cut][tooltip:gain above the corner (dB)]", 0, -50, 50, 0.1),vslider("peak4 [tooltip:corner frequency (hz)][log]", 3520, 20, 22000, 1.01),vslider("bandwidth4 [name:Q][tooltip:transition bandwidth (hz)][log]", 1760, 5, 20000, 1.01))
          ;

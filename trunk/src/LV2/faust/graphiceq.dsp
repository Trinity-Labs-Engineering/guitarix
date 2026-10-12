declare id "graphiceq";
declare name "Graphic EQ";
declare category "Tone Control";
declare description "Graphic EQ";

import("stdfaust.lib");
rd = library("reducemaps.lib");

//geq = fi.filterbank(3, (31.25, 62.5, 125., 250., 500., 1000., 2000., 4000., 8000., 16000.));
geq = fi.filterbank(3, (44., 88., 177., 354., 707., 1414., 2828., 5657., 11384., 18110.));

// Smooth in dB so the zero-initialized smoother is neutral at startup.
g1db = vslider("g1[tooltip:gain (dB) below 31.25 Hz]", 0, -60, 5.2, 0.1);  g1 = g1db : si.smooth(0.999) : ba.db2linear;
g2db = vslider("g2 [tooltip:gain (dB) at 62.5 Hz]", 0, -30, 5.2, 0.1);     g2 = g2db : si.smooth(0.999) : ba.db2linear;
g3db = vslider("g3  [tooltip:gain (dB) at 125 Hz]", 0, -30, 5.2, 0.1);     g3 = g3db : si.smooth(0.999) : ba.db2linear;
g4db = vslider("g4  [tooltip:gain (dB) at 250 Hz]", 0, -30, 5.2, 0.1);     g4 = g4db : si.smooth(0.999) : ba.db2linear;
g5db = vslider("g5  [tooltip:gain (dB) at 500 Hz]", 0, -30, 5.2, 0.1);     g5 = g5db : si.smooth(0.999) : ba.db2linear;
g6db = vslider("g6   [tooltip:gain (dB) at 1 kHz]", 0, -30, 5.2, 0.1);     g6 = g6db : si.smooth(0.999) : ba.db2linear;
g7db = vslider("g7   [tooltip:gain (dB) at 2 kHz]", 0, -30, 5.2, 0.1);     g7 = g7db : si.smooth(0.999) : ba.db2linear;
g8db = vslider("g8   [tooltip:gain (dB) at 4 kHz]", 0, -30, 5.2, 0.1);     g8 = g8db : si.smooth(0.999) : ba.db2linear;
g9db = vslider("g9   [tooltip:gain (dB) at 8 kHz]", 0, -30, 5.2, 0.1);     g9 = g9db : si.smooth(0.999) : ba.db2linear;
g10db = vslider("g10  [tooltip:gain (dB) at 16 kHz]", 0, -30, 5.2, 0.1);  g10 = g10db : si.smooth(0.999) : ba.db2linear;
g11db = vslider("g11  [tooltip:gain (dB) above 16 kHz]", 0, -30, 5.2, 0.1); g11 = g11db : si.smooth(0.999) : ba.db2linear;

v1(x)		= attach(x, envelop(x) : vbargraph("v1[nomidi:no]", -70, +5));
v2(x)		= attach(x, envelop(x) : vbargraph("v2[nomidi:no]", -70, +5));
v3(x)		= attach(x, envelop(x) : vbargraph("v3[nomidi:no]", -70, +5));
v4(x)		= attach(x, envelop(x) : vbargraph("v4[nomidi:no]", -70, +5));
v5(x)		= attach(x, envelop(x) : vbargraph("v5[nomidi:no]", -70, +5));

v6(x)		= attach(x, envelop(x) : vbargraph("v6[nomidi:no]", -70, +5));
v7(x)		= attach(x, envelop(x) : vbargraph("v7[nomidi:no]", -70, +5));
v8(x)		= attach(x, envelop(x) : vbargraph("v8[nomidi:no]", -70, +5));
v9(x)		= attach(x, envelop(x) : vbargraph("v9[nomidi:no]", -70, +5));
v10(x)		= attach(x, envelop(x) : vbargraph("v10[nomidi:no]", -70, +5));
v11(x)		= attach(x, envelop(x) : vbargraph("v11[nomidi:no]", -70, +5));

envelop         = abs : max ~ (1.0/ma.SR) : rd.maxn(4096) ;

flat = (g1db == 0) & (g2db == 0) & (g3db == 0) & (g4db == 0) & (g5db == 0) &
       (g6db == 0) & (g7db == 0) & (g8db == 0) & (g9db == 0) & (g10db == 0) & (g11db == 0);

equalized = geq :(*(g11):v11), (*(g10):v10),(*(g9):v9),(*(g8):v8),(*(g7):v7),(*(g6):v6),
                   (*(g5):v5),(*(g4):v4),(*(g3):v3),(*(g2):v2),(*(g1):v1) :>_;

process = _ <: *(flat), (equalized : *(1-flat)) :> _;

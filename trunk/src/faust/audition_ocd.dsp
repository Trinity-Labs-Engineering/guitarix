declare id "audition_ocd";
declare name "OCD Drive";
declare category "Audition";
declare author "David Braun; Artemis integration";
declare license "MIT";
import("stdfaust.lib");
ocd = library("artemis_ocd.lib");
drive = hslider("drive[name:Drive]",0.4,0,1,0.01):si.smoo;
tone = hslider("tone[name:Tone]",0.5,0,1,0.01):si.smoo;
level = hslider("level[name:Level][unit:dB]",0,-24,12,0.1):si.smoo:ba.db2linear;
hp = hslider("hp[name:HP Mode]",1,0,1,1):si.smoo;
// The upstream ideal-op-amp model has substantial output gain. Scale its output
// for audition at guitar levels; retain the documented circuit nonlinearity.
process = ocd.fulltoneOCD(drive,tone,0.035*level,hp);

declare id   "phaser_mono";
declare name "Phaser Mono";
declare category "Modulation";

import("stdfaust.lib");

phaser_monogx = component("phaser.dsp").phaser_mono(Notches,0,width,frqmin,fratio,frqmax,freq,mdepth,fb,invert)
with {
  Notches = 4;
  //freq  = hslider("Speed [unit:Hz] ", 0.5, 0, 10, 0.01);
  freq     = hslider("lfobpm[name:Speed (bpm)][tooltip:Speed in Beats per Minute]",30,24,360,1)/60;
  
  depth = hslider("depth[name:Depth]", 0.5, 0, 1, 0.01);
  fb	 = 0.5 ;//hslider("feedback gain", 0, 0, 1, 0.01);
  width  = 1000 ;//hslider("Notch width [unit:Hz]", 1000, 10, 5000, 1);
  vibr   = 1 ;//checkbox("VibratoMode[enum:direct | vibrato]");
  frqmin = 100 ;//hslider("MinNotch1Freq [unit:Hz] ", 100, 20, 5000, 1);
  frqmax = 800: max(frqmin) ;//hslider("MaxNotch1Freq [unit:Hz] ", 800, 20, 10000, 1) : max(frqmin);
  fratio = 2.0 ;//hslider("NotchFreq", 1.5, 1.1, 4, 0.01);
  mdepth = depth; // 0 = dry, 1 = full phaser (not vibrato)
  
  invert   = 1 ;//checkbox("invert[enum:linear|invert]");

  
};

wet = vslider("wet_dry[name:Dry/Wet][tooltip:percentage of processed signal in output signal]",  100, 0, 100, 1) : /(100);
dry = 1 - wet;

process =  _<:*(dry),(*(wet): phaser_monogx ):>_;

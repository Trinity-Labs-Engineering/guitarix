// Copyright (C) 2026 Trinity Labs Engineering
// SPDX-License-Identifier: GPL-3.0-or-later
#include "audition/byod_drives.h"
#include "audition/muff.h"
#include "audition/fuzz_machine.h"
#include "audition/gx_fuzz.h"
#include "audition/transistor_fuzz.h"

namespace pluginlib { namespace audition_effects {
using audition::Control;
static const value_pair octave[]={{"0","Off"},{"1","On"},{nullptr,nullptr}};
static const value_pair superTone[]={{"0","Full"},{"1","Scooped"},{nullptr,nullptr}};
static const value_pair neuralModes[]={{"0","1.5"},{"1","2"},{nullptr,nullptr}};
static const value_pair clipping[]={{"0","Germanium"},{"1","Open"},{"2","Silicon"},{nullptr,nullptr}};
static const value_pair percolatorVersions[]={{"0","Stock"},{"1","Alternate"},{nullptr,nullptr}};
static const value_pair kingModes[]={{"0","Distortion"},{"1","Overdrive"},{"2","Both"},{nullptr,nullptr}};
static const value_pair cakeModes[]={{"0","Fizzy"},{"1","Bluesberry"},{"2","Peachy"},{nullptr,nullptr}};
static const value_pair muffVoices[]={{"0","Triangle"},{"1","Ram's Head 73"},{"2","Ram's Head 75"},
    {"3","Russian"},{"4","Hoof"},{"5","AMZ 1"},{"6","AMZ 2"},{"7","Supercollider"},
    {"8","Flat Mids"},{"9","Bump Mids"},{nullptr,nullptr}};

PluginDef* tone_machine() {
    static const std::array<Control,4> controls{{
        {"audition_tone_machine.sustain","Sustain",0.5,0,1,0.01},
        {"audition_tone_machine.tone","Tone",0.5,0,1,0.01},
        {"audition_tone_machine.level","Level",0.3,0,1,0.01},
        {"audition_tone_machine.octave","Octave",1,0,1,1,"Upper octave",octave}}};
    return new audition::Pedal<audition::ToneMachine,4>("audition_tone_machine","Tone Machine","FOXX-inspired octave fuzz; adapted GxToneMachine filters with explicit rectification",controls);
}
PluginDef* super_fuzz() {
    static const std::array<Control,4> controls{{
        {"audition_super_fuzz.expander","Expander",0.5,0,1,0.01},
        {"audition_super_fuzz.level","Level",0.3,0,1,0.01},
        {"audition_super_fuzz.tone","Tone",0,0,1,1,"Full / Scooped",superTone},
        {"audition_super_fuzz.trim","Trim",0.25,0.01,0.49,0.01}}};
    return new audition::Pedal<audition::SuperFuzz,4>("audition_super_fuzz","Super-Fuzz","Univox-inspired octave fuzz; adapted GxSuperFuzz circuit filters and clipping",controls);
}
PluginDef* muff() {
    static const std::array<Control,8> controls{{
        {"audition_muff.sustain","Sustain",0.5,0,1,0.01},
        {"audition_muff.tone","Tone",0.5,0,1,0.01},
        {"audition_muff.mids","Mids",0.5,0,1,0.01},
        {"audition_muff.stages","Stages",2,1,4,1},
        {"audition_muff.voicing","Voicing",2,0,9,1,"Tone-network variants, not complete pedal models",muffVoices},
        {"audition_muff.harmonics","Harmonics",1200,500,22000,10,"Input low-pass frequency (Hz)"},
        {"audition_muff.smoothing","Smoothing",0,-1,1,0.01},
        {"audition_muff.level","Level",-12,-30,6,0.1,"Output trim (dB)"}}};
    return new audition::Pedal<audition::Muff,8>("audition_muff","Muff Laboratory","BYOD Muff drive stages and selectable tone networks with mids control",controls);
}
PluginDef* fuzz_machine() {
    static const std::array<Control,3> controls{{
        {"audition_fuzz_machine.fuzz","Fuzz",0.5,0,1,0.01},
        {"audition_fuzz_machine.mode","Mode",1,0,1,1,"Custom pedal model mode",neuralModes},
        {"audition_fuzz_machine.level","Level",0.5,0,1,0.01}}};
    return new audition::Pedal<audition::FuzzMachine,3>("audition_fuzz_machine","Fuzz Machine","BYOD's conditioned neural model of a custom fuzz pedal",controls);
}
PluginDef* percolator() {
    static const std::array<Control,4> controls{{
        {"audition_percolator.harmonics","Harmonics",0.7,0,1,0.01},
        {"audition_percolator.balance","Balance",0.5,0,1,0.01},
        {"audition_percolator.clipping","Clipping",0,0,2,1,"Germanium / Open / Silicon",clipping},
        {"audition_percolator.version","Version",1,0,1,1,"Circuit component set",percolatorVersions}}};
    return new audition::Pedal<audition::Percolator,4>("audition_percolator","Harmonic Percolator","New mixed germanium/silicon circuit audition model; not hardware calibrated",controls);
}
PluginDef* bias_fuzz() {
    static const std::array<Control,5> controls{{
        {"audition_bias_fuzz.drive","Drive",0.7,0,1,0.01},
        {"audition_bias_fuzz.gate","Gate",0.2,0,1,0.01},
        {"audition_bias_fuzz.compression","Compression",0.3,0,1,0.01},
        {"audition_bias_fuzz.stability","Stability",1,0,1,0.01},
        {"audition_bias_fuzz.level","Level",0.4,0,1,0.01}}};
    return new audition::Pedal<audition::BiasFuzz,5>("audition_bias_fuzz","Bias Fuzz","Experimental two-transistor fuzz with bias, supply and feedback interaction; Fuzz Factory-inspired",controls);
}
PluginDef* mouse() {
    static const std::array<Control,3> controls{{
        {"audition_mouse.distortion","Distortion",0.5,0,1,0.01},
        {"audition_mouse.filter","Filter",0.4,0,1,0.01,"Clockwise rolls off treble"},
        {"audition_mouse.level","Level",-12,-24,12,0.1,"Output trim (dB)"}}};
    return new audition::Pedal<audition::Mouse,3>("audition_mouse","Mouse Drive","BYOD RAT-style WDF clipping core with Filter and output trim",controls);
}
PluginDef* tone_king() {
    static const std::array<Control,4> controls{{
        {"audition_tone_king.drive","Drive",0.5,0,1,0.01},
        {"audition_tone_king.tone","Tone",0.6,0,1,0.01},
        {"audition_tone_king.level","Level",6,-24,12,0.1,"Output trim (dB)"},
        {"audition_tone_king.mode","Mode",1,0,2,1,"Distortion / Overdrive / Both",kingModes}}};
    return new audition::Pedal<audition::ToneKing,4>("audition_tone_king","Tone King","BYOD King of Tone drive stages with audition tone filter and output trim",controls);
}
PluginDef* flapjack() {
    static const std::array<Control,5> controls{{
        {"audition_flapjack.drive","Drive",0.75,0,1,0.01},
        {"audition_flapjack.presence","Presence",0.5,0,1,0.01},
        {"audition_flapjack.low_cut","Low Cut",100,20,750,1,"Low cut frequency (Hz)"},
        {"audition_flapjack.level","Level",-14,-24,12,0.1,"Output trim (dB)"},
        {"audition_flapjack.mode","Mode",1,0,2,1,"Fizzy / Bluesberry / Peachy",cakeModes}}};
    return new audition::Pedal<audition::Flapjack,5>("audition_flapjack","Flapjack","BYOD Hot Cake circuit model with three clipping modes",controls);
}
PluginDef* yen() {
    static const std::array<Control,4> controls{{
        {"audition_yen.gain","Gain",0.5,0,1,0.01},
        {"audition_yen.voice","Voice",0.5,0,1,0.01},
        {"audition_yen.tone","Tone",0.6,0,1,0.01},
        {"audition_yen.level","Level",-6,-24,12,0.1,"Output trim (dB)"}}};
    return new audition::Pedal<audition::Yen,4>("audition_yen","Yen Drive","BYOD Zendrive WDF core with audition tone filter and output trim",controls);
}
} } // namespace pluginlib::audition_effects

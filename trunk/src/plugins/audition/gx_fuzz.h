// Guitarix/MOD octave fuzz adapters, GPL-3.0; see SOURCES.md.
#pragma once
#include "common.h"
namespace audition { namespace legacy {
#define FAUSTFLOAT float
#define N_(s) s
#define always_inline inline
#define min(x,y) (((x)<(y))?(x):(y))
#define max(x,y) (((x)>(y))?(x):(y))
template<int N,class T> inline T faustpower(T x) {return std::pow(x,N);}
#include "vendor/gx/gx_pluginlv2.h"
enum PortIndex {SUSTAIN,TONE,VOLUME,BALANCE,EXPANDER,TRIM};
#include "vendor/gx/foxxtonemachine_on.cc"
#include "vendor/gx/foxxtonemachine_off.cc"
#include "vendor/gx/foxxtonemachine.cc"
#include "vendor/gx/sfp.cc"
#undef min
#undef max
#undef always_inline
#undef N_
#undef FAUSTFLOAT
} // namespace legacy

class ToneMachine {
    legacy::foxxtonemachine_on::Dsp on;
    legacy::foxxtonemachine_off::Dsp off;
    legacy::foxxtonemachine::Dsp post;
    RateBridge bridge;
    float sustain=0.5,tone=0.5,level=0.5;
    double fs=48000,mix=1;
    int octave=1;
public:
    void configure(const Params& p) {sustain=p[0];tone=p[1];level=p[2];octave=int(p[3]);}
    void prepare(double rate,const Params& p) {
        fs=rate;while(fs>48000)fs/=2;bridge.prepare(rate,fs);configure(p);
        on.connect_ports(legacy::SUSTAIN,&sustain,&on);off.connect_ports(legacy::SUSTAIN,&sustain,&off);
        post.connect_ports(legacy::TONE,&tone,&post);post.connect_ports(legacy::VOLUME,&level,&post);
        on.set_samplerate(fs,&on);off.set_samplerate(fs,&off);post.set_samplerate(fs,&post);reset();
    }
    void reset() {on.clear_state(&on);off.clear_state(&off);post.clear_state(&post);bridge.reset();mix=octave;}
    double tick(double x) {
        return bridge.tick(x,[this](double v){
            float a=v,b=v;
            off.mono_audio(1,&a,&a,&off);on.mono_audio(1,&b,&b,&on);
            // The old generated preamp is linear in both positions. Add the
            // octave rectifier explicitly; changing only its EQ cannot double pitch.
            b=std::sqrt(double(b)*b+1e-8)-1e-4;
            mix+=(octave-mix)*(1-std::exp(-1/(0.01*fs)));
            float y=a+(b-a)*mix;post.mono_audio(1,&y,&y,&post);return double(y);
        });
    }
};
class SuperFuzz {
    legacy::sfp::Dsp core;
    RateBridge bridge;
    float expander=0.5,balance=0.3,tone=0,trim=0.25;
public:
    void configure(const Params& p) {expander=p[0];balance=p[1];tone=p[2];trim=p[3];}
    void prepare(double rate,const Params& p) {
        double fs=rate;while(fs>48000)fs/=2;bridge.prepare(rate,fs);configure(p);
        core.connect_ports(legacy::EXPANDER,&expander,&core);core.connect_ports(legacy::BALANCE,&balance,&core);
        core.connect_ports(legacy::TONE,&tone,&core);core.connect_ports(legacy::TRIM,&trim,&core);
        core.set_samplerate(fs,&core);reset();
    }
    void reset() {core.clear_state(&core);bridge.reset();}
    double tick(double x) {return bridge.tick(x,[this](double v){float y=v;core.mono_audio(1,&y,&y,&core);return 10.0*double(y);});}
};
} // namespace audition

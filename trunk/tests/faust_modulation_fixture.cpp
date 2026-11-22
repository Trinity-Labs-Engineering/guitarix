// Offline measurements of the actual registered DSP, including the 32-frame
// production block size. No JACK server, GUI, or synthetic DSP substitute.
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <new>
#include <string>
#include <vector>
#include "gx_plugin.h"
#include "plugins/generated/flanger_gx.cc"
namespace effects {
#include "faust-generated/chorus_mono.cc"
#include "faust-generated/flanger_mono.cc"
#include "faust-generated/tremolo.cc"
#include "faust-generated/phaser_mono.cc"
#include "faust-generated/phase90.cc"
#include "faust-generated/harmonic_tremolo.cc"
#include "faust-generated/ring_modulator.cc"
}
#undef min
#undef max

static bool audio_running = false;
void* operator new(std::size_t size) {
    assert(!audio_running && "allocation in audio callback");
    if (auto* p = std::malloc(size ? size : 1)) return p;
    throw std::bad_alloc();
}
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }

struct Control { float* value; float initial, low, high, step; };
using Controls = std::map<std::string, Control>;
static Controls controls;
static float* register_float(const char* id, const char*, const char*, const char*, float* value,
    float initial, float low, float high, float step, const value_pair*) {
    *value = initial;
    controls[id] = {value, initial, low, high, step};
    return value;
}
static void prepare(PluginDef* p, int rate) {
    controls.clear();
    ParamReg reg{p, register_float, nullptr, nullptr};
    p->register_params(reg);
    p->set_samplerate(rate, p);
    assert(p->mono_audio && !p->stereo_audio && p->clear_state);
    if (p->activate_plugin) p->activate_plugin(true,p);
    audio_running = true;
    p->clear_state(p);
    audio_running = false;
}
static void set(PluginDef* p, const std::string& key, float value) {
    *controls.at(std::string(p->id)+"."+key).value = value;
}
static void defaults(PluginDef* p) {
    for (const auto& [id,c] : controls) *c.value = c.initial;
    audio_running = true;
    p->clear_state(p);
    audio_running = false;
}
static void callback(PluginDef* p, int n, float* in, float* out) {
    audio_running = true;
    p->mono_audio(n,in,out,p);
    audio_running = false;
    for (int i=0; i<n; ++i) assert(std::isfinite(out[i]) && std::abs(out[i]) < 64);
}
static std::vector<float> render(PluginDef* p, const std::vector<float>& input, int block=32, bool inplace=false) {
    auto output = inplace ? input : std::vector<float>(input.size());
    for (size_t i=0; i<input.size(); i+=block) {
        const int n = std::min<size_t>(block,input.size()-i);
        callback(p,n,inplace ? output.data()+i : const_cast<float*>(input.data()+i),output.data()+i);
    }
    return output;
}
static std::vector<float> tone(int rate, int count, double amplitude=.25, double frequency=220) {
    std::vector<float> x(count);
    for (int i=0; i<count; ++i) x[i] = amplitude*std::sin(2*M_PI*frequency*i/rate);
    return x;
}
static double dc(PluginDef* p, int count, float value) {
    return render(p,std::vector<float>(count,value)).back();
}
static double db(double value) { return 20*std::log10(std::max(1e-15,std::abs(value))); }

static double rms(const std::vector<float>& x, int start=0) {
    double power=0; for (size_t i=start;i<x.size();++i) power+=double(x[i])*x[i];
    return std::sqrt(power/(x.size()-start));
}
static std::string mix_key(PluginDef* p) {
    std::string id=p->id;
    return id=="phase90" ? "" : id=="flanger_mono_gx" ? "wet" : id=="ring_modulator" ? "dry/wet" : "wet_dry";
}
static void processing(PluginDef* p,int rate) {
    const auto input=tone(rate,rate/3+19);
    auto expected=render(p,input);
    for (int block:{1,8,37,64,128,256}) {
        p->clear_state(p);
        auto actual=render(p,input,block,true);
        for(size_t i=0;i<input.size();++i) assert(std::abs(actual[i]-expected[i])<1e-6);
    }
    // Dry is exact from the first sample after a resident reset, not a fade
    // from the smoother's zero state. This also catches stale delay history.
    const auto mix=mix_key(p);
    if (!mix.empty()) {
        set(p,mix,0); p->clear_state(p);
        auto dry=render(p,input);
        for(size_t i=0;i<input.size();++i) assert(std::abs(dry[i]-input[i])<1e-6);
    }
    for(const auto& [id,c]:controls) {
        defaults(p);
        for(float v:{c.low,c.high}) { *c.value=v; render(p,input); }
    }
    for(bool high:{false,true}) {
        for(const auto& [id,c]:controls) *c.value=high ? c.high : c.low;
        render(p,tone(rate,rate*2));
        audio_running=true; p->clear_state(p); audio_running=false;
        assert(rms(render(p,std::vector<float>(rate/4)))<1e-12);
    }
    defaults(p);
    // Exercise all Plus feedback polarities at the shortest and longest delay,
    // including the old saved +/-1 endpoint, for enough time to expose runaway.
    if(std::string(p->id)=="flanger_mono_gx") {
        for(float feedback:{-1.f,1.f}) for(float depth:{0.f,5.f}) for(float width:{0.f,10.f}) for(float mix:{-1.f,0.f,1.f}) {
            set(p,"feedback",feedback); set(p,"depth",depth); set(p,"width",width); set(p,"mix",mix);
            auto out=render(p,tone(rate,rate));
            assert(rms(out)<2); // headroom on a 0.25-peak signal
        }
    }
    defaults(p);
}
static void tremolo(PluginDef* p,int rate) {
    if(std::string(p->id)!="tremolo") return;
    for(float bpm:{60.f,240.f,360.f}) {
        set(p,"bpm",bpm); set(p,"depth",1); p->clear_state(p);
        auto y=render(p,std::vector<float>(rate*4,.25));
        auto lo=*std::min_element(y.begin()+rate,y.end())/.25;
        auto hi=*std::max_element(y.begin()+rate,y.end())/.25;
        std::cout<<"tremolo "<<rate<<" Hz "<<bpm<<" BPM gain "<<lo<<".."<<hi<<'\n';
        assert(hi-lo>.60 && hi>.85 && lo<.30);
        // After settling, 50% depth is halfway between unity and the measured
        // full-depth waveform, not halfway along an exponential resistance.
        set(p,"depth",.5); p->clear_state(p);
        auto half=render(p,std::vector<float>(rate*4,.25));
        for(size_t i=rate;i<y.size();++i) assert(std::abs(half[i]-(.125+.5*y[i]))<1e-5);
    }
    set(p,"depth",0); p->clear_state(p);
    assert(std::abs(dc(p,rate,.25)-.25)<1e-7);
    // Count actual envelope cycles: BPM must still mean one LFO cycle per beat.
    set(p,"bpm",120); set(p,"depth",1); p->clear_state(p);
    auto y=render(p,std::vector<float>(rate*6,.25)); int crossings=0;
    for(int i=rate+1;i<rate*6;++i) if(y[i-1]<.125 && y[i]>=.125) ++crossings;
    assert(crossings==10);
}
static void chorus(PluginDef* p,int rate) {
    if(std::string(p->id)!="chorus_mono") return;
    auto input=tone(rate,rate*2,.1,997);
    double wet[2];
    for(int j=0;j<2;++j) {
        set(p,"level",j+1); p->clear_state(p);
        auto y=render(p,input);
        for(size_t i=0;i<y.size();++i) y[i]-=input[i];
        wet[j]=rms(y,rate/2);
    }
    assert(wet[0]>.06 && wet[1]/wet[0]>1.99 && wet[1]/wet[0]<2.01);
}
static void harmonic(PluginDef* p,int rate) {
    if(std::string(p->id)!="harmonic_tremolo") return;
    assert(!p->activate_plugin); // tiny state: no lazy allocation or warmup
    auto x=tone(rate,rate);
    set(p,"depth",0); p->clear_state(p);
    auto dry=render(p,x);
    for(size_t i=0;i<x.size();++i) assert(std::abs(dry[i]-x[i])<1e-7);
    // RMS envelopes well below/above crossover must move in opposition.
    std::vector<double> envelopes[2];
    for(int band=0;band<2;++band) {
        set(p,"depth",1); set(p,"bpm",60); p->clear_state(p);
        auto y=render(p,tone(rate,rate*3,.25,band ? 6000 : 80));
        for(int i=rate;i<rate*3;i+=rate/20) {
            double power=0; for(int j=0;j<rate/20;++j) power+=y[i+j]*y[i+j];
            envelopes[band].push_back(std::sqrt(power/(rate/20)));
        }
    }
    double mean[2]={0,0};
    for(int b=0;b<2;++b) for(double v:envelopes[b]) mean[b]+=v/envelopes[b].size();
    double cov=0,a=0,b=0;
    for(size_t i=0;i<envelopes[0].size();++i) {
        double x=envelopes[0][i]-mean[0],y=envelopes[1][i]-mean[1];
        cov+=x*y; a+=x*x; b+=y*y;
    }
    assert(cov/std::sqrt(a*b)<-.95);
}
static void automation(PluginDef* p,int rate) {
    defaults(p);
    // A constant input reveals discontinuities independently of the waveform.
    // Compare the first sample after each edit to the previous output sample.
    for(const auto& [id,c]:controls) {
        if(id=="tremolo.SINE") continue; // legacy hidden waveform switch
        defaults(p);
        double previous=dc(p,rate,.1);
        for(float value:{c.low,c.high,c.initial}) {
            *c.value=value;
            const auto y=render(p,std::vector<float>(rate/2,.1));
            assert(std::abs(y.front()-previous)<.015);
            previous=y.back();
        }
    }
    defaults(p);
}
int main(int argc,char** argv) {
    using Factory=PluginDef*(*)();
    std::vector<Factory> factories={effects::chorus_mono::plugin,effects::flanger_mono::plugin,
        pluginlib::flanger_gx::plugin,effects::tremolo::plugin,effects::phaser_mono::plugin,
        effects::phase90::plugin,effects::harmonic_tremolo::plugin,effects::ring_modulator::plugin};
    static_assert(sizeof(effects::harmonic_tremolo::Dsp)<2048);
    for(auto factory:factories) {
        auto* p=factory(); prepare(p,48000);
        if(argc>1 && std::string(argv[1])=="--metadata") {
            for(const auto& [id,c]:controls) if(id!="tremolo.SINE")
                std::cout<<id<<'\t'<<c.initial<<'\t'<<c.low<<'\t'<<c.high<<'\t'<<c.step<<'\n';
        } else if(argc>1 && std::string(argv[1])=="--benchmark") {
            auto x=tone(48000,32); float y[32]; constexpr int blocks=30000;
            auto start=std::chrono::steady_clock::now();
            for(int i=0;i<blocks;++i) callback(p,32,x.data(),y);
            double us=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-start).count()/blocks;
            start=std::chrono::steady_clock::now();
            for(int i=0;i<10000;++i) p->clear_state(p);
            double clear=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-start).count()/10000;
            std::cout<<p->id<<" 32-frame mean "<<us<<" us; reset "<<clear<<" us\n";
        } else {
            for(int rate:{44100,48000,96000,192000}) {
                prepare(p,rate); processing(p,rate); automation(p,rate);
                tremolo(p,rate); chorus(p,rate); harmonic(p,rate);
            }
            std::cout<<p->id<<" audio checks passed\n";
        }
        p->delete_instance(p);
    }
}

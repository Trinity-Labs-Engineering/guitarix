#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <new>
#include <string>
#include <vector>
#include "gx_plugin.h"
namespace pluginlib { namespace audition_effects {
PluginDef* tone_machine(); PluginDef* super_fuzz(); PluginDef* muff(); PluginDef* fuzz_machine();
PluginDef* percolator(); PluginDef* bias_fuzz(); PluginDef* mouse(); PluginDef* tone_king();
PluginDef* flapjack(); PluginDef* yen();
} }
#include "gx_faust_support.h"
namespace effects {
#include "faust-generated/audition_ocd.cc"
}
#undef min
#undef max

static bool realtime=false;
void* operator new(std::size_t n) {assert(!realtime);if(void* p=std::malloc(n))return p;throw std::bad_alloc();}
void operator delete(void* p) noexcept {std::free(p);}
void operator delete(void* p,std::size_t) noexcept {std::free(p);}
struct Control {float* value;float initial,low,high,step;};
static std::map<std::string,Control> controls;
float* register_float(const char* id,const char*,const char*,const char*,float* v,float initial,float lo,float hi,float step,const value_pair*) {
    *v=initial;controls[id]={v,initial,lo,hi,step};return v;
}
int* register_int(const char*,const char*,const char*,const char*,int* v,int initial,int,int,const value_pair*) {*v=initial;return v;}
bool* register_bool(const char*,const char*,const char*,const char*,bool* v,bool initial) {*v=initial;return v;}
using Factory=PluginDef*(*)();
static const std::array<Factory,11> factories={
    pluginlib::audition_effects::tone_machine,pluginlib::audition_effects::super_fuzz,
    pluginlib::audition_effects::muff,pluginlib::audition_effects::fuzz_machine,
    pluginlib::audition_effects::percolator,pluginlib::audition_effects::bias_fuzz,
    pluginlib::audition_effects::mouse,pluginlib::audition_effects::tone_king,
    pluginlib::audition_effects::flapjack,pluginlib::audition_effects::yen,effects::audition_ocd::plugin};
void prepare(PluginDef* p,int rate) {
    controls.clear();ParamReg reg{p,register_float,register_int,register_bool};p->register_params(reg);
    p->set_samplerate(rate,p);if(p->activate_plugin)p->activate_plugin(true,p);
    assert(p->mono_audio && !p->stereo_audio && p->clear_state);p->clear_state(p);
}
std::vector<float> input(int rate,int count,double hz=220) {
    std::vector<float> x(count);for(int i=0;i<count;++i)x[i]=0.15*std::sin(2*M_PI*hz*i/rate);return x;
}
std::vector<float> render(PluginDef* p,const std::vector<float>& x,int block=32,bool inplace=false) {
    auto y=inplace?x:std::vector<float>(x.size());
    for(size_t i=0;i<x.size();i+=block) {
        int n=std::min<size_t>(block,x.size()-i);realtime=true;
        p->mono_audio(n,inplace?y.data()+i:const_cast<float*>(x.data())+i,y.data()+i,p);realtime=false;
    }
    for(float v:y)if(!std::isfinite(v)||std::abs(v)>32) {
        std::cerr<<p->id<<" unstable "<<v<<'\n';std::abort();
    }
    return y;
}
double energy(const std::vector<float>& x,size_t start=0) {double e=0;for(size_t i=start;i<x.size();++i)e+=x[i]*x[i];return e;}
double difference(const std::vector<float>& a,const std::vector<float>& b) {
    double e=0;for(size_t i=0;i<a.size();++i)e=std::max(e,std::abs(double(a[i])-b[i]));return e;
}
double harmonic(const std::vector<float>& x,int rate,double hz) {
    double re=0,im=0;
    for(size_t i=rate/10;i<x.size();++i) {double phase=2*M_PI*hz*i/rate;re+=x[i]*std::cos(phase);im+=x[i]*std::sin(phase);}
    return re*re+im*im;
}
void check_character() {
    // A symmetric clipper cannot generate a strong second harmonic. These tests
    // distinguish the requested octave fuzzes from ordinary distortion/EQ changes.
    for(auto factory:{factories[0],factories[1]}) {
        auto* p=factory();prepare(p,48000);
        auto y=render(p,input(48000,48000/2));
        double ratio=harmonic(y,48000,440)/std::max(1e-12,harmonic(y,48000,220));
        std::cout<<p->id<<" octave ratio="<<ratio<<std::endl;
        assert(ratio>2);
        if(factory==factories[0]) {
            *controls.at("audition_tone_machine.octave").value=0;p->clear_state(p);
            auto plain=render(p,input(48000,48000/2));
            assert(harmonic(plain,48000,440)<harmonic(plain,48000,220)*0.02);
        }
        p->delete_instance(p);
    }
    // Discrete selections must change audio, retain a live signal, and not leak
    // history through switching. Include the network conditioning input of the NN.
    for(auto item:std::array<std::pair<int,const char*>,7>{{
        {2,"audition_muff.voicing"},{3,"audition_fuzz_machine.mode"},
        {3,"audition_fuzz_machine.fuzz"},{4,"audition_percolator.clipping"},
        {5,"audition_bias_fuzz.gate"},{7,"audition_tone_king.mode"},{8,"audition_flapjack.mode"}}}) {
        auto* p=factories[item.first]();prepare(p,48000);auto c=controls.at(item.second);
        *c.value=c.low;p->clear_state(p);auto a=render(p,input(48000,24000));
        *c.value=c.high;p->clear_state(p);auto b=render(p,input(48000,24000));
        if(difference(a,b)<1e-3) {std::cerr<<item.second<<" has no audible effect\n";std::abort();}
        p->delete_instance(p);
    }
    // Model rate conversion must also work when host and training rates are not
    // integer multiples, and must keep its phase across 32-sample callbacks.
    for(int rate:{32000,88200}) {
        auto* p=factories[3]();prepare(p,rate);auto x=input(rate,rate/4);
        auto a=render(p,x,32);p->clear_state(p);auto b=render(p,x,19,true);
        assert(difference(a,b)<2e-5 && energy(a)>1e-4);p->delete_instance(p);
    }
}
void check_mouse_startup() {
    // CAT must not generate audio from its 4.5 V circuit bias. Exercise the
    // production callbacks, including controls changed after sample-rate setup
    // and reuse after a sustained note, without extending the activation fade.
    for (int rate : {44100,48000,96000,192000}) {
        auto* p = pluginlib::audition_effects::mouse();
        prepare(p, rate);
        for (float drive : {0.0f,0.1f,0.5f,0.9f,1.0f}) {
            *controls.at("audition_mouse.distortion").value = drive;
            *controls.at("audition_mouse.level").value = 12;
            for (float filter : {0.0f,0.4f,1.0f}) {
                *controls.at("audition_mouse.filter").value = filter;
                for (int repeat = 0; repeat < 2; ++repeat) {
                    realtime = true;
                    p->activate_plugin(false, p);
                    p->activate_plugin(true, p);
                    realtime = false;
                    auto silence = render(p, std::vector<float>(rate / 4));
                    double peak = 0;
                    for (float sample : silence) peak = std::max(peak, std::abs(double(sample)));
                    if (peak > 1e-7) {
                        std::cerr << "CAT zero-input startup: rate=" << rate
                                  << " drive=" << drive << " filter=" << filter
                                  << " repeat=" << repeat << " peak=" << peak << '\n';
                        p->delete_instance(p);
                        std::exit(1);
                    }
                    render(p, input(rate, rate / 10));
                }
            }
        }
        p->delete_instance(p);
    }
}
int main(int argc,char** argv) {
    if (argc > 1 && std::string(argv[1]) == "--mouse-startup") {
        check_mouse_startup();
        return 0;
    }
    bool metadata=argc>1 && std::string(argv[1])=="--metadata";
    bool probe=argc>1 && std::string(argv[1])=="--probe";
    for(auto factory:factories) {
        PluginDef* p=factory();
        if(metadata) {
            controls.clear();ParamReg reg{p,register_float,register_int,register_bool};p->register_params(reg);
            for(const auto& [id,c]:controls)std::cout<<id<<'\t'<<c.initial<<'\t'<<c.low<<'\t'<<c.high<<'\t'<<c.step<<'\n';
            p->delete_instance(p);continue;
        }
        for(int rate:{44100,48000,96000,192000}) {
            if(probe && rate!=48000)continue;
            prepare(p,rate);auto x=input(rate,rate/4+17);auto y=render(p,x);
            float peak=0;for(auto v:y)peak=std::max(peak,std::abs(v));
            std::cout<<p->id<<' '<<rate<<" rms="<<std::sqrt(energy(y,rate/10)/(y.size()-rate/10))<<" peak="<<peak<<std::endl;
            if(probe)continue;
            assert(energy(y,rate/10)>1e-5);
            realtime=true;p->clear_state(p);realtime=false;
            auto other=render(p,x,37,true);
            if(difference(y,other)>2e-5) {std::cerr<<p->id<<" reset/block mismatch "<<difference(y,other)<<'\n';return 1;}
            for(auto& [id,c]:controls) {
                for(float v:{c.low,c.high}) {*c.value=v;render(p,input(rate,rate/30));}
                *c.value=c.initial;
            }
            for(bool high:{false,true}) {
                for(auto& [id,c]:controls)*c.value=high?c.high:c.low;
                render(p,input(rate,rate/5));
            }
            for(auto& [id,c]:controls)*c.value=c.initial;
            realtime=true;p->clear_state(p);realtime=false;
            auto after=render(p,x);
            if(difference(y,after)>2e-5) {std::cerr<<p->id<<" history leak "<<difference(y,after)<<'\n';return 1;}
        }
        p->delete_instance(p);
    }
    if(!metadata && !probe)check_character();
}

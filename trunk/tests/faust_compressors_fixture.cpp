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
#include "gx_faust_support.h"
namespace effects {
#include "faust-generated/studio_compressor.cc"
#include "faust-generated/fet_compressor.cc"
#include "faust-generated/opto_compressor.cc"
#include "faust-generated/compressor.cc"
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
    // These tiny processors must not acquire a lazy activation/allocation path.
    assert(!p->activate_plugin);
    p->clear_state(p);
}
static void set(PluginDef* p, const std::string& key, float value) {
    *controls.at(std::string(p->id)+"."+key).value = value;
}
static void defaults(PluginDef* p) {
    for (const auto& [id,c] : controls) *c.value = c.initial;
    p->clear_state(p);
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

static void processing(PluginDef* p, int rate) {
    const auto input = tone(rate,rate/3+19);
    auto expected = render(p,input);
    for (int block : {1,8,37,64,128,256}) {
        p->clear_state(p);
        auto actual = render(p,input,block,true);
        for (size_t i=0; i<input.size(); ++i) assert(std::abs(actual[i]-expected[i]) < 1e-7);
    }
    // Controls take their stored value on sample one, including after reset.
    set(p,"mix",0);
    p->clear_state(p);
    auto dry = render(p,input);
    for (size_t i=0; i<input.size(); ++i) {
        double target = std::string(p->id)=="studio_compressor" ? input[i]
            : .5*(input[i]+(i ? input[i-1] : 0));
        assert(std::abs(dry[i]-target) < 1e-7);
    }
    defaults(p);
    // No lookahead/whole-sample delay or startup gain fade.
    auto impulse = std::vector<float>(64,0);
    impulse[0] = 1e-4;
    auto response = render(p,impulse);
    assert(response[0] > 4.9e-5);
    for (size_t i=2; i<response.size(); ++i) assert(response[i] == 0);

    for (const auto& [id,c] : controls) {
        for (float v : {c.low,c.high}) {
            defaults(p);
            *c.value = v;
            render(p,input);
        }
    }
    for (bool high : {false,true}) {
        for (const auto& [id,c] : controls) *c.value = high ? c.high : c.low;
        render(p,tone(rate,rate,1.5,41));
        p->clear_state(p);
        for (float x : render(p,std::vector<float>(rate/10))) assert(x == 0);
    }
    defaults(p);
}

static void static_ratios(PluginDef* p, int rate) {
    const std::string id=p->id;
    if (id != "studio_compressor") set(p,"colour",0);
    const double input = std::pow(10.,-6./20.);
    for (int position=0; position<4; ++position) {
        double ratio = std::vector<double>{4,8,12,20}[position];
        if (id=="opto_compressor") ratio = std::min(ratio,10.);
        set(p,"ratio",id=="fet_compressor" ? position : ratio);
        p->clear_state(p);
        double output=dc(p,rate*2,input);
        const double threshold=id=="fet_compressor" ? -18 : -24;
        const double expected=threshold+(-6-threshold)/ratio;
        if (std::abs(db(output)-expected) > .06) {
            std::cerr<<id<<" ratio "<<ratio<<": "<<db(output)<<" expected "<<expected<<'\n';
            std::abort();
        }
    }
    if (id=="studio_compressor") {
        // The standard quadratic knee has 0.5625 dB loss at its centre,
        // at ratio 4 and knee width 6 dB. Ratio 1 is exactly transparent.
        set(p,"ratio",4);
        p->clear_state(p);
        auto in=std::pow(10.,-24./20.);
        assert(std::abs(db(dc(p,rate,in))-(-24.5625)) < .01);
        set(p,"ratio",1);
        p->clear_state(p);
        assert(std::abs(dc(p,rate,.2f)-.2) < 1e-6);
    }
    defaults(p);
}

static void automation(PluginDef* p, int rate) {
    // Constant input isolates parameter discontinuities from natural waveform
    // slope. Every control, including the discrete FET ratios, changes live.
    for (const auto& [id,c] : controls) {
        defaults(p);
        double previous=dc(p,rate,.25f);
        for (float v : {c.low,c.high,c.initial}) {
            *c.value=v;
            auto out=render(p,std::vector<float>(rate/5,.25f));
            double edge=std::abs(out.front()-previous);
            if (edge > .012) {
                std::cerr<<id<<" abrupt parameter edge "<<edge<<'\n';
                std::abort();
            }
            // A slew must not turn into oscillation later in the block.
            for (size_t i=1; i<out.size(); ++i) assert(std::abs(out[i]-out[i-1]) < .012);
            previous=out.back();
        }
    }
    defaults(p);
}

static void timing_controls(PluginDef* p, int rate) {
    if (std::string(p->id)=="opto_compressor") return;
    double attack_area[2], recovered[2];
    for (int slow=0;slow<2;++slow) {
        defaults(p);
        const auto attack=controls.at(std::string(p->id)+".attack");
        *attack.value=slow ? attack.high : attack.low;
        dc(p,rate/10,1e-4f);
        const auto onset=render(p,std::vector<float>(rate/1000,.5f));
        attack_area[slow]=0;
        for (float x:onset) attack_area[slow]+=x;
        defaults(p);
        const auto release=controls.at(std::string(p->id)+".release");
        *release.value=slow ? release.high : release.low;
        dc(p,rate,.5f);
        recovered[slow]=dc(p,rate/5,.001f);
    }
    assert(attack_area[0]<attack_area[1]*.9);
    assert(recovered[0]>recovered[1]*1.5);
    defaults(p);
}

static double third_harmonic(const std::vector<float>& x,int rate) {
    double real=0,imaginary=0;
    for (size_t i=x.size()-rate; i<x.size(); ++i) {
        real+=x[i]*std::cos(2*M_PI*3000*i/rate);
        imaginary+=x[i]*std::sin(2*M_PI*3000*i/rate);
    }
    return 2*std::hypot(real,imaginary)/rate;
}
static void colour_and_memory(PluginDef* p,int rate) {
    const std::string id=p->id;
    if (id=="studio_compressor") return;
    double harmonic[2];
    for (int warm=0; warm<2; ++warm) {
        defaults(p);
        set(p,"colour",warm ? 100 : 0);
        if (id=="opto_compressor") set(p,"threshold",0);
        else set(p,"input",-12);
        auto y=render(p,tone(rate,rate*2,.5,1000));
        harmonic[warm]=third_harmonic(y,rate);
        double mean=0;
        for (size_t i=y.size()-rate;i<y.size();++i) mean+=y[i];
        assert(std::abs(mean/rate)<1e-6);
    }
    assert(harmonic[1]>harmonic[0]*2 && harmonic[1]>1e-5);
    if (id=="opto_compressor") {
        double gain[2];
        for (int sustained=0;sustained<2;++sustained) {
            defaults(p);
            set(p,"colour",0);
            dc(p,sustained ? rate*3 : rate/20,.8f);
            gain[sustained]=db(dc(p,rate/2,1e-4f)/1e-4);
        }
        assert(gain[1]<gain[0]-3); // Same last input, different cell memory.
    }
    defaults(p);
}

static void resident(PluginDef* p,int rate) {
    // The engine processes bypassed tracking plugins into a scratch buffer.
    // A warm instance must agree sample-for-sample with a continuously audible
    // instance at re-entry, including fast FET and long optical histories.
    auto input=tone(rate,rate*2+32,.5,73);
    auto expected=render(p,input);
    p->clear_state(p);
    float scratch[32];
    for (int i=0;i<rate*2;i+=32) {
        int n=std::min(32,rate*2-i);
        std::copy_n(input.data()+i,n,scratch);
        callback(p,n,scratch,scratch);
    }
    float output[32];
    callback(p,32,input.data()+rate*2,output);
    for (int i=0;i<32;++i) assert(std::abs(output[i]-expected[rate*2+i])<1e-7);
    defaults(p);
}

int main(int argc,char** argv) {
    using Factory=PluginDef*(*)();
    std::vector<Factory> factories={effects::studio_compressor::plugin,effects::fet_compressor::plugin,effects::opto_compressor::plugin};
    if (argc>1 && std::string(argv[1])=="--benchmark") factories.push_back(effects::compressor::plugin);
    static_assert(sizeof(effects::studio_compressor::Dsp)<2048);
    static_assert(sizeof(effects::fet_compressor::Dsp)<2048);
    static_assert(sizeof(effects::opto_compressor::Dsp)<2048);
    for (auto factory:factories) {
        auto* p=factory();
        prepare(p,48000);
        if (argc>1 && std::string(argv[1])=="--metadata") {
            for (const auto& [id,c]:controls)
                std::cout<<id<<'\t'<<c.initial<<'\t'<<c.low<<'\t'<<c.high<<'\t'<<c.step<<'\n';
        } else if (argc>1 && std::string(argv[1])=="--benchmark") {
            // Measure the production 32-frame callback; report, don't assert
            // machine-specific wall-clock deadlines on a shared build host.
            auto input=tone(48000,32);
            float output[32];
            constexpr int blocks=150000;
            auto start=std::chrono::steady_clock::now();
            for (int i=0;i<blocks;++i) callback(p,32,input.data(),output);
            double us=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-start).count()/blocks;
            std::cout<<p->id<<" 32-frame mean: "<<us<<" us; one-core load: "<<us/666.6667*100<<"%\n";
        } else {
            for (int rate:{44100,48000,96000,192000}) {
                prepare(p,rate);
                processing(p,rate);
                static_ratios(p,rate);
                automation(p,rate);
                timing_controls(p,rate);
                colour_and_memory(p,rate);
                resident(p,rate);
            }
            std::cout<<p->id<<" audio checks passed\n";
        }
        p->delete_instance(p);
    }
}

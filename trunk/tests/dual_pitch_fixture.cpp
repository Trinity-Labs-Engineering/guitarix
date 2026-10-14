#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <new>
#include <string>
#include <vector>
#include "gx_plugin.h"

static size_t allocations = 0;
void* operator new(size_t size) { ++allocations; if (void* p = std::malloc(size)) return p; throw std::bad_alloc(); }
void* operator new[](size_t size) { return ::operator new(size); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
namespace pluginlib { namespace dual_pitch { PluginDef* plugin(); } }

std::map<std::string, float*> controls;
float* reg_float(const char* id, const char*, const char*, const char*, float* value,
                 float initial, float low, float high, float step, const value_pair*) {
    controls[id] = value; *value = initial;
    const bool pitch = std::string(id).find("voice") != std::string::npos;
    assert(low == (pitch ? -12 : 0)); assert(high == (pitch ? 12 : 100)); assert(step == 1);
    return value;
}
void set(const char* id, float value) { *controls.at(id) = value; }
std::vector<float> run(PluginDef* p, const std::vector<float>& input, float v1, float v2, float dry, bool inplace = false, int block = 257) {
    set("dual_pitch.level1", v1); set("dual_pitch.level2", v2); set("dual_pitch.dry", dry);
    p->clear_state(p);
    std::vector<float> result = input;
    const size_t before = allocations;
    for (int offset = 0; offset < int(input.size()); offset += block) {
        const int count = std::min(block, int(input.size()) - offset);
        p->mono_audio(count, inplace ? result.data() + offset : const_cast<float*>(input.data()) + offset, result.data() + offset, p);
    }
    assert(allocations == before);
    for (float sample : result) assert(std::isfinite(sample));
    return result;
}
void near(const std::vector<float>& a, const std::vector<float>& b) {
    for (size_t i=0; i<a.size(); ++i) assert(std::abs(a[i]-b[i]) < 2e-6f);
}
double amplitude(const std::vector<float>& signal, double freq, int rate) {
    double a=0,b=0;
    for (size_t i=rate; i<signal.size(); ++i) {
        const double phase=2*3.141592653589793*freq*i/rate;
        a += signal[i]*std::cos(phase); b += signal[i]*std::sin(phase);
    }
    return 2*std::sqrt(a*a+b*b)/(signal.size()-rate);
}
int main() {
    auto* p=pluginlib::dual_pitch::plugin();
    ParamReg reg {p, reg_float, nullptr, nullptr};
    assert(p->register_params(reg)==0 && controls.size()==5);
    for (int rate : {44100,48000,96000}) {
        p->set_samplerate(rate,p); p->activate_plugin(true,p);
        std::vector<float> input(2*rate);
        for (size_t i=0;i<input.size();++i) input[i]=.1f*std::sin(2*3.141592653589793*440*i/rate);
        const auto one=run(p,input,50,0,0), two=run(p,input,0,50,0);
        assert(amplitude(one,220,rate)>.06);
        assert(amplitude(two,880,rate)>.06);
        assert(amplitude(one,440,rate)<.005 && amplitude(two,440,rate)<.005);
        auto expected=input;
        for(size_t i=0;i<input.size();++i) expected[i]=one[i]*.5f+two[i]*2.f+input[i]*.6f;
        near(run(p,input,25,100,30),expected);
        near(run(p,input,25,100,30,true,1111),expected);
        near(run(p,input,0,0,50),input);
        auto silence=run(p,input,0,0,0); for(float value:silence) assert(value==0);
        // Reset and bypass re-entry are deterministic and do not allocate.
        const size_t before=allocations;
        p->clear_state(p); p->activate_plugin(false,p); p->activate_plugin(true,p);
        assert(allocations==before);
        near(run(p,input,50,0,0),one);
        for (double frequency : {30.867706,36.708096,41.203445}) {
            for (size_t i=0;i<input.size();++i) input[i]=.1f*std::sin(2*3.141592653589793*frequency*i/rate);
            const auto down=run(p,input,50,0,0), up=run(p,input,0,50,0);
            assert(amplitude(down,frequency*.5,rate)>.06);
            assert(amplitude(up,frequency*2,rate)>.06);
            for(size_t i=0;i<input.size();++i) expected[i]=down[i]+up[i];
            near(run(p,input,50,50,0,true,37),expected);
        }
    }
    // Automation of one voice must not affect the other voice's state.
    set("dual_pitch.voice1",7); set("dual_pitch.voice2",-5);
    std::vector<float> input(16384,.1f);
    auto shifted=run(p,input,50,50,0);
    near(run(p,input,50,50,0,true,37),shifted);
    p->delete_instance(p);
    std::cout<<"dual-pitch-ok\n";
}

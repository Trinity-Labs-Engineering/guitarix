#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <new>
#include <string>
#include <vector>
#include "gx_harmoniser.h"
#include "gx_plugin.h"

namespace pluginlib {
namespace harmoniser { PluginDef* plugin(); }
namespace dual_harmoniser { PluginDef* plugin(); }
}
static size_t allocations = 0;
void* operator new(size_t size) { ++allocations; if (void* p = std::malloc(size)) return p; throw std::bad_alloc(); }
void* operator new[](size_t size) { return ::operator new(size); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
static std::map<std::string, float*> controls;
float* reg_float(const char* id, const char*, const char*, const char*, float* value,
                 float initial, float low, float high, float step, const value_pair*) {
    assert(initial >= low && initial <= high && step == 1);
    controls[id] = value; *value = initial; return value;
}
constexpr double pi = 3.14159265358979323846;
float signal(int i, int rate, double frequency, int kind) {
    const double phase = 2 * pi * frequency * i / rate;
    if (kind == 3) return 0;
    float value = 0.1 * std::sin(phase);
    if (kind >= 1) value += 0.03 * std::sin(2 * phase) + 0.015 * std::sin(3 * phase);
    if (kind == 2) value += 0.08 * (std::sin(phase * std::pow(2., 4./12)) + std::sin(phase * std::pow(2., 7./12)));
    if (kind == 4) value += 0.08 * (std::sin(phase * std::pow(2., 3./12)) + std::sin(phase * std::pow(2., 7./12)));
    if (kind == 5) value += 0.08 * (std::sin(phase * std::pow(2., 3./12)) + std::sin(phase * std::pow(2., 8./12)));
    return value;
}
int main(int argc, char** argv) {
    assert(argc >= 2);
    const std::string command = argv[1];
    if (command == "intervals") {
        for (int root=0; root<12; ++root) for (int mode=0; mode<7; ++mode)
            for (int note=0; note<12; ++note) for (int interval=-7; interval<=7; ++interval)
                std::cout << gx_engine::harmony::interval_shift(note,root,mode,interval) << '\n';
        return 0;
    }
    if (command == "tracker") {
        for (int rate : {44100,48000,96000,192000}) {
            gx_engine::harmony::FundamentalTracker tracker;
            tracker.prepare(rate);
            for (int midi=36; midi<=88; ++midi) for (int kind : {0,1,2,4,5}) {
                const double frequency = 440 * std::pow(2., (midi - 69) / 12.);
                tracker.reset();
                int correct=0, total=0, acquired=0;
                const size_t before = allocations;
                for (int i=0; i<rate/2; ++i) {
                    float value = signal(i,rate,frequency,kind);
                    tracker.push(&value,1);
                    if (tracker.note() == (midi+3)%12 && !acquired) acquired=i;
                    if (i>rate/4 && i%128==0) { ++total; if (tracker.note()==(midi+3)%12) ++correct; }
                }
                assert(allocations==before);
                if (correct!=total || acquired > rate * (kind>=2 ? .2 : .08)) {
                    std::cerr << "tracking rate=" << rate << " midi=" << midi << " kind=" << kind
                        << " correct=" << correct << '/' << total << " detected=" << tracker.frequency() << '\n';
                    return 1;
                }
            }
            // Silence clears the held note; reset must clear filter/history state.
            float silence[512] = {};
            for (int i=0; i<rate; i+=512) tracker.push(silence,512);
            assert(tracker.note()==-1);
            tracker.reset(); assert(tracker.note()==-1);
            // Note changes without reset must update the selected scale degree.
            for (int midi : {45,52,48,61,57}) {
                for (int i=0; i<rate/4; ++i) {
                    float value=signal(i,rate,440*std::pow(2.,(midi-69)/12.),1);
                    tracker.push(&value,1);
                }
                assert(tracker.note()==(midi+3)%12);
            }
            tracker.reset();
            for (int i=0; i<rate; ++i) { float dc=.1f; tracker.push(&dc,1); }
            assert(tracker.note()==-1);
        }
        std::cout << "tracker-ok\n";
        return 0;
    }
    // render dual rate frequency tonic mode interval1 interval2 mix wet dry block inplace kind
    assert(command=="render" && argc==15);
    const bool dual=std::atoi(argv[2]);
    const int rate=std::atoi(argv[3]), block=std::atoi(argv[12]), kind=std::atoi(argv[14]);
    const double frequency=std::atof(argv[4]);
    const bool inplace=std::atoi(argv[13]);
    auto* plugin=dual ? pluginlib::dual_harmoniser::plugin() : pluginlib::harmoniser::plugin();
    const std::string prefix=dual ? "dual_harmoniser." : "harmoniser.";
    ParamReg reg{plugin,reg_float,nullptr,nullptr};
    plugin->register_params(reg);
    assert(controls.size()==(dual ? 6u : 5u));
    *controls.at(prefix+"tonic")=std::atof(argv[5]);
    *controls.at(prefix+"mode")=std::atof(argv[6]);
    *controls.at(prefix+(dual ? "voice1" : "interval"))=std::atof(argv[7]);
    if (dual) { *controls.at(prefix+"voice2")=std::atof(argv[8]); *controls.at(prefix+"mix")=std::atof(argv[9]); }
    else *controls.at(prefix+"wet")=std::atof(argv[10]);
    *controls.at(prefix+"dry")=std::atof(argv[11]);
    plugin->set_samplerate(rate,plugin);
    std::vector<float> input(rate*2), output(rate*2), repeat(rate*2);
    for (int i=0; i<rate*2; ++i) input[i]=signal(i,rate,frequency,kind);
    const size_t before=allocations;
    for (auto* result : {&output,&repeat}) {
        std::copy(input.begin(),input.end(),result->begin());
        plugin->activate_plugin(false,plugin); plugin->clear_state(plugin); plugin->activate_plugin(true,plugin);
        for (int offset=0; offset<int(input.size()); offset+=block) {
            const int count=std::min(block,int(input.size())-offset);
            plugin->mono_audio(count,(inplace ? result->data() : input.data())+offset,result->data()+offset,plugin);
        }
    }
    assert(allocations==before);
    assert(output==repeat);
    for (float value : output) assert(std::isfinite(value));
    std::cout.write(reinterpret_cast<const char*>(output.data()),output.size()*sizeof(float));
    plugin->delete_instance(plugin);
}

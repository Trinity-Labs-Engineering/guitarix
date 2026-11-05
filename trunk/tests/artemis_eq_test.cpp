#include "gx_artemis_eq.h"
#include "gx_spectrum.h"
#include <cassert>
#include <complex>
#include <iostream>
#include <limits>
#include <map>
#include <vector>

std::map<std::string, float*> params;
float* reg_float(const char* id, const char*, const char* type, const char*, float* value, float initial, float, float high, float, const value_pair* labels) {
    assert(type[0] == 'S' || type[0] == 'B');
    if (std::string(id).find("_shape") != std::string::npos) {
        assert(high == 4 && labels);
        assert(std::string(labels[3].value_id) == "Low cut");
        assert(std::string(labels[4].value_id) == "High cut");
    }
    *value = initial; params[id] = value; return value;
}
int* reg_int(const char*, const char*, const char*, const char*, int* v, int initial, int, int, const value_pair*) { *v = initial; return v; }
bool* reg_bool(const char*, const char*, const char*, const char*, bool* v, bool initial) { *v = initial; return v; }
void set(unsigned band, const char* key, float value) { *params.at("artemis_eq.b" + std::to_string(band) + "_" + key) = value; }
void process(PluginDef& eq, std::vector<float>& data) { eq.mono_audio(int(data.size()), data.data(), data.data(), &eq); }
double rms(const std::vector<float>& data, unsigned begin) {
    double energy = 0; for (unsigned i = begin; i < data.size(); ++i) energy += data[i] * data[i];
    return std::sqrt(energy / (data.size() - begin));
}
int main() {
    gx_engine::ArtemisEq eq;
    ParamReg reg{&eq, reg_float, reg_int, reg_bool}; eq.register_params(reg);
    assert(params.size() == 80);
    for (unsigned rate : {8000u, 44100u, 48000u, 96000u, 192000u}) {
        eq.set_samplerate(rate, &eq);
        std::vector<float> original(8192);
        for (unsigned i = 0; i < original.size(); ++i) original[i] = 0.1f * std::sin(i * .079) + .02f * std::cos(i * .007);
        auto output = original; process(eq, output); assert(output == original);
        for (unsigned band = 1; band <= 16; ++band) set(band, "enabled", 1);
        output = original; process(eq, output); assert(output == original);
        // Gain zero stays exact even when shape, frequency, Q are hostile.
        set(1, "q", 0); set(2, "q", std::numeric_limits<float>::quiet_NaN());
        set(3, "frequency", std::numeric_limits<float>::infinity());
        set(4, "shape", -100); output = original; process(eq, output); assert(output == original);
        for (unsigned band = 1; band <= 16; ++band) { set(band,"enabled",0); set(band,"q",1); set(band,"frequency",1000); }
        // Measure the actual audio path, not just coefficient construction.
        for (int shape = 0; shape < 3; ++shape) for (float gain : {-24.f,-6.f,6.f,24.f}) {
            set(1,"enabled",1); set(1,"gain",gain); set(1,"shape",float(shape));
            eq.clear_state(&eq);
            std::vector<float> tone(rate);
            for (unsigned i = 0; i < rate; ++i) tone[i] = .01f * std::sin(6.283185307179586 * 1000 * i / rate);
            const double before = rms(tone, rate/2); process(eq, tone);
            const double actual = 20 * std::log10(rms(tone,rate/2) / before);
            assert(std::abs(actual - (shape ? gain/2 : gain)) < .015);
        }
        const auto measure = [&](double frequency) {
            eq.clear_state(&eq);
            std::vector<float> tone(rate);
            for (unsigned i = 0; i < rate; ++i) tone[i] = .01f * std::sin(6.283185307179586 * frequency * i / rate);
            const double before = rms(tone, rate/2); process(eq, tone);
            return 20 * std::log10(rms(tone,rate/2) / before);
        };
        for (int shape : {3,4}) {
            set(1,"shape",float(shape)); set(1,"gain",0); set(1,"frequency",1000);
            set(1,"q",float(std::sqrt(.5)));
            // Low cut rejects bass, high cut rejects treble, even at zero gain.
            assert(measure(shape == 3 ? 100 : 3000) < -18);
            assert(std::abs(measure(shape == 3 ? 3000 : 100)) < .1);
            assert(std::abs(measure(1000) + 3.0103) < .015);
            // Q controls the corner response; the retained gain is irrelevant.
            for (float q : {.1f,1.f,18.f}) for (float gain : {-24.f,0.f,24.f}) {
                set(1,"q",q); set(1,"gain",gain);
                assert(std::abs(measure(1000) - 20 * std::log10(q)) < .015);
            }
            set(1,"enabled",0); eq.clear_state(&eq);
            output = original; process(eq,output); assert(output == original);
            set(1,"enabled",1);
        }
        // Live extremes and repeated flat/shape/rate transitions remain finite.
        for (int turn = 0; turn < 300; ++turn) {
            set(1,"q",turn % 2 ? .1f : 18.f); set(1,"frequency",turn % 2 ? 20.f : 20000.f);
            set(1,"gain",turn % 3 ? 24.f : -24.f); set(1,"shape",float(turn % 5));
            output.assign(original.begin(), original.begin()+256); process(eq,output);
            for (float v : output) assert(std::isfinite(v) && std::abs(v) < 100);
        }
        set(1,"shape",0); set(1,"gain",0); output = original; process(eq,output);
        output = original; process(eq,output); assert(output == original);
        eq.clear_state(&eq); output = original; process(eq,output); assert(output == original);
    }
    gx_spectrum::Capture queue;
    std::vector<float> signal(256, .25f);
    queue.push(256,signal.data(),signal.data(),48000);
    assert(queue.drain([](float,float,unsigned){}) == 0);
    queue.enabled.store(true);
    for (int i = 0; i < 1000; ++i) queue.push(256,signal.data(),signal.data(),48000);
    assert(queue.drain([](float l,float r,unsigned rate){assert(l == .25f && r == .25f && rate == 48000);}) == 16384);
    // Exercise producer/consumer publication under contention.
    std::atomic<bool> done{false};
    std::thread producer([&] {
        for (int block = 1; block <= 10000; ++block) {
            std::fill(signal.begin(),signal.end(),float(block));
            queue.push(256,signal.data(),signal.data(),48000);
        }
        done.store(true);
    });
    float last = 0;
    do { queue.drain([&](float l,float r,unsigned rate){assert(l == r && l >= last && rate == 48000); last=l;}); } while (!done.load());
    producer.join();
    gx_spectrum::Analyzer analyzer;
    for (unsigned i = 0; i < 8192; ++i)
        analyzer.push(.5f*std::sin(6.283185307179586*750*i/48000), .25f*std::sin(6.283185307179586*1500*i/48000),48000);
    std::cout << analyzer.frame() << '\n';
}

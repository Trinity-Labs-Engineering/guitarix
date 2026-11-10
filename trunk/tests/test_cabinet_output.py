"""Compile the real cabinet callback/registration without the GUI dependencies."""
from pathlib import Path
import os
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class CabinetOutputTests(unittest.TestCase):
    def test_trim_is_post_tone_linear_ramped_and_legacy_neutral(self):
        source = (TRUNK / "src/gx_head/engine/gx_internal_plugins.cpp").read_text()
        methods = source.split("void CabinetConvolver::run_cab_conf(", 1)[1].split("//// STEREO", 1)[0]
        fixture = r'''
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <map>
#include <string>
#include <vector>
#include "gx_plugin.h"
#define N_(s) s
#define always_inline
struct Parameter { float* value; float initial, low, high, step; };
std::map<std::string, Parameter> parameters;
float* reg_float(const char* id, const char*, const char* flags, const char*,
                 float* value, float initial, float low, float high, float step, const value_pair*) {
    if (std::string(flags).find('A') != std::string::npos) return parameters.at(id).value;
    *value = initial;
    parameters[id] = {value, initial, low, high, step};
    return value;
}
int* reg_int(const char*, const char*, const char*, const char*, int* value, int initial, int, int, const value_pair*) {
    *value = initial; return value;
}
#include "faust-generated/cabinet_impulse_former.h"
#include "faust-generated/cabinet_impulse_former.cc"
struct EngineControl { enum { ov_Convolver }; void overload(int, const char*) { assert(false); } };
struct CabinetConvolver : PluginDef {
    float level = 1, output_level = 0, output_gain = 1, bass = 0, treble = 0;
    int cabinet = 0;
    value_pair* cab_names = nullptr;
    std::atomic<bool> scene_update_suspended{false};
    EngineControl engine;
    // Stand-ins for resampling/convolution; the actual tone DSP and complete
    // callback below run unchanged. Track the input to detect upstream gain.
    struct Conv {
        float received = 0;
        __attribute__((noinline)) bool compute(int n, float* data) {
            if (n > 0) received = data[0];
            for (int i = 0; i < n; ++i) data[i] *= 2;
            return true;
        }
    } conv;
    struct Resampler {
        int count;
        int max_out_count(int n) { return n; }
        int up(int n, float* in, float* out) { count = n; std::copy(in,in+n,out); return n; }
        void down(float* in, float* out) { std::copy(in,in+count,out); }
    } smp;
    cabinet_impulse_former::Dsp impf;
    static void run_cab_conf(int, float*, float*, PluginDef*);
    static int register_cab(const ParamReg&);
    CabinetConvolver() {
        ParamReg reg{this, reg_float, reg_int};
        register_cab(reg);
        impf.init(48000);
    }
};
void CabinetConvolver::run_cab_conf(''' + methods + r'''
int main() {
    CabinetConvolver check;
    auto p = parameters.at("cab.output");
    assert(p.initial == 0 && p.low == -60 && p.high == 40 && p.step == .01f);
    for (int n : {1, 64, 1024}) {
        for (float tone : {-10.f, 0.f, 10.f}) {
            for (float db : {-60.f, -24.f, -2.15f, 0.f, 13.15f, 24.f, 40.f}) {
                CabinetConvolver cab;
                cab.bass = tone; cab.treble = -tone;
                std::vector<float> input(n), reference, output;
                for (int i = 0; i < n; ++i) input[i] = .1f + .05f * std::sin(.7f * i);
                reference = input;
                CabinetConvolver::run_cab_conf(n, reference.data(), reference.data(), &cab);
                assert(cab.conv.received == input[0]);
                // First block ramps, next block is precisely the requested dB.
                cab.impf.clear_state_f();
                cab.output_level = db;
                output = input;
                CabinetConvolver::run_cab_conf(n, output.data(), output.data(), &cab);
                const float target = std::pow(10.f, .05f * db);
                for (int i = 0; i < n; ++i) {
                    const float expected = reference[i] * (1 + (target - 1) * (i+1) / n);
                    assert(std::abs(output[i] - expected) < 1.e-4f);
                }
                cab.impf.clear_state_f();
                output = input;
                CabinetConvolver::run_cab_conf(n, output.data(), output.data(), &cab);
                assert(cab.conv.received == input[0]);
                for (int i = 0; i < n; ++i) {
                    assert(std::abs(output[i] - reference[i] * target) < 1.e-4f);
                    if (db == 0) assert(output[i] == reference[i]);
                }
                cab.scene_update_suspended = true;
                std::fill(output.begin(), output.end(), 0);
                CabinetConvolver::run_cab_conf(n, input.data(), output.data(), &cab);
                assert(output == input);
            }
        }
    }
}
'''
        with tempfile.TemporaryDirectory(prefix="gx-cab-output-") as directory:
            cpp, binary = Path(directory) / "fixture.cpp", Path(directory) / "fixture"
            cpp.write_text(fixture)
            subprocess.run([os.environ.get("CXX", "c++"), "-std=c++17", "-O2", "-Wall", "-Wextra", "-Werror",
                            "-Wno-missing-field-initializers", "-Wno-unused-parameter",
                            f"-I{TRUNK / 'src/headers'}", f"-I{TRUNK / 'src'}", str(cpp), "-o", str(binary)], check=True)
            subprocess.run([str(binary)], check=True, timeout=10)


if __name__ == "__main__":
    unittest.main()

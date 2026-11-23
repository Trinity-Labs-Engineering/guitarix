"""Compare guarded imported-IR post gain with its original Faust processor."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class SceneIrGainTests(unittest.TestCase):
    def test_imported_kernel_frequency_gain_and_explicit_trim(self):
        source = (TRUNK / "src/gx_head/engine/gx_convolver.cpp").read_text()
        start = source.index("static bool normalize_mono_ir(")
        normalize = source[start:source.index("\n}", start) + 2]
        fixture = r'''
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <vector>
#include <fftw3.h>
''' + normalize + r'''
int main() {
    for (float level : {0.f, .125f, 1.f, 8.f}) {
        std::vector<float> taps(128);
        taps[48] = -level;
        assert(normalize_mono_ir(taps, .5f));
        assert(taps[48] == -.5f * std::min(level, 1.f));
    }
    // Equal taps add in phase at DC. Alternating taps peak at Nyquist.
    for (float sign : {1.f, -1.f}) {
        std::vector<float> taps(16, .5f);
        for (size_t i = 1; i < taps.size(); i += 2) taps[i] *= sign;
        assert(normalize_mono_ir(taps, 1.f));
        for (float tap : taps) assert(std::abs(std::abs(tap) - 1.f/16) < 1e-7);
    }
    // An interior resonance with an off-bin centre frequency spans more
    // than one file-read chunk. Check magnitude and preservation of phase.
    std::vector<float> original(34001);
    const double omega = 2 * std::acos(-1.) * 731.3 / 48000;
    for (size_t i = 0; i < original.size(); ++i) original[i] = .5 * std::cos(i * omega);
    auto taps = original;
    assert(normalize_mono_ir(taps, 1.f));
    double real = 0, imag = 0;
    for (size_t i = 0; i < taps.size(); ++i) {
        assert(std::abs(taps[i] * original[0] - taps[0] * original[i]) < 1e-9);
        real += taps[i] * std::cos(i * omega);
        imag += taps[i] * std::sin(i * omega);
    }
    assert(std::abs(std::hypot(real, imag) - 1.) < .015);
    taps = {std::numeric_limits<float>::quiet_NaN()};
    assert(!normalize_mono_ir(taps, 1.f));
}
'''
        with tempfile.TemporaryDirectory(prefix="gx-ir-kernel-") as directory:
            cpp, binary = Path(directory) / "fixture.cpp", Path(directory) / "fixture"
            cpp.write_text(fixture)
            subprocess.run([os.environ.get("CXX", "c++"), "-std=c++17", "-O2", str(cpp),
                            "-lfftw3f", "-o", str(binary)], check=True)
            subprocess.run([str(binary)], check=True, timeout=10)

    def test_interactive_equivalence_and_guarded_gain_changes(self):
        header = (TRUNK / "src/headers/gx_internal_plugins.h").read_text()
        definition = "class SceneConvolverMonoPost" + header.split("class SceneConvolverMonoPost", 1)[1].split("\n};", 1)[0] + "\n};"
        source = (TRUNK / "src/gx_head/engine/gx_internal_plugins.cpp").read_text()
        methods = "int SceneConvolverMonoPost::register_par" + source.split("int SceneConvolverMonoPost::register_par", 1)[1].split("ConvolverMonoAdapter::ConvolverMonoAdapter", 1)[0]
        fixture = r'''
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include "gx_plugin.h"
#define N_(s) s
#define always_inline
#include "faust-generated/jconv_post_mono.h"
#include "faust-generated/jconv_post_mono.cc"
std::map<std::string, float*> controls;
float* reg_float(const char* id, const char*, const char*, const char*, float* value,
                 float initial, float, float, float, const value_pair*) {
    *value = initial; controls[id] = value; return value;
}
''' + definition + methods + r'''
int main() {
    for (int rate : {44100, 48000, 96000}) {
        for (int count : {1, 32, 64, 256}) {
            SceneConvolverMonoPost scene;
            scene.register_par(ParamReg{nullptr, reg_float});
            float* level = controls.at("jconv_mono.gain");
            float* mix = controls.at("jconv_mono.wet_dry");
            jconv_post_mono::Dsp old;
            old.init(rate);
            old.register_par(ParamReg{nullptr, reg_float});
            float* old_level = controls.at("jconv_mono.gain");
            float* old_mix = controls.at("jconv_mono.wet_dry");
            assert(*mix == 50.f && *old_mix == 50.f);
            std::vector<float> dry(count), wet(count), actual(count), expected(count);
            for (int i = 0; i < count; ++i) {
                dry[i] = .1f + .01f * std::sin(.4f*i);
                wet[i] = .2f + .02f * std::cos(.3f*i);
            }
            for (float blend : {0.f, 50.f, 90.4296875f, 100.f}) {
                *mix = *old_mix = blend;
                for (float db : {0.f, 20.f, -6.f, -20.f, 12.f}) {
                    *level = *old_level = db;
                    for (int block = 0; block < 20; ++block) {
                        scene.compute(count, dry.data(), wet.data(), actual.data());
                        old.compute(count, dry.data(), wet.data(), expected.data());
                        assert(actual == expected);
                    }
                }
            }
            *mix = 50.f;
            for (float db : {20.f, -6.f, -20.f, 20.f}) {
                *level = db;
                scene.suspend_scene_smoother_for_control();
                scene.compute(count, dry.data(), wet.data(), actual.data());
                assert(actual == dry);
                scene.finish_scene_smoother_control(true);
                actual = dry;
                scene.compute(count, actual.data(), wet.data(), actual.data());
                for (int i = 0; i < count; ++i)
                    assert(std::abs(actual[i] - (.5f*dry[i] + .5f*wet[i]*std::pow(10.f,.05f*db))) < 1.e-5f);
                scene.suspend_scene_smoother_for_control();
                scene.finish_scene_smoother_control(false);
                *level = -db;
                scene.request_scene_smoother_snap();
                scene.compute(count, dry.data(), wet.data(), actual.data());
                assert(scene.finish_scene_smoother_snap());
                for (int i = 0; i < count; ++i)
                    assert(std::abs(actual[i] - (.5f*dry[i] + .5f*wet[i]*std::pow(10.f,-.05f*db))) < 1.e-5f);
            }
            scene.request_scene_smoother_snap();
            assert(!scene.finish_scene_smoother_snap());
            assert(!scene.is_scene_smoother_snap_pending());
        }
    }
}
'''
        with tempfile.TemporaryDirectory(prefix="gx-scene-ir-gain-") as directory:
            cpp, binary = Path(directory) / "fixture.cpp", Path(directory) / "fixture"
            cpp.write_text(fixture)
            subprocess.run([os.environ.get("CXX", "c++"), "-std=c++17", "-O2", "-Wall", "-Wextra", "-Werror",
                            "-Wno-missing-field-initializers", f"-I{TRUNK / 'src/headers'}",
                            f"-I{TRUNK / 'src'}", str(cpp), "-o", str(binary)], check=True)
            subprocess.run([str(binary)], check=True, timeout=10)

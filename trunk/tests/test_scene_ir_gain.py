"""Compare guarded imported-IR post gain with its original Faust processor."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class SceneIrGainTests(unittest.TestCase):
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

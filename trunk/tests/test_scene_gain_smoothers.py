"""Exercise the actual scene gain DSP against the original interactive DSP."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class SceneGainSmootherTests(unittest.TestCase):
    def test_large_scene_changes_and_interactive_equivalence(self):
        header = (TRUNK / "src/headers/gx_internal_plugins.h").read_text()
        definition = "class SceneOutputLevel" + header.split("class SceneOutputLevel", 1)[1].split("\n};", 1)[0] + "\n};"
        source = (TRUNK / "src/gx_head/engine/gx_internal_plugins.cpp").read_text()
        methods = "SceneOutputLevel::SceneOutputLevel" + source.split("SceneOutputLevel::SceneOutputLevel", 1)[1].split("/****************************************************************", 1)[0]
        fixture = r'''
#include <cassert>
#include <cmath>
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include "gx_plugin.h"
#define N_(s) s
#define _(s) s
#define FAUSTFLOAT float
#define always_inline
namespace gx_system {
void atomic_set(volatile int* p, int v) { __atomic_store_n(p, v, __ATOMIC_RELEASE); }
int atomic_get(volatile int& p) { return __atomic_load_n(&p, __ATOMIC_ACQUIRE); }
}
std::map<std::string, float*> controls;
float* reg_float(const char* id, const char*, const char*, const char*, float* value,
                 float initial, float, float, float, const value_pair*) {
    *value = initial; controls[id] = value; return value;
}
namespace gx_effects {
#include "faust-generated/gain.cc"
}
''' + definition + methods + r'''
int main() {
    for (int rate : {44100, 48000, 96000}) {
        for (int count : {1, 32, 64, 256}) {
            SceneOutputLevel scene(true);
            scene.set_samplerate(rate, &scene);
            scene.register_params(ParamReg{&scene, reg_float});
            float* level = controls.at("gain.gain");
            PluginDef* legacy = gx_effects::gain::plugin();
            legacy->set_samplerate(rate, legacy);
            legacy->register_params(ParamReg{legacy, reg_float});
            float* old_level = controls.at("gain.gain");
            std::vector<float> input(count), actual(count), expected(count);
            for (int i = 0; i < count; ++i) input[i] = .1f + .01f * std::sin(.4f*i);
            // Interactive knobs and the audible set_scene path retain exactly
            // the old smoother, including edits while the prior ramp is active.
            for (float db : {0.f, 40.f, -40.f, 12.f, -24.f}) {
                *level = *old_level = db;
                for (int block = 0; block < 20; ++block) {
                    scene.mono_audio(count, input.data(), actual.data(), &scene);
                    legacy->mono_audio(count, input.data(), expected.data(), legacy);
                    assert(actual == expected);
                }
            }
            legacy->delete_instance(legacy);
            for (float db : {-40.f, 40.f, -24.f, 0.f}) {
                *level = db;
                scene.suspend_scene_smoother_for_control();
                scene.mono_audio(count, input.data(), actual.data(), &scene);
                assert(actual == input); // accumulator cannot be touched by RT
                scene.finish_scene_smoother_control(true);
                actual = input; // also exercise the normal in-place path
                scene.mono_audio(count, actual.data(), actual.data(), &scene);
                for (int i = 0; i < count; ++i)
                    assert(std::abs(actual[i] - input[i] * std::pow(10.f, .05f*db)) < 1.e-5f);
                *level = -db;
                scene.suspend_scene_smoother_for_control();
                scene.finish_scene_smoother_control(false); // timeout does not snap
                scene.request_scene_smoother_snap();
                scene.mono_audio(count, input.data(), actual.data(), &scene);
                assert(scene.finish_scene_smoother_snap());
                for (int i = 0; i < count; ++i)
                    assert(std::abs(actual[i] - input[i] * std::pow(10.f, -.05f*db)) < 1.e-5f);
            }
            scene.request_scene_smoother_snap();
            assert(!scene.finish_scene_smoother_snap()); // bypass/timeout cancels
            assert(!scene.is_scene_smoother_snap_pending());
        }
    }
}
'''
        with tempfile.TemporaryDirectory(prefix="gx-scene-gain-") as directory:
            cpp, binary = Path(directory) / "fixture.cpp", Path(directory) / "fixture"
            cpp.write_text(fixture)
            subprocess.run([os.environ.get("CXX", "c++"), "-std=c++17", "-O2", "-Wall", "-Wextra", "-Werror",
                            "-Wno-missing-field-initializers", "-Wno-unused-parameter",
                            f"-I{TRUNK / 'src/headers'}", f"-I{TRUNK / 'src'}", str(cpp), "-o", str(binary)], check=True)
            subprocess.run([str(binary)], check=True, timeout=10)

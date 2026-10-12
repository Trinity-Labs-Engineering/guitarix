from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


TRUNK = Path(__file__).resolve().parents[1]


class EqFlatStartupTests(unittest.TestCase):
    def test_default_eqs_are_flat_from_the_first_sample(self) -> None:
        compiler = os.environ.get("CXX", "c++")
        if shutil.which(compiler) is None:
            self.skipTest(f"C++ compiler is unavailable: {compiler}")

        source = r'''
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
#include "gx_plugin.h"
#include "gx_faust_support.h"
#define _(text) (text)
namespace gx_engine {
namespace gx_effects {
#include "faust-generated/graphiceq.cc"
#include "faust-generated/selecteq.cc"
}
}

float* register_float(const char*, const char*, const char*, const char*,
                      float* value, float initial, float, float, float,
                      const value_pair*)
{
    *value = initial;
    return value;
}

int* register_int(const char*, const char*, const char*, const char*,
                  int* value, int initial, int, int, const value_pair*)
{
    *value = initial;
    return value;
}

bool* register_bool(const char*, const char*, const char*, const char*,
                    bool* value, bool initial)
{
    *value = initial;
    return value;
}

void check_flat(PluginDef* plugin, bool require_exact)
{
    ParamReg reg {plugin, register_float, register_int, register_bool};
    assert(plugin->register_params(reg) == 0);
    plugin->set_samplerate(48000, plugin);
    constexpr int count = 8192;
    std::vector<float> input(count), output(count);
    for (int i = 0; i < count; ++i)
        input[i] = 0.3f * std::sin(0.013f * i)
                 + 0.2f * std::sin(0.271f * i)
                 + ((i % 97) == 0 ? 0.1f : 0.0f);
    plugin->mono_audio(count, input.data(), output.data(), plugin);
    float maximum_error = 0.0f;
    for (int i = 0; i < count; ++i) {
        const float error = std::abs(output[i] - input[i]);
        if (error > maximum_error) maximum_error = error;
    }
    if ((require_exact && maximum_error != 0.0f)
        || maximum_error >= 2.0e-4f) {
        std::cerr << plugin->id << " flat-response error: " << maximum_error << "\n";
        std::abort();
    }
    plugin->delete_instance(plugin);
}

int main()
{
    check_flat(gx_engine::gx_effects::selecteq::plugin(), false);
    check_flat(gx_engine::gx_effects::graphiceq::plugin(), true);
}
'''
        with tempfile.TemporaryDirectory(prefix="gx-eq-flat-test-") as temp_dir:
            executable = Path(temp_dir) / "eq-flat-test"
            compiled = subprocess.run(
                [
                    compiler,
                    "-std=c++20",
                    "-O2",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-Wno-unused-parameter",
                    f"-I{TRUNK / 'src' / 'headers'}",
                    f"-I{TRUNK / 'src' / 'zita-resampler-1.1.0'}",
                    f"-I{TRUNK / 'src'}",
                    "-x", "c++", "-",
                    "-o", str(executable),
                ],
                input=source,
                capture_output=True,
                text=True,
                timeout=30,
            )
            self.assertEqual(compiled.returncode, 0, compiled.stderr)
            completed = subprocess.run(
                [str(executable)], capture_output=True, text=True, timeout=10
            )
        self.assertEqual(completed.returncode, 0, completed.stderr)


if __name__ == "__main__":
    unittest.main()

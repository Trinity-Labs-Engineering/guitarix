from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import textwrap
import unittest


TRUNK = Path(__file__).resolve().parents[1]
FIXTURE = Path(__file__).with_name("poly_pitch_shifter_fixture.cpp")


class PolyPitchShifterTests(unittest.TestCase):
    def run_cpp_fixture(self, source: Path | str, expected_stdout: str) -> None:
        compiler = os.environ.get("CXX", "c++")
        if shutil.which(compiler) is None:
            self.skipTest(f"C++ compiler is unavailable: {compiler}")

        with tempfile.TemporaryDirectory(prefix="gx-poly-pitch-test-") as temp_dir:
            executable = Path(temp_dir) / "poly-pitch-shifter-test"
            if isinstance(source, str):
                fixture = Path(temp_dir) / "fixture.cpp"
                fixture.write_text(source)
            else:
                fixture = source
            subprocess.run(
                [
                    compiler,
                    "-std=c++11",
                    "-O2",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    # The fixture deliberately backs its global new/delete
                    # probes with malloc/free to count callback allocations.
                    "-Wno-mismatched-new-delete",
                    f"-I{TRUNK / 'src' / 'headers'}",
                    str(fixture),
                    "-o",
                    str(executable),
                ],
                check=True,
            )
            completed = subprocess.run(
                [str(executable)],
                capture_output=True,
                text=True,
                timeout=20,
            )

        self.assertEqual(completed.returncode, 0, completed.stderr)
        self.assertEqual(completed.stdout.strip(), expected_stdout)

    def test_tone3000_pitch_audio_and_realtime_contract(self) -> None:
        self.run_cpp_fixture(FIXTURE, "poly-pitch-shifter-ok")

    def test_plugin_wet_dry_gain_mapping_and_unavailable_dsp(self) -> None:
        # Compile the production audio method with its real DSP and a small
        # field-only host stub; this checks the plugin's percent-to-gain
        # conversion and !ready fallback without needing GTK/JACK headers.
        plugin_source = (TRUNK / "src/gx_head/engine/gx_internal_plugins.cpp").read_text()
        start = plugin_source.index("void always_inline smbPitchShift::PitchShift(")
        end = plugin_source.index("\nint smbPitchShift::register_par(", start)
        audio_method = plugin_source[start:end]
        source = textwrap.dedent("""\
            #include <cassert>
            #include <cmath>
            #include <iostream>
            #include <vector>
            #include "gx_poly_pitch_shifter.h"

            namespace gx_engine {
            class smbPitchShift {
            public:
                PolyphonicPitchShifter pitch_shifter;
                bool ready = false;
                float semitones = 7.0f;
                float a = 1.0f, b = 1.0f, c = 1.0f, d = 1.0f, l = 0.0f;
                float wet = 100.0f, dry = 0.0f;
                int octave = 0;
                void PitchShift(int count, float *indata, float *outdata);
            };
            #define always_inline
        """) + audio_method + textwrap.dedent("""\
            #undef always_inline
            } // namespace gx_engine

            int main()
            {
                const int length = 4096;
                std::vector<float> input(length), wet(length), output(length);
                for (int i = 0; i < length; ++i)
                    input[i] = 0.5f*std::sin(i*0.17f) + 0.1f;
                const double ratio = std::pow(2.0, 7.0/12.0);
                gx_engine::PolyphonicPitchShifter reference;
                reference.prepare(48000, 1);
                reference.process(input.data(), wet.data(), length, ratio, 1.0f, 0.0f);
                gx_engine::smbPitchShift plugin;
                plugin.pitch_shifter.prepare(48000, 1);
                for (bool ready : {false, true}) {
                    plugin.ready = ready;
                    for (bool compensate : {false, true}) {
                        plugin.l = compensate ? 1.0f : 0.0f;
                        const int delay = ready && compensate
                            ? reference.latency_samples() : 0;
                        for (bool in_place : {false, true}) {
                            for (float wet_percent : {0.0f, 25.0f, 50.0f, 100.0f}) {
                                for (float dry_percent : {0.0f, 25.0f, 50.0f, 100.0f}) {
                                    plugin.wet = wet_percent;
                                    plugin.dry = dry_percent;
                                    plugin.pitch_shifter.reset();
                                    if (in_place) output = input;
                                    else std::fill(output.begin(), output.end(), -123.0f);
                                    float* source = in_place ? output.data() : input.data();
                                    plugin.PitchShift(length, source, output.data());
                                    for (int i = 0; i < length; ++i) {
                                        const float raw = i >= delay ? input[i - delay] : 0.0f;
                                        const float dry_sample = 0.01f*dry_percent*raw;
                                        const float expected = dry_sample
                                            + (ready ? 0.01f*wet_percent*wet[i] : 0.0f);
                                        assert(std::abs(output[i] - expected) < 2.0e-6f);
                                        if (!ready || wet_percent == 0.0f)
                                            assert(output[i] == dry_sample);
                                    }
                                }
                            }
                        }
                    }
                    float sentinel = 0.37f;
                    plugin.PitchShift(0, nullptr, &sentinel);
                    assert(sentinel == 0.37f);
                }
                std::cout << "poly-pitch-plugin-mix-ok\\n";
            }
        """)
        self.run_cpp_fixture(source, "poly-pitch-plugin-mix-ok")


if __name__ == "__main__":
    unittest.main()

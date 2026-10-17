from pathlib import Path
import os
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class SceneValueTests(unittest.TestCase):
    def test_float_endpoints_and_invalid_scalar_types(self):
        self.run_fixture("scene_value_fixture.cpp")

    def test_nam_silence_bias_is_removed_before_downstream_filters(self):
        self.run_fixture("nam_dc_fixture.cpp")

    def test_electra_clipping_curve_is_continuous_at_silence(self):
        self.run_fixture("clipping_origin_fixture.cpp")

    def test_nam_dc_header_compiles_after_faust_support(self):
        self.run_fixture("nam_dc_fixture.cpp", faust_macros=True)

    def test_scene_value_header_compiles_after_faust_support(self):
        self.run_fixture("scene_value_fixture.cpp", faust_macros=True)

    def run_fixture(self, fixture, *, faust_macros=False):
        with tempfile.TemporaryDirectory(prefix="gx-scene-value-") as directory:
            binary = Path(directory) / "scene-value"
            # Reproduce the application's Faust include order and C++20
            # optimisation flags without building the Guitarix application.
            flags = (["-std=c++20", "-O3", "-fno-math-errno", "-fno-signed-zeros",
                      "-DGX_TEST_FAUST_MACROS", "-Wno-unused-parameter"]
                     if faust_macros else ["-std=c++17"])
            result = subprocess.run([
                os.environ.get("CXX", "c++"), *flags, "-Wall", "-Wextra", "-Werror",
                f"-I{TRUNK / 'src/headers'}", f"-I{TRUNK / 'src/gx_head/engine/tabels'}",
                f"-I{TRUNK / 'src/zita-resampler-1.1.0'}",
                str(Path(__file__).with_name(fixture)),
                "-o", str(binary)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(binary)], check=True, timeout=5)

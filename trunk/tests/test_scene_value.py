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

    def run_fixture(self, fixture):
        with tempfile.TemporaryDirectory(prefix="gx-scene-value-") as directory:
            binary = Path(directory) / "scene-value"
            result = subprocess.run([
                os.environ.get("CXX", "c++"), "-std=c++17", "-Wall", "-Wextra", "-Werror",
                f"-I{TRUNK / 'src/headers'}", f"-I{TRUNK / 'src/gx_head/engine/tabels'}",
                str(Path(__file__).with_name(fixture)),
                "-o", str(binary)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(binary)], check=True, timeout=5)

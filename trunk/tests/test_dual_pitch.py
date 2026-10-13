from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class DualPitchTests(unittest.TestCase):
    def test_production_plugin_audio_and_realtime_contract(self):
        compiler = os.environ.get("CXX", "c++")
        if not shutil.which(compiler):
            self.skipTest("C++ compiler unavailable")
        with tempfile.TemporaryDirectory(prefix="dual-pitch-") as directory:
            executable = Path(directory) / "dual-pitch"
            compiled = subprocess.run([
                compiler, "-std=c++11", "-O2", "-Wall", "-Wextra", "-Werror", "-Wno-unused-parameter",
                "-Wno-mismatched-new-delete", f"-I{TRUNK / 'src/headers'}",
                str(TRUNK / "tests/dual_pitch_fixture.cpp"), str(TRUNK / "src/plugins/dual_pitch.cc"),
                "-o", str(executable),
            ], capture_output=True, text=True, timeout=60)
            self.assertEqual(compiled.returncode, 0, compiled.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout.strip(), "dual-pitch-ok")


if __name__ == "__main__":
    unittest.main()

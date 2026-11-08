"""Scene re-entry must use the current transistor circuit's operating point."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class TransistorSceneResetTests(unittest.TestCase):
    def test_current_controls_match_fresh_circuit_without_old_bias(self):
        compiler = os.environ.get("CXX", "c++")
        if shutil.which(compiler) is None:
            self.skipTest("C++ compiler unavailable")
        with tempfile.TemporaryDirectory() as directory:
            executable = Path(directory) / "reset"
            build = subprocess.run([
                compiler, "-std=c++20", "-O2", f"-I{TRUNK / 'src'}",
                f"-I{TRUNK / 'src/headers'}",
                str(TRUNK / "tests/transistor_scene_reset_fixture.cpp"),
                "-o", str(executable),
            ], capture_output=True, text=True, timeout=60)
            self.assertEqual(build.returncode, 0, build.stderr)
            run = subprocess.run([str(executable)], capture_output=True, text=True, timeout=60)
            self.assertEqual(run.returncode, 0, run.stdout + run.stderr)


if __name__ == "__main__":
    unittest.main()

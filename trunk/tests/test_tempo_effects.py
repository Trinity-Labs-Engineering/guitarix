from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class TempoEffectsTests(unittest.TestCase):
    def test_audio_behavior(self):
        compiler = os.environ.get("CXX", "c++")
        if not shutil.which(compiler):
            self.skipTest("C++ compiler unavailable")
        with tempfile.TemporaryDirectory(prefix="gx-tempo-test-") as directory:
            executable = Path(directory) / "tempo-effects"
            result = subprocess.run([
                compiler, "-std=c++20", "-O1", "-g", "-fsanitize=address,undefined",
                '-D_(x)=x',
                f"-I{TRUNK / 'src/headers'}", f"-I{TRUNK / 'src/zita-resampler-1.1.0'}",
                f"-I{TRUNK / 'src'}", f"-I{TRUNK / 'src/gx_head/engine'}",
                str(TRUNK / "tests/tempo_effects_fixture.cpp"),
                str(TRUNK / "src/plugins/generated/flanger_gx.cc"),
                str(TRUNK / "src/plugins/reversedelay.cc"),
                "-o", str(executable),
            ], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stderr)


if __name__ == "__main__":
    unittest.main()

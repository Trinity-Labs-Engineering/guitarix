import os
from pathlib import Path
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class AmpInstanceTests(unittest.TestCase):
    def test_amp_eq_and_preamp_are_independent_of_both_pedal_instances(self):
        with tempfile.TemporaryDirectory(prefix="gx-amp-instances-") as tmp:
            binary = Path(tmp) / "amp-instances"
            built = subprocess.run([
                os.environ.get("CXX", "c++"), "-std=c++17", "-O1",
                f"-I{TRUNK / 'src/headers'}", f"-I{TRUNK / 'src/faust-generated'}",
                "-I/usr/include/eigen3", str(Path(__file__).with_name("amp_instance_fixture.cpp")),
                str(TRUNK / "src/plugins/jcm800pre.cpp"), "-lzita-resampler", "-o", str(binary),
            ], capture_output=True, text=True, timeout=90)
            self.assertEqual(built.returncode, 0, built.stderr)
            result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=20)
            self.assertEqual(result.returncode, 0, result.stderr)


if __name__ == "__main__":
    unittest.main()

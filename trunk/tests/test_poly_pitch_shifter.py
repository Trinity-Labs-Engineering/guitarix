from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


TRUNK = Path(__file__).resolve().parents[1]
FIXTURE = Path(__file__).with_name("poly_pitch_shifter_fixture.cpp")


class PolyPitchShifterTests(unittest.TestCase):
    def test_polyphonic_octaves_latency_and_rt_contract(self) -> None:
        compiler = os.environ.get("CXX", "c++")
        if shutil.which(compiler) is None:
            self.skipTest(f"C++ compiler is unavailable: {compiler}")

        with tempfile.TemporaryDirectory(prefix="gx-poly-pitch-test-") as temp_dir:
            executable = Path(temp_dir) / "poly-pitch-shifter-test"
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
                    str(FIXTURE),
                    "-o",
                    str(executable),
                ],
                check=True,
            )
            completed = subprocess.run(
                [str(executable)],
                check=True,
                capture_output=True,
                text=True,
                timeout=20,
            )

        self.assertEqual(completed.stdout.strip(), "poly-pitch-shifter-ok")


if __name__ == "__main__":
    unittest.main()

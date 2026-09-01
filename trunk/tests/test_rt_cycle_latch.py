from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


TRUNK = Path(__file__).resolve().parents[1]
FIXTURE = Path(__file__).with_name("rt_cycle_latch_fixture.cpp")


class RtCycleLatchTests(unittest.TestCase):
    def test_generation_predicate_rejects_stale_wakes_and_closes_arm_races(
        self,
    ) -> None:
        compiler = os.environ.get("CXX", "c++")
        if shutil.which(compiler) is None:
            self.skipTest(f"C++ compiler is unavailable: {compiler}")

        with tempfile.TemporaryDirectory(prefix="gx-rt-cycle-latch-test-") as temp_dir:
            executable = Path(temp_dir) / "rt-cycle-latch-test"
            subprocess.run(
                [
                    compiler,
                    "-std=c++11",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-pthread",
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
                timeout=10,
            )

        self.assertEqual(completed.stdout.strip(), "rt-cycle-latch-ok")


if __name__ == "__main__":
    unittest.main()

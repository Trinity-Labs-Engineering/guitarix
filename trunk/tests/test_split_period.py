import os
from pathlib import Path
import subprocess
import tempfile
import unittest


class SplitPeriodTests(unittest.TestCase):
    def test_consecutive_missing_and_overwritten_frames(self):
        tests = Path(__file__).resolve().parent
        with tempfile.TemporaryDirectory() as directory:
            executable = Path(directory) / "split-period"
            subprocess.run([
                os.environ.get("CXX", "c++"), "-std=c++17", "-Wall", "-Werror",
                "-I", str(tests.parent / "src/headers"),
                str(tests / "split_period_fixture.cpp"), "-o", str(executable),
            ], check=True)
            subprocess.run([str(executable)], check=True)

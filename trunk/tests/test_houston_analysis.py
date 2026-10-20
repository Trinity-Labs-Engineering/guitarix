"""Compile and exercise the JUCE-free analysis code linked into Guitarix."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
ENGINE = ROOT / "src/gx_head/engine"
ANALYSIS = ENGINE / "houston"
JSON = ROOT / "src/NAM/NeuralAmpModelerCore/Dependencies/nlohmann"


@unittest.skipUnless(shutil.which("g++"), "g++ required")
class HoustonAnalysisTests(unittest.TestCase):
    def test_core(self):
        with tempfile.TemporaryDirectory() as directory:
            binary = Path(directory) / "analysis-tests"
            subprocess.run([
                "g++", "-std=c++20", "-O2", "-pthread", "-I", str(ENGINE),
                "-I", str(JSON), str(ROOT / "tests/houston_analysis_tests.cpp"),
                str(ANALYSIS / "Analysis.cpp"), str(ANALYSIS / "yin/PitchTracker.cpp"),
                str(ANALYSIS / "yin/YinTracker.cpp"), "-o", str(binary),
            ], check=True)
            subprocess.run([str(binary)], check=True, timeout=60)

    def test_detector_matches_houston_source(self):
        source = ROOT.parents[1] / "houston/plugins/yin-tuner/src/yin"
        if not source.exists():
            self.skipTest("Houston sibling checkout required for provenance check")
        for name in ("YinTracker.cpp", "YinTracker.h", "PitchTracker.cpp", "PitchTracker.h"):
            self.assertEqual((source / name).read_bytes(), (ANALYSIS / "yin" / name).read_bytes())


if __name__ == "__main__":
    unittest.main()

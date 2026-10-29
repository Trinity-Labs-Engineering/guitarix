"""Exercise the actual mono plugin callbacks and Houston/DSP parameter contract."""
from pathlib import Path
import json
import os
import shutil
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class FaustEffectsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        compiler = os.environ.get("CXX", "c++")
        if not shutil.which(compiler):
            raise unittest.SkipTest("C++ compiler unavailable")
        cls.directory = tempfile.TemporaryDirectory(prefix="gx-faust-effects-")
        cls.addClassCleanup(cls.directory.cleanup)
        cls.executable = Path(cls.directory.name) / "faust-effects"
        subprocess.run([
            compiler, "-std=c++20", "-O1", "-g", "-fsanitize=address,undefined",
            "-fno-sanitize-recover=all", '-D_(x)=x',
            f"-I{TRUNK / 'src/headers'}", f"-I{TRUNK / 'src'}",
            str(TRUNK / "tests/faust_effects_fixture.cpp"), "-o", str(cls.executable),
        ], capture_output=True, text=True, check=True, timeout=180)

    def test_audio_behavior(self):
        result = subprocess.run([str(self.executable)], capture_output=True, text=True, timeout=180)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_houston_parameters_match_registered_dsp(self):
        catalog = TRUNK.parents[1] / "houston/models/objects/base/lib/pytarix/models/pedals/lib"
        if not catalog.is_dir():
            self.skipTest("Houston sibling checkout unavailable")
        result = subprocess.run([str(self.executable), "--metadata"], capture_output=True, text=True, check=True)
        registered = {}
        for line in result.stdout.splitlines():
            key, *values = line.split("\t")
            registered[key] = list(map(float, values))
        processors = {key.split(".")[0] for key in registered}
        exposed = {}
        for name in ("Reverb", "Modulation", "Drive"):
            for pedal in json.loads((catalog / f"{name}.json").read_text()):
                if pedal.get("id") in processors:
                    for pot in pedal["pots"]:
                        self.assertNotIn(pot["gtx_name"], exposed)
                        exposed[pot["gtx_name"]] = [pot[k] for k in ("default_val", "min_val", "max_val", "step")]
        self.assertEqual(set(exposed), set(registered))
        for key in registered:
            for actual, expected in zip(exposed[key], registered[key]):
                self.assertAlmostEqual(actual, expected, places=5, msg=key)


if __name__ == "__main__":
    unittest.main()

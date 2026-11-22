from pathlib import Path
import os
import json
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
                compiler, "-std=c++20", "-O1", "-g", "-fsanitize=address,undefined,float-divide-by-zero", "-fno-sanitize-recover=all",
                '-D_(x)=x',
                f"-I{TRUNK / 'src/headers'}", f"-I{TRUNK / 'src/zita-resampler-1.1.0'}",
                f"-I{TRUNK / 'src'}", f"-I{TRUNK / 'src/gx_head/engine'}",
                str(TRUNK / "tests/tempo_effects_fixture.cpp"),
                str(TRUNK / "src/plugins/generated/flanger_gx.cc"),
                str(TRUNK / "src/plugins/reversedelay.cc"),
                "-o", str(executable),
            ], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stderr)
            catalog = TRUNK.parents[1] / "houston/models/objects/base/lib/pytarix/models/pedals/lib/Delay.json"
            defaults = {"dide.highpass": 120, "dide.howpass": 12000}
            # Exercise Houston's actual metadata in the bundle workspace while
            # keeping Guitarix's tests runnable in a standalone checkout.
            if catalog.is_file():
                delay = next(pedal for pedal in json.loads(catalog.read_text()) if pedal["name"] == "Digital Delay")
                defaults = {pot["gtx_name"]: pot["default_val"] for pot in delay["pots"]}
            result = subprocess.run([str(executable), str(defaults["dide.highpass"]), str(defaults["dide.howpass"])], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            registered = {}
            for line in result.stdout.splitlines():
                if line.startswith("META\t"):
                    _, key, *values = line.split("\t")
                    registered[key] = list(map(float, values))
            self.assertEqual(len(registered), 5)
            if catalog.is_file():
                expected = {pot["gtx_name"]: [pot[k] for k in ("default_val", "min_val", "max_val", "step")]
                            for pedal in json.loads(catalog.read_text()) for pot in pedal["pots"]
                            if pot["gtx_name"].endswith(".bpm")}
                self.assertEqual(set(registered), set(expected))
                for key in registered:
                    for actual, wanted in zip(registered[key], expected[key]):
                        self.assertAlmostEqual(actual, wanted, places=5, msg=key)


if __name__ == "__main__":
    unittest.main()

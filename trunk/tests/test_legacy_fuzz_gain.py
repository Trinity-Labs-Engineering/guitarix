"""Check fixed gain compensation without changing the legacy fuzz timbres."""
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]
EFFECTS = "axface bfuzz ffreak foxeylady fumaster fuzzdrive fuzzface fuzzfacefm fuzzfacerm ruin susta muff bmp buzz hornet scream astrofuzz".split()
SOURCES = [TRUNK / "src" / ("faust-generated" if name == "fuzzface" else "plugins/generated") / (name + ".cc")
           for name in EFFECTS]


class LegacyFuzzGainTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory = tempfile.TemporaryDirectory(prefix="gx-legacy-fuzz-")
        cls.addClassCleanup(cls.directory.cleanup)
        directory = Path(cls.directory.name)
        cls.executable = directory / "legacy-fuzz"
        (directory / "legacy_fuzz_plugins.h").write_text("\n".join(
            (f'namespace {name} {{ PluginDef* plugin(); }}' if name == "fuzzface" else
             f'namespace pluginlib::{name} {{ PluginDef* plugin(); }}')
            for name in EFFECTS
        ))
        wrappers = []
        for source in SOURCES:
            wrapper = directory / source.name
            support = '#include "gx_plugin.h"\n#include "gx_faust_support.h"\n' if source.stem == "fuzzface" else ""
            wrapper.write_text('#include <cstring>\n' + support +
                               f'#include "{source}"\n')
            wrappers.append(str(wrapper))
        resampler = directory / "gx_resampler.cpp"
        resampler.write_text((TRUNK / "src/gx_head/engine/gx_resampler.cpp").read_text().replace(
            '#include "engine.h"',
            '#include <cassert>\n#include <cmath>\n#include <cstring>\n#include "gx_resampler.h"',
        ))
        result = subprocess.run([
            os.environ.get("CXX", "c++"), "-std=c++20", "-O1", "-g",
            "-fsanitize=address,undefined", "-fno-sanitize-recover=all", '-D_(x)=x',
            f"-I{directory}", f"-I{TRUNK / 'src/headers'}", f"-I{TRUNK / 'src/plugins'}",
            f"-I{TRUNK / 'src/gx_head/engine/tabels'}", f"-I{TRUNK / 'src/zita-resampler-1.1.0'}",
            str(TRUNK / "tests/legacy_fuzz_gain_fixture.cpp"), *wrappers, str(resampler),
            str(TRUNK / "src/zita-resampler-1.1.0/resampler.cc"),
            str(TRUNK / "src/zita-resampler-1.1.0/resampler-table.cc"),
            "-o", str(cls.executable),
        ], capture_output=True, text=True, timeout=180)
        if result.returncode:
            raise RuntimeError(result.stderr)

    def test_audio_mix_and_block_sizes(self):
        result = subprocess.run([str(self.executable)], capture_output=True, text=True, timeout=180)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_minus_20_db_and_preserved_harmonics(self):
        # Measurements from the uncompensated callbacks at 3e5ff1db. RMS must drop
        # by x10, while relative third/fifth harmonics retain the driven timbre.
        baseline = json.loads((TRUNK / "tests/legacy_fuzz_baseline.json").read_text())
        result = subprocess.run([str(self.executable), "--measure"], capture_output=True, text=True, check=True, timeout=180)
        actual = {}
        for line in result.stdout.splitlines():
            name, rate, amplitude, *metrics = line.split()
            actual[f"{name}/{rate}/{amplitude}"] = list(map(float, metrics))
        self.assertEqual(set(actual), set(baseline))
        for key, (rms, third, fifth) in baseline.items():
            for measured, expected in zip(actual[key], (0.1 * rms, third, fifth)):
                self.assertAlmostEqual(measured, expected, delta=max(2e-6, abs(expected) * 2e-4), msg=key)


if __name__ == "__main__":
    unittest.main()

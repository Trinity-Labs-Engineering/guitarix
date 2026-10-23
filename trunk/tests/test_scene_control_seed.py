from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class SceneControlSeedTests(unittest.TestCase):
    def test_scene_reset_seeds_controls_and_clears_signal_history(self) -> None:
        compiler = os.environ.get("CXX", "c++")
        if shutil.which(compiler) is None:
            self.skipTest(f"C++ compiler is unavailable: {compiler}")
        with tempfile.TemporaryDirectory(prefix="gx-scene-control-seed-") as directory:
            executable = Path(directory) / "scene-control-seed"
            resampler = Path(directory) / "gx_resampler.cpp"
            resampler.write_text((TRUNK / "src/gx_head/engine/gx_resampler.cpp").read_text().replace(
                '#include "engine.h"',
                '#include <cassert>\n#include <cmath>\n#include <cstring>\n#include "gx_resampler.h"',
            ))
            compiled = subprocess.run(
                [compiler, "-std=c++20", "-O2", "-Wall", "-Wextra", "-Werror", "-D_(x)=x",
                 "-Wno-unused-parameter", f"-I{TRUNK / 'src' / 'headers'}",
                 f"-I{TRUNK / 'src' / 'zita-resampler-1.1.0'}", f"-I{TRUNK / 'src'}",
                 f"-I{TRUNK / 'src/plugins'}", f"-I{TRUNK / 'src/gx_head/engine/tabels'}",
                 str(TRUNK / "tests" / "scene_control_seed_fixture.cpp"),
                 str(TRUNK / "src/plugins/generated/bassboom.cc"),
                 str(TRUNK / "src/plugins/generated/ts9sim.cc"),
                 str(resampler), str(TRUNK / "src/zita-resampler-1.1.0/resampler.cc"),
                 str(TRUNK / "src/zita-resampler-1.1.0/resampler-table.cc"),
                 "-o", str(executable)],
                capture_output=True, text=True, timeout=30,
            )
            self.assertEqual(compiled.returncode, 0, compiled.stderr)
            completed = subprocess.run(
                [str(executable)], capture_output=True, text=True, timeout=10,
            )
            self.assertEqual(completed.returncode, 0, completed.stderr)


if __name__ == "__main__":
    unittest.main()

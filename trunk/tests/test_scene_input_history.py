"""Compile the actual RT/control handoff helper and exercise its ownership rules."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


class SceneInputHistoryTests(unittest.TestCase):
    def test_history_queue_and_render_handoff(self):
        compiler = shutil.which("g++")
        if compiler is None:
            self.skipTest("g++ is required for the native history handoff test")
        root = Path(__file__).resolve().parents[1]
        with tempfile.TemporaryDirectory(prefix="gx-scene-history-") as directory:
            executable = Path(directory) / "history-fixture"
            subprocess.run([
                compiler, "-std=c++17", "-O2", "-Wall", "-Wextra", "-pthread",
                "-I", str(root / "src/headers"),
                str(root / "tests/scene_input_history_fixture.cpp"), "-o", str(executable),
            ], check=True, capture_output=True, text=True)
            result = subprocess.run([str(executable)], check=True, capture_output=True,
                                    text=True, timeout=15)
            self.assertIn("DSP ownership pass", result.stdout)


if __name__ == "__main__":
    unittest.main()

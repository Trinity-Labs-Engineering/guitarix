from pathlib import Path
import os
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class SceneSmootherTests(unittest.TestCase):
    def test_continuous_dsp_and_actual_request_acknowledgement(self):
        source = (TRUNK / "src/gx_head/engine/gx_engine.cpp").read_text()
        body = "bool GxEngine::preset_muted_scene_smoothers(" + source.split(
            "bool GxEngine::preset_muted_scene_smoothers(", 1)[1].split(
                "void GxEngine::load_static_plugins()", 1)[0]
        with tempfile.TemporaryDirectory(prefix="gx-scene-smoother-") as directory:
            path = Path(directory)
            (path / "scene_smoother_under_test.h").write_text(body)
            binary = path / "scene-smoother"
            result = subprocess.run([os.environ.get("CXX", "c++"), "-std=c++17", "-Wall", "-Wextra", "-Werror",
                                     f"-I{path}", str(Path(__file__).with_name("scene_smoother_fixture.cpp")),
                                     "-o", str(binary)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=5)
            self.assertEqual(result.returncode, 0, result.stderr)

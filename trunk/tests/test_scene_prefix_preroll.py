from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class ScenePrefixPrerollTests(unittest.TestCase):
    def test_production_prefix_replay_is_guarded_causal_and_bounded(self):
        compiler = os.environ.get("CXX", "c++")
        if shutil.which(compiler) is None:
            self.skipTest(f"C++ compiler is unavailable: {compiler}")
        source = (TRUNK / "src/gx_head/engine/gx_engine_audio.cpp").read_text()
        start = source.index("namespace {\n// Only memoryless/small-filter")
        end = source.index("#ifdef GUITARIX_AS_PLUGIN\nvoid __rt_func StereoModuleChain::process", start)
        header = (TRUNK / "src/headers/gx_modulesequencer.h").read_text()
        status_start = header.index("struct ScenePrefixPrerollStatus {")
        status_end = header.index("class MonoModuleChain:", status_start)
        with tempfile.TemporaryDirectory(prefix="gx-scene-prefix-") as directory:
            directory = Path(directory)
            (directory / "scene_prefix_under_test.h").write_text(source[start:end])
            (directory / "scene_prefix_status_under_test.h").write_text(header[status_start:status_end])
            executable = directory / "scene-prefix"
            built = subprocess.run([
                compiler, "-std=c++17", "-O2", "-Wall", "-Wextra",
                f"-I{TRUNK / 'src/headers'}", f"-I{directory}",
                str(TRUNK / "tests/scene_prefix_preroll_fixture.cpp"), "-o", str(executable),
            ], capture_output=True, text=True, timeout=30)
            self.assertEqual(built.returncode, 0, built.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout.strip(), "scene-prefix-preroll-ok")


if __name__ == "__main__":
    unittest.main()

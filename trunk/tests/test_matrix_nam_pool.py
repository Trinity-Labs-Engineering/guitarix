from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class MatrixNamPoolTests(unittest.TestCase):
    def test_profile_switches_reuse_prepared_models_without_intermediate_prewarming(self):
        compiler = os.environ.get("CXX", "c++")
        if not shutil.which(compiler):
            self.skipTest("C++ compiler unavailable")
        source = (TRUNK / "src/gx_head/engine/gx_neural_plugins.cpp").read_text()
        sections = []
        for start, end in (
            ("void NeuralAmpMulti::begin_scene_parameter_batch(", "bool NeuralAmpMulti::suspend_scene_smoother_for_control("),
            ("std::unique_ptr<nam::DSP> NeuralAmpMulti::take_cached_model(", "inline void NeuralAmpMulti::clear_state_f()"),
            ("void NeuralAmpMulti::load_nam_afile_impl()", "void NeuralAmpMulti::create_nam_afilelist()"),
        ):
            sections.append(start + source.split(start, 1)[1].split(end, 1)[0])
        with tempfile.TemporaryDirectory(prefix="gx-matrix-nam-pool-") as directory:
            path = Path(directory)
            (path / "nam_pool_under_test.h").write_text("\n".join(sections))
            executable = path / "nam-pool"
            compiled = subprocess.run(
                [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", f"-I{path}",
                 str(Path(__file__).with_name("matrix_nam_pool_fixture.cpp")), "-o", str(executable)],
                capture_output=True, text=True,
            )
            self.assertEqual(compiled.returncode, 0, compiled.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout.strip(), "matrix-nam-pool-ok")


if __name__ == "__main__":
    unittest.main()

from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class MatrixRoutingTests(unittest.TestCase):
    def test_actual_audio_loop_and_independent_generated_instances(self):
        compiler = os.environ.get("CXX", "c++")
        if not shutil.which(compiler):
            self.skipTest("C++ compiler unavailable")
        source = (TRUNK / "src/gx_head/engine/gx_engine_audio.cpp").read_text()
        process = source.split("void __rt_func MonoModuleChain::process(", 1)[1]
        process = process.split("    if (rm == ramp_mode_off)", 1)[0]
        process = "void __rt_func MonoModuleChain::process(" + process + "}\n"
        loader = (TRUNK / "src/gx_head/engine/gx_pluginloader.cpp").read_text()
        validation = loader.split("bool PluginList::validate_matrix(", 1)[1]
        validation = "bool PluginList::validate_matrix(" + validation.split(
            "void PluginList::ordered_processing_stereo_list", 1)[0]
        with tempfile.TemporaryDirectory(prefix="gx-matrix-") as directory:
            path = Path(directory)
            (path / "matrix_process_under_test.h").write_text(process)
            (path / "matrix_validation_under_test.h").write_text(validation)
            executable = path / "matrix-routing"
            compiled = subprocess.run(
                [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                 f"-I{path}", f"-I{TRUNK / 'src/headers'}",
                 f"-I{TRUNK / 'src/faust-generated'}",
                 str(Path(__file__).with_name("matrix_routing_fixture.cpp")),
                 "-o", str(executable)], capture_output=True, text=True,
            )
            self.assertEqual(compiled.returncode, 0, compiled.stderr)
            result = subprocess.run([str(executable)], capture_output=True,
                                    text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout.strip(), "matrix-routing-ok")


if __name__ == "__main__":
    unittest.main()

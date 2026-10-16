from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class MatrixTransferTests(unittest.TestCase):
    def test_migration_drains_old_callbacks_before_activation_and_retries(self):
        compiler = os.environ.get("CXX", "c++")
        if not shutil.which(compiler):
            self.skipTest("C++ compiler unavailable")
        source = (TRUNK / "src/gx_head/engine/gx_engine_audio.cpp").read_text()
        method = source.split("bool ModuleSequencer::commit_module_lists(", 1)[1]
        method = "bool ModuleSequencer::commit_module_lists(" + method.split(
            "int ModuleSequencer::sporadic_interval", 1)[0]
        with tempfile.TemporaryDirectory(prefix="gx-matrix-transfer-") as directory:
            path = Path(directory)
            (path / "matrix_transfer_under_test.h").write_text(method)
            executable = path / "matrix-transfer"
            compiled = subprocess.run(
                [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", f"-I{path}",
                 str(Path(__file__).with_name("matrix_transfer_fixture.cpp")),
                 "-o", str(executable)], capture_output=True, text=True,
            )
            self.assertEqual(compiled.returncode, 0, compiled.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout.strip(), "matrix-transfer-ok")


if __name__ == "__main__":
    unittest.main()

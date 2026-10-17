from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class MatrixBankTests(unittest.TestCase):
    def test_rpc_banks_retain_all_five_snapshots_across_ab_promotions(self):
        source = (TRUNK / "src/gx_head/engine/jsonrpc.cpp").read_text()
        sections = []
        for start, end in (("    FUNCTION(matrix_prepare_next)", "    FUNCTION(matrix_promote)"),
                           ("    FUNCTION(matrix_promote)", "    FUNCTION(matrix_release)")):
            sections.append(start + source.split(start, 1)[1].split(end, 1)[0] + "break;\n")
        with tempfile.TemporaryDirectory(prefix="gx-bank-rpc-") as directory:
            path = Path(directory)
            (path / "matrix_bank_rpc_under_test.h").write_text("\n".join(sections))
            binary = path / "matrix-bank-rpc"
            result = subprocess.run([os.environ.get("CXX", "c++"), "-std=c++17", "-Wall", "-Wextra", "-Werror",
                                     f"-I{path}", f"-I{TRUNK / 'src/headers'}",
                                     str(Path(__file__).with_name("matrix_bank_lifecycle_fixture.cpp")), "-o", str(binary)],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("two images, five snapshots", result.stdout)

    def test_nam_cutover_has_no_lifecycle_or_allocation_and_preserves_ownership(self):
        compiler = os.environ.get("CXX", "c++")
        if not shutil.which(compiler):
            self.skipTest("C++ compiler unavailable")
        source = (TRUNK / "src/gx_head/engine/gx_neural_plugins.cpp").read_text()
        sections = []
        for start, end in (
            ("bool NeuralAmp::compile_matrix_model(", "void NeuralAmp::select_matrix_model("),
            ("void NeuralAmp::preset_scene_smoother_targets()", "void NeuralAmp::clear_state_f_static("),
            ("void always_inline NeuralAmp::compute(", "void NeuralAmp::compute_static("),
        ):
            sections.append(start + source.split(start, 1)[1].split(end, 1)[0])
        with tempfile.TemporaryDirectory(prefix="gx-bank-nam-") as directory:
            path = Path(directory)
            (path / "matrix_bank_nam_under_test.h").write_text("\n".join(sections))
            binary = path / "matrix-bank-nam"
            result = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", f"-I{path}",
                                     str(Path(__file__).with_name("matrix_bank_nam_fixture.cpp")), "-o", str(binary)],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("1000 transitions", result.stdout)

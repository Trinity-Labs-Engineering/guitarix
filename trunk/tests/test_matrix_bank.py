from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class MatrixBankTests(unittest.TestCase):
    def test_selector_proxies_use_resident_children_and_bank_positions(self):
        source = (TRUNK / "src/gx_head/engine/jsonrpc.cpp").read_text()
        compiler = source.split("std::shared_ptr<PreparedMatrixBank> CmdConnection::compile_matrix_bank(", 1)[1]
        controls = "    std::map<gx_engine::Plugin*, gx_engine::Plugin*> selector_parents;" + compiler.split(
            "    std::map<gx_engine::Plugin*, gx_engine::Plugin*> selector_parents;", 1)[1].split(
                "    for (const auto& item : values) {", 1)[0]
        order = "    const auto order = " + compiler.split("    const auto order = ", 1)[1].split(
            "    if (compile_image) {", 1)[0]
        values = "    for (const auto& item : values) {" + compiler.split(
            "    for (const auto& item : values) {", 1)[1].split("    cutover->nam.reserve", 1)[0]
        structural = "static bool snapshot_structural_parameter_id(" + source.split(
            "static bool snapshot_structural_parameter_id(", 1)[1].split("class JsonString", 1)[0]
        with tempfile.TemporaryDirectory(prefix="gx-bank-selectors-") as directory:
            path = Path(directory)
            (path / "matrix_bank_selectors_under_test.h").write_text(controls + order)
            (path / "matrix_bank_values_under_test.h").write_text(values)
            (path / "matrix_bank_structural_under_test.h").write_text(structural)
            binary = path / "matrix-bank-selectors"
            result = subprocess.run([os.environ.get("CXX", "c++"), "-std=c++17", "-Wall", "-Wextra", "-Werror",
                                     f"-I{path}", f"-I{TRUNK / 'src/headers'}",
                                     str(Path(__file__).with_name("matrix_bank_selectors_fixture.cpp")), "-o", str(binary)],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)

    def test_rpc_banks_retain_five_matrices_and_snapshots_across_promotions(self):
        source = (TRUNK / "src/gx_head/engine/jsonrpc.cpp").read_text()
        header = (TRUNK / "src/headers/jsonrpc.h").read_text()
        sections = []
        for start, end in (("    FUNCTION(matrix_prepare_next)", "    FUNCTION(matrix_promote)"),
                           ("    FUNCTION(matrix_promote)", "    FUNCTION(matrix_release)"),
                           ("    FUNCTION(snapshot_activate)", "    FUNCTION(set_scene)")):
            sections.append(start + source.split(start, 1)[1].split(end, 1)[0] + "break;\n")
        with tempfile.TemporaryDirectory(prefix="gx-bank-rpc-") as directory:
            path = Path(directory)
            (path / "matrix_bank_capacity_under_test.h").write_text(
                "constexpr unsigned int kExecutableMatrixCapacity" + header.split(
                    "constexpr unsigned int kExecutableMatrixCapacity", 1)[1].split("// Executable images", 1)[0])
            (path / "matrix_bank_lookup_under_test.h").write_text(
                "    std::shared_ptr<PreparedMatrixBank> bank_for_handle" + header.split(
                    "    std::shared_ptr<PreparedMatrixBank> bank_for_handle", 1)[1].split(
                    "    std::map<std::string, std::shared_ptr<PreparedMatrixScene>> matrices;", 1)[0])
            (path / "matrix_bank_rpc_under_test.h").write_text("\n".join(sections))
            binary = path / "matrix-bank-rpc"
            result = subprocess.run([os.environ.get("CXX", "c++"), "-std=c++17", "-Wall", "-Wextra", "-Werror",
                                     f"-I{path}", f"-I{TRUNK / 'src/headers'}",
                                     str(Path(__file__).with_name("matrix_bank_lifecycle_fixture.cpp")), "-o", str(binary)],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("five images, ten snapshots", result.stdout)

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
            (path / "matrix_bank_nam_process_under_test.h").write_text(
                "inline void process_nam_mono(" + source.split("inline void process_nam_mono(", 1)[1].split(
                    "} // namespace", 1)[0])
            (path / "matrix_bank_nam_under_test.h").write_text("\n".join(sections))
            binary = path / "matrix-bank-nam"
            result = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", f"-I{path}", f"-I{TRUNK / 'src/headers'}",
                                     str(Path(__file__).with_name("matrix_bank_nam_fixture.cpp")), "-o", str(binary)],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("1000 transitions", result.stdout)

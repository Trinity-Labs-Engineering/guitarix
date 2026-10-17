from pathlib import Path
import os
import shutil
import shlex
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class MatrixIRPoolTests(unittest.TestCase):
    def test_real_convolver_preparation_stays_stopped_and_cached_ir_produces_audio(self):
        compiler = os.environ.get("CXX", "c++")
        if not shutil.which(compiler):
            self.skipTest("C++ compiler unavailable")
        engine = TRUNK / "src/gx_head/engine"
        source = (engine / "gx_internal_plugins.cpp").read_text()
        start, end = "bool ConvolverAdapter::configure_current_ir()", "bool ConvolverAdapter::conv_prepare()"
        with tempfile.TemporaryDirectory(prefix="gx-matrix-ir-real-") as directory:
            path = Path(directory)
            (path / "ir_pool_under_test.h").write_text(start + source.split(start, 1)[1].split(end, 1)[0])
            for name in ("convolver", "resampler"):
                (path / f"{name}_under_test.cpp").write_text(
                    (engine / f"gx_{name}.cpp").read_text().replace('#include "engine.h"', ''))
            binary = path / "ir-real"
            flags = shlex.split(subprocess.check_output(["pkg-config", "--cflags", "glib-2.0"], text=True))
            compiled = subprocess.run([compiler, "-std=c++17", "-O1", "-pthread", *flags, f"-I{path}",
                f"-I{TRUNK / 'src/headers'}", f"-I{TRUNK / 'src/zita-convolver'}",
                f"-I{TRUNK / 'libgxwmm'}", f"-I{TRUNK / 'libgxw'}",
                str(Path(__file__).with_name("matrix_ir_real_fixture.cpp")),
                str(TRUNK / "src/zita-convolver/zita-convolver.cc"),
                "-lfftw3f", "-lsndfile", "-lzita-resampler", "-o", str(binary)],
                capture_output=True, text=True)
            self.assertEqual(compiled.returncode, 0, compiled.stderr)
            result = subprocess.run([str(binary), directory], capture_output=True, text=True, timeout=20)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout.strip(), "matrix-ir-real-ok")

    def test_preparation_preserves_audible_ir_and_repeated_switches_only_transfer_ownership(self):
        compiler = os.environ.get("CXX", "c++")
        if not shutil.which(compiler):
            self.skipTest("C++ compiler unavailable")
        source = (TRUNK / "src/gx_head/engine/gx_internal_plugins.cpp").read_text()
        start, end = "bool ConvolverAdapter::configure_current_ir()", "bool ConvolverAdapter::conv_prepare()"
        production = start + source.split(start, 1)[1].split(end, 1)[0]
        with tempfile.TemporaryDirectory(prefix="gx-matrix-ir-") as directory:
            path = Path(directory)
            (path / "ir_pool_under_test.h").write_text(production)
            binary = path / "ir-pool"
            compiled = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", f"-I{path}",
                                       str(Path(__file__).with_name("matrix_ir_pool_fixture.cpp")), "-o", str(binary)],
                                      capture_output=True, text=True)
            self.assertEqual(compiled.returncode, 0, compiled.stderr)
            result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout.strip(), "matrix-ir-pool-ok")

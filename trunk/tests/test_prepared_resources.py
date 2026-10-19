"""Execute the production cache methods with counted DSP/convolver constructors."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class PreparedResourceTests(unittest.TestCase):
    def test_preparation_reuse_pinning_bounds_and_audio_ownership(self):
        compiler = os.environ.get("CXX", "c++")
        if not shutil.which(compiler):
            self.skipTest("C++ compiler unavailable")
        neural = (TRUNK / "src/gx_head/engine/gx_neural_plugins.cpp").read_text()
        convolver = (TRUNK / "src/gx_head/engine/gx_internal_plugins.cpp").read_text()
        methods = neural[neural.index("std::unique_ptr<nam::DSP> NeuralAmp::take_cached_model("):
                         neural.index("inline void NeuralAmp::clear_state_f()")]
        methods += convolver[convolver.index("bool ConvolverAdapter::configuration_matches()"):
                             convolver.index("ConvolverStereoAdapter::ConvolverStereoAdapter(")]
        with tempfile.TemporaryDirectory(prefix="gx-prepared-resources-") as directory:
            root = Path(directory)
            (root / "prepared_resources_under_test.h").write_text(methods)
            binary = root / "prepared-resources"
            subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                            "-fsanitize=address,undefined", "-g", f"-I{root}",
                            str(Path(__file__).with_name("prepared_resources_fixture.cpp")),
                            "-o", str(binary)], check=True, capture_output=True, text=True)
            result = subprocess.run([str(binary)], check=True, capture_output=True,
                                    text=True, timeout=20)
        self.assertEqual(result.stdout.strip(), "prepared-resources-ok")


if __name__ == "__main__":
    unittest.main()

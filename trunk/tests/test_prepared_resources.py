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
        internal_header = (TRUNK / "src/headers/gx_internal_plugins.h").read_text()
        settings = internal_header[internal_header.index("class GxJConvSettings {"):
                                   internal_header.index("\nclass ConvolverAdapter;", internal_header.index("class GxJConvSettings {"))]
        settings += convolver[convolver.index("GxJConvSettings::GxJConvSettings()"):
                              convolver.index("void GxJConvSettings::setFullIRPath(")]
        settings += convolver[convolver.index("bool GxJConvSettings::operator==("):
                              convolver.index("void GxJConvSettings::read_gainline(")]
        methods = neural[neural.index("std::unique_ptr<nam::DSP> NeuralAmp::take_cached_model("):
                         neural.index("inline void NeuralAmp::clear_state_f()")]
        methods += neural[neural.index("std::unique_ptr<nam::DSP> NeuralAmpMulti::take_cached_model("):
                          neural.index("inline void NeuralAmpMulti::clear_state_f()")]
        methods += convolver[convolver.index("bool ConvolverAdapter::configuration_matches()"):
                             convolver.index("ConvolverStereoAdapter::ConvolverStereoAdapter(")]
        with tempfile.TemporaryDirectory(prefix="gx-prepared-resources-") as directory:
            root = Path(directory)
            (root / "prepared_resources_under_test.h").write_text(methods)
            (root / "ir_settings_under_test.h").write_text(settings)
            config = neural[neural.index("const std::size_t kNamModelCacheEntries ="):
                            neural.index("constexpr float kDefaultNamSlimmableSize")]
            (root / "nam_cache_config_under_test.h").write_text(config)
            binary = root / "prepared-resources"
            build = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-Wno-deprecated-copy",
                            "-fsanitize=address,undefined", "-g", f"-I{root}",
                            f"-I{TRUNK / 'libgxwmm'}", f"-I{TRUNK / 'libgxw'}",
                            str(Path(__file__).with_name("prepared_resources_fixture.cpp")),
                            "-o", str(binary)], capture_output=True, text=True)
            self.assertEqual(build.returncode, 0, build.stderr)
            for configured in ("", "12", "32", "8", "bad", "999999999999999999999"):
                result = subprocess.run([str(binary)], capture_output=True,
                                        text=True, timeout=20,
                                        env={**os.environ, "GUITARIX_NAM_CACHE_ENTRIES": configured})
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(result.stdout.strip(), "prepared-resources-ok")


if __name__ == "__main__":
    unittest.main()

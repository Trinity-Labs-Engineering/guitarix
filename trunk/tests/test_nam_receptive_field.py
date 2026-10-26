"""Validate finite-history metadata against actual A2 and generic model factories."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest


TRUNK = Path(__file__).resolve().parents[1]
CORE = TRUNK / "src/NAM/NeuralAmpModelerCore"


class NamReceptiveFieldTests(unittest.TestCase):
    def test_factory_history_metadata_and_unchanged_audio(self):
        compiler = os.environ.get("CXX", "c++")
        library = Path(os.environ.get("GUITARIX_NAM_TEST_LIBRARY", str(TRUNK / "build/src/NAM/libnam.a")))
        if shutil.which(compiler) is None or not library.is_file():
            self.skipTest("Build the current production libnam.a and supply a C++ compiler")
        suite = CORE / "tools/test/test_a2_fast.cpp"
        with tempfile.TemporaryDirectory(prefix="gx-nam-history-") as temporary:
            directory = Path(temporary)
            (directory / "history.cpp").write_text(f'''#include "{suite}"
#include "{CORE / 'tools/test/test_container.cpp'}"
int main() {{
    nam::DSP unknown(1, 1, 48000);
    assert(unknown.GetRuntimeDSPInfo().receptive_field_frames == 0);
    test_a2_fast::test_matches_generic_nano();
    test_a2_fast::test_matches_generic_standard();
    test_a2_fast::test_factory_preserves_conditioning_dsp();
    test_container::test_container_slimmable_selects_submodel();
    std::cout << "nam-history-metadata-ok\\n";
}}
''')
            executable = directory / "history"
            built = subprocess.run([
                compiler, "-std=c++20", "-O1", "-DNAM_SAMPLE_FLOAT", "-DNAM_ENABLE_A2_FAST",
                f"-I{CORE}", f"-I{CORE / 'Dependencies/eigen'}",
                f"-I{CORE / 'Dependencies/nlohmann'}", str(directory / "history.cpp"),
                str(CORE / "tools/test/allocation_tracking.cpp"),
                "-Wl,--whole-archive", str(library), "-Wl,--no-whole-archive",
                "-pthread", "-o", str(executable),
            ], capture_output=True, text=True, timeout=90)
            self.assertEqual(built.returncode, 0, built.stderr)
            result = subprocess.run([str(executable)], cwd=CORE, capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("nam-history-metadata-ok", result.stdout)


if __name__ == "__main__":
    unittest.main()

"""Exercise bounded NAM callbacks and selected-only containers from production code."""
from pathlib import Path
import os
import re
import shutil
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]
CORE = TRUNK / "src/NAM/NeuralAmpModelerCore"


class NamMemoryTests(unittest.TestCase):
    def compiler(self):
        compiler = os.environ.get("CXX", "c++")
        if not shutil.which(compiler):
            self.skipTest("C++ compiler unavailable")
        return compiler

    def test_model_and_host_callback_boundaries(self):
        source = (TRUNK / "src/gx_head/engine/gx_neural_plugins.cpp").read_text()
        callbacks = []
        for name in ("NeuralAmp", "NeuralAmpMulti"):
            start = source.index(f"void {name}::compute_static(")
            end = source.index("\n}", start) + 2
            callbacks.append(source[start:end])
        with tempfile.TemporaryDirectory(prefix="gx-nam-blocks-") as directory:
            root = Path(directory)
            (root / "nam_callbacks_under_test.h").write_text("\n".join(callbacks))
            binary = root / "nam-blocks"
            subprocess.run([self.compiler(), "-std=c++17", "-Wall", "-Wextra", "-Werror",
                            "-fsanitize=address,undefined", "-g", f"-I{root}",
                            f"-I{TRUNK / 'src/headers'}",
                            str(Path(__file__).with_name("nam_block_adapter_fixture.cpp")),
                            "-o", str(binary)], check=True, capture_output=True, text=True)
            result = subprocess.run([str(binary)], check=True, capture_output=True,
                                    text=True, timeout=20)
            self.assertEqual(result.stdout.strip(), "nam-blocks-ok")

    def test_container_suite_with_production_float_library(self):
        library = TRUNK / "build/src/NAM/libnam.a"
        if not library.exists():
            self.skipTest("build Guitarix libnam.a first to exercise its production float graphs")
        suite = CORE / "tools/test/test_container.cpp"
        names = re.findall(r"^void (test_\w+)\(\)", suite.read_text(), re.M)
        with tempfile.TemporaryDirectory(prefix="gx-nam-containers-") as directory:
            root = Path(directory)
            driver = root / "containers.cpp"
            driver.write_text(f'#include "{suite}"\nint main() {{\n' +
                              "".join(f"test_container::{name}();\n" for name in names) +
                              'std::cout << "nam-containers-ok\\n";\n}\n')
            binary = root / "containers"
            subprocess.run([self.compiler(), "-std=c++20", "-O0", "-DNAM_SAMPLE_FLOAT",
                            "-DNAM_ENABLE_A2_FAST", f"-I{CORE}",
                            f"-I{CORE / 'Dependencies/eigen'}", f"-I{CORE / 'Dependencies/nlohmann'}",
                            str(driver), str(CORE / "tools/test/allocation_tracking.cpp"),
                            "-Wl,--whole-archive", str(library), "-Wl,--no-whole-archive",
                            "-pthread", "-o", str(binary)],
                           check=True, capture_output=True, text=True, timeout=120)
            result = subprocess.run([str(binary)], cwd=CORE, check=True,
                                    capture_output=True, text=True, timeout=120)
            self.assertEqual(result.stdout.strip(), "nam-containers-ok")

    def test_ordinary_models_preserve_history_when_size_control_changes(self):
        source = (TRUNK / "src/gx_head/engine/gx_neural_plugins.cpp").read_text()
        definitions = ['#include <cassert>\n#include <cmath>\n#include <string>\n',
                       'struct Model { bool slimmable=false; int history=123; };\n',
                       'bool nam_model_is_slimmable(Model* m) { return m->slimmable; }\n',
                       'struct Ramp { enum {DOWN, UP}; int mode=UP; };\n']
        cases = []
        for cls, slot, model, current, slider, size, ramp in (
            ("NeuralAmp", "", "model", "current_file", "fVslider2", "current_model_size", "ramp"),
            ("NeuralAmpMulti", "a", "modela", "current_afile", "fVslider3", "current_model_sizea", "rampA"),
            ("NeuralAmpMulti", "b", "modelb", "current_bfile", "fVslider4", "current_model_sizeb", "rampB"),
        ):
            wrapper = f"Wrapper{slot or 'mono'}"
            load_start = source.index(f"void {cls}::load_nam_{slot}file()")
            guard_start = source.index("        if (has_selection &&", load_start)
            guard_end = source.index("\n        std::unique_ptr<nam::DSP>", guard_start)
            guard = source[guard_start:guard_end]
            size_start = source.index(f"void {cls}::set_nam_{slot}size()")
            size_end = source.index("\n}", size_start) + 2
            callback = source[size_start:size_end].replace(f"{cls}::", f"{wrapper}::")
            definitions.append(f'''struct {wrapper} {{
                Model graph; Model* {model}=&graph; int reloads=0; Ramp {ramp};
                std::string {current}="same"; float {slider}=0.49f, {size}=0.49f;
                void load_nam_{slot}file_impl() {{
                    bool has_selection=true; std::string selected_file="same";
                    {guard}
                    ++reloads; graph.history=0;
                }}
                void set_nam_{slot}size();
            }};\n{callback}\n''')
            cases.append(f'''{{ {wrapper} w; w.{slider}=1.0f;
                w.set_nam_{slot}size(); w.load_nam_{slot}file_impl();
                assert(w.{model}==&w.graph && w.graph.history==123 && w.reloads==0);
                w.graph.slimmable=true; w.set_nam_{slot}size(); assert(w.reloads==1);
            }}''')
        definitions.append("int main() {" + "\n".join(cases) + "}\n")
        with tempfile.TemporaryDirectory(prefix="gx-nam-size-") as directory:
            root = Path(directory)
            driver = root / "size.cpp"
            driver.write_text("\n".join(definitions))
            binary = root / "size"
            subprocess.run([self.compiler(), "-std=c++17", "-Wall", "-Wextra", "-Werror",
                            "-fsanitize=address,undefined", str(driver), "-o", str(binary)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(binary)], check=True, capture_output=True, text=True, timeout=20)


if __name__ == "__main__":
    unittest.main()

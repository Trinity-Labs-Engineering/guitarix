from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


TRUNK = Path(__file__).resolve().parents[1]
FIXTURE = Path(__file__).with_name("scene_chain_reuse_fixture.cpp")


class SceneChainReuseTests(unittest.TestCase):
    def test_resident_bypass_keeps_published_chain_and_initializes_only_new_nodes(self):
        compiler = os.environ.get("CXX", "c++")
        if shutil.which(compiler) is None:
            self.skipTest(f"C++ compiler is unavailable: {compiler}")

        header = (TRUNK / "src/headers/gx_modulesequencer.h").read_text()
        engine = (TRUNK / "src/gx_head/engine/gx_engine_audio.cpp").read_text()
        declaration = header.split("template <class F>\nclass ThreadSafeChainPointer", 1)[1]
        declaration = declaration.split("typedef void (*monochainorder)", 1)[0]
        implementation = header.split("template <class F>\nThreadSafeChainPointer<F>::ThreadSafeChainPointer", 1)[1]
        implementation = implementation.split("/****************************************************************", 1)[0]
        staging = engine.split("bool lists_equal(", 1)[1]
        staging = staging.split("void ProcessingChainBase::clear_module_states", 1)[0]
        production = (
            "template <class F>\nclass ThreadSafeChainPointer" + declaration
            + "template <class F>\nThreadSafeChainPointer<F>::ThreadSafeChainPointer"
            + implementation
            + "bool lists_equal(" + staging
        )

        with tempfile.TemporaryDirectory(prefix="gx-scene-chain-reuse-") as directory:
            path = Path(directory)
            (path / "scene_chain_under_test.h").write_text(production)
            executable = path / "scene-chain-reuse"
            subprocess.run(
                [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                 f"-I{path}", str(FIXTURE), "-o", str(executable)],
                check=True, capture_output=True, text=True,
            )
            completed = subprocess.run(
                [str(executable)], check=True, capture_output=True, text=True, timeout=10
            )
        self.assertEqual(completed.stdout.strip(), "scene-chain-reuse-ok")


if __name__ == "__main__":
    unittest.main()

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
        loader = (TRUNK / "src/gx_head/engine/gx_pluginloader.cpp").read_text()
        plugins = (TRUNK / "src/headers/gx_pluginloader.h").read_text()
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
        transfer = engine.split("void ProcessingChainBase::retain_transferred_plugins(", 1)[1]
        transfer = transfer.split("#ifndef NDEBUG", 1)[0]
        production += "void ProcessingChainBase::retain_transferred_plugins(" + transfer

        with tempfile.TemporaryDirectory(prefix="gx-scene-chain-reuse-") as directory:
            path = Path(directory)
            (path / "scene_chain_under_test.h").write_text(production)
            (path / "matrix_membership_under_test.h").write_text(
                "    bool get_matrix_source()" + plugins.split("    bool get_matrix_source()", 1)[1].split(
                    "    bool rt_scene_state_changed()", 1)[0]
            )
            (path / "matrix_lists_under_test.h").write_text(
                "int Plugin::desired_rt_scene_state()" + loader.split("int Plugin::desired_rt_scene_state()", 1)[1].split(
                    "bool Plugin::commit_rt_scene_state()", 1)[0]
                + "static bool plugin_order(" + loader.split("static bool plugin_order(", 1)[1].split(
                    "void PluginList::ordered_mono_list", 1)[0]
                + "void PluginList::ordered_processing_mono_list(" + loader.split("void PluginList::ordered_processing_mono_list(", 1)[1].split(
                    "bool PluginList::validate_matrix", 1)[0]
                + "void PluginList::ordered_processing_stereo_list(" + loader.split("void PluginList::ordered_processing_stereo_list(", 1)[1].split(
                    "bool PluginList::rt_scene_state_changed()", 1)[0]
            )
            executable = path / "scene-chain-reuse"
            compiled = subprocess.run(
                [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                 f"-I{path}", str(FIXTURE), "-o", str(executable)],
                capture_output=True, text=True,
            )
            self.assertEqual(compiled.returncode, 0, compiled.stderr)
            completed = subprocess.run(
                [str(executable)], check=True, capture_output=True, text=True, timeout=10
            )
        self.assertEqual(completed.stdout.strip(), "scene-chain-reuse-ok")


if __name__ == "__main__":
    unittest.main()

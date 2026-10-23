"""A failed prefix replay must recover even when its capture point disappears."""

from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest


class ScenePrerollAbortRecoveryTests(unittest.TestCase):
    def test_real_callback_recovers_without_nam_or_with_unsupported_upstream(self):
        compiler = os.environ.get("CXX", "c++")
        if shutil.which(compiler) is None:
            self.skipTest(f"C++ compiler is unavailable: {compiler}")
        root = Path(__file__).resolve().parents[1]
        source = (root / "src/gx_head/engine/gx_engine_audio.cpp").read_text()
        start = source.index("namespace {\n// Only memoryless/small-filter")
        end = source.index("#ifdef GUITARIX_AS_PLUGIN\nvoid __rt_func StereoModuleChain::process", start)
        header = (root / "src/headers/gx_modulesequencer.h").read_text()
        status_start = header.index("struct ScenePrefixPrerollStatus {")
        status_end = header.index("class MonoModuleChain:", status_start)
        fixture = (root / "tests/scene_prefix_preroll_fixture.cpp").read_text()
        fixture = fixture[:fixture.index("int main() {")]
        regression = r'''
int main() {
    MonoModuleChain chain;
    chain.scene_input_history.set_samplerate(1000);
    Plugin nam_owner, hog_owner, delay_owner;
    PluginDef nam{"nam", PGNI_DYN_POSITION, reset};
    PluginDef hog{"hogsfoot", PGNI_DYN_POSITION, reset};
    PluginDef delay{"echo", PGNI_DYN_POSITION, reset};
    const std::vector<monochain_data> valid = {
        {render, &nam, &nam_owner}, {render, &hog, &hog_owner},
        {nullptr, nullptr, nullptr}
    };
    chain.nodes = valid;
    for (unsigned int i = 0; i < 20; ++i) chain.callback();
    ScenePrefixPrerollStatus status;
    chain.fail_wait = true;
    assert(!chain.preroll_scene_prefix(true, status));
    assert(chain.scene_input_history.state() == SceneInputHistory::abort_requested);
    chain.fail_wait = false;

    // The next scene has removed NAM, so recovery cannot rely on capture_rt.
    chain.nodes = {{render, &delay, &delay_owner}, {nullptr, nullptr, nullptr}};
    chain.callback();
    assert(chain.scene_input_history.state() == SceneInputHistory::idle);
    assert(chain.preroll_scene_prefix(true, status));
    chain.nodes = valid;
    assert(chain.preroll_scene_prefix(true, status));
    assert(status.attempted && status.history_frames == 0);

    for (unsigned int i = 0; i < 20; ++i) chain.callback();
    chain.fail_wait = true;
    assert(!chain.preroll_scene_prefix(true, status));
    chain.fail_wait = false;
    chain.nodes.insert(chain.nodes.begin(), {render, &delay, &delay_owner});
    chain.callback();
    assert(chain.scene_input_history.state() == SceneInputHistory::idle);
    assert(chain.preroll_scene_prefix(true, status) && !status.attempted);
    chain.nodes = valid;
    assert(chain.preroll_scene_prefix(true, status));
    assert(status.attempted && status.history_frames == 0);

    // Pending frames from an aborted partial replay must never be stitched
    // into the next history window after the control thread abandons them.
    SceneInputHistory history;
    float old[32]; std::fill(old, old + 32, 99.0f);
    assert(!history.capture_rt(old, 32));
    history.request_pause();
    assert(history.capture_rt(old, 32));
    history.request_abort();
    history.acknowledge_abort_rt();
    float fresh[32]; std::fill(fresh, fresh + 32, 7.0f);
    assert(!history.capture_rt(fresh, 32));
    history.request_pause();
    assert(history.capture_rt(fresh, 32));
    std::vector<float> copy(SceneInputHistory::history_capacity);
    assert(history.copy_history(copy.data()) == 32);
    for (unsigned int i = 0; i < 32; ++i) assert(copy[i] == 7.0f);
    std::cout << "abort recovery without capture point passes\n";
}
'''
        with tempfile.TemporaryDirectory(prefix="gx-preroll-recovery-") as directory:
            directory = Path(directory)
            (directory / "scene_prefix_under_test.h").write_text(source[start:end])
            (directory / "scene_prefix_status_under_test.h").write_text(header[status_start:status_end])
            test_source = directory / "recovery.cpp"
            test_source.write_text(fixture + regression)
            executable = directory / "recovery"
            built = subprocess.run([
                compiler, "-std=c++17", "-O2", "-Wall", "-Wextra",
                f"-I{root / 'src/headers'}", f"-I{directory}", str(test_source),
                "-o", str(executable),
            ], capture_output=True, text=True, timeout=30)
            self.assertEqual(built.returncode, 0, built.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("abort recovery without capture point passes", result.stdout)


if __name__ == "__main__":
    unittest.main()

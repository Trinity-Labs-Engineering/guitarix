"""Exercise production staged replay with chronological input and host blocks."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest


TRUNK = Path(__file__).resolve().parents[1]


class SceneSuffixPrerollTests(unittest.TestCase):
    def test_staged_history_catchup_cold_input_and_worker_failure(self):
        compiler = os.environ.get("CXX", "c++")
        if shutil.which(compiler) is None:
            self.skipTest("C++ compiler is unavailable")
        source = (TRUNK / "src/gx_head/engine/gx_engine_audio.cpp").read_text()
        start = source.index("namespace {\n// Only memoryless/small-filter")
        end = source.index("#ifdef GUITARIX_AS_PLUGIN\nvoid __rt_func StereoModuleChain::process", start)
        header = (TRUNK / "src/headers/gx_modulesequencer.h").read_text()
        status_start = header.index("struct ScenePrefixPrerollStatus {")
        status_end = header.index("class MonoModuleChain:", status_start)
        fixture = (TRUNK / "tests/scene_prefix_preroll_fixture.cpp").read_text().split("int main() {", 1)[0]
        fixture += r'''
int main() {
    for (unsigned int prior_callbacks : {4u, 20u}) {
        MonoModuleChain chain;
        chain.scene_input_history.set_samplerate(1000);
        Plugin nam_owner{false}, eq_owner{false}, amp_owner{false}, ir_owner{false};
        NeuralAmp nam; nam.id = "nam"; nam.history_frames = 64;
        NeuralAmpMulti amp; amp.id = "mnam"; amp.history_frames = 96;
        PluginDef eq{"eq", PGNI_DYN_POSITION, reset};
        PluginDef ir{"jconv_mono", PGNI_DYN_POSITION};
        ir.block_frames = 32; ir.sample_rate = 1000;
        chain.nodes = {{render, &nam, &nam_owner}, {render, &eq, &eq_owner},
                       {render, &amp, &amp_owner}, {render, &ir, &ir_owner},
                       {nullptr, nullptr, nullptr}};
        for (unsigned int i = 0; i < prior_callbacks; ++i) chain.callback();
        nam_owner.on = eq_owner.on = amp_owner.on = ir_owner.on = true;
        ScenePrefixPrerollStatus status;
        assert(chain.preroll_scene_prefix(true, status));
        assert(status.attempted && status.catchup_frames == 32);
        assert(status.suffix_catchup_frames == 32 && chain.silent_callbacks == 1);
        if (prior_callbacks == 20) {
            assert(status.suffix_attempted && status.history_frames == 512);
            assert(status.amp_history_frames == 160 && status.ir_history_frames == 64);
            assert(nam.observed.size() == 512 + 32 + 32);
            assert(amp.observed.size() == 160 + 32 + 32);
            assert(ir.observed.size() == 64 + 32 + 32);
            for (unsigned int i = 0; i < nam.observed.size(); ++i)
                assert(nam.observed[i] == float(129 + i));
            for (unsigned int i = 0; i < amp.observed.size(); ++i)
                assert(amp.observed[i] == float(482 + i));
            for (unsigned int i = 0; i < ir.observed.size(); ++i)
                assert(ir.observed[i] == float(578 + i));
            assert(eq.resets == 1);
        } else {
            // A cold scene keeps the live first-note history; it never invents
            // the missing past or partially replays a suffix from zero.
            assert(!status.suffix_attempted && status.history_frames == 0);
            assert(std::strcmp(status.suffix_skip_reason, "insufficient-input-history") == 0);
            assert(nam.observed.size() == 64 && amp.observed.size() == 64);
            assert(eq.resets == 0);
        }
        nam.observed.clear(); amp.observed.clear(); ir.observed.clear();
        amp.history_frames = 0;
        assert(chain.preroll_scene_prefix(true, status) && !status.attempted);
        assert(nam.observed.empty());
        amp.history_frames = 96;
        PluginDef pitch{"smbPitchShift", PGNI_DYN_POSITION}; Plugin pitch_owner;
        chain.nodes.insert(chain.nodes.begin() + 1, {render, &pitch, &pitch_owner});
        assert(chain.preroll_scene_prefix(true, status) && !status.attempted);
        // A-Clipper's multi-second signal pole cannot be reset from this
        // bounded history, even though it belongs to the distortion family.
        pitch.id = "aclipper";
        assert(chain.preroll_scene_prefix(true, status) && !status.attempted);
        chain.nodes.erase(chain.nodes.begin() + 1);
        ir.replay_fail = true;
        bool failed = false;
        try { chain.preroll_scene_prefix(true, status); }
        catch (const std::runtime_error&) { failed = true; }
        assert(failed && chain.scene_input_history.state() == SceneInputHistory::abort_requested);
        chain.callback();
        assert(chain.scene_input_history.state() == SceneInputHistory::idle);
    }
    // Expire the budget after the first partial dependency-cone block.
    // Ownership cannot be returned while intentionally skipped model state
    // remains incomplete, even though the normal replay deadline has expired.
    {
        MonoModuleChain chain; chain.scene_input_history.set_samplerate(1000);
        Plugin nam_owner{false}, amp_owner{false}, ir_owner{false};
        NeuralAmp nam; nam.id = "nam"; nam.history_frames = 64;
        NeuralAmpMulti amp; amp.id = "mnam"; amp.history_frames = 320;
        amp.expire_on_discard = true;
        PluginDef ir{"jconv_mono", PGNI_DYN_POSITION};
        ir.block_frames = 32; ir.sample_rate = 1000;
        chain.nodes = {{render, &nam, &nam_owner}, {render, &amp, &amp_owner},
                       {render, &ir, &ir_owner}, {nullptr, nullptr, nullptr}};
        for (int i = 0; i < 20; ++i) chain.callback();
        nam_owner.on = amp_owner.on = ir_owner.on = true;
        ScenePrefixPrerollStatus status;
        assert(!chain.preroll_scene_prefix(true, status));
        assert(status.discard_budget_deferred && status.amp_discarded_frames == 320);
        assert(amp.discard_pending == 0 && status.amp_history_frames == 384);
        assert(chain.scene_input_history.state() == SceneInputHistory::abort_requested);
        scene_test_clock_expired = false;
        chain.callback(); // render() asserts no incomplete model reaches live audio
        assert(chain.scene_input_history.state() == SceneInputHistory::idle);
        for (int i = 0; i < 20; ++i) chain.callback();
        amp.expire_on_discard = false; amp.throw_on_discard = true;
        bool threw = false;
        try { chain.preroll_scene_prefix(true, status); }
        catch (const std::runtime_error&) { threw = true; }
        assert(threw && amp.discard_resets == 1 && amp.discard_pending == 0);
        assert(chain.scene_input_history.state() == SceneInputHistory::abort_requested);
        chain.callback(); // exceptional partial model was reset before handoff
        assert(chain.scene_input_history.state() == SceneInputHistory::idle);
        for (int i = 0; i < 20; ++i) chain.callback();
        amp.throw_on_discard = false; amp.overflow_on_discard = &chain.scene_input_history;
        assert(!chain.preroll_scene_prefix(true, status));
        assert(status.discard_budget_deferred && status.amp_discarded_frames == 320);
        assert(amp.discard_pending == 0 && amp.discard_resets == 1);
        assert(chain.scene_input_history.state() == SceneInputHistory::abort_requested);
        chain.callback();
        assert(chain.scene_input_history.state() == SceneInputHistory::idle);
    }
    // Only the validated pitch group and an explicit bypassed re-entry may
    // clear/reconstruct dynamics. Same-active dynamics and reordered groups skip.
    {
        MonoModuleChain chain; chain.scene_input_history.set_samplerate(1000);
        Plugin off{false}, on{true};
        NeuralAmp nam; nam.id = "nam"; nam.history_frames = 64;
        NeuralAmpMulti amp; amp.id = "mnam"; amp.history_frames = 96;
        PluginDef pitch{"smbPitchShift", PGNI_DYN_POSITION, reset};
        PluginDef comp{"compressor", PGNI_DYN_POSITION, reset};
        PluginDef fuzz{"axface", PGNI_DYN_POSITION, reset};
        PluginDef wah{"GCB_95", PGNI_DYN_POSITION, reset};
        PluginDef ir{"jconv_mono", PGNI_DYN_POSITION}; ir.block_frames = 32; ir.sample_rate = 1000;
        chain.nodes = {{render, &nam, &off}, {render, &pitch, &off},
                       {render, &comp, &off}, {render, &fuzz, &off}, {render, &wah, &off},
                       {render, &amp, &off}, {render, &ir, &off}, {nullptr, nullptr, nullptr}};
        for (int i = 0; i < 20; ++i) chain.callback();
        off.on = true;
        ScenePrefixPrerollStatus status;
        assert(chain.preroll_scene_prefix(true, status) && !status.attempted);
        assert(comp.resets == 0 && pitch.resets == 0);
        assert(chain.preroll_scene_prefix(true, status, true));
        assert(status.pitch_reentry && status.history_frames == 512);
        assert(comp.resets == 1 && pitch.resets == 1 && fuzz.resets == 1 && wah.resets == 1);
        assert(chain.preroll_scene_prefix(true, status) && !status.attempted);
        assert(comp.resets == 1 && pitch.resets == 1);
        std::swap(chain.nodes[2], chain.nodes[3]);
        assert(chain.preroll_scene_prefix(true, status, true) && !status.attempted);
    }
    std::cout << "scene-suffix-preroll-ok\n";
}
'''
        with tempfile.TemporaryDirectory(prefix="gx-scene-suffix-") as temporary:
            directory = Path(temporary)
            (directory / "scene_prefix_under_test.h").write_text(source[start:end])
            (directory / "scene_prefix_status_under_test.h").write_text(header[status_start:status_end])
            (directory / "suffix.cpp").write_text(fixture)
            executable = directory / "suffix"
            built = subprocess.run([
                compiler, "-std=c++17", "-O2", "-Wall", "-Wextra",
                f"-I{TRUNK / 'src/headers'}", f"-I{directory}",
                str(directory / "suffix.cpp"), "-o", str(executable),
            ], capture_output=True, text=True, timeout=30)
            self.assertEqual(built.returncode, 0, built.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout.strip(), "scene-suffix-preroll-ok")


if __name__ == "__main__":
    unittest.main()

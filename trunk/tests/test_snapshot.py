from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class SnapshotTests(unittest.TestCase):
    def test_native_snapshot_scheduler(self):
        compiler = os.environ.get("CXX", "c++")
        if not shutil.which(compiler):
            self.skipTest("C++ compiler unavailable")
        with tempfile.TemporaryDirectory(prefix="gx-snapshot-") as directory:
            binary = Path(directory) / "snapshot"
            subprocess.run([
                compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-pthread",
                "-I" + str(TRUNK / "src/headers"),
                str(Path(__file__).with_name("snapshot_fixture.cpp")), "-o", str(binary),
            ], check=True, capture_output=True, text=True)
            result = subprocess.run([str(binary)], check=True, capture_output=True, text=True, timeout=15)
            self.assertIn("1000 transitions", result.stdout)

    def test_production_bypass_policy_preserves_state_and_rejects_reset_plugins(self):
        compiler = os.environ.get("CXX", "c++")
        if not shutil.which(compiler):
            self.skipTest("C++ compiler unavailable")
        header = (TRUNK / "src/headers/gx_pluginloader.h").read_text()
        methods = "    bool supports_snapshot_bypass()" + header.split(
            "    bool supports_snapshot_bypass()", 1)[1].split("    int get_position()", 1)[0]
        gate_source = (TRUNK / "src/gx_head/engine/gx_internal_plugins.cpp").read_text()
        gate = "void OutPutGate::outputgate_process(" + gate_source.split(
            "void OutPutGate::outputgate_process(", 1)[1].split("void OutPutGate::outputgate_compute", 1)[0]
        rpc = (TRUNK / "src/gx_head/engine/jsonrpc.cpp").read_text()
        classifier = "static bool snapshot_structural_parameter_id(" + rpc.split(
            "static bool snapshot_structural_parameter_id(", 1)[1].split("class JsonString", 1)[0]
        source = "#include <string>\n" + classifier + r"""
#include <cassert>
namespace gx_system {
int atomic_get(int v) { return v; }
void atomic_set(int* p, int v) { *p = v; }
}
enum { PGNI_MATRIX_SPLITTER = 1, PGNI_MATRIX_COMBINER = 2, PGNI_SNAPSHOT_RESET_REQUIRED = 4 };
struct BoolParameter { bool value = false; void set_snapshot_value(bool v) { value = v; } };
struct Def { int flags = 0; bool mono_audio = true; bool stereo_audio = false; };
struct Plugin {
    Def* pdef;
    BoolParameter* p_on_off;
    int rt_scene_state = 2 | (2 << 2) | 32;
    bool resident = true, source = false;
    bool get_scene_resident() const { return resident; }
    bool get_matrix_source() const { return source; }
    bool get_rt_on_off() const { return (rt_scene_state & 1) != 0; }
""" + methods + r"""
};
struct NoiseGate { Plugin inputlevel; bool off = true; float ngate = 0.25f; };
struct OutPutGate {
    const NoiseGate* noisegate;
    void outputgate_process(int count, float* input, float* output);
};
""" + gate + r"""
int main() {
    for (const char* id : {"amp.select", "test.mode", "test.oversampling", "test.latency",
                          "ir.convolver", "nam.size", "nam.file", "delay.position",
                          "delay.scene_resident", "delay.matrix_lane", "ui.delay"})
        assert(snapshot_structural_parameter_id(id));
    assert(!snapshot_structural_parameter_id("nam.input"));
    assert(!snapshot_structural_parameter_id("delay.on_off"));
    Def def; BoolParameter parameter; Plugin plugin{&def, &parameter};
    assert(plugin.supports_snapshot_bypass());
    const int old_state = plugin.rt_scene_state;
    Plugin::apply_snapshot_bypass(&plugin, 1);
    assert(parameter.value && plugin.rt_scene_state == (old_state | 1));
    Plugin::apply_snapshot_bypass(&plugin, 0);
    assert(!parameter.value && plugin.rt_scene_state == old_state);
    def.flags = PGNI_SNAPSHOT_RESET_REQUIRED;
    assert(!plugin.supports_snapshot_bypass());
    def.flags = PGNI_MATRIX_SPLITTER;
    assert(!plugin.supports_snapshot_bypass());
    def.flags = 0; plugin.source = true;
    assert(!plugin.supports_snapshot_bypass());
    plugin.source = false; plugin.resident = false;
    assert(!plugin.supports_snapshot_bypass());
    // Legacy lifecycle state remains deliberately stale: only the published
    // RT bit controls whether the post gate attenuates the signal.
    NoiseGate noise{Plugin{&def, &parameter}};
    OutPutGate output{&noise};
    float input[] = {1, -1}, result[] = {1, -1};
    Plugin::apply_snapshot_bypass(&noise.inputlevel, 1);
    assert(noise.off);
    output.outputgate_process(2, input, result);
    assert(result[0] == 0.25f && result[1] == -0.25f);
    Plugin::apply_snapshot_bypass(&noise.inputlevel, 0);
    result[0] = 1; result[1] = -1;
    output.outputgate_process(2, input, result);
    assert(result[0] == 1 && result[1] == -1);
}
"""
        with tempfile.TemporaryDirectory(prefix="gx-snapshot-policy-") as directory:
            path = Path(directory)
            (path / "policy.cpp").write_text(source)
            subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                            str(path / "policy.cpp"), "-o", str(path / "policy")],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(path / "policy")], check=True, timeout=10)


if __name__ == "__main__":
    unittest.main()

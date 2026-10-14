"""Exercise the production status handler, dispatch table and native plugins."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class HarmoniserRpcTests(unittest.TestCase):
    def test_status_dispatch_pitch_key_bypass_and_reset(self):
        source = (TRUNK / 'src/gx_head/engine/jsonrpc.cpp').read_text()
        start = source.index('    FUNCTION(harmoniser_status)')
        end = source.index('    FUNCTION(plugin_load_ui)', start)
        handler = source[start:end].replace('FUNCTION(harmoniser_status)', 'void harmoniser_status()')
        fixture = r'''
#include <cassert>
#include <cmath>
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include <stdexcept>
#include "gx_harmoniser_plugin.h"
#include "jsonrpc_methods-generated.h"
struct CmdConnection { struct methodnames { const char* name; jsonrpc_method m_id; }; };
#include "jsonrpc_methods-generated.cc"
namespace pluginlib {
namespace harmoniser { PluginDef* plugin(); }
namespace dual_harmoniser { PluginDef* plugin(); }
}
namespace gx_engine { enum GxEngineState { kEngineOn, kEngineBypass }; }
struct Plugin {
    PluginDef* def = nullptr; bool enabled = true;
    PluginDef* get_pdef() { return def; }
    bool get_on_off() { return enabled; }
};
struct Engine {
    struct List {
        Plugin p;
        Plugin* find_plugin(const std::string& id) { return p.def && id == p.def->id ? &p : nullptr; }
    } pluginlist;
    gx_engine::GxEngineState state = gx_engine::kEngineOn;
    gx_engine::GxEngineState get_state() { return state; }
};
struct Service { struct Jack { Engine engine; Engine& get_engine() { return engine; } } jack; } serv;
struct Value { std::string value; std::string getString() const { return value; } } id;
std::vector<Value*> params;
struct Writer {
    std::map<std::string, long long> values;
    void begin_object() { values.clear(); }
    void end_object() {}
    template<class T> void write_kv(const char* key, T value) { values[key] = value; }
} jw;
struct RpcError : std::runtime_error { RpcError(int, const char* message) : std::runtime_error(message) {} };
std::map<std::string,float*> controls;
float* reg_float(const char* key,const char*,const char*,const char*,float* value,float initial,float,float,float,const value_pair*) {
    controls[key] = value; *value = initial; return value;
}
''' + handler + r'''
int main() {
    const auto* method = Perfect_Hash::in_word_set("harmoniser_status", 17);
    assert(method && method->m_id == RPCM_harmoniser_status);
    for (bool dual : {false,true}) {
        controls.clear();
        auto* plugin = dual ? pluginlib::dual_harmoniser::plugin() : pluginlib::harmoniser::plugin();
        serv.jack.engine.pluginlist.p.def = plugin;
        serv.jack.engine.pluginlist.p.enabled = true;
        id.value = plugin->id; params = {&id};
        ParamReg reg{plugin,reg_float,nullptr,nullptr}; plugin->register_params(reg);
        plugin->set_samplerate(48000,plugin);
        harmoniser_status(); assert(jw.values["locked"] == 0 && jw.values["note"] == -1);
        std::vector<float> audio(4800), out(4800);
        for (int i=0;i<4800;i++) audio[i]=.1*std::sin(2*3.141592653589793*277.182631*i/48000);
        plugin->mono_audio(4800,audio.data(),out.data(),plugin);
        harmoniser_status(); assert(jw.values["locked"] == 1 && jw.values["outOfKey"] == 1 && jw.values["note"] == 4);
        const auto generation = jw.values["generation"];
        *controls.at(id.value+".tonic")=4; // C# is now in key; no new note needed.
        plugin->mono_audio(4800,audio.data(),out.data(),plugin);
        harmoniser_status(); assert(jw.values["locked"] == 1 && jw.values["outOfKey"] == 0 && jw.values["generation"] != generation);
        serv.jack.engine.pluginlist.p.enabled = false;
        harmoniser_status(); assert(jw.values["locked"] == 0 && jw.values["active"] == 0);
        serv.jack.engine.pluginlist.p.enabled = true;
        serv.jack.engine.state = gx_engine::kEngineBypass;
        harmoniser_status(); assert(jw.values["locked"] == 0 && jw.values["active"] == 0);
        serv.jack.engine.state = gx_engine::kEngineOn;
        plugin->clear_state(plugin);
        harmoniser_status(); assert(jw.values["locked"] == 0 && jw.values["note"] == -1);
        plugin->delete_instance(plugin); serv.jack.engine.pluginlist.p.def=nullptr;
    }
    for (const char* bad : {"nam","harmoniser"}) {
        id.value=bad; bool rejected=false;
        try { harmoniser_status(); } catch (const RpcError&) { rejected=true; }
        assert(rejected);
    }
    params.clear(); bool rejected=false;
    try { harmoniser_status(); } catch (const RpcError&) { rejected=true; }
    assert(rejected);
}
'''
        with tempfile.TemporaryDirectory(prefix='harmoniser-rpc-') as directory:
            path=Path(directory); cpp=path/'fixture.cpp'; cpp.write_text(fixture)
            subprocess.run([os.environ.get('CXX','c++'),'-std=c++11','-O2',
                f'-I{TRUNK / "src/headers"}',f'-I{TRUNK / "src/gx_head/engine"}',str(cpp),
                str(TRUNK / 'src/plugins/harmoniser.cc'),str(TRUNK / 'src/plugins/dual_harmoniser.cc'),
                '-o',str(path/'fixture')],check=True)
            subprocess.run([str(path/'fixture')],check=True,timeout=30)


if __name__ == '__main__':
    unittest.main()

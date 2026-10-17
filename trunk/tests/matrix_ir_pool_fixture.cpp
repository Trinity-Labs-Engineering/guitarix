// Exercise production ConvolverAdapter preparation/ownership with counted DSPs.
#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <sched.h>
#include <string>
#include <thread>
#include <vector>
namespace boost { struct mutex { struct scoped_lock { explicit scoped_lock(mutex&) {} }; }; }
struct GxJConvSettings {
    std::string file;
    bool operator==(const GxJConvSettings& other) const { return file == other.file; }
    bool getGainCor() const { return false; }
    float getGain() const { return 1; }
    std::string getFullIRPath() const { return file; }
    unsigned getDelay() const { return 0; }
    unsigned getOffset() const { return 0; }
    unsigned getLength() const { return 0; }
    int getGainline() const { return 0; }
};
static int configurations = 0, warmups = 0;
struct GxConvolver {
    unsigned samplerate = 48000, buffersize = 32;
    bool running = false;
    std::string file;
    void set_samplerate(unsigned value) { samplerate = value; }
    void set_buffersize(unsigned value) { buffersize = value; }
    unsigned get_samplerate() const { return samplerate; }
    unsigned get_buffersize() const { return buffersize; }
    void set_sync(bool) {}
    bool configure(std::string path, float, float, unsigned, unsigned, unsigned, unsigned, unsigned, unsigned, int) {
        ++configurations;
        if (path == "missing") return false;
        file = path;
        return true;
    }
    bool start(int, int) { running = true; return true; }
    bool compute(int, float*, float*, float*, float*) { assert(running); ++warmups; return true; }
    void set_not_runnable() { running = false; }
    void stop_process() { running = false; }
    bool checkstate() { return true; }
};
struct Engine {
    unsigned rate = 48000, block = 32;
    unsigned get_samplerate() const { return rate; }
    unsigned get_buffersize() const { return block; }
};
struct ConvolverAdapter {
    std::unique_ptr<GxConvolver> conv{new GxConvolver};
    struct PreparedIR { GxJConvSettings settings; std::unique_ptr<GxConvolver> processor; };
    std::vector<PreparedIR> prepared_irs;
    boost::mutex activate_mutex;
    Engine engine;
    GxJConvSettings jcset, configured_jcset;
    unsigned configured_samplerate = 0, configured_buffersize = 0;
    bool has_configured_ir = false;
    bool configure_current_ir();
    bool matrix_ir_ready(const GxJConvSettings&);
    bool prepare_matrix_ir(const GxJConvSettings&);
};
#include "ir_pool_under_test.h"
int main() {
    ConvolverAdapter adapter;
    adapter.jcset = {"audible"};
    assert(adapter.configure_current_ir());
    adapter.conv->running = true;
    auto* audible = adapter.conv.get();
    assert(adapter.prepare_matrix_ir({"next"}));
    assert(adapter.conv.get() == audible && audible->running && audible->file == "audible");
    assert(adapter.prepare_matrix_ir({"next"}));
    assert(configurations == 2 && warmups == 1);
    for (int i = 0; i < 1000; ++i) {
        // The production restart path first stops RT use under sync().
        adapter.conv->running = false;
        adapter.jcset = {i % 2 ? "audible" : "next"};
        assert(adapter.configure_current_ir());
        assert(adapter.conv->file == adapter.jcset.file);
        assert(adapter.matrix_ir_ready({"next"}) && adapter.matrix_ir_ready({"audible"}));
        assert(configurations == 2 && warmups == 1);
    }
    assert(!adapter.prepare_matrix_ir({"missing"}));
    assert(adapter.conv->file == "audible");
    for (int i = 0; i < 7; ++i) assert(adapter.prepare_matrix_ir({"extra" + std::to_string(i)}));
    const int count = configurations;
    assert(!adapter.prepare_matrix_ir({"overflow"}) && configurations == count);
    adapter.engine.block = 64;
    assert(!adapter.matrix_ir_ready({"audible"}) && !adapter.matrix_ir_ready({"next"}));
    adapter.prepared_irs.clear();
    assert(adapter.prepare_matrix_ir({"next"}));
    assert(adapter.prepared_irs.back().processor->get_buffersize() == 64);
    std::cout << "matrix-ir-pool-ok\n";
}

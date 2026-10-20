// Test host for the exact Service linked into Guitarix; no JUCE or DSP engine.
#include "houston/Service.h"
#include <atomic>
#include <csignal>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <iostream>
#include <jack/jack.h>
#include <cmath>

volatile std::sig_atomic_t stopped = 0;
struct Signal {
    jack_port_t* left;
    jack_port_t* right;
    unsigned rate;
    uint64_t position = 0;
    static int process(jack_nframes_t n, void* data) {
        auto& self = *static_cast<Signal*>(data);
        auto* l = static_cast<float*>(jack_port_get_buffer(self.left, n));
        auto* r = static_cast<float*>(jack_port_get_buffer(self.right, n));
        for (unsigned i = 0; i < n; ++i) {
            const double phase = 2 * 3.141592653589793 * (self.position++) / self.rate;
            l[i] = .25 * std::sin(440 * phase);
            r[i] = 1.1 * std::sin(880 * phase);
        }
        return 0;
    }
};
int main(int argc, char**) {
    std::signal(SIGTERM, [](int) { stopped = 1; });
    std::signal(SIGINT, [](int) { stopped = 1; });
    try {
        if (argc > 1) {
            jack_status_t status{};
            auto* client = jack_client_open("gx_analysis_test_signal", JackNoStartServer, &status);
            if (!client) return 2;
            Signal signal{
                jack_port_register(client, "left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0),
                jack_port_register(client, "right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0),
                jack_get_sample_rate(client)};
            jack_set_process_callback(client, Signal::process, &signal);
            if (jack_activate(client)) return 3;
            while (!stopped) std::this_thread::sleep_for(std::chrono::milliseconds(20));
            jack_deactivate(client);
            jack_client_close(client);
            return 0;
        }
        gx_houston::Service service;
        const char* server = std::getenv("JACK_DEFAULT_SERVER");
        service.start("houston_vl1", server ? server : "");
        while (!stopped) std::this_thread::sleep_for(std::chrono::milliseconds(20));
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}

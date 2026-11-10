#include "gx_route_input_gate.h"
#include <array>
#include <cassert>
#include <iostream>

int main() {
    for (unsigned rate : {44100u, 48000u, 96000u}) {
        for (unsigned block : {1u, 16u, 32u, 128u, 257u}) {
            gx_engine::RouteInputGate gate;
            gate.set_samplerate(rate);
            std::array<float, 257> audio{}, original{}, second{};
            for (unsigned i = 0; i < block; ++i) audio[i] = original[i] = (i % 7 - 3.0f) * .031f;
            gate.process(audio.data(), block);
            assert(audio == original);
            auto token = gate.request(true);
            assert(!gate.ready(token));
            unsigned elapsed = 0;
            float previous = 1;
            while (!gate.ready(token)) {
                audio.fill(1);
                second.fill(-2);
                gate.process(audio.data(), block, second.data());
                for (unsigned i = 0; i < block; ++i) {
                    assert(audio[i] <= previous && audio[i] >= 0);
                    assert(second[i] == -2 * audio[i]);
                    previous = audio[i];
                }
                elapsed += block;
                assert(elapsed <= rate * .002 + 2 * block);
            }
            // An arbitrarily large contact pulse cannot enter DSP history.
            audio.fill(100);
            second.fill(-200);
            gate.process(audio.data(), block, second.data());
            for (unsigned i = 0; i < block; ++i) assert(audio[i] == 0 && second[i] == 0);
            token = gate.request(false);
            assert(!gate.ready(token));
            previous = 0;
            elapsed = 0;
            while (!gate.ready(token)) {
                audio.fill(1);
                second.fill(-2);
                gate.process(audio.data(), block, second.data());
                for (unsigned i = 0; i < block; ++i) {
                    assert(audio[i] >= previous && audio[i] <= 1);
                    assert(second[i] == -2 * audio[i]);
                    previous = audio[i];
                }
                elapsed += block;
                assert(elapsed <= rate * .002 + 2 * block);
            }
            audio = original;
            gate.process(audio.data(), block);
            assert(audio == original);
            // A replacement request must not reuse an old endpoint ACK.
            gate.request(true);
            audio.fill(1);
            gate.process(audio.data(), 1);
            token = gate.request(false);
            assert(!gate.ready(token));
            while (!gate.ready(token)) gate.process(audio.data(), 1);
        }
    }
    std::cout << "route input gate pass\n";
}

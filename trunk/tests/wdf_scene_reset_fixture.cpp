#include <algorithm>
#include <cmath>
#include <iostream>
#include "plugins/audition/byod_drives.h"

template<class Core>
bool quiet_reset(const char* name, audition::Params defaults, int controls,
                 int level, int mode = -1) {
    for (int rate : {44100, 48000, 96000, 192000}) {
        Core core;
        core.prepare(rate, defaults);
        for (double value : {0.0, 0.5, 1.0}) {
            auto current = defaults;
            for (int i = 0; i < controls; ++i) current[i] = value;
            current[level] = 12;
            for (int selection = 0; selection < (mode < 0 ? 1 : 3); ++selection) {
                if (mode >= 0) current[mode] = selection;
                for (int repeat = 0; repeat < 2; ++repeat) {
                    core.configure(current);
                    core.reset();
                    core.configure(current);
                    double peak = 0;
                    for (int i = 0; i < rate / 4; ++i)
                        peak = std::max(peak, std::abs(core.tick(0)));
                    if (peak > 1e-7) {
                        std::cerr << name << " rate=" << rate << " control=" << value
                                  << " mode=" << selection << " repeat=" << repeat
                                  << " zero-input peak=" << peak << '\n';
                        return false;
                    }
                    // Reuse a previously sounding instance, not just a fresh one.
                    for (int i = 0; i < rate / 20; ++i)
                        core.tick(0.12 * std::sin(2 * audition::pi * 73 * i / rate));
                }
            }
        }
    }
    return true;
}

int main() {
    const bool yen = quiet_reset<audition::Yen>("Yen", {.5,.5,.6,-6}, 3, 3);
    const bool king = quiet_reset<audition::ToneKing>("Tone King", {.5,.6,6,1}, 2, 2, 3);
    return yen && king ? 0 : 1;
}

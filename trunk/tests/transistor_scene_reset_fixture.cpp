#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include "plugins/audition/transistor_fuzz.h"

template<class Model>
void check(const audition::Params& defaults, const audition::Params& target) {
    for (double rate : {44100., 48000., 96000.}) {
        Model resident, fresh;
        resident.prepare(rate, defaults);
        for (int i=0;i<1000;++i) resident.tick(.15*std::sin(i*.17));
        resident.configure(target);
        resident.reset();
        fresh.prepare(rate, target);
        double error=0;
        for (int i=0;i<10000;++i) {
            double x=i<1000 ? 0 : .15*std::sin(2*audition::pi*375*i/rate);
            error=std::max(error,std::abs(resident.tick(x)-fresh.tick(x)));
        }
        std::cout << rate << " reset error " << error << '\n';
        assert(error < 1e-7);
        // Returning to the original controls must not retain the new bias.
        resident.configure(defaults); resident.reset(); fresh.prepare(rate,defaults);
        for (int i=0;i<2000;++i) {
            double x=.1*std::sin(i*.13);
            assert(std::abs(resident.tick(x)-fresh.tick(x)) < 1e-7);
        }
    }
}
int main() {
    check<audition::Percolator>({.7,.5,0,1},{.1592578125,.10216796875,0,0});
    check<audition::BiasFuzz>({.7,.2,.3,1,.4},{.2612109375,.2,.3,1,.06140625});
}

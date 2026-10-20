#pragma once
#include <memory>
#include <string>

namespace gx_houston {
// Owns an input-only JACK observer, bounded sample queue and normal-priority
// analysis/HTTP workers. It never reads or mutates Guitarix's DSP engine.
class Service {
public:
    Service();
    ~Service();
    void start(const std::string& instance, const std::string& jackServer);
    void stop();
private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
}

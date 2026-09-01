#include <atomic>
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

#include "gx_rt_cycle_latch.h"

namespace {

timespec deadline_after(unsigned int usecs) {
    timespec deadline;
    clock_gettime(CLOCK_REALTIME, &deadline);
    const long ns_in_sec = 1000000000L;
    deadline.tv_nsec += static_cast<long>(usecs) * 1000L;
    if (deadline.tv_nsec >= ns_in_sec) {
        deadline.tv_nsec -= ns_in_sec;
        ++deadline.tv_sec;
    }
    return deadline;
}

} // namespace

int main() {
    gx_engine::RtCycleLatch latch;

    // A callback completed before this arm must not satisfy the new wait.
    latch.notify_rt();
    latch.arm();
    const auto timeout_started = std::chrono::steady_clock::now();
    assert(!latch.wait_until(deadline_after(5000)));
    const auto timeout_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - timeout_started).count();
    assert(timeout_elapsed >= 3);
    assert(timeout_elapsed < 50);

    // Exercise the arm/notify boundary repeatedly. The semaphore is only a
    // hint; generation advancement must make every raced notification stick.
    for (int iteration = 0; iteration < 1000; ++iteration) {
        std::atomic<bool> armed(false);
        std::thread rt([&]() {
            while (!armed.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }
            latch.notify_rt();
        });
        latch.arm();
        armed.store(true, std::memory_order_release);
        assert(latch.wait_until(deadline_after(50000)));
        rt.join();
    }

    // A consumed generation cannot satisfy a second wait. Ramp completion
    // loops rely on each call advancing the latch to the next RT callback.
    assert(!latch.wait_until(deadline_after(5000)));
    latch.notify_rt();
    assert(latch.wait_until(deadline_after(5000)));

    std::cout << "rt-cycle-latch-ok\n";
    return 0;
}

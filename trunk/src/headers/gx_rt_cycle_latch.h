/*
 * A generation-qualified control/RT-thread rendezvous.
 *
 * The semaphore is only a wake-up hint.  The generation is the predicate:
 * this prevents a stale semaphore token from satisfying a newly armed wait,
 * and prevents a notification racing with arm()'s drain from being lost.
 */

#pragma once

#include <atomic>
#include <cerrno>
#include <semaphore.h>
#include <time.h>

namespace gx_engine {

class RtCycleLatch {
private:
    sem_t wake_sem;
    std::atomic<unsigned int> generation_counter;
    unsigned int armed_generation;

public:
    RtCycleLatch()
        : wake_sem(), generation_counter(0), armed_generation(0) {
        sem_init(&wake_sem, 0, 0);
    }

    ~RtCycleLatch() {
        sem_destroy(&wake_sem);
    }

    RtCycleLatch(const RtCycleLatch&) = delete;
    RtCycleLatch& operator=(const RtCycleLatch&) = delete;

    void arm() {
        // Sample the predicate before draining hints.  If the RT thread runs
        // in between, generation_counter changes and wait_until() succeeds
        // even when this drain consumes that callback's semaphore token.
        armed_generation = generation_counter.load(std::memory_order_acquire);
        while (sem_trywait(&wake_sem) == 0) {
        }
    }

    void notify_rt() {
        generation_counter.fetch_add(1, std::memory_order_release);

        // Keep the semaphore bounded.  A racy missed post is harmless because
        // wait_until() rechecks the generation before and after every wait.
        int value = 0;
        if (sem_getvalue(&wake_sem, &value) == 0 && value == 0) {
            sem_post(&wake_sem);
        }
    }

    bool consume_completed_generation() {
        const unsigned int completed =
            generation_counter.load(std::memory_order_acquire);
        if (completed == armed_generation) {
            return false;
        }
        // Callers such as ramp waits intentionally invoke wait_until()
        // repeatedly without a separate arm() between audio blocks.
        armed_generation = completed;
        return true;
    }

    bool wait_until(const timespec& deadline) {
        while (!consume_completed_generation()) {
#ifdef __APPLE__
            // macOS does not provide sem_timedwait.  Existing Guitarix waits
            // were unbounded on this platform too.
            if (sem_wait(&wake_sem) == 0) {
                continue;
            }
#else
            if (sem_timedwait(&wake_sem, &deadline) == 0) {
                continue;
            }
#endif
            if (errno == EINTR) {
                continue;
            }
            // Close the timeout boundary race: a callback completing as the
            // syscall expires still satisfies the generation predicate.
            return consume_completed_generation();
        }
        return true;
    }

    unsigned int generation() const {
        return generation_counter.load(std::memory_order_acquire);
    }
};

} // namespace gx_engine

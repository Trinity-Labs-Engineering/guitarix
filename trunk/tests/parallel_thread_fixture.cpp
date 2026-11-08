// Reproduce completion arriving between the caller's predicate check and lock.
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cerrno>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include <thread>
#include <pthread.h>

static pthread_t caller;
static std::atomic<bool> inject{false}, finish{false}, entered{false};
static std::atomic<unsigned> broadcasts{0}, waits{0};
static int lock_with_completion(pthread_mutex_t* mutex) {
    if (pthread_equal(pthread_self(), caller) && inject.exchange(false)) {
        const auto before = broadcasts.load();
        finish.store(true);
        while (broadcasts.load() == before) std::this_thread::yield();
    }
    return pthread_mutex_lock(mutex);
}
static int broadcast_completion(pthread_cond_t* cond) {
    const int result = pthread_cond_broadcast(cond);
    broadcasts.fetch_add(1);
    return result;
}
static int count_wait(pthread_cond_t* cond, pthread_mutex_t* mutex, const timespec* until) {
    waits.fetch_add(1);
    return pthread_cond_timedwait(cond, mutex, until);
}
#define pthread_mutex_lock lock_with_completion
#define pthread_cond_broadcast broadcast_completion
#define pthread_cond_timedwait count_wait
#include "ParallelThread.h"
#undef pthread_mutex_lock
#undef pthread_cond_broadcast
#undef pthread_cond_timedwait

struct Job {
    unsigned input = 0, output = 0;
    void run() {
        entered.store(true);
        while (!finish.load()) std::this_thread::yield();
        output = input * 3 + 1;
    }
};
int main() {
    caller = pthread_self();
    ParallelThread thread;
    Job job;
    thread.set<0, Job, &Job::run>(&job);
    thread.start();
    while (!broadcasts.load()) std::this_thread::yield();
    assert(thread.getProcess());
    job.input = 17;
    thread.runProcess();
    while (!entered.load()) std::this_thread::yield();
    const auto before = waits.load();
    inject.store(true);
    thread.processWait();
    assert(job.output == 52);
    // Work and notification finished before the mutex was acquired: sleeping
    // here loses the notification and burns the entire 400 us timeout.
    assert(waits.load() == before);
    for (unsigned i = 0; i < 10000; ++i) {
        assert(thread.getProcess());
        job.input = i;
        thread.runProcess();
        thread.processWait();
        assert(job.output == i * 3 + 1);
    }
    thread.stop();
    puts("parallel-thread-ok");
}

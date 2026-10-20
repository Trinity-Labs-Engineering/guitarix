#include "houston/Service.h"
#include "Analysis.h"
#include "third_party/httplib.h"
#include <jack/jack.h>
#include <jack/ringbuffer.h>
#include <atomic>
#include <cstring>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <stdexcept>
#include <pthread.h>

namespace gx_houston {
namespace {
constexpr unsigned maxFrames = 8192;
constexpr size_t queueBytes = 1 << 20;
constexpr const char* portNames[] = {"signal_in", "pre_in_l", "pre_in_r", "post_in_l", "post_in_r"};
static_assert(std::atomic<unsigned>::is_always_lock_free, "RT counters must be lock-free");
static_assert(std::atomic<uint64_t>::is_always_lock_free, "RT counters must be lock-free");

int portFromEnv(const char* name, int fallback) {
    const char* text = std::getenv(name);
    if (!text || !*text) return fallback;
    char* end = nullptr;
    const long value = std::strtol(text, &end, 10);
    if (*end || value < 1 || value > 65535)
        throw std::invalid_argument(std::string("Invalid HTTP port: ") + name);
    return int(value);
}
void normalPriority() {
    sched_param param{};
    pthread_setschedparam(pthread_self(), SCHED_OTHER, &param);
}
void reply(httplib::Response& response, const Json& json) {
    response.set_header("Access-Control-Allow-Origin", "*");
    response.set_header("Cache-Control", "no-store");
    response.set_content(json.dump(), "application/json");
}
}

struct Service::Impl {
    struct Header {
        uint64_t end;
        unsigned frames, rate, generation;
    };
    jack_client_t* client = nullptr;
    std::array<jack_port_t*, 5> ports{};
    jack_ringbuffer_t* queue = nullptr;
    std::atomic<bool> stopping{false}, jackDown{false}, workerFailed{false};
    std::atomic<unsigned> sampleRate{48000}, bufferSize{0}, xruns{0}, generation{1};
    std::atomic<uint64_t> drops{0}, blocks{0};
    uint64_t endSample = 0; // exclusively owned by the JACK producer
    std::string clientName;
    std::mutex mutex; // workers only; never acquired by any JACK callback
    Analysis analysis;
    bool tunerEnabled = false;
    float cpuLoad = 0;
    Json sources = Json::object();
    std::array<bool, 5> connected{};
    std::array<std::unique_ptr<httplib::Server>, 3> servers;
    std::array<std::thread, 3> listeners;
    std::thread worker;
    std::chrono::steady_clock::time_point started = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point lastBlock = started;

    ~Impl() { stop(); }

    static int process(jack_nframes_t frames, void* context) noexcept {
        auto& self = *static_cast<Impl*>(context);
        self.endSample += frames;
        const size_t bytes = sizeof(Header) + size_t(frames) * sizeof(float) * self.ports.size();
        if (!frames || frames > maxFrames || jack_ringbuffer_write_space(self.queue) < bytes) {
            self.drops.fetch_add(1, std::memory_order_relaxed);
            return 0;
        }
        const Header header{self.endSample, frames, self.sampleRate.load(std::memory_order_relaxed),
                            self.generation.load(std::memory_order_acquire)};
        // No locks, allocation, math, connection queries or network I/O. A full
        // queue drops telemetry rather than waiting for an analysis/UI thread.
        jack_ringbuffer_write(self.queue, reinterpret_cast<const char*>(&header), sizeof(header));
        for (auto* port : self.ports) {
            const auto* data = static_cast<const char*>(jack_port_get_buffer(port, frames));
            jack_ringbuffer_write(self.queue, data, frames * sizeof(float));
        }
        self.blocks.fetch_add(1, std::memory_order_relaxed);
        return 0;
    }

    void start(const std::string& instance, const std::string& jackServer) {
        jack_status_t status{};
        auto options = JackOptions(JackNoStartServer | JackUseExactName);
        clientName = instance + "_analysis";
        if (jackServer.empty()) client = jack_client_open(clientName.c_str(), options, &status);
        else client = jack_client_open(clientName.c_str(), JackOptions(options | JackServerName),
                                       &status, jackServer.c_str());
        if (!client) throw std::runtime_error("Could not open Guitarix analysis JACK client");
        queue = jack_ringbuffer_create(queueBytes);
        if (!queue) throw std::runtime_error("Could not allocate Guitarix analysis queue");
        // Touch the queue before activation, even if the host cannot lock memory.
        std::memset(queue->buf, 0, queue->size);
        jack_ringbuffer_mlock(queue);
        for (size_t i = 0; i < ports.size(); ++i) {
            ports[i] = jack_port_register(client, portNames[i], JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
            if (!ports[i]) throw std::runtime_error("Could not register Guitarix analysis input");
        }
        if (jack_set_thread_init_callback(client, [](void*) {
#if defined(__linux__)
                // Keep the observer distinguishable from DSP and legacy tuner
                // workers in Artemis' realtime scheduler health checks.
                pthread_setname_np(pthread_self(), "gx-analysis-rt");
#endif
            }, this))
            throw std::runtime_error("Could not name Guitarix analysis callback");
        if (jack_set_process_callback(client, process, this) ||
            jack_set_sample_rate_callback(client, [](jack_nframes_t rate, void* ctx) {
                static_cast<Impl*>(ctx)->sampleRate.store(rate); return 0;
            }, this) ||
            jack_set_buffer_size_callback(client, [](jack_nframes_t frames, void* ctx) {
                static_cast<Impl*>(ctx)->bufferSize.store(frames); return 0;
            }, this) ||
            jack_set_xrun_callback(client, [](void* ctx) {
                static_cast<Impl*>(ctx)->xruns.fetch_add(1, std::memory_order_relaxed); return 0;
            }, this))
            throw std::runtime_error("Could not register Guitarix analysis callbacks");
        jack_on_shutdown(client, [](void* ctx) { static_cast<Impl*>(ctx)->jackDown.store(true); }, this);
        sampleRate.store(jack_get_sample_rate(client));
        bufferSize.store(jack_get_buffer_size(client));
        analysis.prepare(sampleRate.load());
        const int httpPorts[] = {portFromEnv("HOUSTON_TUNER_PORT", 7714),
            portFromEnv("HOUSTON_METER_PORT_PRE", 7712), portFromEnv("HOUSTON_METER_PORT_POST", 7713)};
        // Bind synchronously: collisions disable telemetry without changing DSP.
        for (unsigned i = 0; i < servers.size(); ++i) {
            servers[i] = std::make_unique<httplib::Server>();
            configureHttp(*servers[i], i);
            if (!servers[i]->bind_to_port("127.0.0.1", httpPorts[i]))
                throw std::runtime_error("Could not bind Guitarix analysis HTTP port " + std::to_string(httpPorts[i]));
        }
        if (jack_activate(client)) throw std::runtime_error("Could not activate Guitarix analysis client");
        worker = std::thread([this] {
            normalPriority();
            try { run(); }
            catch (...) { workerFailed.store(true); } // telemetry failure must not terminate DSP
        });
        for (unsigned i = 0; i < servers.size(); ++i)
            listeners[i] = std::thread([this, i] { normalPriority(); servers[i]->listen_after_bind(); });
    }

    void stop() {
        stopping.store(true);
        for (auto& server : servers) if (server) server->stop();
        for (auto& listener : listeners) if (listener.joinable()) listener.join();
        if (worker.joinable()) worker.join();
        if (client) {
            if (!jackDown.load()) jack_deactivate(client);
            jack_client_close(client);
            client = nullptr;
        }
        if (queue) { jack_ringbuffer_free(queue); queue = nullptr; }
    }

    void refreshConnections() {
        if (jackDown.load()) { connected.fill(false); return; }
        cpuLoad = jack_cpu_load(client);
        for (size_t i = 0; i < ports.size(); ++i) {
            Json list = Json::array();
            const char** names = jack_port_get_all_connections(client, ports[i]);
            if (names) {
                for (auto p = names; *p; ++p) list.push_back(*p);
                jack_free(names);
            }
            connected[i] = !list.empty();
            sources[portNames[i]] = std::move(list);
        }
    }

    void run() {
        std::vector<float> samples(ports.size() * maxFrames);
        auto nextConnections = std::chrono::steady_clock::now();
        while (!stopping.load()) {
            // Finite drain: a continuously producing JACK thread cannot keep us
            // in this loop forever. Meter envelopes use every retained block.
            size_t remaining = jack_ringbuffer_read_space(queue);
            {
                std::lock_guard<std::mutex> lock(mutex);
                while (remaining >= sizeof(Header)) {
                    Header header{};
                    jack_ringbuffer_peek(queue, reinterpret_cast<char*>(&header), sizeof(header));
                    const size_t dataBytes = size_t(header.frames) * sizeof(float) * ports.size();
                    const size_t bytes = sizeof(header) + dataBytes;
                    if (remaining < bytes) break; // producer has not published all channels yet
                    jack_ringbuffer_read_advance(queue, sizeof(header));
                    jack_ringbuffer_read(queue, reinterpret_cast<char*>(samples.data()), dataBytes);
                    remaining -= bytes;
                    if (analysis.rate != header.rate) analysis.prepare(header.rate);
                    const auto n = header.frames;
                    analysis.pre.process(samples.data() + n, samples.data() + 2*n, n, header.rate);
                    analysis.post.process(samples.data() + 3*n, samples.data() + 4*n, n, header.rate);
                    if (tunerEnabled && header.generation == generation.load())
                        analysis.append(samples.data(), n, header.end);
                    lastBlock = std::chrono::steady_clock::now();
                }
                analysis.analyse();
                analysis.expire();
                const auto now = std::chrono::steady_clock::now();
                if (now - lastBlock > std::chrono::milliseconds(150)) {
                    analysis.pre.reset(true, false);
                    analysis.post.reset(true, false);
                }
                if (now >= nextConnections) {
                    refreshConnections();
                    nextConnections = now + std::chrono::milliseconds(250);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    Json portMap(unsigned kind) const {
        if (kind == 0) return {{"signal_in", clientName + ":signal_in"}};
        const std::string prefix = kind == 1 ? "pre" : "post";
        return {{"post_in_l", clientName + ":" + prefix + "_in_l"},
                {"post_in_r", clientName + ":" + prefix + "_in_r"}};
    }

    Json health(unsigned kind) const {
        return {{"ok", !jackDown.load() && !stopping.load() && !workerFailed.load()}, {"implementation", "guitarix-native"},
            {"service", kind == 0 ? "houston-tuner" : "houston-meter"},
            {"jack_client_name", clientName}, {"sample_rate", sampleRate.load()},
            {"buffer_size", bufferSize.load()}, {"shutdown_requested", jackDown.load()},
            {"shutdown_reason", jackDown.load() ? "JACK server shutdown" : ""},
            {"uptime_seconds", std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count()},
            {"ports", portMap(kind)}};
    }

    Json snapshot(unsigned kind) const {
        const bool ready = !jackDown.load() && !stopping.load() && !workerFailed.load();
        Json root{{"service", {{"ready", ready}, {"shutdown", !ready}, {"shutdown_requested", !ready},
            {"shutdown_reason", ready ? "" : "Analysis unavailable"},
            {"last_warning", workerFailed.load() ? "Analysis worker failed" : ""},
            {"last_event", tunerEnabled ? "listening" : "idle"}, {"implementation", "guitarix-native"},
            {"uptime_seconds", health(kind)["uptime_seconds"]}}},
            {"jack", {{"client_name", clientName}, {"sample_rate", sampleRate.load()},
            {"buffer_size", bufferSize.load()}, {"xrun_count", xruns.load()},
            {"cpu_load_percent", cpuLoad},
            {"process_block_count", blocks.load()}, {"dropped_process_blocks", drops.load()},
            {"analysis_backlog_skips", analysis.backlogSkips}, {"ports", portMap(kind)}}}};
        if (!kind) {
            root["tuner"] = analysis.tuner();
            root["service"]["last_event"] = root["tuner"]["detected"].get<bool>() ? "tracking" : "listening";
        } else {
            const auto offset = kind == 1 ? 1u : 3u;
            root["meter"] = (kind == 1 ? analysis.pre : analysis.post).snapshot(
                ready && connected[offset], ready && connected[offset + 1]);
            auto& meter = root["meter"];
            meter["sample_rate"] = sampleRate.load();
            meter["buffer_size"] = bufferSize.load();
            meter["xrun_count"] = xruns.load();
            for (unsigned ch = 0; ch < 2; ++ch) {
                const std::string suffix = ch ? "r" : "l";
                const auto list = sources.value(portNames[offset + ch], Json::array());
                meter["source_ports_" + suffix] = list;
                meter["source_port_" + suffix] = list.empty() ? Json("") : list[0];
                meter["resolved_source_ports"]["post_in_" + suffix] = list;
                root["jack"]["sources"]["post_in_" + suffix] = list;
            }
        }
        return root;
    }

    Json action(unsigned kind, const std::string& name, const Json& params) {
        if (kind == 0) {
            if (name == "analysis.start" || name == "analysis.stop" || name == "service.shutdown") {
                const bool enabled = name == "analysis.start";
                if (enabled != tunerEnabled) {
                    tunerEnabled = enabled;
                    analysis.enable(tunerEnabled);
                    generation.fetch_add(1);
                }
                return {{"analysis_enabled", tunerEnabled}, {"shutdown_requested", false}};
            }
            return analysis.action(name, params);
        }
        const bool levels = name == "meter.reset" || name == "meter.reset_all" || name == "meter.reset_levels";
        const bool clipping = name == "meter.reset" || name == "meter.reset_all" || name == "meter.reset_clipping";
        if (!levels && !clipping) throw std::invalid_argument("Unknown meter action '" + name + "'.");
        (kind == 1 ? analysis.pre : analysis.post).reset(levels, clipping);
        return {{"levels_reset", levels}, {"clipping_reset", clipping}};
    }

    void configureHttp(httplib::Server& server, unsigned kind) {
        // cpp-httplib defaults to a hardware-sized pool per server. Bound both
        // threads and queued requests; the audio producer never uses this pool.
        server.new_task_queue = [] { return new httplib::ThreadPool(1, 1, 8); };
        server.set_payload_max_length(8192);
        server.set_read_timeout(0, 200000);
        server.set_write_timeout(0, 200000);
        server.set_keep_alive_max_count(4);
        server.set_keep_alive_timeout(1);
        server.Options(".*", [](const httplib::Request&, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Headers", "Content-Type");
            res.set_header("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
            res.status = 204;
        });
        for (const std::string path : {"/", "/health", "/ports", "/state", "/levels"}) {
            server.Get(path, [this, kind, path](const httplib::Request&, httplib::Response& res) {
                std::lock_guard<std::mutex> lock(mutex);
                if (path == "/health" || path == "/") reply(res, health(kind));
                else if (path == "/ports") reply(res, portMap(kind));
                else reply(res, snapshot(kind));
            });
        }
        server.Post("/api", [this, kind](const httplib::Request& req, httplib::Response& res) {
            try {
                const auto body = Json::parse(req.body);
                if (!body.is_object() || !body.contains("action") || !body["action"].is_string())
                    throw std::invalid_argument("'action' must be a string.");
                auto params = body.value("params", Json::object());
                if (params.is_null()) params = Json::object();
                if (!params.is_object()) throw std::invalid_argument("'params' must be an object.");
                const std::string name = body["action"];
                std::lock_guard<std::mutex> lock(mutex);
                const auto result = action(kind, name, params);
                reply(res, {{"ok", true}, {"action", name}, {"result", result}, {"state", snapshot(kind)}});
            } catch (const std::exception& error) {
                res.status = 400;
                reply(res, {{"ok", false}, {"error", error.what()}});
            }
        });
    }
};

Service::Service() = default;
Service::~Service() = default;
void Service::start(const std::string& instance, const std::string& jackServer) {
    stop();
    auto next = std::make_unique<Impl>();
    next->start(instance, jackServer);
    impl = std::move(next);
}
void Service::stop() { impl.reset(); }
}

/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdint>
#include <locale>
#include <sstream>
#include <string>
#include <thread>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

namespace gx_spectrum {
// One producer (the FX callback) and one consumer (the telemetry worker).
// A full queue drops telemetry, never waits for the reader. Sequence counters
// wrap with unsigned arithmetic. Neither the queue nor the server is reset
// by preset changes. HTTP/FFT work never enters Guitarix's control loop.
class Capture {
    static constexpr unsigned capacity = 16384;
    struct Sample { float left, right; unsigned rate; };
    std::array<Sample, capacity> samples{};
    std::atomic<unsigned> write{0}, read{0};
public:
    std::atomic<bool> enabled{false};
    static_assert(ATOMIC_INT_LOCK_FREE == 2 && ATOMIC_BOOL_LOCK_FREE == 2, "Spectrum capture must be lock-free");
    void push(unsigned count, const float* left, const float* right, unsigned rate) {
        if (!enabled.load(std::memory_order_relaxed)) return;
        const unsigned w = write.load(std::memory_order_relaxed);
        const unsigned available = capacity - (w - read.load(std::memory_order_acquire));
        if (count > available) return;
        for (unsigned i = 0; i < count; ++i) samples[(w + i) % capacity] = {left[i], right[i], rate};
        write.store(w + count, std::memory_order_release);
    }
    template<class Consumer> unsigned drain(Consumer consumer) {
        unsigned r = read.load(std::memory_order_relaxed);
        const unsigned w = write.load(std::memory_order_acquire), count = w - r;
        while (r != w) { const auto& s = samples[r++ % capacity]; consumer(s.left, s.right, s.rate); }
        read.store(r, std::memory_order_release);
        return count;
    }
};

class Analyzer {
public:
    static constexpr unsigned size = 4096, points = 384;
private:
    std::array<std::array<float, size>, 2> history{};
    std::array<double, size> window{};
    std::array<std::complex<double>, size / 2> twiddle{};
    std::array<std::complex<double>, size> fft{};
    unsigned position = 0, filled = 0, sample_rate = 48000, sequence = 0;
    double window_sum = 0;
    void transform(unsigned channel) {
        for (unsigned i = 0; i < size; ++i) fft[i] = history[channel][(position + i) % size] * window[i];
        for (unsigned i = 1, j = 0; i < size; ++i) {
            unsigned bit = size >> 1;
            for (; j & bit; bit >>= 1) j ^= bit;
            j ^= bit;
            if (i < j) std::swap(fft[i], fft[j]);
        }
        for (unsigned length = 2; length <= size; length <<= 1)
            for (unsigned start = 0; start < size; start += length)
                for (unsigned j = 0; j < length / 2; ++j) {
                    const auto a = fft[start + j], b = fft[start + j + length / 2] * twiddle[j * (size / length)];
                    fft[start + j] = a + b; fft[start + j + length / 2] = a - b;
                }
    }
public:
    Analyzer() {
        for (unsigned i = 0; i < size; ++i) {
            window[i] = 0.5 - 0.5 * std::cos(6.283185307179586 * i / (size - 1));
            window_sum += window[i];
        }
        for (unsigned i = 0; i < size / 2; ++i) twiddle[i] = std::polar(1.0, -6.283185307179586 * i / size);
    }
    void push(float left, float right, unsigned rate) {
        if (rate < 8000 || rate > 384000) return;
        if (rate != sample_rate) { sample_rate = rate; history = {}; position = filled = 0; }
        history[0][position] = std::isfinite(left) ? left : 0;
        history[1][position] = std::isfinite(right) ? right : 0;
        position = (position + 1) % size; filled = std::min(size, filled + 1);
    }
    void silence() { history = {}; filled = 0; }
    std::string frame() {
        std::ostringstream out; out.imbue(std::locale::classic()); out.precision(7);
        out << "{\"version\":2,\"source\":\"guitarix\",\"sequence\":" << ++sequence
            << ",\"sample_rate\":" << sample_rate << ",\"fft_size\":" << size
            << ",\"min_frequency\":20,\"max_frequency\":20000,\"frequencies\":[";
        std::array<double, points> frequencies{};
        for (unsigned i = 0; i < points; ++i) {
            frequencies[i] = 20 * std::pow(1000.0, double(i) / (points - 1));
            if (i) out << ',';
            out << frequencies[i];
        }
        out << ']';
        for (unsigned channel = 0; channel < 2; ++channel) {
            transform(channel);
            std::array<double, size / 2 + 1> db{};
            double peak_db = -96, peak_hz = 0;
            for (unsigned bin = 0; bin <= size / 2; ++bin) {
                const double hz = double(bin) * sample_rate / size;
                db[bin] = std::max(-96.0, 20 * std::log10(std::max(1e-12, 2 * std::abs(fft[bin]) / window_sum)));
                if (hz >= 20 && hz <= 20000 && db[bin] > peak_db) { peak_db = db[bin]; peak_hz = hz; }
            }
            const char* name = channel == 0 ? "left" : "right";
            out << ",\"" << name << "_magnitudes_db\":[";
            for (unsigned i = 0; i < points; ++i) {
                double value = -96;
                if (frequencies[i] < sample_rate * 0.5 && filled > 0) {
                    const double bin = frequencies[i] * size / sample_rate;
                    const unsigned low = unsigned(bin), high = std::min(size / 2, low + 1);
                    value = db[low] + (db[high] - db[low]) * (bin - low);
                    // Preserve narrow high-frequency peaks when display points
                    // span multiple FFT bins.
                    const unsigned end = std::min(size / 2, unsigned((i + 1 < points ? std::sqrt(frequencies[i] * frequencies[i+1]) : 20000) * size / sample_rate));
                    const unsigned begin = std::max(1u, unsigned((i ? std::sqrt(frequencies[i-1] * frequencies[i]) : 20) * size / sample_rate));
                    if (end > begin + 1) for (unsigned b = begin; b <= end; ++b) value = std::max(value, db[b]);
                }
                if (i) out << ',';
                out << value;
            }
            out << "],\"" << name << "_peak_frequency_hz\":" << peak_hz << ",\"" << name << "_peak_db\":" << peak_db;
        }
        out << '}'; return out.str();
    }
};

class Server {
    Capture capture;
    std::atomic<bool> stopping{false};
    std::thread worker;
    int listener = -1;
    void run() {
        Analyzer analyzer;
        using Clock = std::chrono::steady_clock;
        auto last_request = Clock::now() - std::chrono::seconds(3);
        auto last_frame = last_request;
        auto last_audio = last_request;
        bool silent = true;
        std::string cached = analyzer.frame();
        while (!stopping.load(std::memory_order_relaxed)) {
            const auto now = Clock::now();
            capture.enabled.store(now - last_request < std::chrono::seconds(2), std::memory_order_relaxed);
            const unsigned received = capture.drain([&](float l, float r, unsigned rate) { analyzer.push(l, r, rate); });
            if (received) { last_audio = now; silent = false; }
            if (!silent && now - last_audio > std::chrono::milliseconds(250)) {
                analyzer.silence(); cached = analyzer.frame(); silent = true;
            }
            if (received && now - last_frame >= std::chrono::milliseconds(16)) { cached = analyzer.frame(); last_frame = now; }
            pollfd ready{listener, POLLIN, 0};
            if (::poll(&ready, 1, capture.enabled.load(std::memory_order_relaxed) ? 5 : 50) <= 0) continue;
            const int client = ::accept(listener, nullptr, nullptr);
            if (client < 0) continue;
            // Bound a slow client's time; it can only delay telemetry.
            timeval timeout{0, 50000};
            ::setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
            ::setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
            char request[2048]{};
            const int bytes = int(::recv(client, request, sizeof(request) - 1, 0));
            const bool spectrum = bytes > 0 && std::string(request).find("GET /spectrum ") == 0;
            const bool health = bytes > 0 && std::string(request).find("GET /health ") == 0;
            if (spectrum) last_request = Clock::now();
            const std::string body = spectrum ? cached : health ? "{\"ok\":true,\"service\":\"guitarix-spectrum\"}" : "{}";
            const std::string response = std::string("HTTP/1.1 ") + (spectrum || health ? "200 OK" : "404 Not Found")
                + "\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nCache-Control: no-store\r\nConnection: close\r\nContent-Length: "
                + std::to_string(body.size()) + "\r\n\r\n" + body;
            size_t sent = 0;
            while (sent < response.size()) {
                const auto n = ::send(client, response.data() + sent, response.size() - sent, MSG_NOSIGNAL);
                if (n <= 0) break;
                sent += size_t(n);
            }
            ::close(client);
        }
    }
public:
    ~Server() {
        stopping.store(true, std::memory_order_relaxed);
        if (worker.joinable()) worker.join();
        if (listener >= 0) ::close(listener);
    }
    bool start(unsigned port) {
        if (worker.joinable()) return true;
        if (!port || port > 65535) return false;
        listener = ::socket(AF_INET, SOCK_STREAM, 0);
        if (listener < 0) return false;
        int reuse = 1; ::setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        sockaddr_in address{}; address.sin_family = AF_INET; address.sin_port = htons(port);
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (::bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) || ::listen(listener, 4)) {
            ::close(listener); listener = -1; return false;
        }
        ::fcntl(listener, F_SETFL, O_NONBLOCK);
        try { worker = std::thread([this] { run(); }); }
        catch (...) { ::close(listener); listener = -1; return false; }
        return true;
    }
    void push(unsigned count, const float* left, const float* right, unsigned rate) { capture.push(count, left, right, rate); }
};
} // namespace gx_spectrum

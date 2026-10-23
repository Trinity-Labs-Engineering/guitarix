// Link against the production libnam.a with NAM_SAMPLE_FLOAT to measure live
// graph allocations and compare large callbacks against bounded processing.
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <malloc.h>
#include <map>
#include <string>
#include <vector>

#include "container.h"
#include "get_dsp.h"
#include "gx_nam_block_adapter.h"
#include "lstm.h"
#include "wavenet/model.h"

static void register_parsers() {
    auto& registry = nam::ConfigParserRegistry::instance();
    if (!registry.has("Linear")) registry.registerParser("Linear", nam::linear::create_config);
    if (!registry.has("LSTM")) registry.registerParser("LSTM", nam::lstm::create_config);
    if (!registry.has("WaveNet")) registry.registerParser("WaveNet", nam::wavenet::create_config);
    if (!registry.has("SlimmableContainer"))
        registry.registerParser("SlimmableContainer", nam::container::create_config);
}

static std::map<std::string, long> memory_kib() {
    malloc_trim(0);
    std::ifstream input("/proc/self/smaps_rollup");
    std::map<std::string, long> result;
    for (std::string line; std::getline(input, line);) {
        for (const std::string key : {"Pss", "Private_Dirty", "Locked"}) {
            if (line.rfind(key + ":", 0) == 0)
                result[key] = std::stol(line.substr(key.size() + 1));
        }
    }
    const auto heap = mallinfo2();
    result["LiveHeap"] = (heap.uordblks + heap.hblkhd) / 1024;
    return result;
}

static std::unique_ptr<nam::DSP> load(const std::filesystem::path& path, double size, bool bounded) {
    auto dsp = bounded ? nam::get_dsp(path, nam::PrewarmMode::SKIP, size)
                       : nam::get_dsp(path, nam::PrewarmMode::SKIP);
    if (auto* slim = dynamic_cast<nam::SlimmableModel*>(dsp.get())) slim->SetSlimmableSize(size);
    dsp->ResetAndPrewarm(dsp->GetExpectedSampleRate(), bounded ? gx_engine::kNamModelBlockFrames : 8256);
    return dsp;
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "usage: nam-memory-benchmark MODEL SIZE legacy|bounded|compare [COPIES]\n";
        return 2;
    }
    register_parsers();
    const std::filesystem::path path(argv[1]);
    const double size = std::stod(argv[2]);
    const std::string mode(argv[3]);
    if (mode == "compare") {
        auto reference = load(path, size, false);
        auto bounded = load(path, size, true);
        reference->Reset(reference->GetExpectedSampleRate(), 32769);
        bounded->Reset(bounded->GetExpectedSampleRate(), gx_engine::kNamModelBlockFrames);
        double max_error = 0.0;
        for (const int frames : {1, 32, 255, 256, 257, 8192, 32769, 17}) {
            std::vector<NAM_SAMPLE> input(frames), expected(frames), actual(frames + 2, 123.0f);
            for (int i = 0; i < frames; ++i) input[i] = .1f * std::sin(i * .37f);
            NAM_SAMPLE* in = input.data();
            NAM_SAMPLE* out = expected.data();
            reference->process(&in, &out, frames);
            gx_engine::process_nam_mono_bounded(bounded.get(), input.data(), actual.data() + 1, frames);
            if (actual.front() != 123.0f || actual.back() != 123.0f) return 3;
            for (int i = 0; i < frames; ++i) {
                if (!std::isfinite(actual[i + 1])) return 4;
                max_error = std::max(max_error, std::abs(double(actual[i + 1]) - expected[i]));
            }
        }
        std::cout << "max_absolute_error=" << max_error << '\n';
        return max_error <= 1.0e-5 ? 0 : 5;
    }
    if (mode != "legacy" && mode != "bounded") return 2;
    const int copies = argc > 4 ? std::stoi(argv[4]) : 12;
    if (copies < 1 || copies > 64) return 2;
    std::vector<std::unique_ptr<nam::DSP>> models;
    models.reserve(copies);
    const auto before = memory_kib();
    for (int i = 0; i < copies; ++i) models.push_back(load(path, size, mode == "bounded"));
    const auto after = memory_kib();
    std::cout << "mode=" << mode << " size=" << size << " copies=" << copies;
    for (const auto& [key, value] : after)
        std::cout << ' ' << key << "_delta_KiB=" << value - before.at(key);
    std::cout << '\n';
}

/* Guitarix parallel virtual loop helpers. GPL-2.0-or-later. */
#pragma once

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>

namespace gx_engine {

// Registration is a control-thread operation. The scoped indirection preserves
// the original DSP object passed to generated and handwritten callbacks while
// giving each instance independent parameter storage and identifiers.
class ScopedParamNamespace {
    const ParamReg& parent;
    std::string replacement;
    ScopedParamNamespace* previous;
    static ScopedParamNamespace*& current() {
        static thread_local ScopedParamNamespace* value = nullptr;
        return value;
    }
    std::string rename(const char* id) const {
        const std::string value(id);
        const size_t dot = value.find('.');
        return replacement.empty()
            ? value.substr(0, dot) + "__2" + (dot == std::string::npos ? "" : value.substr(dot))
            : replacement + (dot == std::string::npos ? "" : value.substr(dot));
    }
    static float* reg_float(const char* id, const char* name, const char* type,
                           const char* tip, float* var, float value, float low,
                           float high, float step, const value_pair* values) {
        ScopedParamNamespace& self = *current();
        return self.parent.registerFloatVar(self.rename(id).c_str(), name, type,
            tip, var, value, low, high, step, values);
    }
    static int* reg_int(const char* id, const char* name, const char* type,
                       const char* tip, int* var, int value, int low, int high,
                       const value_pair* values) {
        ScopedParamNamespace& self = *current();
        return self.parent.registerIntVar(self.rename(id).c_str(), name, type,
            tip, var, value, low, high, values);
    }
    static bool* reg_bool(const char* id, const char* name, const char* type,
                         const char* tip, bool* var, bool value) {
        ScopedParamNamespace& self = *current();
        return self.parent.registerBoolVar(self.rename(id).c_str(), name, type,
            tip, var, value);
    }
public:
    ParamReg reg;
    ScopedParamNamespace(const ParamReg& source, PluginDef* plugin,
                         const std::string& prefix = std::string())
        : parent(source), replacement(prefix), previous(current()), reg(source) {
        reg.plugin = plugin;
        reg.registerFloatVar = reg_float;
        reg.registerIntVar = reg_int;
        reg.registerBoolVar = reg_bool;
        current() = this;
    }
    ~ScopedParamNamespace() { current() = previous; }
};

class SecondPluginInstance : public PluginDef {
    PluginDef* child;
    std::string instance_id;
    std::string parameter_prefix;
    std::vector<std::string> group_names;
    std::vector<const char*> group_pointers;
    static SecondPluginInstance& self(PluginDef* p) {
        return *static_cast<SecondPluginInstance*>(p);
    }
    static void mono(int n, float* in, float* out, PluginDef* p) {
        PluginDef* c = self(p).child; c->mono_audio(n, in, out, c);
    }
    static void stereo(int n, float* a, float* b, float* x, float* y, PluginDef* p) {
        PluginDef* c = self(p).child; c->stereo_audio(n, a, b, x, y, c);
    }
    static void init(unsigned int rate, PluginDef* p) {
        PluginDef* c = self(p).child; c->set_samplerate(rate, c);
    }
    static int activate(bool value, PluginDef* p) {
        PluginDef* c = self(p).child; return c->activate_plugin(value, c);
    }
    static void reset(PluginDef* p) {
        PluginDef* c = self(p).child; c->clear_state(c);
    }
    static int register_parameters(const ParamReg& reg) {
        auto& instance = self(reg.plugin);
        PluginDef* child = instance.child;
        ScopedParamNamespace names(reg, child, instance.parameter_prefix);
        return child->register_params(names.reg);
    }
    static void destroy(PluginDef* p) { delete &self(p); }
public:
    explicit SecondPluginInstance(PluginDef* original, const std::string& prefix = std::string())
        : PluginDef(*original), child(original),
          instance_id(prefix.empty() ? std::string(original->id) + "__2" : prefix),
          parameter_prefix(prefix) {
        id = instance_id.c_str();
        mono_audio = child->mono_audio ? mono : nullptr;
        stereo_audio = child->stereo_audio ? stereo : nullptr;
        set_samplerate = child->set_samplerate ? init : nullptr;
        activate_plugin = child->activate_plugin ? activate : nullptr;
        clear_state = child->clear_state ? reset : nullptr;
        register_params = child->register_params ? register_parameters : nullptr;
        delete_instance = destroy;
        // The legacy GTK layouts contain literal first-instance parameter ids.
        // Houston renders controls from the catalog; never bind a clone's GTK
        // controls to the original effect by reusing those layouts.
        load_ui = nullptr;
        if (child->groups) {
            for (const char** p = child->groups; *p; p += 2) {
                std::string group(p[0]);
                if (!group.empty() && group[0] == '.') {
                    const size_t end = group.find('.', 1);
                    if (prefix.empty())
                        group.insert(end == std::string::npos ? group.size() : end, "__2");
                    else
                        group = "." + prefix + (end == std::string::npos ? "" : group.substr(end));
                }
                group_names.push_back(group);
                group_names.push_back(p[1]);
            }
            for (const auto& name : group_names) group_pointers.push_back(name.c_str());
            group_pointers.push_back(nullptr);
            groups = group_pointers.data();
        }
    }
    ~SecondPluginInstance() { if (child->delete_instance) child->delete_instance(child); }
};

// The HAE balance law: at the centre each splitter output has unity gain;
// the combiner averages the lanes. At either extreme it selects one lane.
inline float matrix_balance(float balance) {
    return std::isfinite(balance) ? std::max(0.0f, std::min(1.0f, (balance + 1.0f) * 0.5f)) : 0.5f;
}
inline void matrix_split(float* upper, float* lower, int n, float balance) {
    const float b = matrix_balance(balance);
    const float a_gain = std::min(1.0f, 2.0f * (1.0f - b));
    const float b_gain = std::min(1.0f, 2.0f * b);
    for (int i = 0; i < n; ++i) {
        lower[i] = upper[i] * b_gain;
        upper[i] *= a_gain;
    }
}
inline void matrix_combine(float* upper, const float* lower, int n, float balance) {
    const float b = matrix_balance(balance);
    for (int i = 0; i < n; ++i) upper[i] = upper[i] * (1.0f - b) + lower[i] * b;
}

class MatrixBoundary : public PluginDef {
    int smoothing_samples = 480;
    int remaining = 0;
    float current = 0.0f;
    float target = 0.0f;
    float step = 0.0f;
    float next_balance() {
        const float requested = matrix_balance(balance) * 2.0f - 1.0f;
        if (requested != target) {
            target = requested;
            remaining = smoothing_samples;
            step = (target - current) / remaining;
        }
        if (remaining > 0) {
            current += step;
            if (--remaining == 0) current = target;
        }
        return current;
    }
public:
    float balance;
    explicit MatrixBoundary(bool split) : PluginDef(), balance(0.0f) {
        version = PLUGINDEF_VERSION;
        flags = static_cast<int>(PGN_GUI) | (split ? PGNI_MATRIX_SPLITTER : PGNI_MATRIX_COMBINER);
        id = split ? "matrix_splitter" : "matrix_combiner";
        name = split ? "Splitter" : "Combiner";
        category = "Routing";
        mono_audio = passthrough;
        register_params = register_parameters;
        set_samplerate = init;
        clear_state = reset;
        delete_instance = destroy;
    }
    void split_audio(float* upper, float* lower, int n) {
        for (int i = 0; i < n; ++i) matrix_split(upper + i, lower + i, 1, next_balance());
    }
    void combine_audio(float* upper, const float* lower, int n) {
        for (int i = 0; i < n; ++i) matrix_combine(upper + i, lower + i, 1, next_balance());
    }
    static void passthrough(int n, float* in, float* out, PluginDef*) {
        if (in != out) std::copy(in, in + n, out);
    }
private:
    static void init(unsigned int rate, PluginDef* p) {
        auto& self = *static_cast<MatrixBoundary*>(p);
        self.smoothing_samples = std::max(1, static_cast<int>(rate / 100));
        reset(p);
    }
    static void reset(PluginDef* p) {
        auto& self = *static_cast<MatrixBoundary*>(p);
        self.current = self.target = matrix_balance(self.balance) * 2.0f - 1.0f;
        self.remaining = 0;
        self.step = 0;
    }
    static int register_parameters(const ParamReg& reg) {
        auto& self = *static_cast<MatrixBoundary*>(reg.plugin);
        const std::string id = std::string(self.id) + ".balance";
        reg.registerFloatVar(id.c_str(), "Balance", "S", "Upper / lower lane balance",
                             &self.balance, 0.0f, -1.0f, 1.0f, 0.01f, nullptr);
        return 0;
    }
    static void destroy(PluginDef* p) { delete static_cast<MatrixBoundary*>(p); }
};
} // namespace gx_engine

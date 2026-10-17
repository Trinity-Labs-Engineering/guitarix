/*
 * Copyright (C) 2011 Hermann Meyer, James Warden, Andreas Degert, Pete Shorthose
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

// utility class
// FIXME should be moved somewhere else
struct stringcomp {
    inline bool operator() (const char* lhs, const char* rhs) const {
	return strcmp(lhs, rhs) < 0;
    }
};

namespace gx_engine {

class EngineControl;

/****************************************************************
 ** class Plugin
 ** Defines audio processing module and variables for
 ** user interface
*/

enum {			       // additional flags for PluginDef (used internally)
    PGNI_DYN_POSITION = 0x10000, // plugin is part of dynamically ordered rack
    PGNI_NOT_OWN      = 0x20000, // not owned by PluginList
    PGNI_UI_REG       = 0x40000, // Plugin registered in user interface
    PGNI_IS_LV2       = 0x80000, // Plugin is in LV2 format
    PGNI_IS_LADSPA    = 0x100000, // Plugin is in LADSPA format
    // Stateful processors whose cold reset causes a long audible startup may
    // keep their state across resident bypass/re-entry. Use this narrowly:
    // effects with audible tails or modulation history must still reset.
    PGNI_RESIDENT_PRESERVE_STATE = 0x200000,
    // Input-dependent processors can stay current without affecting the
    // audible chain: while resident and bypassed, run their mono DSP on a
    // discarded copy of the signal at their rack position. This also implies
    // that re-entry must not clear the state which the background pass warmed.
    PGNI_RESIDENT_TRACK_BYPASS = 0x400000,
    PGNI_MATRIX_SPLITTER = 0x800000,
    PGNI_MATRIX_COMBINER = 0x1000000,
    // Snapshot bypass normally preserves resident DSP. Plugins which require
    // lifecycle/reset work must explicitly opt out and use set_scene instead.
    PGNI_SNAPSHOT_RESET_REQUIRED = 0x2000000,
};

class Plugin {
private:
    PluginDef *pdef;
    BoolParameter *p_box_visible; ///< In Rack: UI Interface Box visible
    BoolParameter *p_plug_visible; ///< minibox visible (false: full box)
    BoolParameter *p_on_off;	   ///< Audio Processing
    // Houston song scenes may keep a module in the immutable processing
    // chain while its normal on/off control selects a transparent bypass.
    // This flag is intentionally hidden, non-MIDI and non-preset state.
    BoolParameter *p_scene_resident;
    IntParameter  *p_position; ///< Position in Rack / Audio Processing Chain
    IntParameter  *p_effect_post_pre; ///< pre/post amp position (post = 0)
    IntParameter  *p_matrix_lane; ///< 0 serial, 1 upper, 2 lower
    BoolParameter *p_matrix_source; ///< stereo processor in the mono virtual loop
    BoolParameter *p_matrix_resident; ///< keep a pooled stereo processor in the mono loop
    // Lock-free snapshot of the on/off and scene-resident controls. Parameter
    // callbacks only schedule a chain transaction; the snapshot is published
    // while the old chain is ramped down (or externally muted).
    mutable int rt_scene_state;
    int pos_tmp;
    void set_midi_on_off_blocked(bool v);
    int desired_rt_scene_state() const;
public:
    PluginDef *get_pdef() { return pdef; }
    void set_pdef(PluginDef *p) { pdef = p; }
    enum { POST_WEIGHT = 2000 };
    Plugin(PluginDef *pl=0);
    Plugin(gx_system::JsonParser& jp, ParamMap& pmap);
    void writeJSON(gx_system::JsonWriter& jw);
    bool get_box_visible() const { return p_box_visible && p_box_visible->get_value(); }
    bool get_plug_visible() const { return p_plug_visible && p_plug_visible->get_value(); }
    bool get_on_off() const { return p_on_off->get_value(); }
    bool get_scene_resident() const {
        return p_scene_resident && p_scene_resident->get_value();
    }
    bool get_processing_active() const {
        return get_on_off() || get_scene_resident();
    }
    bool get_rt_on_off() const {
        return (gx_system::atomic_get(rt_scene_state) & 1) != 0;
    }
    int get_matrix_lane() const { return p_matrix_lane ? p_matrix_lane->get_value() : 0; }
    bool get_matrix_source() const { return p_matrix_source && p_matrix_source->get_value(); }
    bool get_matrix_resident() const { return p_matrix_resident && p_matrix_resident->get_value(); }
    bool get_matrix_mono() const { return get_matrix_source() || get_matrix_resident(); }
    int get_rt_matrix_lane() { return (gx_system::atomic_get(rt_scene_state) >> 2) & 3; }
    bool get_rt_matrix_source() { return (gx_system::atomic_get(rt_scene_state) & 16) != 0; }
    bool get_rt_matrix_mono() { return (gx_system::atomic_get(rt_scene_state) & (16 | 32)) != 0; }
    // Only changing callback ownership needs a drain; selecting a different
    // source within the prepared mono pool does not transfer a processor.
    bool matrix_source_state_changed() { return get_matrix_mono() != get_rt_matrix_mono(); }
    bool rt_scene_state_changed() {
        return gx_system::atomic_get(rt_scene_state) != desired_rt_scene_state();
    }
    // Returns false if a resident plugin could not be reactivated. In that
    // case its normal on/off parameter and RT snapshot remain bypassed.
    bool commit_rt_scene_state();
    bool supports_snapshot_bypass() const {
        return get_scene_resident() && !get_matrix_source() &&
            !(pdef->flags & (PGNI_MATRIX_SPLITTER | PGNI_MATRIX_COMBINER |
                            PGNI_SNAPSHOT_RESET_REQUIRED)) &&
            (pdef->mono_audio || pdef->stereo_audio);
    }
    static void apply_snapshot_bypass(void* target, double value) {
        Plugin* plugin = static_cast<Plugin*>(target);
        plugin->p_on_off->set_snapshot_value(value != 0);
        const int old = gx_system::atomic_get(plugin->rt_scene_state);
        gx_system::atomic_set(&plugin->rt_scene_state, (old & ~1) | (value != 0 ? 1 : 0));
    }
    // Resolved bank controls only: no observers or lifecycle calls on JACK.
    static double read_matrix_state(void* target) {
        return gx_system::atomic_get(static_cast<Plugin*>(target)->rt_scene_state);
    }
    static void apply_matrix_state(void* target, double value) {
        Plugin* p = static_cast<Plugin*>(target);
        const int state = static_cast<int>(value);
        p->p_on_off->set_snapshot_value(state & 1);
        if (p->p_scene_resident) p->p_scene_resident->set_snapshot_value(state & 2);
        if (p->p_matrix_lane) p->p_matrix_lane->set_snapshot_value((state >> 2) & 3);
        if (p->p_matrix_source) p->p_matrix_source->set_snapshot_value(state & 16);
        if (p->p_matrix_resident) p->p_matrix_resident->set_snapshot_value(state & 32);
        gx_system::atomic_set(&p->rt_scene_state, state);
    }
    int get_position() const { return p_position->get_value(); }
    int get_effect_post_pre() const { return p_effect_post_pre->get_value(); }
    void set_box_visible(bool v) const { if (p_box_visible) p_box_visible->set(v); }
    void set_plug_visible(bool v) const { if (p_plug_visible) p_plug_visible->set(v); }
    void set_on_off(bool v) const { p_on_off->set(v); }
    void clear_matrix_routing() const {
        if (p_matrix_source) p_matrix_source->set(false);
        if (p_matrix_resident) p_matrix_resident->set(false);
        if (p_matrix_lane) p_matrix_lane->set(0);
    }
    void set_scene_resident(bool v) const {
        if (p_scene_resident) p_scene_resident->set(v);
    }
    void set_position(int v) const { p_position->set(v); }
    void set_effect_post_pre(int v) const { p_effect_post_pre->set(v); }
    const std::string& id_box_visible() const { return p_box_visible->id(); }
    const std::string& id_plug_visible() const { return p_plug_visible->id(); }
    const std::string& id_on_off() const { return p_on_off->id(); }
    const std::string& id_scene_resident() const {
        return p_scene_resident->id();
    }
    const std::string& id_position() const { return p_position->id(); }
    const std::string& id_effect_post_pre() const { return p_effect_post_pre->id(); }
    inline int position_weight() { return get_effect_post_pre() ? get_position() : get_position() + POST_WEIGHT; }
    void register_vars(ParamMap& param, EngineControl& seq);
    void copy_position(const Plugin& plugin);
    friend class PluginListBase;
    friend class PluginList;
    friend void printlist(const char *title, const list<Plugin*>& modules, bool header);
};

/****************************************************************
 ** class UiBuilderBase
 */

class UiBuilderBase: public UiBuilder {
public:
    virtual bool load(Plugin *p) = 0;
};

/****************************************************************
 ** class ParamRegImpl
 */

class ParamRegImpl: public ParamReg {
private:
    static ParamMap *pmap;
    static float *registerFloatVar_(
	const char* id, const char* name, const char* tp,
	const char* tooltip, float* var, float val,
	float low, float up, float step, const value_pair* values=0);
    static int *registerIntVar_(
	const char* id, const char* name, const char* tp,
	const char* tooltip, int* var, int val,
	int low, int up, const value_pair* values=0);
    static bool *registerBoolVar_(
	const char* id, const char* name, const char* tp,
	const char* tooltip, bool* var, bool val);
public:
    ParamRegImpl(ParamMap* pm);
};

/****************************************************************
 ** class PluginList
 ** container of plugins for all processing chains
 */

enum PluginPos { // where to add a plugin (per processing chain)
    PLUGIN_POS_START,
    PLUGIN_POS_RACK,
    PLUGIN_POS_END		// keep last one
};

typedef PluginDef *(*plugindef_creator)();

class PluginListBase {
public:
    typedef pair<const std::string, Plugin*> map_pair;
    typedef map<const std::string, Plugin*> pluginmap;
protected:
    enum PluginPosInternal {
	PLUGIN_POS_RACK_STEREO = PLUGIN_POS_END+1,
	PLUGIN_POS_COUNT		// keep last one
    };
    pluginmap pmap;
    sigc::signal<void,const char*,bool> insert_remove;
public:
    PluginListBase();
    ~PluginListBase();
    void cleanup();
    Plugin *find_plugin(const std::string& id) const;
    Plugin *lookup_plugin(const std::string& id) const;
    void append_rack(UiBuilderBase& ui);
    void writeJSON(gx_system::JsonWriter& jw);
    void readJSON(gx_system::JsonParser& jp, ParamMap& pmap);
    pluginmap::iterator begin() { return pmap.begin(); }
    pluginmap::iterator end() { return pmap.end(); }
    int insert_plugin(Plugin *pvars);
    void update_plugin(Plugin *pvars);
    void delete_module(Plugin *pl);
};

class PluginList: public PluginListBase {
    EngineControl& seq;
    int plugin_pos[PLUGIN_POS_COUNT];
    int add_module(Plugin *pl, PluginPos pos, int flags);
public:
    PluginList(EngineControl& seq);
    ~PluginList();
    void set_samplerate(int samplerate); // call set_samplerate of all plugins
    int load_from_path(const string& path, PluginPos pos = PLUGIN_POS_RACK);
    int load_library(const string& path, PluginPos pos = PLUGIN_POS_RACK);
    int add(Plugin *pl, PluginPos pos, int flags);
    Plugin *add(PluginDef *p, PluginPos pos = PLUGIN_POS_RACK, int flags=0);
    int add(PluginDef **p, PluginPos pos = PLUGIN_POS_RACK, int flags=0);
    int add(plugindef_creator *p, PluginPos pos = PLUGIN_POS_RACK, int flags=0);
    int check_version(PluginDef *p);
    void registerGroup(PluginDef *pd, ParameterGroups& groups);
    void registerParameter(Plugin *pl, ParamMap& param, ParamRegImpl& preg);
    void registerPlugin(Plugin *pl, ParamMap& param, ParameterGroups& groups);
    void unregisterGroup(PluginDef *pd, ParameterGroups& groups);
    void unregisterParameter(Plugin *pl, ParamMap& param);
    void rescueParameter(Plugin *pl, ParamMap& param);
    void unregisterPlugin(Plugin *pl, ParamMap& param, ParameterGroups& groups);
    void registerAllPlugins(ParamMap& param, ParameterGroups& groups);
    void ordered_mono_list(list<Plugin*>& mono, int mode);
    void ordered_stereo_list(list<Plugin*>& stereo, int mode);
    void ordered_processing_mono_list(list<Plugin*>& mono, int mode);
    void ordered_processing_stereo_list(list<Plugin*>& stereo, int mode);
    bool rt_scene_state_changed();
    bool commit_rt_scene_states();
    bool matrix_sources_changed();
    bool validate_matrix(const std::map<std::string, int>& values, std::string& error) const;
    void ordered_list(list<Plugin*>& l, bool stereo, int flagmask, int flagvalue);
    sigc::signal<void,const char*,bool>& signal_insert_remove() { return insert_remove; }
#ifndef NDEBUG
    void printlist(bool ordered = true);
#endif
};

#ifndef NDEBUG
void printlist(const char *title, const list<Plugin*>& modules, bool header=true);
#else 
inline void printlist(const char *, const list<Plugin*>&, bool=true) {}
#endif

} // !namespace gx_engine

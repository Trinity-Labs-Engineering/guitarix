/*
 * Copyright (C) 2009, 2010 Hermann Meyer, James Warden, Andreas Degert
 * Copyright (C) 2011 Pete Shorthose
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
 * --------------------------------------------------------------------------
 *
 *  This is the gx_head interface to the jackd audio / midi server
 *
 * --------------------------------------------------------------------------
 */

#include <errno.h>              // NOLINT
#include <algorithm>
#include <vector>
#if defined(__linux__)
#include <sys/prctl.h>
#endif
#ifndef GUITARIX_AS_PLUGIN
#include <jack/statistics.h>    // NOLINT
#include <jack/jack.h>          // NOLINT
#include <jack/thread.h>        // NOLINT
#endif
#include "engine.h"           // NOLINT

#ifdef HAVE_JACK_SESSION
#include <dlfcn.h>
#endif


namespace gx_jack {
#ifndef GUITARIX_AS_PLUGIN

static_assert(ATOMIC_INT_LOCK_FREE == 2,
              "JACK telemetry requires lock-free 32-bit atomics");
static_assert(ATOMIC_LLONG_LOCK_FREE == 2,
              "JACK telemetry requires lock-free 64-bit atomics");

static inline void set_jack_thread_name(const char *name) {
#if defined(__linux__)
    prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(name), 0, 0, 0);
#else
    (void)name;
#endif
}

/****************************************************************
 ** class GxRtCheck
 ** check if user have realtime priority
 */

GxRtCheck::GxRtCheck() :
    isRT(false),
    _execute(true)
    {run();}

GxRtCheck::~GxRtCheck() {}

bool GxRtCheck::run_check() {
    isRT = true;
#if defined(__linux__) || defined(_UNIX) || defined(__APPLE__)
    sched_param sch_params;
    sch_params.sched_priority = 50;
    if (pthread_setschedparam(_thd.native_handle(), SCHED_FIFO, &sch_params)) {
        gx_print_info(
            _("Jack init"),
            boost::format(_("Can't use Realtime Priority")));
        isRT = false;
    }
#elif defined(_WIN32)
    // HIGH_PRIORITY_CLASS, THREAD_PRIORITY_TIME_CRITICAL
    if (SetThreadPriority(_thd.native_handle(), 15)) {
        isRT = false;
    }
#else
    //system does not supports thread priority!
    isRT = false;
#endif
    _execute.store(false, std::memory_order_release);
    if (_thd.joinable()) {
        cv.notify_one();
        _thd.join();
    }
    return isRT;
}

void GxRtCheck::run() {
    _thd = std::thread([this]() {
        while (_execute.load(std::memory_order_acquire)) {
            std::unique_lock<std::mutex> lk(m);
            cv.wait(lk);
        }
    });
}

/****************************************************************
 ** class GxJack
 ****************************************************************/

static const char *jack_amp_postfix = "_amp";
static const char *jack_fx_postfix = "_fx";

string GxJack::get_default_instancename() {
    static const char *default_jack_instancename = "gx_head";
    return default_jack_instancename;
}


/****************************************************************
 ** rt_watchdog
 */

static unsigned int rt_watchdog_counter;

#ifndef SCHED_IDLE
#define SCHED_IDLE SCHED_OTHER  // non-linux systems
#endif

static void *rt_watchdog_run(void *p) {
    struct sched_param  spar;
    spar.sched_priority = 0;
    pthread_setschedparam(pthread_self(), SCHED_IDLE, &spar);
    while (true) {
	gx_system::atomic_set(&rt_watchdog_counter, 0);
	usleep(1000000);
    }
    return NULL;
}

static int rt_watchdog_limit = 0;

static void rt_watchdog_start() {
    if (rt_watchdog_limit > 0) {
	pthread_attr_t      attr;
	pthread_attr_init(&attr);
	pthread_t pthr;
	if (pthread_create(&pthr, &attr, rt_watchdog_run, 0)) {
	    gx_print_error("watchdog", _("can't create thread"));
	}
	pthread_attr_destroy(&attr);
    }
}

static inline bool rt_watchdog_check_alive(unsigned int bs, unsigned int sr) {
    if (rt_watchdog_limit > 0) {
	if (gx_system::atomic_get(rt_watchdog_counter) > rt_watchdog_limit*(2*sr)/bs) {
	    return false;
	}
	gx_system::atomic_inc(&rt_watchdog_counter);
    }
    return true;
}


/****************************************************************
 ** class MidiCC
 */

MidiCC::MidiCC(gx_engine::GxEngine& engine_)
    : engine(engine_) {
    for (int i = 0; i < max_midi_cc_cnt; i++) {
        send_cc[i] = false;
    }
}

bool MidiCC::send_midi_cc(int _cc, int _pg, int _bgn, int _num) {
    int c = engine.controller_map.get_midi_channel();
    if (c) _cc |=c-1;
    for(int i = 0; i < max_midi_cc_cnt; i++) {
        if (send_cc[i].load(std::memory_order_acquire)) {
            if (cc_num[i] == _cc && pg_num[i] == _pg &&
                bg_num[i] == _bgn && me_num[i] == _num)
                return true;
        } else if (!send_cc[i].load(std::memory_order_acquire)) {
            cc_num[i] = _cc;
            pg_num[i] = _pg;
            bg_num[i] = _bgn;
            me_num[i] = _num;
            send_cc[i].store(true, std::memory_order_release);
            return true;
        }
    }
#ifndef NDEBUG
    cerr << "Internal error: MidiCC overflow" << endl;
    assert(false);
#endif
    return false;
}

/****************************************************************
 ** GxJack ctor, dtor
 */

GxJack::GxJack(gx_engine::GxEngine& engine_)
    : sigc::trackable(),
      rtc(),
      IS_RT(false),
      engine(engine_),
      jack_is_down(false),
      jack_is_exit(true),
      bypass_insert(false),
      mmessage(engine_),
#ifdef HAVE_JACK_SESSION
      session_event(0),
      session_event_ins(0),
      session_callback_seen(0),
#endif
      connection_queue(),
      connection_changed(),
      buffersize_change(),
      client_change_rt(),
      client_change(),
      client_instance(),
      jack_sr(),
      jack_bs(),
      insert_buffer(NULL),
      client_activity_mutex(),
      jack_shutdown_seen(false),
      amp_client_active(false),
      fx_client_active(false),
      client_activity_generation(0),
      client_activity_transition_usecs(0),
      amp_callback_count_at_activate(0),
      fx_callback_count_at_activate(0),
      client_activity_amp_rc(0),
      client_activity_fx_rc(0),
      client_activity_rollback_amp_rc(0),
      client_activity_rollback_fx_rc(0),
      client_activity_rollback_incomplete(false),
      client_activity_last_error(),
      xrun_count(0),
      last_xrun_usecs(0),
      reported_xrun_count(0),
      ports(),
      client(0),
      client_insert(0),
      client_name(),
      client_insert_name(),
      session(),
      session_ins(),
      shutdown(),
      connection(),
      single_client(false) {
    reset_performance_telemetry();
    connection_queue.new_data.connect(sigc::mem_fun(*this, &GxJack::fetch_connection_data));
    client_change_rt.connect(client_change);
    GxExit::get_instance().signal_exit().connect(
	sigc::mem_fun(*this, &GxJack::cleanup_slot));
    static_assert(performance_poll_interval_ms >= 1000,
                  "performance reporting must be batched at no more than 1 Hz");
    Glib::signal_timeout().connect(
        sigc::mem_fun(this, &GxJack::poll_xrun), performance_poll_interval_ms);
}

GxJack::~GxJack() {
    gx_jack_cleanup();
}

void GxJack::rt_watchdog_set_limit(int limit) {
    rt_watchdog_limit = limit;
    if (limit > 0) {
	rt_watchdog_start();
    }
}


/****************************************************************
 ** load state, save state
 */

void GxJack::read_connections(gx_system::JsonParser& jp) {
    jp.next(gx_system::JsonParser::begin_object);
    while (jp.peek() == gx_system::JsonParser::value_key) {
        list<string> *i;
        jp.next(gx_system::JsonParser::value_key);
        if (jp.current_value() == "input") {
            i = &ports.input.conn;
        } else if (jp.current_value() == "output1") {
            i = &ports.output1.conn;
        } else if (jp.current_value() == "output2") {
            i = &ports.output2.conn;
        } else if (jp.current_value() == "midi_input") {
            i = &ports.midi_input.conn;
        } else if (jp.current_value() == "midi_output") {
            i = &ports.midi_output.conn;
        } else if (jp.current_value() == "insert_out") {
            i = &ports.insert_out.conn;
        } else if (jp.current_value() == "insert_in") {
            i = &ports.insert_in.conn;
        } else {
	    gx_print_warning(
		_("recall state"),
		_("unknown jack ports section: ") + jp.current_value());
            jp.skip_object();
            continue;
        }
	i->clear();
        jp.next(gx_system::JsonParser::begin_array);
        while (jp.peek() == gx_system::JsonParser::value_string) {
            jp.next();
            i->push_back(jp.current_value());
        }
        jp.next(gx_system::JsonParser::end_array);
    }
    jp.next(gx_system::JsonParser::end_object);
}

void GxJack::write_jack_port_connections(
    gx_system::JsonWriter& w, const char *key, const PortConnection& pc, bool replace) {
    w.write_key(key);
    w.begin_array();
    if (client && pc.port) {
	const char** pl = jack_port_get_connections(pc.port);
	if (pl) {
	    for (const char **p = pl; *p; p++) {
		if (replace) {
		    w.write(make_clientvar(*p));
		} else {
		    w.write(*p);
		}
	    }
	    free(pl);
	}
    } else {
	for (list<string>::const_iterator i = pc.conn.begin(); i != pc.conn.end(); ++i) {
	    w.write(*i);
	}
    }
    w.end_array(true);
}

void GxJack::write_connections(gx_system::JsonWriter& w) {
    w.begin_object(true);
    write_jack_port_connections(w, "input", ports.input);
    write_jack_port_connections(w, "output1", ports.output1);
    write_jack_port_connections(w, "output2", ports.output2);
    write_jack_port_connections(w, "midi_input", ports.midi_input);
    write_jack_port_connections(w, "midi_output", ports.midi_output);
    if (!single_client) {
    write_jack_port_connections(w, "insert_out", ports.insert_out, true);
    write_jack_port_connections(w, "insert_in", ports.insert_in, true);
    }
    w.end_object(true);
}


/****************************************************************
 ** client connection init and cleanup
 */
int GxJack::is_power_of_two (unsigned int x)
{
    return ((x != 0) && ((x & (~x + 1)) == x));
}

// ----- pop up a dialog for starting jack
bool GxJack::gx_jack_init(bool startserver, int wait_after_connect, const gx_system::CmdlineOptions& opt) {
    AVOIDDENORMALS();
    IS_RT = rtc.run_check();
    single_client = opt.get_jack_single();
    int jackopt = (startserver ? JackNullOption : JackNoStartServer);
    client_instance = opt.get_jack_instancename();
    if (client_instance.empty()) {
    if (!single_client) {
        client_instance = get_default_instancename();
    } else {
        client_instance = "guitarix";
    }
    } else {
	jackopt |= JackUseExactName;
    }

    std::string ServerName = opt.get_jack_servername();

    set_jack_down(false);
    set_jack_exit(true);
    {
        std::lock_guard<std::mutex> lock(client_activity_mutex);
        jack_shutdown_seen.store(false, std::memory_order_release);
        amp_client_active = false;
        fx_client_active = false;
        client_activity_generation = 0;
        client_activity_transition_usecs = 0;
        client_activity_amp_rc = 0;
        client_activity_fx_rc = 0;
        client_activity_rollback_amp_rc = 0;
        client_activity_rollback_fx_rc = 0;
        client_activity_rollback_incomplete = false;
        client_activity_last_error.clear();
    }
    engine.set_stateflag(gx_engine::GxEngine::SF_INITIALIZING);

    //ports = JackPorts(); //FIXME

    if (!single_client) {
        client_name = client_instance + jack_amp_postfix;
    } else {
        client_name = client_instance;
    }
    client_insert_name = client_instance + jack_fx_postfix;
    jack_status_t jackstat;
#ifdef HAVE_JACK_SESSION
    // try to open jack gxjack.client
    if (!opt.get_jack_uuid().empty()) {
        client = jack_client_open(
	    client_name.c_str(), JackOptions(jackopt | JackSessionID),
	    &jackstat, opt.get_jack_uuid().c_str());
    } else {
        if (ServerName.empty()) {
        client = jack_client_open(client_name.c_str(), JackOptions(jackopt), &jackstat);
        } else {
        client = jack_client_open(client_name.c_str(), JackOptions(jackopt | JackServerName),
        &jackstat, ServerName.c_str());
        }
    }
#else
    if (ServerName.empty()) {
    client = jack_client_open(client_name.c_str(), JackOptions(jackopt), &jackstat);
    } else {
    client = jack_client_open(client_name.c_str(), JackOptions(jackopt | JackServerName),
    &jackstat, ServerName.c_str());
    }
#endif
    // ----- only start the insert gxjack.client when the amp gxjack.client is true
    if (client && !single_client) {
	// it is maybe not the 1st gx_head instance ?
	// session handler can change name without setting JackNameNotUnique in return status; jack bug??
	// this code depends on jackd only appending a suffix to make a client name unique
	std::string name = jack_get_client_name(client);
	std::string generated_suffix = name.substr(client_name.size());
	std::string base = name.substr(0, client_name.size()-strlen(jack_amp_postfix));
	client_instance = base + generated_suffix;
	client_name = name;
	client_insert_name = base + jack_fx_postfix + generated_suffix;
#ifdef HAVE_JACK_SESSION
        if (!opt.get_jack_uuid2().empty()) {
            client_insert = jack_client_open(
		client_insert_name.c_str(),
		JackOptions(jackopt | JackSessionID | JackUseExactName),
		&jackstat, opt.get_jack_uuid2().c_str());
        } else {
            if (ServerName.empty()) {
            client_insert = jack_client_open(
		client_insert_name.c_str(),
		JackOptions(jackopt | JackUseExactName ), &jackstat);
        } else {
            client_insert = jack_client_open(
		client_insert_name.c_str(),
		JackOptions(jackopt | JackUseExactName | JackServerName),
        &jackstat, ServerName.c_str());
        }
        }
#else
        if (ServerName.empty()) {
        client_insert = jack_client_open(
	    client_insert_name.c_str(),
	    JackOptions(jackopt | JackUseExactName), &jackstat);
        } else {
        client_insert = jack_client_open(
	    client_insert_name.c_str(),
	    JackOptions(jackopt | JackUseExactName | JackServerName),
        &jackstat, ServerName.c_str());
        }
#endif
	if (!client_insert) {
	    jack_client_close(client);
	    client = 0;
	}
    }

    if (!client) {
	if (!(jackstat & JackServerFailed)) {
	    if ((jackstat & JackServerError) && (jackopt & JackUseExactName)) {
		gx_print_error(
		    _("Jack Init"),
		    boost::format(_("can't get requested jack instance name '%1%'"))
		    % client_instance);
	    } else {
		gx_print_error(
		    _("Jack Init"),
		    _("unknown jack server communication error"));
	    }
	}
	return false;
    }

    // ----------------------------------
    set_jack_down(false);

    if (wait_after_connect) {
	usleep(wait_after_connect);
    }
    // A reconnect starts a new measurement window. Both JACK clients are
    // inactive here, so no callback can race the reset.
    reset_performance_telemetry();
    const jack_nframes_t sample_rate = jack_get_sample_rate(client);
    jack_sr.store(sample_rate, std::memory_order_release);
    gx_print_info(
	_("Jack init"),
	boost::format(_("The jack sample rate is %1%/sec")) % sample_rate);

    const jack_nframes_t buffer_size = jack_get_buffer_size(client);
    jack_bs.store(buffer_size, std::memory_order_release);
	if (!is_power_of_two(buffer_size)) {
    gx_print_warning(
	_("Jack init"),
	boost::format(_("The jack buffer size is %1%/frames is not power of two, Convolver won't run"))
	% buffer_size);
	} else {
    gx_print_info(
	_("Jack init"),
	boost::format(_("The jack buffer size is %1%/frames ... "))
	% buffer_size);
	}
		
	// create buffer to bypass the insert ports
    insert_buffer = new float[buffer_size];
    if (IS_RT) IS_RT = jack_is_realtime(client) ? true : false;
    gx_jack_callbacks();
    client_change(); // might load port connection definitions
    if (opt.get_jack_uuid().empty() && !opt.get_jack_noconnect()) {
	// when not loaded by session manager
	gx_jack_init_port_connection(opt);
    }
    set_jack_exit(false);
	if (sample_rate > 96000) {
    gx_print_fatal(
		    _("Jack Init"),
		    _("Sample rates above 96kHz ain't be supported"));
		return false;
	}
    return true;
}

void GxJack::cleanup_slot(bool otherthread) {
    if (!otherthread) {
	gx_jack_cleanup();
    } else {
	// called from other thread. Since most cleanup functions are
	// not thread safe, just do minimal jack cleanup
        std::lock_guard<std::mutex> lock(client_activity_mutex);
        if (jack_shutdown_seen.load(std::memory_order_acquire)) {
            amp_client_active = false;
            fx_client_active = false;
            client = 0;
            client_insert = 0;
            return;
	}
	if (client) {
	    if (!is_jack_down() && amp_client_active) {
		engine.start_ramp_down();
		engine.wait_ramp_down_finished();
	    }
	    jack_deactivate(client);
	    amp_client_active = false;
	    jack_client_close(client);
	    client = 0;
	}
	if (client_insert) {
	    jack_deactivate(client_insert);
	    fx_client_active = false;
	    jack_client_close(client_insert);
	    client_insert = 0;
	}
    }
}

// -----Function that cleans the jack stuff on shutdown
void GxJack::gx_jack_cleanup() {
    std::lock_guard<std::mutex> lock(client_activity_mutex);
    if (!client || is_jack_down()) {
	return;
    }
    if (jack_shutdown_seen.load(std::memory_order_acquire)) {
        amp_client_active = false;
        fx_client_active = false;
        client = 0;
        client_insert = 0;
        return;
    }
    if (amp_client_active || fx_client_active) {
        engine.start_ramp_down();
        engine.wait_ramp_down_finished();
    }
    set_jack_exit(true);
    engine.set_stateflag(gx_engine::GxEngine::SF_INITIALIZING);
    jack_deactivate(client);
    amp_client_active = false;
    if (!single_client) {
        jack_deactivate(client_insert);
        fx_client_active = false;
    }
    jack_port_unregister(client, ports.input.port);
    jack_port_unregister(client, ports.midi_input.port);
    if (!single_client) {
        jack_port_unregister(client, ports.insert_out.port);
    } else {
        jack_port_unregister(client, ports.output1.port);
        jack_port_unregister(client, ports.output2.port);
    }
#if defined(USE_MIDI_OUT) || defined(USE_MIDI_CC_OUT)
    jack_port_unregister(client, ports.midi_output.port);
#endif
    if (!single_client) {
        jack_port_unregister(client_insert, ports.insert_in.port);
        jack_port_unregister(client_insert, ports.output1.port);
        jack_port_unregister(client_insert, ports.output2.port);
    }
    jack_client_close(client);
    client = 0;
    if (!single_client) jack_client_close(client_insert);
    client_insert = 0;
    delete[] insert_buffer;
    insert_buffer = NULL;
    client_change();
}

// ---- Jack server connection / disconnection
bool GxJack::gx_jack_connection(bool connect, bool startserver, int wait_after_connect, const gx_system::CmdlineOptions& opt) {
    if (connect) {
	if (client) {
	    return true;
	}
	if (!gx_jack_init(startserver, wait_after_connect, opt)) {
	    return false;
	}
	engine.set_rack_changed();
	engine.clear_stateflag(gx_engine::GxEngine::SF_INITIALIZING);
    } else {
	if (!client) {
	    return true;
	}
	gx_jack_cleanup();
    }
    connection();
    connection_queue.portchange();
    return true;
}


/****************************************************************
 ** port connections
 */

std::string GxJack::make_clientvar(const std::string& s) {
    std::size_t n = s.find(':');
    if (n == s.npos) {
	return s; // no ':' in jack port name??
    }
    if (s.compare(0, n, client_name) == 0) {
	return "%A" + s.substr(n);
    }
    if (s.compare(0, n, client_insert_name) == 0) {
	return "%F" + s.substr(n);
    }
    return s;
}

std::string GxJack::replace_clientvar(const std::string& s) {
    if (s.compare(0, 3, "%A:") == 0) {
	return client_name + s.substr(2);
    }
    if (s.compare(0, 3, "%F:") == 0) {
	return client_insert_name + s.substr(2);
    }
    return s;
}

// ----- connect ports if we know them
void GxJack::gx_jack_init_port_connection(const gx_system::CmdlineOptions& opt) {
    // set autoconnect capture to user capture port
    if (!opt.get_jack_input().empty()) {
        jack_connect(client, opt.get_jack_input().c_str(),
                     jack_port_name(ports.input.port));
    } else {
        list<string>& l = ports.input.conn;
        for (list<string>::iterator i = l.begin(); i != l.end(); ++i) {
            jack_connect(client, i->c_str(), jack_port_name(ports.input.port));
        }
    }

    // set autoconnect midi to user midi port
    if (ports.midi_input.port && !opt.get_jack_midi().empty()) {
        jack_connect(client, opt.get_jack_midi().c_str(),
                     jack_port_name(ports.midi_input.port));
    } else {
        list<string>& l = ports.midi_input.conn;
        for (list<string>::iterator i = l.begin(); i != l.end(); ++i) {
            jack_connect(client, i->c_str(), jack_port_name(ports.midi_input.port));
        }
    }

    if (!single_client) {
    // set autoconnect to user playback ports
    if (opt.get_jack_output(0).empty() && opt.get_jack_output(1).empty()) {
        list<string>& l1 = ports.output1.conn;
        for (list<string>::iterator i = l1.begin(); i != l1.end(); ++i) {
            jack_connect(client_insert, jack_port_name(ports.output1.port), i->c_str());
        }
        list<string>& l2 = ports.output2.conn;
        for (list<string>::iterator i = l2.begin(); i != l2.end(); ++i) {
            jack_connect(client_insert, jack_port_name(ports.output2.port), i->c_str());
        }
    } else {
	if (!opt.get_jack_output(0).empty()) {
	    jack_connect(client_insert,
			 jack_port_name(ports.output1.port),
			 opt.get_jack_output(0).c_str());
	}
	if (!opt.get_jack_output(1).empty()) {
	    jack_connect(client_insert,
			 jack_port_name(ports.output2.port),
			 opt.get_jack_output(1).c_str());
	}
    }
    
    } else {
// set autoconnect to user playback ports
    if (opt.get_jack_output(0).empty() && opt.get_jack_output(1).empty()) {
        list<string>& l1 = ports.output1.conn;
        for (list<string>::iterator i = l1.begin(); i != l1.end(); ++i) {
            jack_connect(client, jack_port_name(ports.output1.port), i->c_str());
        }
        list<string>& l2 = ports.output2.conn;
        for (list<string>::iterator i = l2.begin(); i != l2.end(); ++i) {
            jack_connect(client, jack_port_name(ports.output2.port), i->c_str());
        }
    } else {
	if (!opt.get_jack_output(0).empty()) {
	    jack_connect(client,
			 jack_port_name(ports.output1.port),
			 opt.get_jack_output(0).c_str());
	}
	if (!opt.get_jack_output(1).empty()) {
	    jack_connect(client,
			 jack_port_name(ports.output2.port),
			 opt.get_jack_output(1).c_str());
	}
    }
        
    }

#if defined(USE_MIDI_OUT) || defined(USE_MIDI_CC_OUT)
    // autoconnect midi output port
    list<string>& lmo = ports.midi_output.conn;
    for (list<string>::iterator i = lmo.begin(); i != lmo.end(); ++i) {
        jack_connect(client, jack_port_name(ports.midi_output.port), i->c_str());
    }
#endif

    if (!single_client) {
    // autoconnect to insert ports
    list<string>& lins_in = ports.insert_in.conn;
    list<string>& lins_out = ports.insert_out.conn;
    bool ifound = false, ofound = false;
    for (list<string>::iterator i = lins_in.begin(); i != lins_in.end(); ++i) {
        int rc = jack_connect(client_insert, replace_clientvar(*i).c_str(),
                              jack_port_name(ports.insert_in.port));
        if (rc == 0 || rc == EEXIST) {
            ifound = true;
        }
    }
    jack_port_t* port_a = jack_port_by_name(client, jack_port_name(ports.insert_out.port));
    for (list<string>::iterator i = lins_out.begin(); i != lins_out.end(); ++i) {
	std::string port = replace_clientvar(*i);
	if (!jack_port_connected_to(port_a, port.c_str())) {
	    int rc = jack_connect(client, jack_port_name(ports.insert_out.port),
				  port.c_str());
	    if (rc == 0 || rc == EEXIST) {
		ofound = true;
	    }
	} else {
	    ofound = true;
	}
    }
    if (!ifound || !ofound) {
        jack_connect(client_insert, jack_port_name(ports.insert_out.port),
		     (client_insert_name+":in_0").c_str());
    }
    }
}


/****************************************************************
 ** callback installation and port registration
 */

// ----- set gxjack.client callbacks and activate gxjack.client
void GxJack::gx_jack_callbacks() {
    // ----- set the jack callbacks
    // JACK invokes these initializers in each newly-created client thread,
    // before process callbacks begin. Keep thread naming out of the first RT
    // audio period so startup telemetry cannot itself cause or hide an XRUN.
    if (jack_set_thread_init_callback(client, gx_jack_thread_init_main, this) != 0) {
        gx_print_warning(_("Jack Init"), _("Can't install gx_amp thread initializer"));
    }
    if (!single_client &&
        jack_set_thread_init_callback(client_insert, gx_jack_thread_init_insert, this) != 0) {
        gx_print_warning(_("Jack Init"), _("Can't install gx_amp_fx thread initializer"));
    }
    jack_set_xrun_callback(client, gx_jack_xrun_callback, this);
    jack_set_sample_rate_callback(client, gx_jack_srate_callback, this);
    jack_on_shutdown(client, shutdown_callback_client, this);
    if (!single_client) {
        jack_on_shutdown(client_insert, shutdown_callback_client_insert, this);
    }
    jack_set_buffer_size_callback(client, gx_jack_buffersize_callback, this);
    jack_set_port_registration_callback(client, gx_jack_portreg_callback, this);
    jack_set_port_connect_callback(client, gx_jack_portconn_callback, this);
#ifdef HAVE_JACK_SESSION
    if (jack_set_session_callback_fp) {
        jack_set_session_callback_fp(client, gx_jack_session_callback, this);
        if (!single_client) jack_set_session_callback_fp(client_insert, gx_jack_session_callback_ins, this);
    }
#endif

    // register ports for gx_amp
    ports.input.port = jack_port_register(
	client, "in_0", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    ports.midi_input.port = jack_port_register(
	client, "midi_in_1", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
    if (!single_client) {
        ports.insert_out.port = jack_port_register(
        client, "out_0", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    } else {
        ports.output1.port = jack_port_register(
        client, "out_0", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
        ports.output2.port = jack_port_register(
        client, "out_1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    }
#if defined(USE_MIDI_OUT) || defined(USE_MIDI_CC_OUT)
    ports.midi_output.port = jack_port_register(
	client, "midi_out_1", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
#else
    ports.midi_output.port = 0;
#endif

    if (!single_client) {
    // register ports for gx_amp_fx
        ports.insert_in.port = jack_port_register(
          client_insert, "in_0", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
        ports.output1.port = jack_port_register(
          client_insert, "out_0", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
        ports.output2.port = jack_port_register(
          client_insert, "out_1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    }

    engine.init(get_jack_sr(), get_jack_bs(), get_is_rt() ? SCHED_FIFO : SCHED_OTHER,
		get_is_rt() ? jack_client_real_time_priority(client) : 0);
    // Resolve and fault in the JACK clock path before the first measured audio
    // period. Otherwise telemetry can create the startup outlier it reports.
    (void)jack_get_time();
    jack_set_process_callback(client, gx_jack_process, this);
    if (!single_client) jack_set_process_callback(client_insert, gx_jack_insert_process, this);
    amp_callback_count_at_activate =
        callback_rings[callback_stream_main].count.load(std::memory_order_acquire);
    const int amp_activate_rc = jack_activate(client);
    amp_client_active = amp_activate_rc == 0;
    if (amp_activate_rc != 0) {
        gx_print_fatal(
	    _("Jack Activation"),
	    string(_("Can't activate JACK gx_amp client")));
    }
    if (!single_client) {
        fx_callback_count_at_activate =
            callback_rings[callback_stream_insert].count.load(std::memory_order_acquire);
        const int fx_activate_rc = jack_activate(client_insert);
        fx_client_active = fx_activate_rc == 0;
        if (fx_activate_rc != 0) {
            gx_print_fatal(_("Jack Activation"),
                        string(_("Can't activate JACK gx_amp_fx client")));
        }
    } else {
        fx_client_active = false;
    }
}

JackClientActivityStatus GxJack::client_activity_status_unlocked(
    bool ok, bool changed) const {
    JackClientActivityStatus status;
    status.ok = ok;
    status.changed = changed;
    status.amp_active = amp_client_active;
    status.fx_active = !single_client && fx_client_active;
    status.amp_present = client != 0;
    status.fx_present = !single_client && client_insert != 0;
    status.single_client = single_client;
    status.active = status.amp_active &&
        (single_client || status.fx_active);
    status.any_active = status.amp_active || status.fx_active;
    status.inactive = !status.any_active;
    status.generation = client_activity_generation;
    status.transition_usecs = client_activity_transition_usecs;
    status.amp_callback_count =
        callback_rings[callback_stream_main].count.load(std::memory_order_acquire);
    status.fx_callback_count = single_client ? 0 :
        callback_rings[callback_stream_insert].count.load(std::memory_order_acquire);
    status.amp_callbacks_since_activate =
        status.amp_callback_count >= amp_callback_count_at_activate
        ? status.amp_callback_count - amp_callback_count_at_activate : 0;
    status.fx_callbacks_since_activate =
        status.fx_callback_count >= fx_callback_count_at_activate
        ? status.fx_callback_count - fx_callback_count_at_activate : 0;
    status.amp_ramp_mode =
        static_cast<int>(engine.mono_chain.get_ramp_mode());
    status.fx_ramp_mode =
        static_cast<int>(engine.stereo_chain.get_ramp_mode());
    status.engine_ready = status.active &&
        status.amp_callbacks_since_activate > 0 &&
        (single_client || status.fx_callbacks_since_activate > 0) &&
        status.amp_ramp_mode ==
            static_cast<int>(gx_engine::ProcessingChainBase::ramp_mode_off) &&
        status.fx_ramp_mode ==
            static_cast<int>(gx_engine::ProcessingChainBase::ramp_mode_off);
    status.amp_rc = client_activity_amp_rc;
    status.fx_rc = client_activity_fx_rc;
    status.rollback_amp_rc = client_activity_rollback_amp_rc;
    status.rollback_fx_rc = client_activity_rollback_fx_rc;
    status.rollback_incomplete = client_activity_rollback_incomplete;
    status.last_error = client_activity_last_error;
    return status;
}

JackClientActivityStatus GxJack::set_client_activity(bool active) {
    std::lock_guard<std::mutex> lock(client_activity_mutex);
    const jack_time_t started_at = jack_get_time();
    bool changed = false;

    client_activity_transition_usecs = 0;
    client_activity_amp_rc = 0;
    client_activity_fx_rc = 0;
    client_activity_rollback_amp_rc = 0;
    client_activity_rollback_fx_rc = 0;
    client_activity_rollback_incomplete = false;
    client_activity_last_error.clear();

    if (!client || (!single_client && !client_insert) || is_jack_down() ||
        is_jack_exit() ||
        jack_shutdown_seen.load(std::memory_order_acquire)) {
        client_activity_last_error = "JACK clients are unavailable";
        client_activity_transition_usecs =
            static_cast<unsigned long long>(jack_get_time() - started_at);
        return client_activity_status_unlocked(false, false);
    }

    if (active) {
        if (!amp_client_active) {
            amp_callback_count_at_activate =
                callback_rings[callback_stream_main].count.load(
                    std::memory_order_acquire);
            client_activity_amp_rc = jack_activate(client);
            if (jack_shutdown_seen.load(std::memory_order_acquire)) {
                amp_client_active = false;
                fx_client_active = false;
                client_activity_last_error =
                    "JACK shut down during jack_activate";
            } else if (client_activity_amp_rc == 0) {
                amp_client_active = true;
                changed = true;
            }
        }
        if (!single_client && !fx_client_active &&
            client_activity_amp_rc == 0 &&
            !jack_shutdown_seen.load(std::memory_order_acquire)) {
            fx_callback_count_at_activate =
                callback_rings[callback_stream_insert].count.load(
                    std::memory_order_acquire);
            client_activity_fx_rc = jack_activate(client_insert);
            if (jack_shutdown_seen.load(std::memory_order_acquire)) {
                amp_client_active = false;
                fx_client_active = false;
                client_activity_last_error =
                    "JACK shut down during jack_activate";
            } else if (client_activity_fx_rc == 0) {
                fx_client_active = true;
                changed = true;
            }
        }
        if (client_activity_last_error.empty() &&
            (client_activity_amp_rc != 0 || client_activity_fx_rc != 0)) {
            // A partial active state is neither useful nor quiet. Roll back
            // every active client after a failed activation attempt.
            if (!single_client && fx_client_active &&
                !jack_shutdown_seen.load(std::memory_order_acquire)) {
                client_activity_rollback_fx_rc =
                    jack_deactivate(client_insert);
                if (client_activity_rollback_fx_rc == 0) {
                    fx_client_active = false;
                    changed = true;
                }
            }
            if (amp_client_active &&
                !jack_shutdown_seen.load(std::memory_order_acquire)) {
                client_activity_rollback_amp_rc = jack_deactivate(client);
                if (client_activity_rollback_amp_rc == 0) {
                    amp_client_active = false;
                    changed = true;
                }
            }
            client_activity_rollback_incomplete =
                amp_client_active || (!single_client && fx_client_active);
            if (jack_shutdown_seen.load(std::memory_order_acquire)) {
                client_activity_last_error =
                    "JACK shut down during activation rollback";
            } else {
                client_activity_last_error =
                    "jack_activate failed (amp=" +
                    std::to_string(client_activity_amp_rc) + ", fx=" +
                    std::to_string(client_activity_fx_rc) +
                    "; rollback amp=" +
                    std::to_string(client_activity_rollback_amp_rc) + ", fx=" +
                    std::to_string(client_activity_rollback_fx_rc) + ")";
            }
        }
    } else {
        if (!single_client && fx_client_active) {
            client_activity_fx_rc = jack_deactivate(client_insert);
            if (jack_shutdown_seen.load(std::memory_order_acquire)) {
                amp_client_active = false;
                fx_client_active = false;
                client_activity_last_error =
                    "JACK shut down during jack_deactivate";
            } else if (client_activity_fx_rc == 0) {
                fx_client_active = false;
                changed = true;
            }
        }
        // Failure policy is quiet-first: still deactivate the amp if the FX
        // client failed, then report the exact partial state to the caller.
        if (amp_client_active &&
            !jack_shutdown_seen.load(std::memory_order_acquire)) {
            client_activity_amp_rc = jack_deactivate(client);
            if (jack_shutdown_seen.load(std::memory_order_acquire)) {
                amp_client_active = false;
                fx_client_active = false;
                client_activity_last_error =
                    "JACK shut down during jack_deactivate";
            } else if (client_activity_amp_rc == 0) {
                amp_client_active = false;
                changed = true;
            }
        }
        if (client_activity_last_error.empty() &&
            (client_activity_amp_rc != 0 || client_activity_fx_rc != 0)) {
            if (client_activity_last_error.empty()) {
                client_activity_last_error =
                    "jack_deactivate failed (amp=" +
                    std::to_string(client_activity_amp_rc) + ", fx=" +
                    std::to_string(client_activity_fx_rc) + ")";
            }
        }
    }

    client_activity_transition_usecs =
        static_cast<unsigned long long>(jack_get_time() - started_at);
    const bool reached_target = active
        ? amp_client_active && (single_client || fx_client_active)
        : !amp_client_active && (single_client || !fx_client_active);
    if (reached_target && changed && client_activity_last_error.empty()) {
        ++client_activity_generation;
    }
    return client_activity_status_unlocked(
        reached_target && client_activity_last_error.empty(), changed);
}

JackClientActivityStatus GxJack::jack_deactivate_clients() {
    return set_client_activity(false);
}

JackClientActivityStatus GxJack::jack_activate_clients() {
    return set_client_activity(true);
}

JackClientActivityStatus GxJack::get_jack_client_activity_status() const {
    std::lock_guard<std::mutex> lock(client_activity_mutex);
    const bool available =
        client && (single_client || client_insert) && !is_jack_down() &&
        !is_jack_exit() &&
        !jack_shutdown_seen.load(std::memory_order_acquire);
    return client_activity_status_unlocked(
        available && client_activity_last_error.empty(), false);
}


/****************************************************************
 ** jack process callbacks
 */

void GxJack::gx_jack_thread_init_main(void*) {
    set_jack_thread_name("gx-jack-main");
}

void GxJack::gx_jack_thread_init_insert(void*) {
    set_jack_thread_name("gx-jack-insert");
}

void __rt_func GxJack::process_midi_cc(void *buf, jack_nframes_t nframes) {
    // midi CC output processing
    for (int i = mmessage.next(); i >= 0; i = mmessage.next(i)) {
        unsigned char* midi_send = jack_midi_event_reserve(buf, i, mmessage.size(i));
        if (midi_send) {
            mmessage.fill(midi_send, i);
        }
    }
}

// must only be used inside gx_jack_process
void *GxJack::get_midi_buffer(jack_nframes_t nframes) {
    if (!ports.midi_output.port) {
	return 0;
    }
    void *midi_port_buf = jack_port_get_buffer(ports.midi_output.port, nframes);
    if (midi_port_buf) {
	jack_midi_clear_buffer(midi_port_buf);
    }
    return midi_port_buf;
}

static inline float *get_float_buf(jack_port_t *port, jack_nframes_t nframes) {
    return static_cast<float *>(jack_port_get_buffer(port, nframes));
}

inline void GxJack::check_overload() {
    if (!rt_watchdog_check_alive(
            jack_bs.load(std::memory_order_relaxed),
            jack_sr.load(std::memory_order_relaxed))) {
	engine.overload(gx_engine::EngineControl::ov_User, "watchdog thread");
    }
}

// ----- main jack process method gx_amp, mono -> mono
// RT process thread
int __rt_func GxJack::gx_jack_process(jack_nframes_t nframes, void *arg) {
    const jack_time_t callback_started_at = jack_get_time();
    gx_system::measure_start();
    GxJack& self = *static_cast<GxJack*>(arg);
    if (!self.is_jack_exit()) {
	self.engine.mono_chain.post_rt_started();
	if (!self.engine.mono_chain.is_stopped()) {
	    self.check_overload();
	}
	self.transport_state = jack_transport_query (self.client, &self.current);
        // gx_head DSP computing
    float *obuf = self.insert_buffer;
    if (!self.single_client) {
        obuf = get_float_buf(self.ports.insert_out.port, nframes);
    } 
	self.engine.mono_chain.process(
	    nframes,
	    get_float_buf(self.ports.input.port, nframes),
	    obuf);

    if (self.bypass_insert && !self.single_client) {
        memcpy(self.insert_buffer, obuf, nframes*sizeof(float));
    }

        // midi input processing
	if (self.ports.midi_input.port) {
	    self.engine.controller_map.compute_midi_in(
		jack_port_get_buffer(self.ports.midi_input.port, nframes), arg);
	}
        // jack transport support
    if ( self.transport_state != self.old_transport_state) {
        self.engine.controller_map.process_trans(self.transport_state);
        self.old_transport_state = self.transport_state;
    }
    }
    // midi CC output processing
    void *buf = self.get_midi_buffer(nframes);
    self.process_midi_cc(buf, nframes);

    gx_system::measure_pause();
    self.engine.mono_chain.post_rt_finished();
    if (self.single_client) {
        self.gx_jack_insert_process(nframes, arg);
    }
    self.record_callback_time(
        callback_stream_main,
        static_cast<unsigned int>(jack_get_time() - callback_started_at));
    return 0;
}

// ----- main jack process method, gx_fx_amp, mono -> stereo
// RT process_insert thread
int __rt_func GxJack::gx_jack_insert_process(jack_nframes_t nframes, void *arg) {
    GxJack& self = *static_cast<GxJack*>(arg);
    const jack_time_t callback_started_at = self.single_client ? 0 : jack_get_time();
    gx_system::measure_cont();
    if (!self.is_jack_exit()) {
	self.engine.stereo_chain.post_rt_started();
	if (!self.engine.stereo_chain.is_stopped()) {
	    self.check_overload();
	}
        // gx_head DSP computing
    float *ibuf = NULL;
    if (!self.bypass_insert && !self.single_client) {
	    ibuf = get_float_buf(self.ports.insert_in.port, nframes);
	} else {
	    ibuf = self.insert_buffer;
	}
	self.engine.stereo_chain.process(
	    nframes, ibuf, ibuf,
	    get_float_buf(self.ports.output1.port, nframes),
	    get_float_buf(self.ports.output2.port, nframes));
    }
    gx_system::measure_stop();
    self.engine.stereo_chain.post_rt_finished();
    if (!self.single_client) {
        self.record_callback_time(
            callback_stream_insert,
            static_cast<unsigned int>(jack_get_time() - callback_started_at));
    }
    return 0;
}

void __rt_func GxJack::record_callback_time(
    CallbackStream stream, unsigned int elapsed_usecs) {
    // The main and insert JACK callbacks each own a separate ring. This keeps
    // publication single-producer without a lock or retry loop in either RT
    // thread. Sequence and duration are one atomic word so an RPC snapshot can
    // never pair fields from different ring generations.
    const unsigned long long sequence =
        callback_rings[stream].count.load(std::memory_order_relaxed);
    const unsigned long long sequence_tag =
        static_cast<unsigned int>(sequence + 1);
    const unsigned long long packed =
        (sequence_tag << 32) | static_cast<unsigned long long>(elapsed_usecs);
    callback_rings[stream].samples[sequence % callback_sample_capacity].store(
        packed, std::memory_order_relaxed);
    // Each ring has exactly one writer, so publishing with a release store is
    // sufficient and avoids an unnecessary read-modify-write in the RT path.
    callback_rings[stream].count.store(sequence + 1, std::memory_order_release);
}

void GxJack::reset_performance_telemetry() {
    xrun_count.store(0, std::memory_order_relaxed);
    last_xrun_usecs.store(0, std::memory_order_relaxed);
    reported_xrun_count = 0;
    for (unsigned int stream = 0; stream < callback_stream_count; ++stream) {
        for (unsigned int i = 0; i < callback_sample_capacity; ++i) {
            callback_rings[stream].samples[i].store(0, std::memory_order_relaxed);
        }
        callback_rings[stream].count.store(0, std::memory_order_release);
    }
}

void GxJack::get_callback_performance(
    unsigned long long& count,
    unsigned int& sample_count,
    unsigned int& p99_usecs,
    unsigned int& p999_usecs,
    unsigned int& max_usecs) const {
    count = 0;
    std::vector<unsigned int> samples;
    samples.reserve(
        static_cast<size_t>(callback_sample_capacity) *
        static_cast<size_t>(callback_stream_count));
    for (unsigned int stream = 0; stream < callback_stream_count; ++stream) {
        const unsigned long long stream_count =
            callback_rings[stream].count.load(std::memory_order_acquire);
        count += stream_count;
        const unsigned long long available =
            std::min<unsigned long long>(stream_count, callback_sample_capacity);
        const unsigned long long first = stream_count - available;
        for (unsigned long long sequence = first; sequence < stream_count; ++sequence) {
            const unsigned long long packed =
                callback_rings[stream].samples[sequence % callback_sample_capacity].load(
                    std::memory_order_acquire);
            const unsigned int expected = static_cast<unsigned int>(sequence + 1);
            if (static_cast<unsigned int>(packed >> 32) == expected) {
                samples.push_back(static_cast<unsigned int>(packed));
            }
        }
    }
    sample_count = static_cast<unsigned int>(samples.size());
    if (samples.empty()) {
        p99_usecs = p999_usecs = max_usecs = 0;
        return;
    }
    std::sort(samples.begin(), samples.end());
    const size_t last = samples.size() - 1;
    p99_usecs = samples[std::min(last, (samples.size() * 990 + 999) / 1000 - 1)];
    p999_usecs = samples[std::min(last, (samples.size() * 999 + 999) / 1000 - 1)];
    max_usecs = samples[last];
}

void GxJack::get_callback_performance_for_stream(
    bool insert_stream,
    unsigned long long& count,
    unsigned int& sample_count,
    unsigned int& p99_usecs,
    unsigned int& p999_usecs,
    unsigned int& max_usecs) const {
    const unsigned int stream = insert_stream
        ? callback_stream_insert : callback_stream_main;
    count = callback_rings[stream].count.load(std::memory_order_acquire);
    const unsigned long long available =
        std::min<unsigned long long>(count, callback_sample_capacity);
    const unsigned long long first = count - available;
    std::vector<unsigned int> samples;
    samples.reserve(static_cast<size_t>(available));
    for (unsigned long long sequence = first; sequence < count; ++sequence) {
        const unsigned long long packed =
            callback_rings[stream].samples[sequence % callback_sample_capacity].load(
                std::memory_order_acquire);
        const unsigned int expected = static_cast<unsigned int>(sequence + 1);
        if (static_cast<unsigned int>(packed >> 32) == expected) {
            samples.push_back(static_cast<unsigned int>(packed));
        }
    }
    sample_count = static_cast<unsigned int>(samples.size());
    if (samples.empty()) {
        p99_usecs = p999_usecs = max_usecs = 0;
        return;
    }
    std::sort(samples.begin(), samples.end());
    const size_t last = samples.size() - 1;
    p99_usecs = samples[std::min(last, (samples.size() * 990 + 999) / 1000 - 1)];
    p999_usecs = samples[std::min(last, (samples.size() * 999 + 999) / 1000 - 1)];
    max_usecs = samples[last];
}


/****************************************************************
 ** port connection callback
 */

PortConnRing::PortConnRing()
    : ring(jack_ringbuffer_create(20*sizeof(PortConnData))), // just a number...
      send_changes(false),
      overflow(false),
      new_data(),
      portchange() {
    if (!ring) {
	gx_print_fatal(
	    _("Jack init"), _("can't get memory for ringbuffer"));
    }
    jack_ringbuffer_mlock(ring);
}

PortConnRing::~PortConnRing() {
    jack_ringbuffer_free(ring);
}

void PortConnRing::push(const char *a, const char *b, bool conn) {
    if (is_overflow()) {
	return;
    }
    if (send_changes) {
	PortConnData p(a, b, conn);
	size_t sz = jack_ringbuffer_write(ring, reinterpret_cast<const char*>(&p), sizeof(p));
	if (sz != sizeof(p)) {
	    set_overflow();
	} else {
	    jack_ringbuffer_write_advance(ring, sz);
	}
    }
    new_data();
}

bool PortConnRing::pop(PortConnData *p) {
    if (is_overflow()) {
	jack_ringbuffer_reset(ring);
	portchange();
	clear_overflow();
	return false;
    }
    size_t sz = jack_ringbuffer_read(ring, reinterpret_cast<char*>(p), sizeof(*p));
    if (sz == 0) {
	return false;
    }
    assert(sz == sizeof(*p));
    jack_ringbuffer_read_advance(ring, sz);
    return true;
}

void GxJack::fetch_connection_data() {
    // check if we are connected
    if (client) {
	const char** port = jack_port_get_connections(ports.input.port);
	if (port) { // might be 0 (e.g. due to race conditions)
	    engine.clear_stateflag(gx_engine::GxEngine::SF_NO_CONNECTION);
	    free(port);
	} else {
	    engine.set_stateflag(gx_engine::GxEngine::SF_NO_CONNECTION);
	}
    }
    while (true) {
	PortConnData p;
	bool fetched = connection_queue.pop(&p);
	if (!fetched) {
	    break;
	}
	if (client) {
	    connection_changed(p.name_a, p.name_b, p.connect);
	}
    }
}

// jackd1: RT process thread
// jackd2: not RT thread
void GxJack::gx_jack_portconn_callback(jack_port_id_t a, jack_port_id_t b, int connect, void* arg) {
    GxJack& self = *static_cast<GxJack*>(arg);
    if (!self.client) {
	return;
    }
    jack_port_t* port_a = jack_port_by_id(self.client, a);
    jack_port_t* port_b = jack_port_by_id(self.client, b);
    if (!port_a || !port_b) {
        return;
    }
    self.connection_queue.push(jack_port_name(port_a), jack_port_name(port_b), connect);
}


/****************************************************************
 ** callbacks: portreg, buffersize, samplerate, shutdown, xrun
 */

// ----- fetch available jack ports other than gx_head ports
// jackd1: RT process thread
// jackd2: not RT thread
void GxJack::gx_jack_portreg_callback(jack_port_id_t pid, int reg, void* arg) {
    GxJack& self = *static_cast<GxJack*>(arg);
    if (!self.client) {
        return;
    }
    jack_port_t* port = jack_port_by_id(self.client, pid);
    if (!port || jack_port_is_mine(self.client, port)) {
        return;
    }
    self.connection_queue.portchange();
}

// ----jack sample rate change callback
// seems to be run in main thread (just once, no possibility
// to change the samplerate when jack is running?)
int GxJack::gx_jack_srate_callback(jack_nframes_t samplerate, void* arg) {
    GxJack& self = *static_cast<GxJack*>(arg);
    if (self.jack_sr.load(std::memory_order_relaxed) == samplerate) {
	return 0;
    }
    self.engine.set_stateflag(gx_engine::GxEngine::SF_JACK_RECONFIG);
    self.jack_sr.store(samplerate, std::memory_order_release);
    self.engine.set_samplerate(samplerate);
    self.engine.clear_stateflag(gx_engine::GxEngine::SF_JACK_RECONFIG);
    return 0;
}

// ---- jack buffer size change callback
// RT process thread
int GxJack::gx_jack_buffersize_callback(jack_nframes_t nframes, void* arg) {
    GxJack& self = *static_cast<GxJack*>(arg);
    if (self.jack_bs.load(std::memory_order_relaxed) == nframes) {
	return 0;
    }
    self.engine.set_stateflag(gx_engine::GxEngine::SF_JACK_RECONFIG);
    self.jack_bs.store(nframes, std::memory_order_release);
    self.engine.set_buffersize(nframes);
    self.engine.clear_stateflag(gx_engine::GxEngine::SF_JACK_RECONFIG);
    self.buffersize_change();
	// create buffer to bypass the insert ports
	delete[] self.insert_buffer;
	self.insert_buffer = NULL;
    self.insert_buffer = new float[nframes];
    return 0;
}

// ---- jack shutdown callback in case jackd shuts down on us
void GxJack::gx_jack_shutdown_callback() {
    set_jack_exit(true);
    engine.set_stateflag(gx_engine::GxEngine::SF_INITIALIZING);
    shutdown();
}

void GxJack::shutdown_callback_client(void *arg) {
    GxJack& self = *static_cast<GxJack*>(arg);
    self.jack_shutdown_seen.store(true, std::memory_order_release);
    self.amp_client_active = false;
    self.fx_client_active = false;
    jack_client_t *insert_to_close = 0;
    bool notify_client_change = true;
    {
        std::unique_lock<std::mutex> lock(
            self.client_activity_mutex, std::try_to_lock);
        if (lock.owns_lock()) {
            if (self.client) {
                self.client = 0;
            }
            if (!self.single_client && self.client_insert) {
                insert_to_close = self.client_insert;
                self.client_insert = 0;
            }
        }
    }
    if (insert_to_close) {
        jack_client_close(insert_to_close);
    }
    if (notify_client_change) {
        self.client_change_rt();
    }
    self.gx_jack_shutdown_callback();
}

void GxJack::shutdown_callback_client_insert(void *arg) {
    GxJack& self = *static_cast<GxJack*>(arg);
    self.jack_shutdown_seen.store(true, std::memory_order_release);
    self.amp_client_active = false;
    self.fx_client_active = false;
    jack_client_t *amp_to_close = 0;
    bool notify_client_change = true;
    {
        std::unique_lock<std::mutex> lock(
            self.client_activity_mutex, std::try_to_lock);
        if (lock.owns_lock()) {
            self.client_insert = 0;
            if (self.client) {
                amp_to_close = self.client;
                self.client = 0;
            }
        }
    }
    if (amp_to_close) {
        jack_client_close(amp_to_close);
    }
    if (notify_client_change) {
        self.client_change_rt();
    }
    self.gx_jack_shutdown_callback();
}

void GxJack::report_xrun() {
    gx_print_warning(
	_("Jack XRun"),
	(boost::format(_(" delay of at least %1% microsecs")) % get_last_xrun()).str());
}

bool GxJack::poll_xrun() {
    const unsigned long long current = get_xrun_count();
    if (current == reported_xrun_count) {
	return true;
    }
    reported_xrun_count = current;
    if (!engine.mono_chain.is_stopped()) {
	engine.overload(gx_engine::EngineControl::ov_XRun, "xrun");
    }
    report_xrun();
    return true;
}

// ---- jack xrun callback
int GxJack::gx_jack_xrun_callback(void* arg) {
    GxJack& self = *static_cast<GxJack*>(arg);
    if (!self.client) {
	return 0;
    }
    self.last_xrun_usecs.store(
	static_cast<unsigned int>(jack_get_xrun_delayed_usecs(self.client)),
	std::memory_order_release);
    self.xrun_count.fetch_add(1, std::memory_order_release);
    return 0;
}

/****************************************************************
 ** jack session
 */

#ifdef HAVE_JACK_SESSION
jack_set_session_callback_type GxJack::jack_set_session_callback_fp =
    reinterpret_cast<jack_set_session_callback_type>(
	dlsym(RTLD_DEFAULT, "jack_set_session_callback"));
jack_get_uuid_for_client_name_type GxJack::jack_get_uuid_for_client_name_fp =
    reinterpret_cast<jack_get_uuid_for_client_name_type>(
	dlsym(RTLD_DEFAULT, "jack_get_uuid_for_client_name"));
jack_client_get_uuid_type GxJack::jack_client_get_uuid_fp =
    reinterpret_cast<jack_client_get_uuid_type>(
	dlsym(RTLD_DEFAULT, "jack_client_get_uuid"));

int GxJack::return_last_session_event() {
    jack_session_event_t *event = get_last_session_event();
    if (event) {
	session_callback_seen += 1;
	jack_session_reply(client, event);
	jack_session_event_free(event);
	gx_system::atomic_set_0(&session_event);
    }
    return session_callback_seen;
}

int GxJack::return_last_session_event_ins() {
    jack_session_event_t *event = get_last_session_event_ins();
    if (event) {
	session_callback_seen -= 1;
	jack_session_reply(client_insert, event);
	jack_session_event_free(event);
	gx_system::atomic_set_0(&session_event_ins);
    }
    return session_callback_seen;
}

string GxJack::get_uuid_insert() {
    // should be const char* but jack_free doesn't like it
    char* uuid;
    if (jack_client_get_uuid_fp) {
	uuid = jack_client_get_uuid_fp(client_insert);
    } else if (jack_get_uuid_for_client_name_fp) {
	uuid = jack_get_uuid_for_client_name_fp(
	    client_insert, client_insert_name.c_str());
    } else {
	assert(false);
	gx_print_error(_("session save"), _("can't get client uuid"));
	return "";
    }
    string ret(uuid);
    jack_free(uuid);
    return ret;
}

void GxJack::gx_jack_session_callback(jack_session_event_t *event, void *arg) {
    GxJack& self = *static_cast<GxJack*>(arg);
    jack_session_event_t *np = 0;
    if (!gx_system::atomic_compare_and_exchange(&self.session_event, np, event)) {
	gx_print_error("jack","last session not cleared");
	return;
    }
    self.session();
}

void GxJack::gx_jack_session_callback_ins(jack_session_event_t *event, void *arg) {
    GxJack& self = *static_cast<GxJack*>(arg);
    jack_session_event_t *np = 0;
    if (!gx_system::atomic_compare_and_exchange(&self.session_event_ins, np, event)) {
	gx_print_error("jack","last session not cleared");
	return;
    }
    self.session_ins();
}
#endif // HAVE_JACK_SESSION
#endif // GUITARIX_AS_PLUGIN
} /* end of gx_jack namespace */

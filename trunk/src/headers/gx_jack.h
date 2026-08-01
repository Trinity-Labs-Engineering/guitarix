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
 */

/* ------- This is the JACK namespace ------- */

#pragma once

#ifndef SRC_HEADERS_GX_JACK_H_
#define SRC_HEADERS_GX_JACK_H_

#include <atomic>

#ifndef GUITARIX_AS_PLUGIN

#include <jack/jack.h>          // NOLINT
#include <jack/midiport.h>
#include <jack/ringbuffer.h>

#ifdef HAVE_JACK_SESSION
#include <jack/session.h>
#endif

namespace gx_engine {
class GxEngine;
}

namespace gx_jack {

/****************************************************************
 ** class GxRtCheck
 ** check if user have realtime priority
 */

class GxRtCheck {
private:
    std::thread _thd;
    std::mutex m;
    std::condition_variable cv;
    bool isRT;
    std::atomic<bool> _execute;
    bool set_priority();
    void run();

public:
    bool run_check();
    GxRtCheck();
    ~GxRtCheck();
};

/****************************************************************
 ** port connection callback
 */

struct PortConnData {
public:
    PortConnData() {} // no init
    PortConnData(const char *a, const char *b, bool conn)
	: name_a(a), name_b(b), connect(conn) {}
    ~PortConnData() {}
    const char *name_a;
    const char *name_b;
    bool connect;
};

struct JackClientActivityStatus {
    bool ok;
    bool changed;
    bool active;
    bool any_active;
    bool inactive;
    bool amp_active;
    bool fx_active;
    bool amp_present;
    bool fx_present;
    bool single_client;
    unsigned long long generation;
    unsigned long long transition_usecs;
    unsigned long long amp_callback_count;
    unsigned long long fx_callback_count;
    unsigned long long amp_callbacks_since_activate;
    unsigned long long fx_callbacks_since_activate;
    int amp_ramp_mode;
    int fx_ramp_mode;
    bool engine_ready;
    int amp_rc;
    int fx_rc;
    int rollback_amp_rc;
    int rollback_fx_rc;
    bool rollback_incomplete;
    std::string last_error;
};

class PortConnRing {
private:
    jack_ringbuffer_t *ring;
    bool send_changes;
    int overflow;  // should be bool but gives compiler error
    void set_overflow() { gx_system::atomic_set(&overflow, true); }
    void clear_overflow()  { gx_system::atomic_set(&overflow, false); }
    bool is_overflow() { return gx_system::atomic_get(overflow); }
public:
    Glib::Dispatcher new_data;
    Glib::Dispatcher portchange;
    void push(const char *a, const char *b, bool conn);
    bool pop(PortConnData*);
    void set_send(bool v) { send_changes = v; }
    PortConnRing();
    ~PortConnRing();
};


/****************************************************************
 ** class GxJack
 */

class PortConnection {
public:
    jack_port_t *port;
    list<string> conn;
};

class JackPorts {
public:
    PortConnection input;
    PortConnection midi_input;
    PortConnection insert_out;
    PortConnection midi_output;
    PortConnection insert_in;
    PortConnection output1;
    PortConnection output2;
};

#ifdef HAVE_JACK_SESSION
extern "C" {
    typedef int (*jack_set_session_callback_type)(
	jack_client_t *, JackSessionCallback, void *arg);
    typedef char *(*jack_get_uuid_for_client_name_type)(
	jack_client_t *, const char *);
    typedef char *(*jack_client_get_uuid_type)(jack_client_t *);
}
#endif


class MidiCC {
private:
    gx_engine::GxEngine& engine;
    static const int max_midi_cc_cnt = 25;
    std::atomic<bool> send_cc[max_midi_cc_cnt];
    int cc_num[max_midi_cc_cnt];
    int pg_num[max_midi_cc_cnt];
    int bg_num[max_midi_cc_cnt];
    int me_num[max_midi_cc_cnt];
public:
    MidiCC(gx_engine::GxEngine& engine_);
    bool send_midi_cc(int _cc, int _pg, int _bgn, int _num);
    inline int next(int i = -1) const;
    inline int size(int i)  const { return me_num[i]; }
    inline void fill(unsigned char *midi_send, int i);
};

inline int MidiCC::next(int i) const {
    while (++i < max_midi_cc_cnt) {
        if (send_cc[i].load(std::memory_order_acquire)) {
            return i;
        }
    }
    return -1;
}

inline void MidiCC::fill(unsigned char *midi_send, int i) {
    if (size(i) == 3) {
        midi_send[2] =  bg_num[i];
    }
    midi_send[1] = pg_num[i];    // program value
    midi_send[0] = cc_num[i];    // controller+ channel
    send_cc[i].store(false, std::memory_order_release);
}

class GxJack: public sigc::trackable {
 private:
    GxRtCheck           rtc;
    bool                IS_RT;
    gx_engine::GxEngine& engine;
    std::atomic<bool>   jack_is_down;
    std::atomic<bool>   jack_is_exit;
    bool                bypass_insert;
    MidiCC              mmessage;
    static int          gx_jack_srate_callback(jack_nframes_t, void* arg);
    static int          gx_jack_xrun_callback(void* arg);
    static int          gx_jack_buffersize_callback(jack_nframes_t, void* arg);
    static int          gx_jack_process(jack_nframes_t, void* arg);
    static int          gx_jack_insert_process(jack_nframes_t, void* arg);
    static void         gx_jack_thread_init_main(void* arg);
    static void         gx_jack_thread_init_insert(void* arg);

    static void         shutdown_callback_client(void* arg);
    static void         shutdown_callback_client_insert(void* arg);
    void                gx_jack_shutdown_callback();
    static void         gx_jack_portreg_callback(jack_port_id_t, int, void* arg);
    static void         gx_jack_portconn_callback(jack_port_id_t a, jack_port_id_t b, int connect, void* arg);
#ifdef HAVE_JACK_SESSION
    jack_session_event_t *session_event;
    jack_session_event_t *session_event_ins;
    int                 session_callback_seen;
    static void         gx_jack_session_callback(jack_session_event_t *event, void *arg);
    static void         gx_jack_session_callback_ins(jack_session_event_t *event, void *arg);
    static jack_set_session_callback_type jack_set_session_callback_fp;
    static jack_get_uuid_for_client_name_type jack_get_uuid_for_client_name_fp;
    static jack_client_get_uuid_type jack_client_get_uuid_fp;
#endif
    void                cleanup_slot(bool otherthread);
    void                fetch_connection_data();
    PortConnRing        connection_queue;
    sigc::signal<void,string,string,bool> connection_changed;
    Glib::Dispatcher    buffersize_change;

    Glib::Dispatcher    client_change_rt;
    sigc::signal<void>  client_change;
    string              client_instance;
    std::atomic<jack_nframes_t> jack_sr;   // jack sample rate
    std::atomic<jack_nframes_t> jack_bs;   // jack buffer size
    float               *insert_buffer;
    mutable std::mutex  client_activity_mutex;
    std::atomic<bool>   jack_shutdown_seen;
    std::atomic<bool>   amp_client_active;
    std::atomic<bool>   fx_client_active;
    unsigned long long  client_activity_generation;
    unsigned long long  client_activity_transition_usecs;
    unsigned long long  amp_callback_count_at_activate;
    unsigned long long  fx_callback_count_at_activate;
    int                 client_activity_amp_rc;
    int                 client_activity_fx_rc;
    int                 client_activity_rollback_amp_rc;
    int                 client_activity_rollback_fx_rc;
    bool                client_activity_rollback_incomplete;
    std::string         client_activity_last_error;
    std::atomic<unsigned long long> xrun_count;
    std::atomic<unsigned int> last_xrun_usecs;
    unsigned long long  reported_xrun_count;
    enum { performance_poll_interval_ms = 1000 };
    enum CallbackStream {
        callback_stream_main,
        callback_stream_insert,
        callback_stream_count
    };
    enum { callback_sample_capacity = 2048 };
    struct alignas(64) CallbackTelemetryRing {
        std::atomic<unsigned long long> count;
        // Each sample packs the low 32 bits of (sequence + 1) above the
        // duration. A single atomic publication prevents readers from pairing
        // a new duration with an old sequence while the ring wraps.
        std::atomic<unsigned long long> samples[callback_sample_capacity];
    };
    // Main and insert callbacks normally run on different cores. Per-stream,
    // cache-line-aligned rings avoid bouncing one shared counter between them.
    CallbackTelemetryRing callback_rings[callback_stream_count];
    void record_callback_time(CallbackStream stream, unsigned int elapsed_usecs);
    void reset_performance_telemetry();
    bool poll_xrun();
    void report_xrun();
    void write_jack_port_connections(
	gx_system::JsonWriter& w, const char *key, const PortConnection& pc, bool replace=false);
    std::string make_clientvar(const std::string& s);
    std::string replace_clientvar(const std::string& s);
    int is_power_of_two (unsigned int x);
    bool                gx_jack_init(bool startserver, int wait_after_connect,
				     const gx_system::CmdlineOptions& opt);
    void                gx_jack_init_port_connection(const gx_system::CmdlineOptions& opt);
    void                gx_jack_callbacks();
    void                gx_jack_cleanup();
    JackClientActivityStatus client_activity_status_unlocked(
        bool ok, bool changed) const;
    JackClientActivityStatus set_client_activity(bool active);
    inline void         check_overload();
    void                process_midi_cc(void *buf, jack_nframes_t nframes);

 public:
    JackPorts           ports;

    jack_client_t*      client;
    jack_client_t*      client_insert;
    
    jack_position_t      current;
    jack_transport_state_t transport_state;
    jack_transport_state_t old_transport_state;

    jack_nframes_t      get_jack_sr() const { return jack_sr.load(std::memory_order_acquire); }
    jack_nframes_t      get_jack_bs() const { return jack_bs.load(std::memory_order_acquire); }
    float               get_jcpu_load() { return client ? jack_cpu_load(client) : -1; }
    bool                get_is_rt() { return client ? IS_RT: false; }
    jack_nframes_t      get_time_is() { return client ? jack_frame_time(client) : 0; }

public:
    GxJack(gx_engine::GxEngine& engine_);
    ~GxJack();

    void                set_jack_down(bool v) {
        jack_is_down.store(v, std::memory_order_release);
    }
    void                set_jack_exit(bool v) {
        jack_is_exit.store(v, std::memory_order_release);
    }

    void                set_jack_insert(bool v) { bypass_insert = v;}
    bool                gx_jack_connection(bool connect, bool startserver,
					   int wait_after_connect, const gx_system::CmdlineOptions& opt);
    unsigned long long  get_xrun_count() const { return xrun_count.load(std::memory_order_acquire); }
    unsigned int        get_last_xrun() const { return last_xrun_usecs.load(std::memory_order_acquire); }
    void                get_callback_performance(unsigned long long& count,
                                                 unsigned int& sample_count,
                                                 unsigned int& p99_usecs,
                                                 unsigned int& p999_usecs,
                                                 unsigned int& max_usecs) const;
    JackClientActivityStatus jack_deactivate_clients();
    JackClientActivityStatus jack_activate_clients();
    JackClientActivityStatus get_jack_client_activity_status() const;
    void*               get_midi_buffer(jack_nframes_t nframes);
    bool                send_midi_cc(int cc_num, int pgm_num, int bgn, int num);

    void                read_connections(gx_system::JsonParser& jp);
    void                write_connections(gx_system::JsonWriter& w);
    static string       get_default_instancename();
    const string&       get_instancename() { return client_instance; }
    string              client_name;
    string              client_insert_name;
    Glib::Dispatcher    session;
    Glib::Dispatcher    session_ins;
    Glib::Dispatcher    shutdown;
    bool                is_jack_down() const {
        return jack_is_down.load(std::memory_order_acquire);
    }
    Glib::Dispatcher    connection;
    bool                is_jack_exit() const {
        return jack_is_exit.load(std::memory_order_acquire);
    }
    sigc::signal<void>& signal_client_change() { return client_change; }
    sigc::signal<void,string,string,bool>& signal_connection_changed() { return connection_changed; }
    Glib::Dispatcher&   signal_portchange() { return connection_queue.portchange; }
    Glib::Dispatcher&   signal_buffersize_change() { return buffersize_change; }
    void                send_connection_changes(bool v) { connection_queue.set_send(v); }
    static void         rt_watchdog_set_limit(int limit);
    gx_engine::GxEngine& get_engine() { return engine; }
    bool                single_client;
#ifdef HAVE_JACK_SESSION
    jack_session_event_t *get_last_session_event() {
	return gx_system::atomic_get(session_event);
    }
    jack_session_event_t *get_last_session_event_ins() {
	return gx_system::atomic_get(session_event_ins);
    }
    int                 return_last_session_event();
    int                 return_last_session_event_ins();
    string              get_uuid_insert();
#endif
};

inline bool GxJack::send_midi_cc(int cc_num, int pgm_num, int bgn, int num) {
    if (!client) {
        return false;
    }
    return mmessage.send_midi_cc(cc_num, pgm_num, bgn, num);
}

} /* end of jack namespace */

#endif  // SRC_HEADERS_GX_JACK_H_
#endif

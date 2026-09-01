#include <cassert>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <glibmm/ustring.h>
#include <sigc++/sigc++.h>

namespace boost {
class noncopyable {
 protected:
    noncopyable() = default;
    ~noncopyable() = default;

 private:
    noncopyable(const noncopyable&);
    noncopyable& operator=(const noncopyable&);
};
}

namespace gx_engine {
class EngineControl;
}

using namespace std;

#define GX_JSON_WRITER_UNIT_TEST
#include "../src/gx_head/engine/gx_json.cpp"

int main() {
    gx_system::JsonStringWriter scene;
    scene.begin_object();
    scene.write_kv("applied", 3);
    scene.write_bool_kv("topologyChanged", true);
    scene.write_bool_kv("chainCommitted", true);
    scene.write_bool_kv("chainSettled", false);
    scene.end_object();
    std::cout << scene.get_string() << '\n';

    // Existing status RPCs intentionally use numeric 0/1. The explicit
    // boolean API must not silently change generic write_kv(bool) callers.
    gx_system::JsonStringWriter legacy;
    legacy.begin_object();
    legacy.write_kv("ok", true);
    legacy.end_object();
    std::cout << legacy.get_string() << '\n';
    return 0;
}

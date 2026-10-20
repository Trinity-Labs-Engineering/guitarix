"""Opt-in isolated JACK/HTTP test: GUITARIX_ANALYSIS_JACK_TEST=1 python3 -m unittest
discover -s tests -p test_houston_service.py. Uses a named dummy server, never the
player's JACK server. GUITARIX_ANALYSIS_FIXTURE can select an already built host.
"""
import ctypes
import json
import os
from pathlib import Path
import shutil
import socket
import subprocess
import tempfile
import time
import unittest
from urllib.request import Request, urlopen

ROOT = Path(__file__).resolve().parents[1]
ENGINE = ROOT / "src/gx_head/engine/houston"


@unittest.skipUnless(os.getenv("GUITARIX_ANALYSIS_JACK_TEST") == "1" and shutil.which("jackd"),
                     "opt-in named JACK dummy server required")
class ServiceTests(unittest.TestCase):
    def test_jack_http_lifecycle(self):
        with tempfile.TemporaryDirectory() as directory:
            directory = Path(directory)
            binary = os.getenv("GUITARIX_ANALYSIS_FIXTURE", str(directory / "host"))
            if not os.getenv("GUITARIX_ANALYSIS_FIXTURE"):
                subprocess.run([
                    "g++", "-std=c++20", "-O2", "-pthread",
                    "-I", str(ROOT / "src/headers"),
                    "-I", str(ROOT / "src/NAM/NeuralAmpModelerCore/Dependencies/nlohmann"),
                    str(ROOT / "tests/houston_service_fixture.cpp"),
                    *map(str, (ENGINE / "Service.cpp", ENGINE / "Analysis.cpp",
                               ENGINE / "yin/PitchTracker.cpp", ENGINE / "yin/YinTracker.cpp")),
                    "-ljack", "-o", binary,
                ], check=True)
            env = os.environ.copy()
            server_name = f"gx-analysis-test-{os.getpid()}"
            env["JACK_DEFAULT_SERVER"] = server_name
            env["XDG_CONFIG_HOME"] = str(directory / "config")
            env["XDG_CACHE_HOME"] = str(directory / "cache")
            Path(env["XDG_CONFIG_HOME"]).mkdir()
            Path(env["XDG_CACHE_HOME"]).mkdir()
            allocated = []
            for _ in range(4):
                sock = socket.socket()
                sock.bind(("127.0.0.1", 0))
                allocated.append(sock)
            ports = [sock.getsockname()[1] for sock in allocated]
            for sock in allocated:
                sock.close()
            for key, port in zip(("HOUSTON_TUNER_PORT", "HOUSTON_METER_PORT_PRE", "HOUSTON_METER_PORT_POST"), ports):
                env[key] = str(port)
            processes = []
            client = None
            lib = ctypes.CDLL("libjack.so.0")
            lib.jack_client_open.restype = ctypes.c_void_p
            lib.jack_client_open.argtypes = [ctypes.c_char_p, ctypes.c_uint, ctypes.POINTER(ctypes.c_uint), ctypes.c_char_p]
            lib.jack_connect.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p]
            lib.jack_disconnect.argtypes = lib.jack_connect.argtypes
            lib.jack_client_close.argtypes = [ctypes.c_void_p]
            lib.jack_set_buffer_size.argtypes = [ctypes.c_void_p, ctypes.c_uint]
            with open(directory / "jack.log", "w+") as log:
                try:
                    processes.append(subprocess.Popen(
                        ["jackd", "--no-realtime", "--name", server_name, "-d", "dummy", "-r", "48000", "-p", "128"],
                        env=env, stdout=log, stderr=log))
                    status = ctypes.c_uint()
                    for _ in range(50):
                        client = lib.jack_client_open(b"gx_analysis_test_ctl", 1 | 4, ctypes.byref(status), server_name.encode())
                        if client:
                            break
                        time.sleep(.1)
                    self.assertTrue(client, "named dummy JACK server did not start")
                    guitarix = os.getenv("GUITARIX_ANALYSIS_HOST")
                    command = [binary]
                    if guitarix:
                        env["GUITARIX_HOUSTON_ANALYSIS"] = "1"
                        command = [guitarix, "-N", "-K", "-p", str(ports[3]),
                                   "-n", "houston_vl1", "-s", server_name,
                                   "--style-dir", str(ROOT / "build/rcstyles"),
                                   "--builder-dir", str(ROOT / "src/gx_head/builder")]
                    host = subprocess.Popen(command, env=env, stdout=log, stderr=log)
                    processes.append(host)

                    def request(kind, path, action=None):
                        data = None if action is None else json.dumps({"action": action, "params": {}}).encode()
                        req = Request(f"http://127.0.0.1:{ports[kind]}{path}", data=data,
                                      headers={"Content-Type": "application/json"})
                        with urlopen(req, timeout=2) as response:
                            return json.load(response)

                    for _ in range(50):
                        try:
                            health = request(0, "/health")
                            break
                        except OSError:
                            time.sleep(.1)
                    else:
                        self.fail("native service did not start")
                    self.assertEqual(health["implementation"], "guitarix-native")
                    self.assertFalse(request(0, "/state")["tuner"]["analysis_enabled"])
                    processes.append(subprocess.Popen([binary, "--source"], env=env, stdout=log, stderr=log))
                    routes = [("left", "signal_in"), ("left", "pre_in_l"), ("right", "pre_in_r"),
                              ("right", "post_in_l"), ("left", "post_in_r")]
                    for source, target in routes:
                        for _ in range(50):
                            rc = lib.jack_connect(client, f"gx_analysis_test_signal:{source}".encode(),
                                                  f"houston_vl1_analysis:{target}".encode())
                            if rc == 0:
                                break
                            time.sleep(.05)
                        self.assertEqual(rc, 0)
                    request(0, "/api", "analysis.start")
                    time.sleep(1.0)
                    tuner = request(0, "/state")["tuner"]
                    self.assertTrue(tuner["detected"])
                    self.assertAlmostEqual(tuner["frequency_hz"], 440, delta=.2)
                    pre, post = request(1, "/state")["meter"], request(2, "/state")["meter"]
                    self.assertTrue(pre["stereo_available"] and post["stereo_available"])
                    self.assertAlmostEqual(pre["peak_l"], .25, delta=.01)
                    self.assertAlmostEqual(pre["peak_r"], 1.1, delta=.01)
                    self.assertAlmostEqual(post["peak_l"], 1.1, delta=.01)
                    self.assertTrue(pre["clipped_r"] and post["clipped_l"])
                    if guitarix:
                        with socket.create_connection(("127.0.0.1", ports[3]), timeout=2) as rpc:
                            reader = rpc.makefile("rb")
                            for method in ("jack_deactivate", "jack_activate"):
                                rpc.sendall(json.dumps({"jsonrpc": "2.0", "id": 1, "method": method,
                                                        "params": []}).encode() + b"\n")
                                result = json.loads(reader.readline())
                                self.assertNotIn("error", result)
                                self.assertTrue(result["result"]["ok"], result)
                                time.sleep(.15)
                                self.assertTrue(request(0, "/state")["tuner"]["detected"])
                                self.assertTrue(request(1, "/state")["meter"]["available"])
                            reader.close()
                    # Worker framing survives a JACK quantum change.
                    self.assertEqual(lib.jack_set_buffer_size(client, 256), 0)
                    time.sleep(.3)
                    self.assertEqual(request(0, "/health")["buffer_size"], 256)
                    self.assertAlmostEqual(request(0, "/state")["tuner"]["frequency_hz"], 440, delta=.2)
                    # No observer-owned output exists, and disabling tuner work
                    # cannot close the shared host or either meter endpoint.
                    self.assertEqual(set(request(1, "/ports")), {"post_in_l", "post_in_r"})
                    request(0, "/api", "service.shutdown")
                    self.assertIsNone(host.poll())
                    self.assertFalse(request(0, "/state")["tuner"]["detected"])
                    before = request(1, "/state")["jack"]["process_block_count"]
                    time.sleep(.1)
                    self.assertGreater(request(1, "/state")["jack"]["process_block_count"], before)
                    request(0, "/api", "analysis.start")
                    time.sleep(.5)
                    self.assertTrue(request(0, "/state")["tuner"]["detected"])
                    lib.jack_disconnect(client, b"gx_analysis_test_signal:left", b"houston_vl1_analysis:signal_in")
                    time.sleep(.25)
                    self.assertFalse(request(0, "/state")["tuner"]["detected"])
                    state = request(0, "/state")
                    print("Native JACK smoke:", state["jack"])
                    print("Native host memory:", [line for line in Path(f"/proc/{host.pid}/status").read_text().splitlines()
                                                 if line.startswith(("VmRSS:", "VmSize:", "Threads:"))])
                finally:
                    if client:
                        lib.jack_client_close(client)
                    for process in reversed(processes):
                        if process.poll() is None:
                            process.terminate()
                            try:
                                process.wait(timeout=5)
                            except subprocess.TimeoutExpired:
                                process.kill()
                                process.wait(timeout=2)
                    if any(process.returncode not in (0, -15) for process in processes):
                        log.seek(0)
                        print(log.read()[-6000:])


if __name__ == "__main__":
    unittest.main()

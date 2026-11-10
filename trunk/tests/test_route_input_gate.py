from pathlib import Path
import subprocess
import tempfile
import unittest


class RouteInputGateTests(unittest.TestCase):
    def test_fades_suppression_identity_and_acknowledgements(self):
        root = Path(__file__).resolve().parents[1]
        with tempfile.TemporaryDirectory() as directory:
            executable = Path(directory) / "gate"
            subprocess.run([
                "g++", "-std=c++17", "-O2", "-Wall", "-Wextra",
                "-I", str(root / "src/headers"),
                str(root / "tests/route_input_gate_fixture.cpp"),
                "-o", str(executable),
            ], check=True, capture_output=True)
            subprocess.run([str(executable)], check=True, timeout=10)

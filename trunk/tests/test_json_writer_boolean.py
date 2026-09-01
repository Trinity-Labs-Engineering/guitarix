from __future__ import annotations

import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile
import unittest


TRUNK = Path(__file__).resolve().parents[1]
FIXTURE = Path(__file__).with_name("json_writer_boolean_fixture.cpp")
JSONRPC_SOURCE = TRUNK / "src" / "gx_head" / "engine" / "jsonrpc.cpp"


class JsonWriterBooleanWireTests(unittest.TestCase):
    def test_set_scene_acknowledgement_uses_explicit_boolean_writer(self) -> None:
        source = JSONRPC_SOURCE.read_text(encoding="utf-8")
        set_scene = source.split("FUNCTION(set_scene) {", 1)[1].split(
            "FUNCTION(get) {", 1
        )[0]

        for field, value in (
            ("topologyChanged", "topology_changed"),
            ("chainCommitted", "topology_changed"),
            ("chainSettled", "chain_settled"),
        ):
            self.assertIn(f'jw.write_bool_kv("{field}", {value});', set_scene)
            self.assertNotIn(f'jw.write_kv("{field}",', set_scene)

    def test_explicit_booleans_use_json_literals_without_changing_legacy_status(self) -> None:
        compiler = os.environ.get("CXX", "c++")
        if shutil.which(compiler) is None:
            self.skipTest(f"C++ compiler is unavailable: {compiler}")

        pkg_config = subprocess.run(
            ["pkg-config", "--cflags", "glibmm-2.4", "sigc++-2.0"],
            check=True,
            capture_output=True,
            text=True,
        )
        with tempfile.TemporaryDirectory(prefix="gx-json-writer-test-") as temp_dir:
            executable = Path(temp_dir) / "json-writer-boolean-test"
            subprocess.run(
                [
                    compiler,
                    "-std=c++20",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    *shlex.split(pkg_config.stdout),
                    f"-I{TRUNK / 'src' / 'headers'}",
                    str(FIXTURE),
                    "-o",
                    str(executable),
                ],
                check=True,
            )
            wire_lines = subprocess.run(
                [str(executable)],
                check=True,
                capture_output=True,
                text=True,
            ).stdout.splitlines()

        self.assertEqual(
            wire_lines[0],
            '{"applied": 3,"topologyChanged": true,'
            '"chainCommitted": true,"chainSettled": false}',
        )
        scene = json.loads(wire_lines[0])
        self.assertIs(type(scene["topologyChanged"]), bool)
        self.assertIs(type(scene["chainCommitted"]), bool)
        self.assertIs(type(scene["chainSettled"]), bool)

        self.assertEqual(wire_lines[1], '{"ok": 1}')
        self.assertIs(type(json.loads(wire_lines[1])["ok"]), int)


if __name__ == "__main__":
    unittest.main()

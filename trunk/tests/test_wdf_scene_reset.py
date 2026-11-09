"""Quiet startup at current controls, including reuse after a sustained note."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class WdfSceneResetTests(unittest.TestCase):
    def test_zero_input_after_control_changes_and_reactivation(self):
        with tempfile.TemporaryDirectory(prefix='gx-wdf-reset-') as directory:
            executable = Path(directory) / 'reset'
            subprocess.run([
                os.environ.get('CXX', 'c++'), '-std=c++20', '-O2',
                '-fsanitize=address,undefined', '-fno-sanitize-recover=all',
                f'-I{TRUNK / "src/headers"}', f'-I{TRUNK / "src"}',
                str(TRUNK / 'tests/wdf_scene_reset_fixture.cpp'), '-o', str(executable),
            ], check=True, capture_output=True, text=True, timeout=120)
            result = subprocess.run([str(executable)], capture_output=True,
                                    text=True, timeout=120)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)


if __name__ == '__main__':
    unittest.main()

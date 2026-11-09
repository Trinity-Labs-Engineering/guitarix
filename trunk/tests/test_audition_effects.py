"""Exercise actual plugin callbacks, state reuse, RT allocation and Houston metadata."""
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class AuditionEffectsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory = tempfile.TemporaryDirectory(prefix='gx-audition-')
        cls.addClassCleanup(cls.directory.cleanup)
        cls.executable = Path(cls.directory.name) / 'audition'
        result = subprocess.run([
            os.environ.get('CXX', 'c++'), '-std=c++20', '-O1', '-g',
            '-fsanitize=address,undefined', '-fno-sanitize-recover=all',
            '-I/usr/include/eigen3', f'-I{TRUNK / "src/headers"}', f'-I{TRUNK / "src"}', '-D_(x)=x',
            str(TRUNK / 'tests/audition_effects_fixture.cpp'),
            str(TRUNK / 'src/plugins/audition_effects.cc'), '-o', str(cls.executable),
        ], capture_output=True, text=True, timeout=180)
        if result.returncode:
            raise RuntimeError(result.stderr)

    def test_audio_and_state(self):
        result = subprocess.run([str(self.executable)], capture_output=True, text=True, timeout=180)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_cat_zero_input_activation_with_current_controls(self):
        result = subprocess.run([str(self.executable), '--mouse-startup'],
                                capture_output=True, text=True, timeout=180)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_houston_control_contract(self):
        catalog = TRUNK.parents[1] / 'houston/models/objects/base/lib/pytarix/models/pedals/lib/Audition.json'
        if not catalog.exists():
            self.skipTest('Houston sibling checkout unavailable')
        result = subprocess.run([str(self.executable), '--metadata'], capture_output=True, text=True, check=True)
        actual = {line.split('\t')[0]: list(map(float, line.split('\t')[1:])) for line in result.stdout.splitlines()}
        expected = {p['gtx_name']: [p[k] for k in ('default_val', 'min_val', 'max_val', 'step')]
                    for effect in json.loads(catalog.read_text()) for p in effect['pots']}
        self.assertEqual(set(expected), set(actual))
        for key, values in expected.items():
            for a, b in zip(values, actual[key]):
                self.assertAlmostEqual(a, b, places=5, msg=key)


if __name__ == '__main__':
    unittest.main()

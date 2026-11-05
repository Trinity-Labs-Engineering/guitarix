from pathlib import Path
import json
import os
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]

class ArtemisEqTests(unittest.TestCase):
    def test_audio_flatness_responses_extremes_and_native_stereo_capture(self):
        with tempfile.TemporaryDirectory() as directory:
            exe = Path(directory) / 'eq-test'
            result = subprocess.run([os.environ.get('CXX','c++'), '-std=c++20', '-O2', '-pthread',
                '-Wall','-Wextra','-Werror','-fsanitize=undefined,float-divide-by-zero',
                '-fno-sanitize-recover=all', f'-I{TRUNK / "src/headers"}',
                f'-I{TRUNK / "src/zita-resampler-1.1.0"}',
                str(TRUNK / 'tests/artemis_eq_test.cpp'), '-o', str(exe)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stderr)
            frame = json.loads(result.stdout)
            self.assertEqual(frame['source'], 'guitarix')
            self.assertEqual(len(frame['frequencies']), 384)
            self.assertEqual(frame['left_peak_frequency_hz'], 750)
            self.assertEqual(frame['right_peak_frequency_hz'], 1500)
            self.assertAlmostEqual(frame['left_peak_db'], -6.0206, places=2)
            self.assertAlmostEqual(frame['right_peak_db'], -12.0412, places=2)
            self.assertEqual(frame['sample_rate'], 48000)

if __name__ == '__main__':
    unittest.main()

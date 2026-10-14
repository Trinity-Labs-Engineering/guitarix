"""Compile the production plugins; compare the reference and measure rendered audio.

Run with python3 -m unittest discover -s trunk/tests -p test_harmoniser.py.
Requires a C++ compiler and NumPy for FFT measurements.
"""
import importlib.util
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

import numpy as np

TRUNK = Path(__file__).resolve().parents[1]
REFERENCE = TRUNK.parents[2] / 'DOCS/MODEPY/MODE_ENCODING.py'


class HarmoniserTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary = tempfile.TemporaryDirectory(prefix='harmoniser-')
        cls.executable = Path(cls.temporary.name) / 'harmoniser'
        subprocess.run([
            os.environ.get('CXX', 'c++'), '-std=c++11', '-O2', '-Wall', '-Wextra', '-Werror',
            '-Wno-unused-parameter', '-Wno-mismatched-new-delete', f'-I{TRUNK / "src/headers"}',
            str(TRUNK / 'tests/harmoniser_fixture.cpp'),
            str(TRUNK / 'src/plugins/harmoniser.cc'), str(TRUNK / 'src/plugins/dual_harmoniser.cc'),
            '-o', str(cls.executable),
        ], check=True, capture_output=True, text=True)

    @classmethod
    def tearDownClass(cls):
        cls.temporary.cleanup()

    def render(self, *, dual=0, rate=48000, frequency=220, tonic=3, mode=0,
               interval=2, interval2=4, mix=50, wet=50, dry=0, block=32, inplace=0, kind=0):
        result = subprocess.run([str(self.executable), 'render', *map(str, (
            dual, rate, frequency, tonic, mode, interval, interval2, mix, wet, dry, block, inplace, kind,
        ))], check=True, capture_output=True, timeout=30)
        return np.frombuffer(result.stdout, dtype=np.float32)

    def assert_frequency(self, audio, rate, expected):
        tail = audio[rate:]
        spectrum = np.abs(np.fft.rfft(tail * np.hanning(len(tail)), n=rate*4))
        frequencies = np.fft.rfftfreq(rate*4, 1/rate)
        peak = frequencies[np.argmax(spectrum)]
        self.assertLess(abs(1200 * np.log2(peak / expected)), 8, (peak, expected))
        self.assertGreater(np.sqrt(np.mean(tail**2)), .025)

    def test_all_15120_settings_match_python_source_of_truth(self):
        spec = importlib.util.spec_from_file_location('mode_reference', REFERENCE)
        reference = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(reference)
        actual = list(map(int, subprocess.check_output([self.executable, 'intervals'], text=True).split()))
        expected = [reference.interval_shift(note, reference.INTERVALS[abs(interval)], interval > 0, tonic, mode)
                    for tonic in reference.ALL_12_NOTES for mode in reference.MODE_BINARY
                    for note in reference.ALL_12_NOTES for interval in range(-7, 8)]
        self.assertEqual(actual, expected)

    def test_lowest_fundamental_tracking_and_no_realtime_allocations(self):
        result = subprocess.run([self.executable, 'tracker'], capture_output=True, text=True, timeout=90)
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_actual_audio_all_intervals_at_three_sample_rates(self):
        # A in C Major: every ascending/descending interval, including unison.
        shifts = [-12,-10,-9,-7,-5,-4,-2,0,2,3,5,7,8,10,12]
        for rate in (44100,48000,96000):
            for interval, shift in zip(range(-7,8), shifts):
                with self.subTest(rate=rate, interval=interval):
                    self.assert_frequency(self.render(rate=rate, interval=interval), rate, 220*2**(shift/12))

    def test_modes_tonics_and_out_of_mode_octave_are_audible(self):
        for tonic, mode, frequency, interval, shift in (
            (3,0,261.625565,2,4), (3,5,261.625565,2,3),
            (0,0,220,-2,-3), (8,3,349.228231,3,6),
            (3,0,277.182631,2,12), (3,0,277.182631,-2,-12),
            (3,0,277.182631,0,0),
        ):
            with self.subTest(tonic=tonic, mode=mode, interval=interval):
                self.assert_frequency(self.render(tonic=tonic, mode=mode, frequency=frequency, interval=interval),48000,frequency*2**(shift/12))

    def test_audio_across_guitar_register(self):
        for frequency in (82.406889,110,164.813778,329.627557,659.255114,1318.510228):
            for interval, shift in ((-7,-12),(-2,-4),(2,3),(7,12)):
                with self.subTest(frequency=frequency, interval=interval):
                    self.assert_frequency(self.render(frequency=frequency, interval=interval),48000,frequency*2**(shift/12))

    def test_dual_voice_mix_endpoints_and_linear_blend(self):
        one = self.render(dual=1,mix=100)
        two = self.render(dual=1,mix=0)
        self.assert_frequency(one,48000,220*2**(3/12))
        self.assert_frequency(two,48000,220*2**(7/12))
        np.testing.assert_allclose(self.render(dual=1,mix=35),one*.35+two*.65,atol=3e-7)
        t=np.arange(len(one))/48000
        np.testing.assert_allclose(self.render(dual=1,mix=35,dry=50),one*.35+two*.65+.1*np.sin(2*np.pi*220*t),atol=3e-7)

    def test_dry_wet_gains_inplace_block_sizes_silence_and_reset(self):
        wet = self.render()
        np.testing.assert_allclose(self.render(wet=100),wet*2,atol=3e-7)
        dry = self.render(wet=0,dry=50)
        np.testing.assert_allclose(self.render(wet=0,dry=100),dry*2,atol=3e-7)
        self.assertTrue(np.all(self.render(wet=0,dry=0)==0))
        self.assertTrue(np.all(self.render(kind=3)==0))
        for dual in (0,1):
            expected=self.render(dual=dual,kind=2,dry=30)
            for block in (1,37,257,1111):
                np.testing.assert_allclose(self.render(dual=dual,kind=2,dry=30,block=block,inplace=1),expected,atol=3e-6)

    def test_chord_is_shifted_from_its_lowest_note(self):
        # C major chord -> a major third shift selected from low C.
        rate=48000
        audio=self.render(frequency=130.812783,interval=2,kind=2)
        spectrum=np.abs(np.fft.rfft(audio[rate:]*np.hanning(rate),n=rate*4))
        for source in (130.812783,130.812783*2**(4/12),130.812783*2**(7/12)):
            expected=source*2**(4/12)
            index=round(expected*4)
            self.assertGreater(max(spectrum[index-4:index+5]),max(spectrum)*.2)


if __name__ == '__main__':
    unittest.main()

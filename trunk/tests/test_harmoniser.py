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
               interval=2, interval2=4, mix=50, wet=50, dry=0, block=32, inplace=0, kind=0, audio=None):
        result = subprocess.run([str(self.executable), 'render-input' if audio is not None else 'render', *map(str, (
            dual, rate, frequency, tonic, mode, interval, interval2, mix, wet, dry, block, inplace, kind,
        ))], input=None if audio is None else np.asarray(audio,dtype=np.float32).tobytes(), check=True, capture_output=True, timeout=30)
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

    def test_modes_tonics_and_chromatic_interval_fallback_are_audible(self):
        for tonic, mode, frequency, interval, shift in (
            (3,0,261.625565,2,4), (3,5,261.625565,2,3),
            (0,0,220,-2,-3), (8,3,349.228231,3,6),
            (3,0,277.182631,2,3), (3,0,277.182631,-2,-4),
            (3,0,277.182631,0,0),
        ):
            with self.subTest(tonic=tonic, mode=mode, interval=interval):
                self.assert_frequency(self.render(tonic=tonic, mode=mode, frequency=frequency, interval=interval),48000,frequency*2**(shift/12))

    def test_no_note_or_key_can_turn_a_non_octave_interval_into_an_octave(self):
        values = np.array(list(map(int, subprocess.check_output([self.executable, 'intervals'], text=True).split())))
        settings = values.reshape(12,7,12,15)
        families = [(12,), (10,11), (8,9), (6,7), (5,6), (3,4), (1,2), (0,), (1,2), (3,4), (5,6), (6,7), (8,9), (10,11), (12,)]
        for index, allowed in enumerate(families):
            self.assertTrue(np.isin(np.abs(settings[:,:,:,index]),allowed).all())
            self.assertTrue((settings[:,:,:,index] * (index-7) >= 0).all())

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

    def trace(self, audio, rate):
        result = subprocess.run([self.executable,'trace',str(rate)],input=np.asarray(audio,dtype=np.float32).tobytes(),capture_output=True,check=True)
        return np.array(list(map(int,result.stdout.split())))

    def test_short_pitch_glitches_do_not_replace_an_established_note(self):
        for rate in (44100,48000,96000):
            t=np.arange(2*rate)/rate
            frequency=np.full(len(t),220.0)
            # Brief nonharmonic attack contamination and isolated false tones.
            for start in (.35,.65,.95,1.25):
                frequency[(t>=start)&(t<start+.010)]=277.182631
            audio=.1*np.sin(2*np.pi*np.cumsum(frequency)/rate)
            notes=self.trace(audio,rate)
            self.assertTrue(np.all(notes[50:]==0),np.unique(notes[50:],return_counts=True))

    def test_chromatic_attack_and_retrigger_never_emit_octave_fallback(self):
        for rate in (44100,48000,96000):
            t=np.arange(2*rate)/rate
            # A sharp attack settles from C# to C. Previously the accepted C#
            # reading commanded +12/-12 until the detector reached C Major.
            local=np.mod(t,.5)
            frequency=np.where(local<.09,277.182631,261.625565)
            envelope=np.where(local<.38,.1,0.0)
            audio=envelope*np.sin(2*np.pi*np.cumsum(frequency)/rate)
            for dual,mix in ((0,50),(1,100),(1,0)):
                for interval in (-2,2):
                    wet=self.render(audio=audio,rate=rate,dual=dual,mix=mix,interval=interval,interval2=interval)
                    for start in (0,.5,1,1.5):
                        attack=wet[int((start+.04)*rate):int((start+.18)*rate)]
                        spectrum=np.abs(np.fft.rfft(attack*np.hanning(len(attack)),n=rate*2))
                        hz=np.fft.rfftfreq(rate*2,1/rate)
                        desired=(hz>190)&(hz<240) if interval<0 else (hz>300)&(hz<370)
                        octave=(hz>120)&(hz<145) if interval<0 else (hz>480)&(hz<580)
                        self.assertGreater(np.max(spectrum[desired]),5)
                        self.assertLess(np.max(spectrum[octave])/np.max(spectrum[desired]),.12,(rate,dual,mix,interval,start))


if __name__ == '__main__':
    unittest.main()

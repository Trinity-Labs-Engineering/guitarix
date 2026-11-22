"""Audio regressions for sustained chords, independent of the splice implementation."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

import numpy as np


TRUNK = Path(__file__).resolve().parents[1]


def chord(frequencies, rate):
    time = np.arange(3 * rate) / rate
    audio = np.zeros(time.size)
    partials = []
    for note, frequency in enumerate(frequencies):
        for harmonic in range(1, 13):
            partials.append(frequency * harmonic)
            audio += np.sin(2 * np.pi * frequency * harmonic * time + note * .31 + harmonic * .16) / (harmonic**1.2 * len(frequencies))
    return (.3 * audio).astype(np.float32), np.array(partials)


def off_note_energy(audio, partials, semitones, rate):
    sustained = audio[rate:]
    power = abs(np.fft.rfft(sustained * np.hanning(len(sustained)), n=4 * rate))**2
    hz = np.fft.rfftfreq(4 * rate, 1 / rate)
    expected = partials * 2**(semitones / 12)
    # Allow finite-window width and small tuning error, but not the extra
    # frequencies/sidebands which make a held chord sound metallic or wavery.
    mask = (abs(hz[:, None] - expected[None, :]) < np.maximum(2., expected[None, :] * .003)).any(axis=1)
    return 10 * np.log10((power[~mask].sum() + 1e-15) / (power[mask].sum() + 1e-15))


class PitchChordTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory = tempfile.TemporaryDirectory(prefix='pitch-chords-')
        cls.addClassCleanup(cls.directory.cleanup)
        compiler = os.environ.get('CXX', 'c++')
        cls.shared = Path(cls.directory.name) / 'shared'
        cls.capo = Path(cls.directory.name) / 'capo'
        flags = [compiler, '-std=c++11', '-O2', '-Wall', '-Wextra', '-Werror',
                 '-Wno-unused-parameter', '-Wno-mismatched-new-delete', f'-I{TRUNK / "src/headers"}']
        subprocess.run(flags + [str(TRUNK / 'tests/poly_pitch_shifter_fixture.cpp'), '-o', str(cls.shared)], check=True)
        subprocess.run(flags + [str(TRUNK / 'tests/virtual_capo_fixture.cpp'), str(TRUNK / 'src/plugins/virtual_capo.cc'), '-o', str(cls.capo)], check=True)

    def render(self, source, rate, semitones, bass=0, capo=False):
        args = ([str(self.capo), str(rate), str(semitones), str(bass)] if capo else
                [str(self.shared), 'render-input', str(rate), str(semitones), '1', str(bass)])
        result = subprocess.run(args, input=source.tobytes(), capture_output=True, check=True, timeout=20)
        output = np.frombuffer(result.stdout, dtype=np.float32)
        self.assertEqual(output.shape, source.shape)
        self.assertTrue(np.isfinite(output).all())
        return output

    def test_shared_engine_preserves_beating_chords_across_rates_and_modes(self):
        limits = {-7: 2., -2: -8., 2: -6., 4: 2., 7: 2., 12: 2.}
        for rate in (44100, 48000, 96000):
            source, partials = chord([196., 207.652349], rate)
            reference_rms = np.sqrt(np.mean(source[rate:]**2))
            for bass in (0, 1):
                for semitones, limit in limits.items():
                    with self.subTest(rate=rate, bass=bass, semitones=semitones):
                        output = self.render(source, rate, semitones, bass)
                        self.assertLess(off_note_energy(output, partials, semitones, rate), limit)
                        # Previously one chord could lose ~40% of its level at
                        # -7 as its two notes repeatedly cancelled in the fade.
                        self.assertAlmostEqual(np.sqrt(np.mean(output[rate:]**2)) / reference_rms, 1., delta=.15)

    def test_common_six_string_chord_transposes_cleanly(self):
        for rate in (44100, 48000, 96000):
            source, partials = chord([110., 130.812783, 164.813778, 220., 261.625565, 329.627557], rate)
            for bass in (0, 1):
                for semitones, limit in ((-2, -12.), (2, -9.)):
                    with self.subTest(rate=rate, bass=bass, semitones=semitones):
                        output = self.render(source, rate, semitones, bass)
                        self.assertLess(off_note_energy(output, partials, semitones, rate), limit)
                        sustained = output[rate:]
                        window = np.hanning(len(sustained))
                        spectrum = abs(np.fft.rfft(sustained * window, n=4 * rate)) * 2 / window.sum()
                        hz = np.fft.rfftfreq(4 * rate, 1 / rate)
                        # A low artifact score must not be earned by losing a
                        # string: retain the three distinct low fundamentals.
                        for frequency in (110., 130.812783, 164.813778):
                            expected = frequency * 2**(semitones / 12)
                            self.assertGreater(spectrum[abs(hz - expected) < 2].max(), .03)

    def test_capo_additional_range_improves_sustained_chords(self):
        for rate in (44100, 48000, 96000):
            source, partials = chord([110., 130.812783, 164.813778, 220., 261.625565, 329.627557], rate)
            for semitones in (-12, -2, 2, 7, 12):
                with self.subTest(rate=rate, semitones=semitones):
                    fast = off_note_energy(self.render(source, rate, semitones), partials, semitones, rate)
                    capo = off_note_energy(self.render(source, rate, semitones, capo=True), partials, semitones, rate)
                    self.assertLess(capo, fast - 1.)

    def test_capo_runtime_controls_reset_and_in_place_processing(self):
        result = subprocess.run([str(self.capo)], capture_output=True, text=True, check=True, timeout=30)
        self.assertEqual(result.stdout.strip(), 'virtual-capo-ok')

    def test_capo_has_no_unshifted_dry_leak(self):
        rate = 48000
        source = (.3 * np.sin(2 * np.pi * 440 * np.arange(3 * rate) / rate)).astype(np.float32)
        for semitones in (-2, 2):
            output = self.render(source, rate, semitones, capo=True)[rate:]
            time = np.arange(output.size) / rate
            dry = abs(np.mean(output * np.exp(-2j * np.pi * 440 * time))) * 2
            self.assertLess(dry, .002)
            self.assertGreater(np.sqrt(np.mean(output**2)), .19)

    def test_capo_bass_low_b_stays_in_tune(self):
        for rate in (44100, 48000, 96000):
            source, _ = chord([30.867706], rate)
            for semitones in (-12, 12):
                with self.subTest(rate=rate, semitones=semitones):
                    output = self.render(source, rate, semitones, bass=1, capo=True)[rate:]
                    power = abs(np.fft.rfft(output * np.hanning(len(output)), n=rate * 8))
                    actual = np.argmax(power) / 8
                    expected = 30.867706 * 2**(semitones / 12)
                    self.assertLess(abs(1200 * np.log2(actual / expected)), 12)


if __name__ == '__main__':
    unittest.main()

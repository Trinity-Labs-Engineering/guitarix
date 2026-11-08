from pathlib import Path
import os
import subprocess
import tempfile
import unittest


class FuzzLSTMTests(unittest.TestCase):
    def test_vector_and_fallback_match_scalar_model_audio(self):
        trunk = Path(__file__).resolve().parents[1]
        eigen = trunk / 'src/NAM/NeuralAmpModelerCore/Dependencies/eigen'
        with tempfile.TemporaryDirectory(prefix='gx-fuzz-lstm-') as folder:
            for extra in ([], ['-DAUDITION_SCALAR_MATH']):
                with self.subTest(flags=extra):
                    executable = Path(folder) / 'fixture'
                    subprocess.run([
                        os.environ.get('CXX', 'c++'), '-std=c++20', '-O3',
                        '-fno-math-errno', '-fno-signed-zeros',
                        f'-I{eigen}', f'-I{trunk / "src/headers"}',
                        f'-I{trunk / "src"}', '-D_(x)=x', *extra,
                        str(Path(__file__).with_name('fuzz_lstm_fixture.cpp')),
                        '-o', str(executable),
                    ], check=True)
                    result = subprocess.run([str(executable)], capture_output=True,
                                            text=True, timeout=120, check=True)
                    self.assertIn('samples=768000', result.stdout)

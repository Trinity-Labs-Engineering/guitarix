from pathlib import Path
import os
import subprocess
import tempfile
import unittest


class ParallelThreadTests(unittest.TestCase):
    def test_completion_before_wait_does_not_sleep_or_lose_output(self):
        trunk = Path(__file__).resolve().parents[1]
        with tempfile.TemporaryDirectory(prefix='gx-parallel-thread-') as folder:
            executable = Path(folder) / 'fixture'
            subprocess.run([
                os.environ.get('CXX', 'c++'), '-std=c++20', '-O2', '-pthread',
                '-Wall', '-Wextra', '-Werror', f'-I{trunk / "src/headers"}',
                str(Path(__file__).with_name('parallel_thread_fixture.cpp')),
                '-o', str(executable),
            ], check=True)
            result = subprocess.run([str(executable)], capture_output=True,
                                    text=True, timeout=20, check=True)
            self.assertEqual(result.stdout.strip(), 'parallel-thread-ok')

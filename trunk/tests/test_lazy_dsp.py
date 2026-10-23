from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest

TRUNK = Path(__file__).resolve().parents[1]


class LazyDspTests(unittest.TestCase):
    def test_dormant_storage_resident_reuse_and_failed_allocation_cleanup(self):
        compiler = os.environ.get("CXX", "c++")
        if not shutil.which(compiler):
            self.skipTest("C++ compiler unavailable")
        with tempfile.TemporaryDirectory(prefix="gx-lazy-dsp-") as directory:
            executable = Path(directory) / "lazy-dsp"
            # The production resampler implementation only needs these
            # headers; avoid pulling JACK/UI initialization into this fixture.
            resampler = Path(directory) / "gx_resampler.cpp"
            resampler.write_text((TRUNK / "src/gx_head/engine/gx_resampler.cpp").read_text().replace(
                '#include "engine.h"',
                '#include <cassert>\n#include <cmath>\n#include <cstring>\n#include "gx_resampler.h"',
            ))
            sources = [TRUNK / "tests/lazy_dsp_fixture.cpp"] + [
                TRUNK / f"src/plugins/generated/{name}.cc"
                for name in ("mbdel", "mbe", "mbchor", "zita_rev1")
            ]
            sources += [resampler, TRUNK / "src/zita-resampler-1.1.0/resampler.cc",
                        TRUNK / "src/zita-resampler-1.1.0/resampler-table.cc"]
            build = subprocess.run([
                compiler, "-std=c++17", "-O0", "-g", "-fsanitize=address,undefined",
                "-D_(x)=x", f"-I{TRUNK / 'src/headers'}", f"-I{TRUNK / 'src'}",
                f"-I{TRUNK / 'src/gx_head/engine/tabels'}",
                f"-I{TRUNK / 'src/zita-resampler-1.1.0'}",
                *map(str, sources), "-o", str(executable),
            ], capture_output=True, text=True, timeout=180)
            self.assertEqual(build.returncode, 0, build.stderr)
            run = subprocess.run([str(executable)], capture_output=True, text=True, timeout=60)
            self.assertEqual(run.returncode, 0, run.stderr)
            self.assertEqual(run.stdout.strip(), "lazy-dsp-ok")


if __name__ == "__main__":
    unittest.main()

from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess
import unittest


TRUNK = Path(__file__).resolve().parents[1]


class PeakEqCompileTests(unittest.TestCase):
    def test_generated_eq_compiles_with_faust_support_macros(self) -> None:
        compiler = os.environ.get("CXX", "c++")
        if shutil.which(compiler) is None:
            self.skipTest(f"C++ compiler is unavailable: {compiler}")

        # Match gx_faust_plugins.cpp's include order: the support header's
        # min/max macros must remain defined when compiling the generated EQ.
        source = """
#include "gx_plugin.h"
#include "gx_faust_support.h"
#define _(text) (text)
namespace gx_engine {
namespace gx_effects {
#include "faust-generated/peak_eq.cc"
}
}
"""
        completed = subprocess.run(
            [
                compiler,
                "-std=c++20",
                "-Wall",
                "-Werror",
                "-fsyntax-only",
                f"-I{TRUNK / 'src' / 'headers'}",
                f"-I{TRUNK / 'src' / 'zita-resampler-1.1.0'}",
                f"-I{TRUNK / 'src'}",
                "-x", "c++", "-",
            ],
            input=source,
            capture_output=True,
            text=True,
            timeout=30,
        )
        self.assertEqual(completed.returncode, 0, completed.stderr)


if __name__ == "__main__":
    unittest.main()

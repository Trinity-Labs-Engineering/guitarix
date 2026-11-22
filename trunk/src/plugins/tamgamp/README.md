# Tamgamp preamp circuits

This directory contains the Peavey 5150 II lead, Mesa Dual Rectifier red and
Mesa DC-3 lead preamps from Tamgamp revision
`426da74142fcb6b7687a35b2b1dda3392e171b92`. The preferred Faust sources,
generated C++, circuit tables, schematics, code generation Makefile and Faust
support code are included. The circuit code and tables are unmodified. The
three rows in `normalization.h` come from the upstream
`src/resources/ampsim.cpp`. [Tamgamp source at this revision](https://github.com/sadko4u/tamgamp.lv2/tree/426da74142fcb6b7687a35b2b1dda3392e171b92)

Cyrus Adkisson originated Tamgamp. Vladimir Sadovnikov developed the software.
The Guitarix team developed DK Builder and contributed circuit and Faust code.
The upstream notices and LGPL-3.0-or-later terms are supplied in
`AUTHORS`, `COPYING.LESSER` and `COPYING`.

The Guitarix adapter in `../metal_preamp.cc` binds gain and the three tone
controls, preserves the upstream normalization, and runs each preamp at a
minimum internal rate of 96 kHz on supported device rates. Guitarix supplies
the sample-rate conversion, power stages and scene switching. The combined
audio engine is distributed under GPL-3.0-or-later.

Building Guitarix with its usual Waf command compiles the checked-in headers.
To regenerate them with the upstream Faust 2.15.11 toolchain, run
`make -C src/faust BUILD_DIR=../../include/generated FAUST=faust` from this
directory. The standalone install carries this directory and the adapter in
`share/doc/artemis-guitarix/metal-heads/`.

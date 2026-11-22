declare id "phase90";
declare name "Phase 90";
declare category "Modulation";
declare author "David Braun; Artemis integration";
declare license "MIT";

import("stdfaust.lib");
circuit = library("artemis_circuits.lib");
mod = library("artemis_modulation.lib");

rate = hslider("rate[name:Rate][unit:Hz][scale:log]", 0.5, 0.1, 10, 0.01) : mod.smooth_control;
level = hslider("level[name:Level][unit:dB]", -5, -20, 6, 0.1) : ba.db2linear : mod.smooth_control;
// The circuit already mixes dry and all-pass paths. Restore polarity for parallel racks;
// -5 dB default approximately offsets the model's 1.788x output-stage gain.
process = circuit.mxrPhase90(rate) : *(-level);

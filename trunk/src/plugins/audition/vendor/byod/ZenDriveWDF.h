// Adapted from Chowdhury-DSP/BYOD (GPL-3.0); see ../../SOURCES.md.
// Artemis: standalone double-precision core, scalar R-type clipping.
#pragma once




class ZenDriveWDF
{
public:
    ZenDriveWDF() = default;

    void prepare (double sampleRate)
    {
        dcReady = false;

        R4.setVoltage (4.5f);
        R5_R6_C5.setVoltage (4.5f);

        Vin_C3.prepare ((double) sampleRate);
        Rv9_C4.prepare ((double) sampleRate);
        R5_R6_C5.prepare ((double) sampleRate);
    }

    void setParameters(double voice, double gain) {
        dcReady = false;
        R5_R6_C5.setResistanceValue(R5 + voice * R6);
        Rv9_C4.setResistanceValue(R9 * std::max(gain, 0.001));
    }

    inline double processSample (double x) noexcept
    {
        Vin_C3.setVoltage (x);

        diodes.incident (P3.reflected());
        P3.incident (diodes.reflected());

        return wdft::voltage<double> (RL);
    }

    // Solve only the DC bias/feedback path, with the coupling capacitors open.
    // Keep the existing diode implementation and port impedances: this changes
    // initial state, not the audio circuit. Return the output DC for the wrapper.
    double reset (double gainResistance)
    {
        Rv9_C4.setResistanceValue (gainResistance);
        const double currentPerVolt = (1 - opAmpOutputResistance / loadResistance)
            / (opAmpGain * opAmpInputResistance);
        const auto outputAt = [=] (double feedbackVoltage) {
            return (4.5 + feedbackVoltage)
                / (1 + (biasResistance + opAmpInputResistance) * currentPerVolt);
        };
        const auto residual = [&] (double feedbackVoltage) {
            const double current = outputAt (feedbackVoltage) * currentPerVolt;
            const double diodeCurrent = -current - feedbackVoltage / gainResistance;
            const double incident = feedbackVoltage + P3.wdf.R * diodeCurrent;
            diodes.incident (incident);
            return 0.5 * (incident + diodes.reflected()) - feedbackVoltage;
        };
        double feedbackVoltage = dcFeedbackVoltage;
        // Two bounded scalar Newton steps suffice in this microvolt DC range;
        // no audio warm-up, allocation or convergence-dependent loop on reset.
        if (!dcReady) {
            feedbackVoltage = -outputAt (0) * currentPerVolt * gainResistance;
            for (int step = 0; step < 2; ++step) {
                const double error = residual (feedbackVoltage);
                const double slope = (residual (feedbackVoltage + 1e-8)
                    - residual (feedbackVoltage - 1e-8)) / 2e-8;
                feedbackVoltage -= error / slope;
            }
            dcFeedbackVoltage = feedbackVoltage;
            dcReady = true;
        }
        const double output = outputAt (feedbackVoltage);
        const double current = output * currentPerVolt;
        const double input = 4.5 - biasResistance * current;
        const double negativeInput = output - feedbackVoltage;
        residual (feedbackVoltage);
        Vin_C3.reset();
        Vin_C3.setVoltage (0);
        Vin_C3.incident (input);
        R5_R6_C5.resetToVoltage (negativeInput);
        Rv9_C4.resetToVoltage (feedbackVoltage);
        const auto impedances = R.getPortImpedances();
        R.restoreWaves ({feedbackVoltage + R.wdf.R * current,
                        input + impedances[0] * current, negativeInput, 0,
                        feedbackVoltage - R.wdf.R * current,
                        input - impedances[0] * current, negativeInput, 2 * output});
        return output;
    }

    static constexpr double biasResistance = 470.0e3;
    static constexpr double loadResistance = 1.0e6;
    static constexpr double opAmpGain = 100.0;
    static constexpr double opAmpInputResistance = 1.0e9;
    static constexpr double opAmpOutputResistance = double(1.0e-1f);

    // Port B
    wdft::CapacitiveVoltageSourceT<double> Vin_C3 { 470.0e-9f };
    wdft::ResistiveVoltageSourceT<double> R4 { biasResistance };
    wdft::WDFParallelT<double, decltype (Vin_C3), decltype (R4)> P1 { Vin_C3, R4 };

    // Port C
    static constexpr auto R5 = 1.0e3f;
    static constexpr auto R6 = 10.0e3f;
    wdft::ResistiveCapacitiveVoltageSourceT<double> R5_R6_C5 { R5 + R6, 100.0e-9f };

    // Port D
    wdft::ResistorT<double> RL { loadResistance };

    struct ImpedanceCalc
    {
        template <typename RType>
        static double calcImpedance (RType& R)
        {
            constexpr double Ag = opAmpGain;
            constexpr double Ri = opAmpInputResistance;
            constexpr double Ro = opAmpOutputResistance;

            const auto [Rb, Rc, Rd] = R.getPortImpedances();

            // This scattering matrix was derived using the R-Solver python script (https://github.com/jatinchowdhury18/R-Solver),
            // invoked with command: r_solver.py --adapt 0 --out scratch/tube_screamer_scatt.txt scratch/tube_screamer.txt
            R.setSMatrixData ({ { 0, (Ag * Rd * Ri - Rc * Rd + Rc * Ro) / ((Rb + Rc) * Rd + Rd * Ri - (Rb + Rc + Ri) * Ro), -((Ag + 1) * Rd * Ri + Rb * Rd - (Rb + Ri) * Ro) / ((Rb + Rc) * Rd + Rd * Ri - (Rb + Rc + Ri) * Ro), -Ro / (Rd - Ro) },
                                { -(Rb * Rc * Rd - Rb * Rc * Ro) / ((Ag + 1) * Rc * Rd * Ri + Rb * Rc * Rd - (Rb * Rc + (Rb + Rc) * Rd + (Rc + Rd) * Ri) * Ro), ((Ag + 1) * Rc * Rc * Rd * Ri + (Ag + 1) * Rc * Rd * Ri * Ri - Rb * Rb * Rc * Rd + (Rb * Rb * Rc - (Rc + Rd) * Ri * Ri + (Rb * Rb - Rc * Rc) * Rd - (Rc * Rc + 2 * Rc * Rd) * Ri) * Ro) / ((Ag + 1) * Rc * Rd * Ri * Ri + ((Ag + 2) * Rb * Rc + (Ag + 1) * Rc * Rc) * Rd * Ri + (Rb * Rb * Rc + Rb * Rc * Rc) * Rd - (Rb * Rb * Rc + Rb * Rc * Rc + (Rc + Rd) * Ri * Ri + (Rb * Rb + 2 * Rb * Rc + Rc * Rc) * Rd + (2 * Rb * Rc + Rc * Rc + 2 * (Rb + Rc) * Rd) * Ri) * Ro), ((Ag + 1) * Rb * Rc * Rd * Ri + Rb * Rb * Rc * Rd - (Rb * Rb * Rc + 2 * (Rb * Rb + Rb * Rc) * Rd + (Rb * Rc + 2 * Rb * Rd) * Ri) * Ro) / ((Ag + 1) * Rc * Rd * Ri * Ri + ((Ag + 2) * Rb * Rc + (Ag + 1) * Rc * Rc) * Rd * Ri + (Rb * Rb * Rc + Rb * Rc * Rc) * Rd - (Rb * Rb * Rc + Rb * Rc * Rc + (Rc + Rd) * Ri * Ri + (Rb * Rb + 2 * Rb * Rc + Rc * Rc) * Rd + (2 * Rb * Rc + Rc * Rc + 2 * (Rb + Rc) * Rd) * Ri) * Ro), -Rb * Rc * Ro / ((Ag + 1) * Rc * Rd * Ri + Rb * Rc * Rd - (Rb * Rc + (Rb + Rc) * Rd + (Rc + Rd) * Ri) * Ro) },
                                { -(Rb * Rc * Rd + Rc * Rd * Ri - (Rb * Rc + Rc * Ri) * Ro) / ((Ag + 1) * Rc * Rd * Ri + Rb * Rc * Rd - (Rb * Rc + (Rb + Rc) * Rd + (Rc + Rd) * Ri) * Ro), (Ag * Rc * Rd * Ri * Ri + Rb * Rc * Rc * Rd + (Ag * Rb * Rc + (2 * Ag + 1) * Rc * Rc) * Rd * Ri - (Rb * Rc * Rc + 2 * (Rb * Rc + Rc * Rc) * Rd + (Rc * Rc + 2 * Rc * Rd) * Ri) * Ro) / ((Ag + 1) * Rc * Rd * Ri * Ri + ((Ag + 2) * Rb * Rc + (Ag + 1) * Rc * Rc) * Rd * Ri + (Rb * Rb * Rc + Rb * Rc * Rc) * Rd - (Rb * Rb * Rc + Rb * Rc * Rc + (Rc + Rd) * Ri * Ri + (Rb * Rb + 2 * Rb * Rc + Rc * Rc) * Rd + (2 * Rb * Rc + Rc * Rc + 2 * (Rb + Rc) * Rd) * Ri) * Ro), -((Ag + 1) * Rc * Rc * Rd * Ri + Rb * Rc * Rc * Rd - (Rb * Rc * Rc - Rd * Ri * Ri - (Rb * Rb - Rc * Rc) * Rd + (Rc * Rc - 2 * Rb * Rd) * Ri) * Ro) / ((Ag + 1) * Rc * Rd * Ri * Ri + ((Ag + 2) * Rb * Rc + (Ag + 1) * Rc * Rc) * Rd * Ri + (Rb * Rb * Rc + Rb * Rc * Rc) * Rd - (Rb * Rb * Rc + Rb * Rc * Rc + (Rc + Rd) * Ri * Ri + (Rb * Rb + 2 * Rb * Rc + Rc * Rc) * Rd + (2 * Rb * Rc + Rc * Rc + 2 * (Rb + Rc) * Rd) * Ri) * Ro), -(Rb * Rc + Rc * Ri) * Ro / ((Ag + 1) * Rc * Rd * Ri + Rb * Rc * Rd - (Rb * Rc + (Rb + Rc) * Rd + (Rc + Rd) * Ri) * Ro) },
                                { (Ag * Rc * Rd * Ri - ((Rb + Rc) * Rd + Rd * Ri) * Ro) / ((Ag + 1) * Rc * Rd * Ri + Rb * Rc * Rd - (Rb * Rc + (Rb + Rc) * Rd + (Rc + Rd) * Ri) * Ro), ((Ag * Ag + 2 * Ag) * Rc * Rd * Rd * Ri * Ri + (2 * Ag * Rb * Rc + Ag * Rc * Rc) * Rd * Rd * Ri + (Rc * Rd * Ri + (Rb * Rc + Rc * Rc) * Rd) * Ro * Ro - ((Rb * Rc + Rc * Rc) * Rd * Rd + (2 * Ag * Rc * Rd + Ag * Rd * Rd) * Ri * Ri + ((Ag * Rb + (Ag + 1) * Rc) * Rd * Rd + (2 * Ag * Rb * Rc + Ag * Rc * Rc) * Rd) * Ri) * Ro) / ((Ag + 1) * Rc * Rd * Rd * Ri * Ri + ((Ag + 2) * Rb * Rc + (Ag + 1) * Rc * Rc) * Rd * Rd * Ri + (Rb * Rb * Rc + Rb * Rc * Rc) * Rd * Rd + (Rb * Rb * Rc + Rb * Rc * Rc + (Rc + Rd) * Ri * Ri + (Rb * Rb + 2 * Rb * Rc + Rc * Rc) * Rd + (2 * Rb * Rc + Rc * Rc + 2 * (Rb + Rc) * Rd) * Ri) * Ro * Ro - ((Rb * Rb + 2 * Rb * Rc + Rc * Rc) * Rd * Rd + ((Ag + 2) * Rc * Rd + Rd * Rd) * Ri * Ri + 2 * (Rb * Rb * Rc + Rb * Rc * Rc) * Rd + (2 * (Rb + Rc) * Rd * Rd + ((Ag + 4) * Rb * Rc + (Ag + 2) * Rc * Rc) * Rd) * Ri) * Ro), -(Ag * Rb * Rc * Rd * Rd * Ri + (Ag * Ag + Ag) * Rc * Rd * Rd * Ri * Ri - ((2 * Rb + Rc) * Rd * Ri + Rd * Ri * Ri + (Rb * Rb + Rb * Rc) * Rd) * Ro * Ro + ((Rb * Rb + Rb * Rc) * Rd * Rd - (Ag * Rc * Rd + (Ag - 1) * Rd * Rd) * Ri * Ri - (Ag * Rb * Rc * Rd + ((Ag - 2) * Rb + (Ag - 1) * Rc) * Rd * Rd) * Ri) * Ro) / ((Ag + 1) * Rc * Rd * Rd * Ri * Ri + ((Ag + 2) * Rb * Rc + (Ag + 1) * Rc * Rc) * Rd * Rd * Ri + (Rb * Rb * Rc + Rb * Rc * Rc) * Rd * Rd + (Rb * Rb * Rc + Rb * Rc * Rc + (Rc + Rd) * Ri * Ri + (Rb * Rb + 2 * Rb * Rc + Rc * Rc) * Rd + (2 * Rb * Rc + Rc * Rc + 2 * (Rb + Rc) * Rd) * Ri) * Ro * Ro - ((Rb * Rb + 2 * Rb * Rc + Rc * Rc) * Rd * Rd + ((Ag + 2) * Rc * Rd + Rd * Rd) * Ri * Ri + 2 * (Rb * Rb * Rc + Rb * Rc * Rc) * Rd + (2 * (Rb + Rc) * Rd * Rd + ((Ag + 4) * Rb * Rc + (Ag + 2) * Rc * Rc) * Rd) * Ri) * Ro), -((Ag + 1) * Rc * Rd * Rd * Ri + Rb * Rc * Rd * Rd - (Rb * Rc + Rc * Ri) * Ro * Ro - ((Rb + Rc) * Rd * Rd + Rd * Rd * Ri) * Ro) / ((Ag + 1) * Rc * Rd * Rd * Ri + Rb * Rc * Rd * Rd + (Rb * Rc + (Rb + Rc) * Rd + (Rc + Rd) * Ri) * Ro * Ro - (2 * Rb * Rc * Rd + (Rb + Rc) * Rd * Rd + ((Ag + 2) * Rc * Rd + Rd * Rd) * Ri) * Ro) } });

            const auto Ra = ((Ag + 1) * Rc * Rd * Ri + Rb * Rc * Rd - (Rb * Rc + (Rb + Rc) * Rd + (Rc + Rd) * Ri) * Ro) / ((Rb + Rc) * Rd + Rd * Ri - (Rb + Rc + Ri) * Ro);
            return Ra;
        }
    };

    wdft::RtypeAdaptor<double, 0, ImpedanceCalc, decltype (P1), decltype (R5_R6_C5), decltype (RL)> R { P1, R5_R6_C5, RL };

    // Port A
    static constexpr auto R9 = 500.0e3f;
    wdft::ResistorCapacitorParallelT<double> Rv9_C4 { R9, 100.0e-12f };
    wdft::WDFParallelT<double, decltype (Rv9_C4), decltype (R)> P3 { Rv9_C4, R };

    wdft::DiodePairT<double, decltype (P1), wdft::DiodeQuality::Best, OmegaProvider> diodes { P1, 5.241435962608312e-10f, 0.07877217375325735f };

private:
    // Controls and sample rate invalidate the operating point, audio history
    // does not. Repeated bypass/unbypass need not solve it again.
    bool dcReady = false;
    double dcFeedbackVoltage = 0;
};

// Adapted from Chowdhury-DSP/BYOD (GPL-3.0); see ../../SOURCES.md.
// Artemis: standalone double-precision core, scalar R-type clipping.
#pragma once




class MouseDriveWDF
{
public:
    MouseDriveWDF() = default;

    void prepare (double sampleRate)
    {
        Vin_C1.prepare ((double) sampleRate);
        C2.prepare ((double) sampleRate);
        Rd_C4.prepare ((double) sampleRate);
        R4_C5.prepare ((double) sampleRate);
        R5_C6.prepare ((double) sampleRate);
        R6_C7.prepare ((double) sampleRate);

        R2.setVoltage (4.5f);
    }

    inline double process (double x) noexcept
    {
        Vin_C1.setVoltage (x);
        diodes.incident (Sd.reflected());
        const auto y = wdft::voltage<double> (diodes);
        Sd.incident (diodes.reflected());
        return y;
    }

    // Initialise the zero-input DC operating point directly. A finite warm-up
    // leaves the AC-coupling capacitors charging for seconds, and saving those
    // waves also ties subsequent resets to the preparation-time drive control.
    // At DC the capacitors are open: the bias/feedback path carries current I,
    // Vout = (Ag * Ri - Ro) * I, and the output coupling capacitor isolates the
    // diode pair (zero diode voltage/current). Series WDF ports reverse polarity.
    void reset (double distortionResistance)
    {
        Rd_C4.setResistanceValue (distortionResistance);
        const double current = 4.5 / (biasResistance + inputSeriesResistance
            + (opAmpGain + 1) * opAmpInputResistance + distortionResistance - opAmpOutputResistance);
        const double output = (opAmpGain * opAmpInputResistance - opAmpOutputResistance) * current;
        const double input = 4.5 - (biasResistance + inputSeriesResistance) * current;
        const double feedback = output + distortionResistance * current;
        const double acrossDrive = distortionResistance * current;
        Vin_C1.reset();
        Vin_C1.setVoltage (0);
        Vin_C1.incident (4.5 - biasResistance * current);
        C2.incident (-input);
        R4_C5.resetToVoltage (-feedback);
        R5_C6.resetToVoltage (-feedback);
        Rd_C4.resetToVoltage (acrossDrive);
        R6_C7.resetToVoltage (-output);
        const auto impedances = R.getPortImpedances();
        R.restoreWaves ({-input - impedances[0] * current, -feedback,
                        acrossDrive - impedances[2] * current, output,
                        -input + impedances[0] * current, -feedback,
                        acrossDrive + impedances[2] * current, output});
        diodes.wdf.a = diodes.wdf.b = 0;
    }

    static constexpr double biasResistance = 1.0e6;
    static constexpr double inputSeriesResistance = 1.0e3;
    static constexpr double opAmpGain = 100.0;
    static constexpr double opAmpInputResistance = 10.0e6;
    static constexpr double opAmpOutputResistance = double(1.0e-1f);

    // Port A
    wdft::CapacitiveVoltageSourceT<double> Vin_C1 { 22.0e-9f };
    wdft::ResistiveVoltageSourceT<double> R2 { biasResistance };
    wdft::WDFParallelT<double, decltype (Vin_C1), decltype (R2)> P1 { Vin_C1, R2 };

    wdft::ResistorT<double> R3 { inputSeriesResistance };
    wdft::WDFSeriesT<double, decltype (P1), decltype (R3)> S2 { P1, R3 };

    wdft::CapacitorT<double> C2 { 1.0e-9f };
    wdft::WDFParallelT<double, decltype (S2), decltype (C2)> Pa { S2, C2 };

    // Port B
    wdft::ResistorCapacitorSeriesT<double> R4_C5 { 47.0f, 2.2e-6f };
    wdft::ResistorCapacitorSeriesT<double> R5_C6 { 560.0f, 4.7e-6f };
    wdft::WDFParallelT<double, decltype (R4_C5), decltype (R5_C6)> Pb { R4_C5, R5_C6 };

    // Port C
    static constexpr double Rdistortion = 100.0e3f;
    wdft::ResistorCapacitorParallelT<double> Rd_C4 { 0.5f * Rdistortion, 100.0e-12f };

    // R-Type
    struct ImpedanceCalc
    {
        template <typename RType>
        static double calcImpedance (RType& R)
        {
            constexpr double Ag = opAmpGain;
            constexpr double Ri = opAmpInputResistance;
            constexpr double Ro = opAmpOutputResistance;

            const auto [Ra, Rb, Rc] = R.getPortImpedances();
            const auto Rd = -(((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro) / (Ra * Rb + Ra * Rc + Rb * Rc + Rb * Ri + Ag * Rb * Ri + Rc * Ri - (Ra + Rb + Ri) * Ro));

            R.setSMatrixData ({ { (Ra * Rd * (Rb + Rc - Ro) + Rc * Ri * Ro + Rb * (Rc + Ri) * Ro) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), (Ra * (-(Rc * Rd) + (Rc + Rd) * Ro)) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), (Ra * Rb * (Rd - Ro)) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), -((Ra * Rb) / (Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri))) },
                                { (-(Rb * Rd * (Rc + Ag * Ri)) + Rb * (Rc + Rd) * Ro) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), -((Rc * Rd * (Ra + Ri) + (Ra * (Rb - Rd) - Rd * Ri + Rb * (Rc + Ri)) * Ro) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro)), (Rb * (Ra + Ri) * (Rd - Ro)) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), -((Rb * (Ra + Ri)) / (Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri))) },
                                { -((Rc * (Ag * Rd * Ri + Rb * (-Rd + Ro))) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro)), (Rc * Rd * (Ra + Ri + Ag * Ri) - Rc * (Ra + Ri) * Ro) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), (Rc * Rd * (Ra + Rb + Ri) + Rb * (Ra + Ri) * Ro) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), -((Rc * (Ra + Rb + Ri)) / (Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri))) },
                                { (Rd * (Ag * (Rb + Rc) * Ri - Rb * Ro)) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro), -((Rd * (Ag * Rc * Ri + (Ra + Ri) * Ro)) / ((Ra * (Rb + Rc) + Rc * Ri + Rb * (Rc + Ri)) * Ro)), -((Rd + Ro) / Ro), 0 } });

            return Rd;
        }
    };
    wdft::RtypeAdaptor<double, 3, ImpedanceCalc, decltype (Pa), decltype (Pb), decltype (Rd_C4)> R { Pa, Pb, Rd_C4 };

    // Port D
    wdft::ResistorCapacitorSeriesT<double> R6_C7 { 1.0e3f, 4.7e-6f };
    wdft::WDFSeriesT<double, decltype (R), decltype (R6_C7)> Sd { R, R6_C7 };

    wdft::DiodePairT<double, decltype (Sd), wdft::DiodeQuality::Best, OmegaProvider> diodes { Sd, 5.0e-9f, 25.85e-3f, 2.0f };

private:
};

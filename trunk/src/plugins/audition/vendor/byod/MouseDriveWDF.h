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

    // Port A
    wdft::CapacitiveVoltageSourceT<double> Vin_C1 { 22.0e-9f };
    wdft::ResistiveVoltageSourceT<double> R2 { 1.0e6f };
    wdft::WDFParallelT<double, decltype (Vin_C1), decltype (R2)> P1 { Vin_C1, R2 };

    wdft::ResistorT<double> R3 { 1.0e3f };
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
            constexpr double Ag = 100.0f; // op-amp gain
            constexpr double Ri = 10.0e6f; // op-amp input impedance
            constexpr double Ro = 1.0e-1f; // op-amp output impedance

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

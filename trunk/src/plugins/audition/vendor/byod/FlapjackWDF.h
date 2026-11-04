// Adapted from Chowdhury-DSP/BYOD (GPL-3.0); see ../../SOURCES.md.
// Artemis: standalone double-precision core, scalar R-type clipping.
#pragma once

#include "FlapjackOpAmpRType.h"
#include "FlapjackWDFScatteringMatrix.h"


class FlapjackWDF
{
public:
    FlapjackWDF() = default;

    void prepare (double fs)
    {
        Vin_C1.prepare ((double) fs);
        Vin_C1.incident (-4.83726311f);
        Vin_C1.reflected();

        C2.prepare ((double) fs);
        C2.incident (4.83662891f);
        C2.reflected();

        C6.prepare ((double) fs);
        C6.incident (-5.02740574f);
        C6.reflected();

        C7.prepare ((double) fs);
        C7.incident (5.07801962f);
        C7.reflected();

        C8_C9.prepare ((double) fs);
        C8_C9.incident (5.0518117f);
        C8_C9.reflected();

        Vb.setVoltage (4.9f);
    }

    void reset()
    {
        Vin_C1.reset();
        C2.reset();
        C6.reset();
        C7.reset();
        C8_C9.reset();
    }

    void setParams (double driveParam, double presenceParam, double lowCutHz)
    {
        chowdsp::wdft::ScopedDeferImpedancePropagation deferImpedance { R };

        R4_Rdrive.setResistanceValue (R4 + driveParam * Rdrive);

        Rp_m.setResistanceValue (presenceParam * Rpresence);
        Rp_p.setResistanceValue ((1.0f - presenceParam) * Rpresence);

        const auto c8c9Val = 1.0f / ((2.0 * 3.14159265358979323846) * RlevelVal * lowCutHz);
        C8_C9.setCapacitanceValue (c8c9Val);
    }

    template <FlapjackClipMode clipMode>
    inline double processSample (double x)
    {
        Vin_C1.setVoltage (x);
        R.compute<clipMode>();

        return wdft::voltage<double> (Rlevel);
    }

    // Port A
    wdft::CapacitiveVoltageSourceT<double> Vin_C1 { 10.0e-9f };
    wdft::ResistiveVoltageSourceT<double> Vb { 1.0e6f };
    wdft::WDFParallelT<double, decltype (Vin_C1), decltype (Vb)> Pin { Vin_C1, Vb };
    wdft::ResistorT<double> R2 { 10.0e3f };
    wdft::WDFSeriesT<double, decltype (R2), decltype (Pin)> Sa { R2, Pin };

    // Port B
    wdft::CapacitorT<double> C2 { 10.0e-6f };
    static constexpr auto R4 = 220.0f;
    static constexpr auto Rdrive = 47.0e3f;
    wdft::ResistorT<double> R4_Rdrive { R4 };
    wdft::WDFSeriesT<double, decltype (C2), decltype (R4_Rdrive)> Sb { C2, R4_Rdrive };

    // Port C
    wdft::ResistorT<double> R3 { 100.0e3f };

    // Port D
    wdft::ResistorT<double> R5 { 10.0e3f };

    // Port E
    wdft::ResistorT<double> R6 { 10.0e3f };

    // Port F
    wdft::CapacitorT<double> C6 { 22.0e-9f };

    // Port G
    static constexpr auto Rpresence = 47.0e3f;
    wdft::ResistorT<double> Rp_p { 0.5f * Rpresence };

    // Port H
    wdft::ResistorT<double> Rp_m { 0.5f * Rpresence };

    // Port J
    wdft::CapacitorT<double> C7 { 82.0e-9f };
    wdft::ResistorT<double> R7 { 1.0e3f };
    wdft::WDFSeriesT<double, decltype (C7), decltype (R7)> Sj { C7, R7 };

    // Port K
    wdft::CapacitorT<double> C8_C9 { 47.0e-9f };
    static constexpr double RlevelVal = 47.0e3f;
    wdft::ResistorT<double> Rlevel { RlevelVal };
    wdft::WDFSeriesT<double, decltype (C8_C9), decltype (Rlevel)> Sk { C8_C9, Rlevel };

    struct ImpedanceCalc
    {
        template <typename RType>
        static void calcImpedance (RType& R)
        {
            const auto [Ra, Rb, Rc, Rd, Re, Rf, Rg, Rh, Rj, Rk] = R.getPortImpedances();
            static constexpr double Ag = 100000.0f; // op-amp gain
            static constexpr double Ri = 500.0e6f; // op-amp input impedance
            static constexpr double Ro = 1.0e-1f; // op-amp output impedance
            matrix_setter (Ra, Rb, Rc, Rd, Re, Rf, Rg, Rh, Rj, Rk, Ri, Ro, Ag, R);
        }
    };

    using RType = FlapjackOpAmpRType<double,
                                     ImpedanceCalc,
                                     decltype (Sa),
                                     decltype (Sb),
                                     decltype (R3),
                                     decltype (R5),
                                     decltype (R6),
                                     decltype (C6),
                                     decltype (Rp_p),
                                     decltype (Rp_m),
                                     decltype (Sj),
                                     decltype (Sk)>;
    RType R { Sa, Sb, R3, R5, R6, C6, Rp_p, Rp_m, Sj, Sk };
};

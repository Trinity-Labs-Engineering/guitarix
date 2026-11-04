// Adapted from Chowdhury-DSP/BYOD (GPL-3.0); see ../../SOURCES.md.
// Artemis: standalone double-precision core, scalar R-type clipping.
#pragma once



class KingOfToneClipper
{
public:
    KingOfToneClipper() = default;

    void processBlock (double* x, int numSamples) noexcept
    {
        for (int n = 0; n < numSamples; ++n)
        {
            R12_Vs.setVoltage (x[n]);

            dp.incident (S1.reflected());
            S1.incident (dp.reflected());

            x[n] = wdft::voltage<double> (dp);
        }
    }

    wdft::ResistiveVoltageSourceT<double> R12_Vs { 1.0e3f };
    wdft::PolarityInverterT<double, decltype (R12_Vs)> S1 { R12_Vs };
    wdft::DiodePairT<double, decltype (S1)> dp { S1, 2.52e-9f, 25.85e-3f, 1.752f };

private:
};

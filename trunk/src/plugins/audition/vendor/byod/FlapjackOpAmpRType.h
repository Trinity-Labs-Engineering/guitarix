// Adapted from Chowdhury-DSP/BYOD (GPL-3.0); see ../../SOURCES.md.
// Artemis: standalone double-precision core, scalar R-type clipping.
#pragma once



enum class FlapjackClipMode
{
    HardClip = 1,
    AlgSigmoid,
    LightFold,
};

template <typename T, typename ImpedanceCalculator, typename... PortTypes>
class FlapjackOpAmpRType : public wdft::RootWDF
{
public:
    /** Number of ports connected to RootRtypeAdaptor */
    static constexpr auto numPorts = int (sizeof...(PortTypes));

    explicit FlapjackOpAmpRType (PortTypes&... dps) : downPorts (std::tie (dps...))
    {
        b_vec.clear();
        a_vec.clear();

        wdft::rtype_detail::forEachInTuple ([&] (auto& port, size_t)
                                            { port.connectToParent (this); },
                                            downPorts);
    }

    void reset() { a_vec.clear(); b_vec.clear(); }

    /** Recomputes internal variables based on the incoming impedances */
    void calcImpedance() override
    {
        ImpedanceCalculator::calcImpedance (*this);
    }

    constexpr auto getPortImpedances()
    {
        std::array<T, numPorts> portImpedances {};
        wdft::rtype_detail::forEachInTuple ([&] (auto& port, size_t i)
                                            { portImpedances[i] = port.wdf.R; },
                                            downPorts);

        return portImpedances;
    }

    /** Use this function to set the scattering matrix data. */
    void setSMatrixData (const T (&mat)[numPorts][numPorts])
    {
        for (int i = 0; i < numPorts; ++i)
            for (int j = 0; j < numPorts; ++j)
                S_matrix[j][i] = mat[i][j];
    }

    /** Computes both the incident and reflected waves at this root node. */
    template <FlapjackClipMode clipMode>
    inline void compute() noexcept
    {
        wdft::rtype_detail::RtypeScatter (S_matrix, a_vec, b_vec);

        for (int i = 0; i < numPorts; ++i) {
            T voltage = -0.5 * (a_vec[i] + b_vec[i]);
            if constexpr (clipMode == FlapjackClipMode::HardClip)
                voltage = std::clamp(voltage, T(0), T(9));
            else if constexpr (clipMode == FlapjackClipMode::AlgSigmoid) {
                T v = voltage - 4.5;
                voltage = v / std::sqrt(1.0 + 0.75 * (v / 4.5) * (v / 4.5)) + 4.5;
            } else
                voltage = 8.0 * std::sin(0.125 * voltage);
            b_vec[i] = -2.0 * voltage - a_vec[i];
        }

        wdft::rtype_detail::forEachInTuple ([&] (auto& port, size_t i)
                                            {
                                          port.incident (b_vec[i]);
                                          a_vec[i] = port.reflected(); },
                                            downPorts);
    }

    // Artemis: snapshot only cached waves, never copy the topology's references.
    std::array<T, 2 * numPorts> saveWaves() const {
        std::array<T, 2 * numPorts> state{};
        for(int i=0;i<numPorts;++i) {state[i]=a_vec[i];state[i+numPorts]=b_vec[i];}
        return state;
    }
    void restoreWaves(const std::array<T, 2 * numPorts>& state) {
        // Refresh nested series/parallel caches after capacitor state restoration.
        wdft::rtype_detail::forEachInTuple([](auto& port, size_t){ port.reflected(); }, downPorts);
        for(int i=0;i<numPorts;++i) {a_vec[i]=state[i];b_vec[i]=state[i+numPorts];}
    }

private:
    std::tuple<PortTypes&...> downPorts; // tuple of ports connected to RtypeAdaptor

    wdft::rtype_detail::Matrix<T, numPorts> S_matrix; // square matrix representing S
    wdft::rtype_detail::AlignedArray<T, numPorts> a_vec; // temp matrix of inputs to Rport
    wdft::rtype_detail::AlignedArray<T, numPorts> b_vec; // temp matrix of outputs from Rport
};

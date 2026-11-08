// BYOD Fuzz Machine model adapter. Weights: Jatin Chowdhury, GPL-3.0.
// Standalone PyTorch-compatible LSTM inference; see SOURCES.md.
#pragma once
#include "common.h"
#include <Eigen/Core>
#include "models/weights.h"
// glibc's AArch64 vector ABI uses libm's vector activations while
// evaluating four independent LSTM units together. Older/non-glibc platforms
// retain scalar libm; the model, both mode histories and rates are unchanged.
#if defined(__aarch64__) && defined(__GLIBC__)
#if __GLIBC_PREREQ(2, 40) && !defined(AUDITION_SCALAR_MATH)
#define AUDITION_VECTOR_MATH 1
#include <arm_neon.h>
extern "C" float32x4_t _ZGVnN4v_expf(float32x4_t) __attribute__((aarch64_vector_pcs));
extern "C" float32x4_t _ZGVnN4v_tanhf(float32x4_t) __attribute__((aarch64_vector_pcs));
#endif
#endif
namespace audition {
class FuzzLSTM {
    const FuzzWeights* weights=nullptr;
    Eigen::Matrix<float,24,1> hidden=Eigen::Matrix<float,24,1>::Zero(), cell=hidden;
public:
    void load(const FuzzWeights& w) { weights=&w; hidden.setZero();cell.setZero(); }
    double tick(double x,double fuzz) {
        const auto& w=*weights;
        Eigen::Map<const Eigen::Matrix<float,96,24,Eigen::RowMajor>> recurrent(&w.recurrent[0][0]);
        Eigen::Matrix<float,96,1> gates=recurrent*hidden;
        for(int i=0;i<96;++i)gates[i]+=w.bias[i]+w.input[i][0]*float(x)+w.input[i][1]*float(fuzz);
#if defined(AUDITION_VECTOR_MATH)
        float inputGate[24], forgetGate[24], outputGate[24], candidate[24], cellTanh[24];
        const auto one = vdupq_n_f32(1.0f);
        const auto sigmoid = [one](const float* v) {
            return vdivq_f32(one, vaddq_f32(one, _ZGVnN4v_expf(vnegq_f32(vld1q_f32(v)))));
        };
        for (int i=0; i<24; i+=4) {
            vst1q_f32(inputGate+i, sigmoid(gates.data()+i));
            vst1q_f32(forgetGate+i, sigmoid(gates.data()+i+24));
            vst1q_f32(candidate+i, _ZGVnN4v_tanhf(vld1q_f32(gates.data()+i+48)));
            vst1q_f32(outputGate+i, sigmoid(gates.data()+i+72));
        }
        for (int i=0; i<24; ++i)
            cell[i]=forgetGate[i]*cell[i]+inputGate[i]*candidate[i];
        for (int i=0; i<24; i+=4)
            vst1q_f32(cellTanh+i, _ZGVnN4v_tanhf(vld1q_f32(cell.data()+i)));
#else
        auto sigmoid=[](float v){return 1.0f/(1.0f+std::exp(-v));};
#endif
        double y=w.outputBias+x; // Model metadata skip=1: residual input is essential.
        for(int i=0;i<24;++i) {
#if defined(AUDITION_VECTOR_MATH)
            hidden[i]=outputGate[i]*cellTanh[i];
#else
            cell[i]=sigmoid(gates[i+24])*cell[i]+sigmoid(gates[i])*std::tanh(gates[i+48]);
            hidden[i]=sigmoid(gates[i+72])*std::tanh(cell[i]);
#endif
            y+=w.output[i]*hidden[i];
        }
        return y;
    }
};
class FuzzMachine {
    std::array<FuzzLSTM,2> models,initial;
    std::array<OnePole,2> dc;
    std::array<double,2> zero{};
    RateBridge bridge;
    double fs=96000,fuzz=0.5,level=0.75,blend=1;
    int mode=1;
public:
    void configure(const Params& p) {fuzz=p[0];mode=int(p[1]);level=1.5*p[2];}
    void prepare(double rate,const Params& p) {
        bool family44=std::fmod(rate,44100.0)==0;
        fs=family44?88200:96000;bridge.prepare(rate,fs);configure(p);
        models[0].load(family44?fuzz_15_88:fuzz_15);models[1].load(family44?fuzz_2_88:fuzz_2);
        for(int i=0;i<2;++i) {
            dc[i].highpass(30,fs);
            for(int j=0;j<int(fs);++j)zero[i]=models[i].tick(0,fuzz);
        }
        initial=models;reset();
    }
    void reset() {models=initial;for(auto& f:dc)f.reset();bridge.reset();blend=mode;}
    double tick(double x) {
        return level*bridge.tick(x,[this](double v){
            double a=dc[0].tick(models[0].tick(v,fuzz)-zero[0]);
            double b=dc[1].tick(models[1].tick(v,fuzz)-zero[1]);
            blend+=(mode-blend)*(1-std::exp(-1/(0.01*fs)));
            return a+(b-a)*blend;
        });
    }
};
} // namespace audition

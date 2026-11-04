// Adaptations of Jatin Chowdhury's BYOD processors, GPL-3.0.
// See SOURCES.md for pinned upstream sources and modifications.
#pragma once
#include "common.h"
#include "vendor/chowdsp_wdf/chowdsp_wdf.h"

namespace audition {
namespace wdft = chowdsp::wdft;
struct OmegaProvider {
    template<class T> static T omega(T x) {
        // Solve w+log(w)=x. Avoid overflow in exp(x) for large port waves.
        T w = x > T(1) ? x-std::log(x) : std::exp(x);
        w = std::max(w, T(1e-30));
        for (int i=0; i<4; ++i) {
            T f = w+std::log(w)-x;
            w = std::max(T(1e-30), w - f*w/(w+1));
        }
        return w;
    }
};
#include "vendor/byod/MouseDriveWDF.h"
#include "vendor/byod/ZenDriveWDF.h"
#include "vendor/byod/KingOfToneOverdrive.h"
#include "vendor/byod/KingOfToneClipper.h"
#include "vendor/byod/FlapjackWDF.h"

class Mouse {
    MouseDriveWDF wdf;
    std::array<double,8> initialWaves{};
    decltype(wdf.diodes.wdf) initialDiode{};
    decltype(std::make_tuple(wdf.Vin_C1,wdf.C2,wdf.Rd_C4,wdf.R4_C5,wdf.R5_C6,wdf.R6_C7)) initial{wdf.Vin_C1,wdf.C2,wdf.Rd_C4,wdf.R4_C5,wdf.R5_C6,wdf.R6_C7};
    RateBridge bridge;
    OnePole tone, dc;
    double fs=96000, level=1, lastDrive=-1;
public:
    void configure(const Params& p) {
        if (p[0]!=lastDrive) {
            wdf.Rd_C4.setResistanceValue(std::max(1.0, 100000*logPot(p[0])));
            lastDrive=p[0];
        }
        // RAT Filter: 1k + 100k rheostat feeding 3.3 nF; clockwise is darker.
        tone.lowpass(1/(2*pi*(1000+100000*p[1])*3.3e-9),fs);
        level=db(p[2]);
    }
    void prepare(double rate,const Params& p) {
        fs=2*rate; bridge.prepare(rate,fs); lastDrive=-1;
        wdf.prepare(fs); configure(p); dc.highpass(20,fs);
        for(int i=0;i<12000;++i) raw(0);
        initial=std::make_tuple(wdf.Vin_C1,wdf.C2,wdf.Rd_C4,wdf.R4_C5,wdf.R5_C6,wdf.R6_C7);
        initialWaves=wdf.R.saveWaves();initialDiode=wdf.diodes.wdf;
        reset();
    }
    void reset() {
        std::tie(wdf.Vin_C1,wdf.C2,wdf.Rd_C4,wdf.R4_C5,wdf.R5_C6,wdf.R6_C7)=initial;
        wdf.R.restoreWaves(initialWaves);wdf.diodes.wdf=initialDiode;
        bridge.reset(); tone.reset(); dc.reset(); lastDrive=-1;
    }
    double raw(double x) { return dc.tick(tone.tick(wdf.process(x))); }
    double tick(double x) { return level*bridge.tick(x,[this](double v){return raw(v);}); }
};

class Yen {
    ZenDriveWDF wdf;
    std::array<double,8> initialWaves{};
    decltype(wdf.diodes.wdf) initialDiode{};
    decltype(std::make_tuple(wdf.Vin_C3,wdf.Rv9_C4,wdf.R5_R6_C5)) initial{wdf.Vin_C3,wdf.Rv9_C4,wdf.R5_R6_C5};
    RateBridge bridge;
    OnePole tone, dc;
    double fs=96000, level=1, offset=0, lastGain=-1, lastVoice=-1;
public:
    void configure(const Params& p) {
        if(p[0]!=lastGain || p[1]!=lastVoice) {
            wdf.setParameters(1-p[1],logPot(p[0])); lastGain=p[0]; lastVoice=p[1];
        }
        tone.lowpass(600*std::pow(20.0,p[2]),fs); level=db(p[3]);
    }
    void prepare(double rate,const Params& p) {
        fs=rate*2; bridge.prepare(rate,fs); lastGain=lastVoice=-1;
        wdf.prepare(fs); configure(p); dc.highpass(20,fs);
        for(int i=0;i<20000;++i) offset=wdf.processSample(0);
        initial=std::make_tuple(wdf.Vin_C3,wdf.Rv9_C4,wdf.R5_R6_C5);
        initialWaves=wdf.R.saveWaves();initialDiode=wdf.diodes.wdf;reset();
    }
    void reset() {
        std::tie(wdf.Vin_C3,wdf.Rv9_C4,wdf.R5_R6_C5)=initial;
        wdf.R.restoreWaves(initialWaves);wdf.diodes.wdf=initialDiode;
        bridge.reset(); tone.reset(); dc.reset(); lastGain=lastVoice=-1;
    }
    double tick(double x) {
        return level*bridge.tick(x,[this](double v){return tone.tick(dc.tick((wdf.processSample(v*0.5)-offset)*db(-10)));});
    }
};

class ToneKing {
    KingOfToneOverdrive od;
    std::array<double,8> initialWaves{};
    decltype(od.dp.wdf) initialDiode{};
    KingOfToneClipper clip;
    decltype(od.R9_C7_Vin) initial{10000,0.1e-6};
    RateBridge bridge;
    OnePole input,tone;
    Filter<3> gain;
    Filter<1> bypass;
    std::array<OnePole,3> dc;
    double fs=96000, level=1, lastDrive=-1;
    std::array<double,3> mix{0,1,0};
    int mode=1;
public:
    void configure(const Params& p) {
        if(lastDrive!=p[0]) {
            // BYOD's complete third-order drive-amplifier transfer function.
            double rd=10000+100000*std::pow(p[0],1.5);
            double r1b2=1e-8*1e-8*33000*27000, r1b1=1e-8*(33000+27000);
            double r1a2=1e-8*1e-8*(33000+27000), r1a0=2e-8;
            double r2a1=100e-12*rd;
            std::array<double,4> a{r1b2*r2a1,r1b1*r2a1+r1b2,r2a1+r1b1,1};
            auto b=a; b[1]+=r1a2*rd; b[3]+=r1a0*rd;
            gain.analog(b,a,2*pi*4000/std::tan(pi*4000/fs)); lastDrive=p[0];
        }
        tone.lowpass(700*std::pow(18.0,p[1]),fs); level=db(p[2]); mode=int(p[3]);
    }
    double raw(double x) {
        double v=gain.tick(input.tick(x*0.2));
        double over=v; od.processBlock(&over,1);
        double direct=bypass.tick(v)*db(-30)+4.5;
        double both=std::clamp(over,3.0,6.0), hard=std::clamp(direct,3.0,6.0);
        clip.processBlock(&hard,1); clip.processBlock(&both,1);
        double modes[3]={dc[0].tick(hard*db(45)), dc[1].tick(over*db(-12)), dc[2].tick(both*db(27))};
        double y=0;
        for(int i=0;i<3;++i) {mix[i]+=(double(i==mode)-mix[i])*(1-std::exp(-1/(0.01*fs))); y+=mix[i]*modes[i];}
        return tone.tick(y);
    }
    void prepare(double rate,const Params& p) {
        fs=rate*2; bridge.prepare(rate,fs); lastDrive=-1;
        input.highpass(1/(2*pi*1e-8*1e6),fs);
        bypass.analog({0.1e-6*(10000+220000),1},{0.1e-6*10000,1},2*fs);
        for(auto& f:dc)f.highpass(20,fs);
        od.prepare(fs); configure(p);
        for(int i=0;i<20000;++i)raw(0);
        initial=od.R9_C7_Vin;initialWaves=od.R.saveWaves();initialDiode=od.dp.wdf;reset();
    }
    void reset() {
        od.R9_C7_Vin=initial;od.R.restoreWaves(initialWaves);od.dp.wdf=initialDiode;bridge.reset(); input.reset(); gain.reset(); bypass.reset(); tone.reset();
        // DC is removed analytically at each branch on reset, preventing a bias step.
        for(auto& f:dc)f.reset();
        lastDrive=-1; mix={0,0,0}; mix[mode]=1;
        for(int i=0;i<3;++i) {
            double zero=i==1?4.5*db(-12):0;
            if(i!=1) {zero=4.5;clip.processBlock(&zero,1);zero*=db(i==0?45:27);}
            dc[i].z[0]=-dc[i].b[0]*zero;
        }
    }
    double tick(double x) {return level*bridge.tick(x,[this](double v){return raw(v);});}
};

class Flapjack {
    FlapjackWDF wdf;
    std::array<double,20> initialWaves{};
    decltype(std::make_tuple(wdf.Vin_C1,wdf.C2,wdf.C6,wdf.C7,wdf.C8_C9)) initial{wdf.Vin_C1,wdf.C2,wdf.C6,wdf.C7,wdf.C8_C9};
    RateBridge bridge;
    OnePole dc;
    Params last{};
    double fs=96000, level=1, offset=0;
    int mode=1;
public:
    void configure(const Params& p) {
        mode=int(p[4]);
        if(p[0]!=last[0]||p[1]!=last[1]||p[2]!=last[2]||p[4]!=last[4]) {
            double drive=p[0]*(mode==2?0.9:1);
            wdf.setParams((1-drive)*(1-drive),std::clamp(p[1],0.001,0.999),p[2]); last=p;
        }
        level=-db(-12+(mode==0?3:mode==1?4.5:5.5))*db(p[3]);
    }
    double circuit(double x) {
        if(mode==0)return wdf.processSample<FlapjackClipMode::HardClip>(x);
        if(mode==1)return wdf.processSample<FlapjackClipMode::AlgSigmoid>(x);
        return wdf.processSample<FlapjackClipMode::LightFold>(x);
    }
    void prepare(double rate,const Params& p) {
        fs=2*rate; bridge.prepare(rate,fs); wdf.prepare(fs);last.fill(-1);configure(p);dc.highpass(20,fs);
        for(int i=0;i<20000;++i)offset=circuit(0);
        initial=std::make_tuple(wdf.Vin_C1,wdf.C2,wdf.C6,wdf.C7,wdf.C8_C9);initialWaves=wdf.R.saveWaves();reset();
    }
    void reset() {
        std::tie(wdf.Vin_C1,wdf.C2,wdf.C6,wdf.C7,wdf.C8_C9)=initial;
        wdf.R.restoreWaves(initialWaves);bridge.reset();dc.reset();last.fill(-1);
    }
    double tick(double x) {return level*bridge.tick(x,[this](double v){return dc.tick(circuit(v)-offset);});}
};
} // namespace audition

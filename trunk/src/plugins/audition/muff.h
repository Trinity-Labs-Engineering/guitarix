// Circuit equations adapted from BYOD BigMuffClippingStage/BigMuffTone,
// Copyright Jatin Chowdhury, GPL-3.0. See SOURCES.md.
#pragma once
#include "common.h"
namespace audition {
struct MuffStage {
    Filter<1> input;
    double y=0.7, charge=0;
    void prepare(double fs) { input.analog({100e-9*100e3,0},{100e-9*110e3,1},2*fs);input.reset();y=0.7;charge=0; }
    double tick(double x,double g) {
        double u=input.tick(x);
        constexpr double A=-10000.0/150, vt=0.02585, twoIs=5.04e-9, R=100e3, G=1/470e3;
        for(int i=0;i<16;++i) {
            double v=y-0.7, arg=std::clamp(v/vt,-40.0,40.0);
            double f=u+R*(twoIs*std::sinh(arg)+v*(G+g)-charge)-y/A;
            double df=R*(twoIs/vt*std::cosh(arg)+G+g)-1/A;
            double step=std::clamp(f/df,-0.1,0.1); y-=step;
            if(std::abs(step)<1e-10)break;
        }
        charge=2*(y-0.7)*g-charge;
        return y;
    }
};
class Muff {
    std::array<MuffStage,4> stages, initial;
    std::array<OnePole,4> dc, initialDC;
    std::array<Filter<2>,10> tones;
    OnePole input;
    RateBridge bridge;
    double fs=96000,g=0,sustain=1,level=1,stageMix=1,voiceMix=2;
    int stage=1,voice=2;
    Params last{};
    void toneCoefficients(double tone,double mids) {
        // R8, C8, C9, R5 fixed part, R5 variable part, tone-pot resistance.
        static constexpr double parts[10][6]={
            {22e3,10e-9,4e-9,7700,28600,100e3}, {33e3,10e-9,4e-9,11550,42900,100e3},
            {39e3,10e-9,4e-9,7700,28600,100e3}, {20e3,10e-9,3.9e-9,7700,28600,100e3},
            {39e3,6.8e-9,6.8e-9,2200,25000,100e3}, {39e3,10e-9,12e-9,3300,25000,100e3},
            {470e3,1.5e-9,15e-9,3300,25000,250e3}, {20e3,10e-9,3.9e-9,10000,25000,100e3},
            {39e3,10e-9,10e-9,7700,28600,100e3}, {39e3,5.6e-9,10e-9,7700,28600,100e3}};
        for(int i=0;i<10;++i) {
            const auto& c=parts[i]; double r8=c[0], c8=c[1], c9=c[2], r5=c[3]+mids*c[4], rt=c[5];
            double za=tone*rt,zb=(1-tone)*rt,phi=c8*c9*r5*r8;
            double b1=c9*r5*(r8+rt), fc=std::sqrt(1/(2*pi*r8*c8)*1/(2*pi*r5*c9));
            tones[i].analog({phi*za,b1,r5+zb},{phi*rt,c8*r8*(r5+rt)+b1,r5+r8+rt},2*pi*fc/std::tan(pi*fc/fs));
        }
    }
public:
    void configure(const Params& p) {
        sustain=0.4*std::pow(5.0,p[0]);
        if(p[1]!=last[1]||p[2]!=last[2])toneCoefficients(p[1],p[2]);
        // BYOD's Harmonics frequency control, expressed directly in Hz.
        if(p[5]!=last[5])input.lowpass(p[5],fs);
        g=2*fs*(470e-12+p[6]*200e-12);
        stage=int(p[3])-1;voice=int(p[4]);level=db(p[7])*db(13)*2;last=p;
    }
    double raw(double x) {
        x=input.tick(x)*sustain;
        double taps[4];
        for(int i=0;i<4;++i) {x=stages[i].tick(x,g);taps[i]=dc[i].tick(x)*(i%2==0?-1:1);}
        stageMix+=(stage-stageMix)*(1-std::exp(-1/(0.01*fs)));
        voiceMix+=(voice-voiceMix)*(1-std::exp(-1/(0.01*fs)));
        int i=std::clamp(int(stageMix),0,3);double f=stageMix-i;
        double y=taps[i]+f*(taps[std::min(i+1,3)]-taps[i]);
        double filtered[10];for(int j=0;j<10;++j)filtered[j]=tones[j].tick(y);
        i=std::clamp(int(voiceMix),0,9);f=voiceMix-i;
        return filtered[i]+f*(filtered[std::min(i+1,9)]-filtered[i]);
    }
    void prepare(double rate,const Params& p) {
        fs=rate*2;bridge.prepare(rate,fs);last.fill(-1);configure(p);
        stageMix=stage;voiceMix=voice;
        for(auto& s:stages)s.prepare(fs);
        for(auto& f:dc)f.highpass(16,fs);
        for(int i=0;i<20000;++i)raw(0);
        initial=stages;initialDC=dc;reset();
    }
    void reset() {
        stages=initial;dc=initialDC;input.reset();bridge.reset();for(auto& f:tones)f.reset();
        stageMix=stage;voiceMix=voice;last.fill(-1);
    }
    double tick(double x) {return level*bridge.tick(x,[this](double v){return raw(v);});}
};
} // namespace audition

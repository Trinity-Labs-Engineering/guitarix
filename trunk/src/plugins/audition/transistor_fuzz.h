// New circuit-inspired audition models. Copyright (C) 2026 Trinity Labs Engineering
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "common.h"
namespace audition {
// Small modified-nodal solver with Ebers-Moll transistors and backward-Euler
// capacitor companions. All storage/iteration counts are bounded. No allocations.
template<int N> struct Circuit {
    std::array<double,N> v{}, initial{};
    double jac[N][N]{}, residual[N]{};
    struct Capacitor {int a,b;double c,previous;};
    std::array<Capacitor,12> caps{};
    int count=0;
    double fs=96000, input=0, supply=9;
    double voltage(int n) const {return n>=0?v[n]:n==-2?input:n==-3?supply:0;}
    void capacitor(int a,int b,double c) {caps[count++]={a,b,c,0};}
    void stamp(int row,int col,double x) {if(row>=0&&col>=0)jac[row][col]+=x;}
    void current(int row,double x) {if(row>=0)residual[row]+=x;}
    void conductance(int a,int b,double g,double history=0) {
        double i=g*(voltage(a)-voltage(b))-history;
        current(a,i);current(b,-i);stamp(a,a,g);stamp(a,b,-g);stamp(b,b,g);stamp(b,a,-g);
    }
    void resistor(int a,int b,double r) {conductance(a,b,1/r);}
    static std::pair<double,double> junction(double voltage,double is,double vt) {
        double arg=voltage/vt, e=std::exp(std::min(arg,40.0));
        double i=is*(e*(1+std::max(0.0,arg-40))-1), g=is*e/vt;
        return {i,g};
    }
    void diode(int a,int b,double is,double vt) {
        auto [i,g]=junction(voltage(a)-voltage(b),is,vt);
        current(a,i);current(b,-i);stamp(a,a,g);stamp(a,b,-g);stamp(b,b,g);stamp(b,a,-g);
    }
    void transistor(int b,int c,int e,int sign,double is,double beta) {
        auto [f,gf]=junction(sign*(voltage(b)-voltage(e)),is,0.02585);
        auto [r,gr]=junction(sign*(voltage(b)-voltage(c)),is,0.02585);
        constexpr double reverseBeta=5;
        double ic=f-(1+1/reverseBeta)*r, ib=f/beta+r/reverseBeta;
        current(c,sign*ic);current(b,sign*ib);current(e,-sign*(ic+ib));
        double dcB=gf-(1+1/reverseBeta)*gr,dcC=(1+1/reverseBeta)*gr,dcE=-gf;
        double dbB=gf/beta+gr/reverseBeta,dbC=-gr/reverseBeta,dbE=-gf/beta;
        stamp(c,b,dcB);stamp(c,c,dcC);stamp(c,e,dcE);
        stamp(b,b,dbB);stamp(b,c,dbC);stamp(b,e,dbE);
        stamp(e,b,-dcB-dbB);stamp(e,c,-dcC-dbC);stamp(e,e,-dcE-dbE);
    }
    template<class Stamp> void solve(Stamp build,bool dc=false) {
        for(int iteration=0;iteration<(dc?120:20);++iteration) {
            std::memset(jac,0,sizeof(jac));std::memset(residual,0,sizeof(residual));
            for(int i=0;i<N;++i)conductance(i,-1,1e-10);
            build();
            if(!dc)for(int i=0;i<count;++i) {const auto& c=caps[i];conductance(c.a,c.b,c.c*fs,c.c*fs*c.previous);}
            double error=0;for(auto x:residual)error=std::max(error,std::abs(x));
            if(error<1e-10)break;
            // Partial-pivot Gaussian elimination of J * delta = residual.
            for(int i=0;i<N;++i) {
                int pivot=i;for(int j=i+1;j<N;++j)if(std::abs(jac[j][i])>std::abs(jac[pivot][i]))pivot=j;
                if(pivot!=i) {for(int k=i;k<N;++k)std::swap(jac[i][k],jac[pivot][k]);std::swap(residual[i],residual[pivot]);}
                if(std::abs(jac[i][i])<1e-20)jac[i][i]=1e-20;
                for(int j=i+1;j<N;++j) {
                    double f=jac[j][i]/jac[i][i];
                    for(int k=i+1;k<N;++k)jac[j][k]-=f*jac[i][k];
                    residual[j]-=f*residual[i];
                }
            }
            std::array<double,N> delta{};double largest=0;
            for(int i=N-1;i>=0;--i) {
                double d=residual[i];for(int k=i+1;k<N;++k)d-=jac[i][k]*delta[k];
                delta[i]=d/jac[i][i];largest=std::max(largest,std::abs(delta[i]));
            }
            // Damping prevents exponential overflow on input/parameter discontinuities.
            double scale=largest>0.25?0.25/largest:1;
            for(int i=0;i<N;++i)v[i]-=scale*delta[i];
            if(largest<1e-9)break;
        }
        for(int i=0;i<count;++i)caps[i].previous=voltage(caps[i].a)-voltage(caps[i].b);
    }
    void save() {initial=v;}
    void reset() {v=initial;input=0;for(int i=0;i<count;++i)caps[i].previous=voltage(caps[i].a)-voltage(caps[i].b);}
};

class Percolator {
    Circuit<8> c;
    RateBridge bridge;
    OnePole dc;
    double harmonics=0.5,level=0.5,sourceResistance=1000;
    int clipping=0,version=1;
    void stamp() {
        c.resistor(0,-2,sourceResistance);
        c.resistor(1,2,version?51000:220000);c.resistor(2,-1,version?91000:20000);
        c.resistor(4,5,version?3.9e6:750000);c.resistor(5,-3,91000);
        c.resistor(6,-1,50000);c.resistor(6,7,4700);
        c.transistor(1,2,3,-1,1e-7,40);c.transistor(4,5,3,1,1e-14,200);
        if(clipping==0) {c.diode(-1,6,1e-6,0.04);c.diode(7,-1,1e-6,0.04);}
        else if(clipping==2) {c.diode(6,-1,2.52e-9,0.09);c.diode(-1,6,2.52e-9,0.09);}
    }
public:
    void configure(const Params& p) {
        harmonics=logPot(p[0]);sourceResistance=1000+100000*harmonics*(1-harmonics);
        level=p[1];clipping=int(p[2]);version=int(p[3]);
        c.caps[3].c=version?2.2e-6:1e-6;c.caps[4].c=version?1.5e-9:1e-9;
    }
    void prepare(double rate,const Params& p) {
        c.fs=2*rate;c.supply=8.7;c.count=0;
        c.capacitor(0,-1,100e-12);c.capacitor(0,1,47e-9);c.capacitor(3,-1,47e-6);
        c.capacitor(4,2,2.2e-6);c.capacitor(2,-1,1.5e-9);c.capacitor(5,6,100e-9);
        configure(p);c.v={0,0.2,0.2,0.5,1.1,3,0,0};c.input=0;c.solve([this]{stamp();},true);c.save();
        dc.highpass(20,c.fs);bridge.prepare(rate,c.fs);reset();
    }
    void reset() {c.reset();dc.reset();bridge.reset();}
    double tick(double x) {
        return level*bridge.tick(x,[this](double v){c.input=v*harmonics;c.solve([this]{stamp();});return dc.tick(c.v[6]);});
    }
};

class BiasFuzz {
    Circuit<6> c;
    RateBridge bridge;
    OnePole dc;
    double drive=0.7,gate=0.2,comp=0.3,stability=1,level=0.4;
    void stamp() {
        // Two germanium-like gain stages with collector/base feedback. Gate loads
        // the second base, Compression changes emitter degeneration, and Stability
        // changes supply voltage and AC positive feedback around both stages.
        c.resistor(0,3,100000);c.resistor(1,-3,33000);c.resistor(2,-3,8200);
        c.resistor(1,-1,2000+200000*std::pow(1-gate,3));
        c.resistor(3,-1,500+9500*comp);c.resistor(4,-1,10+1000*(1-drive));
        c.resistor(5,0,22000+2e6*stability*stability);
        c.transistor(0,1,-1,1,1e-8,70);c.transistor(1,2,3,1,1e-8,120);
    }
public:
    void configure(const Params& p) {
        drive=p[0];gate=p[1];comp=p[2];stability=p[3];level=p[4];c.supply=1.5+7.5*stability;
    }
    void prepare(double rate,const Params& p) {
        c.fs=rate*2;c.count=0;
        c.capacitor(-2,0,2.2e-6);c.capacitor(3,4,22e-6);c.capacitor(2,5,10e-9);
        // Small junction capacitances provide finite transistor bandwidth.
        c.capacitor(0,1,100e-12);c.capacitor(1,2,100e-12);
        configure(p);c.v={0.2,0.8,4.5,0.5,0,0.2};c.input=0;c.solve([this]{stamp();},true);c.save();
        bridge.prepare(rate,c.fs);dc.highpass(25,c.fs);reset();
    }
    void reset() {c.reset();dc.reset();dc.z[0]=-dc.b[0]*c.v[2];bridge.reset();}
    double tick(double x) {
        return 0.1*level*bridge.tick(x,[this](double v){
            c.input=v*(0.25+1.75*drive);c.solve([this]{stamp();});return dc.tick(c.v[2]);
        });
    }
};
} // namespace audition

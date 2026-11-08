// Frozen scalar recurrence used to check optimized LSTM activations.
#include "plugins/audition/fuzz_machine.h"
#include <cstdio>
#include <chrono>
namespace audition {
class ScalarFuzzLSTM {
    const FuzzWeights* weights=nullptr;
    Eigen::Matrix<float,24,1> hidden=Eigen::Matrix<float,24,1>::Zero(), cell=hidden;
public:
    void load(const FuzzWeights& w) { weights=&w; hidden.setZero();cell.setZero(); }
    double tick(double x,double fuzz) {
        const auto& w=*weights;
        Eigen::Map<const Eigen::Matrix<float,96,24,Eigen::RowMajor>> recurrent(&w.recurrent[0][0]);
        Eigen::Matrix<float,96,1> gates=recurrent*hidden;
        for(int i=0;i<96;++i)gates[i]+=w.bias[i]+w.input[i][0]*float(x)+w.input[i][1]*float(fuzz);
        auto sigmoid=[](float v){return 1.0f/(1.0f+std::exp(-v));};
        double y=w.outputBias+x; // Model metadata skip=1: residual input is essential.
        for(int i=0;i<24;++i) {
            cell[i]=sigmoid(gates[i+24])*cell[i]+sigmoid(gates[i])*std::tanh(gates[i+48]);
            hidden[i]=sigmoid(gates[i+72])*std::tanh(cell[i]);
            y+=w.output[i]*hidden[i];
        }
        return y;
    }
};
}
int main(int argc, char**) {
 using namespace audition;
 if(argc>1) {
  FuzzLSTM optimized;ScalarFuzzLSTM scalar;
  optimized.load(fuzz_2);scalar.load(fuzz_2);
  double sums[2]={},seconds[2]={};
  for(int version=0;version<2;++version) {
   auto start=std::chrono::steady_clock::now();
   for(int i=0;i<200000;++i) {
    double x=i%128<64?.3:-.3;
    sums[version]+=version?scalar.tick(x,.5):optimized.tick(x,.5);
   }
   seconds[version]=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
  }
  printf("optimized_seconds=%.6f scalar_seconds=%.6f ratio=%.3f checksums=%.9g,%.9g\n",seconds[0],seconds[1],seconds[1]/seconds[0],sums[0],sums[1]);return 0;
 }
 double peak=0,sum=0;size_t samples=0;
 const FuzzWeights* models[]={&fuzz_15,&fuzz_2,&fuzz_15_88,&fuzz_2_88};
 for(auto weights:models) for(double fuzz:{0.0,0.5,1.0}) {
  FuzzLSTM optimized;ScalarFuzzLSTM scalar;optimized.load(*weights);scalar.load(*weights);
  unsigned random=1234;
  for(int i=0;i<64000;++i) {
   random=random*1664525u+1013904223u;
   double noise=double(random>>8)/16777216.0-.5;
   double x=i<8000?0:i<16000?.95*(i%256<128?1:-1):i<32000?.4*std::sin(i*.041)+.3*std::sin(i*.092):noise;
   if(i>=48000) x=(i%997==0?16.0:-4.0*noise);
   double control=i<24000?fuzz:i<48000?1-fuzz:double(i%8000)/7999;
   double a=optimized.tick(x,control),b=scalar.tick(x,control),d=std::abs(a-b);
   if(!std::isfinite(a)||!std::isfinite(b))return 2;
   peak=std::max(peak,d);sum+=(a-b)*(a-b);++samples;
  }
 }
 printf("samples=%zu peak=%.12g rms=%.12g\n",samples,peak,std::sqrt(sum/samples));
 return peak>0.00001 || std::sqrt(sum/samples)>0.000001 ? 1 : 0;
}

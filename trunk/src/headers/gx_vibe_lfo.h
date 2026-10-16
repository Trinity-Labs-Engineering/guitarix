/* Per-instance Faust LFO state for independent Univibe pedals. GPL-2.0-or-later. */
#pragma once

class VibeMonoLfo {
public:

double fConst0;
FAUSTFLOAT fVslider0;
FAUSTFLOAT	*fVslider0_;
int iVec0[2];
double fRec2[2];
double fRec1[2];
double fRec0[2];
int	fSampleRate;

void clear_state_f(PluginDef* = 0)
{
	for (int l0 = 0; l0 < 2; l0 = l0 + 1) iVec0[l0] = 0;
	for (int l1 = 0; l1 < 2; l1 = l1 + 1) fRec2[l1] = 0.0;
	for (int l2 = 0; l2 < 2; l2 = l2 + 1) fRec1[l2] = 0.0;
	for (int l3 = 0; l3 < 2; l3 = l3 + 1) fRec0[l3] = 0.0;
}

void init(unsigned int sample_rate, PluginDef* = 0)
{
	fSampleRate = sample_rate;
	fConst0 = 1.6755160819145563 / std::min<double>(1.92e+05, std::max<double>(1.0, static_cast<double>(fSampleRate)));
	clear_state_f();
}

void compute(int count, FAUSTFLOAT *output0)
{
#define fVslider0 (*fVslider0_)
	double fSlow0 = fConst0 * static_cast<double>(fVslider0);
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		iVec0[0] = 1;
		fRec2[0] = fRec2[1] - fSlow0 * fRec0[1];
		fRec1[0] = fSlow0 * fRec2[0] + static_cast<double>(1 - iVec0[1]) + fRec1[1];
		fRec0[0] = fRec1[0];
		output0[i0] = static_cast<FAUSTFLOAT>(0.5 * (fRec0[0] + 1.0));
		iVec0[1] = iVec0[0];
		fRec2[1] = fRec2[0];
		fRec1[1] = fRec1[0];
		fRec0[1] = fRec0[0];
	}
#undef fVslider0
}

int register_params(const ParamReg& reg)
{
	fVslider0_ = reg.registerFloatVar("univibe_mono.bpm",N_("BPM"),"SA",N_("LFO in Beats per Minute"),&fVslider0, 264.0, 24.0, 3.6e+02, 1.0, 0);
	return 0;
}

};

class VibeStereoLfo {
public:

double fConst0;
FAUSTFLOAT fVslider0;
FAUSTFLOAT	*fVslider0_;
int iVec0[2];
double fRec0[2];
double fRec1[2];
FAUSTFLOAT fVslider1;
FAUSTFLOAT	*fVslider1_;
int	fSampleRate;

void clear_state_f(PluginDef* = 0)
{
	for (int l0 = 0; l0 < 2; l0 = l0 + 1) iVec0[l0] = 0;
	for (int l1 = 0; l1 < 2; l1 = l1 + 1) fRec0[l1] = 0.0;
	for (int l2 = 0; l2 < 2; l2 = l2 + 1) fRec1[l2] = 0.0;
}

void init(unsigned int sample_rate, PluginDef* = 0)
{
	fSampleRate = sample_rate;
	fConst0 = 1.6755160819145563 / std::min<double>(1.92e+05, std::max<double>(1.0, static_cast<double>(fSampleRate)));
	clear_state_f();
}

void compute(int count, FAUSTFLOAT *output0, FAUSTFLOAT *output1)
{
#define fVslider0 (*fVslider0_)
#define fVslider1 (*fVslider1_)
	double fSlow0 = fConst0 * static_cast<double>(fVslider0);
	double fSlow1 = std::sin(fSlow0);
	double fSlow2 = std::cos(fSlow0);
	double fSlow3 = 6.283185307179586 * static_cast<double>(fVslider1);
	double fSlow4 = std::cos(fSlow3);
	double fSlow5 = std::sin(fSlow3);
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		iVec0[0] = 1;
		fRec0[0] = fSlow1 * fRec1[1] + fSlow2 * fRec0[1];
		fRec1[0] = static_cast<double>(1 - iVec0[1]) + fSlow2 * fRec1[1] - fSlow1 * fRec0[1];
		output0[i0] = static_cast<FAUSTFLOAT>(0.5 * (fRec0[0] + 1.0));
		output1[i0] = static_cast<FAUSTFLOAT>(0.5 * (fSlow4 * fRec0[0] + fSlow5 * fRec1[0] + 1.0));
		iVec0[1] = iVec0[0];
		fRec0[1] = fRec0[0];
		fRec1[1] = fRec1[0];
	}
#undef fVslider0
#undef fVslider1
}

int register_params(const ParamReg& reg)
{
	fVslider0_ = reg.registerFloatVar("univibe.bpm",N_("BPM"),"SA",N_("LFO in Beats per Minute"),&fVslider0, 264.0, 24.0, 3.6e+02, 1.0, 0);
	fVslider1_ = reg.registerFloatVar("univibe.stereo",N_("Phase"),"SA",N_("LFO phase shift between left and right channels"),&fVslider1, 0.11, -0.5, 0.5, 0.01, 0);
	return 0;
}

};

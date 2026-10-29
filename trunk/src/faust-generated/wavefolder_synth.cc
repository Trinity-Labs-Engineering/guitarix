// generated from file '../src/faust/wavefolder_synth.dsp' by dsp2cc:
// Code generated with Faust (https://faust.grame.fr)


namespace wavefolder_synth {

class Dsp: public PluginDef {
private:
	int fSampleRate;
	double fConst0;
	double fConst1;
	FAUSTFLOAT fHslider0;
	double fConst2;
	double fConst3;
	double fRec0[2];
	double fVec0[2];
	double fConst4;
	double fConst5;
	FAUSTFLOAT fHslider1;
	double fRec5[2];
	FAUSTFLOAT fHslider2;
	double fRec6[2];
	double fConst6;
	double fConst7;
	double fRec7[2];
	FAUSTFLOAT fHslider3;
	double fRec8[2];
	double fVec1[2];
	double fConst8;
	FAUSTFLOAT fHslider4;
	double fRec9[2];
	double fRec2[2];
	double fRec3[2];
	double fRec4[2];
	double fRec1[2];
	FAUSTFLOAT fHslider5;
	double fRec10[2];

	void clear_state_f();
	void init(unsigned int sample_rate);
	void compute(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0);
	int register_par(const ParamReg& reg);

	static void clear_state_f_static(PluginDef*);
	static void init_static(unsigned int sample_rate, PluginDef*);
	static void compute_static(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0, PluginDef*);
	static int register_params_static(const ParamReg& reg);
	static void del_instance(PluginDef *p);
public:
	Dsp();
	~Dsp();
};



Dsp::Dsp()
	: PluginDef() {
	version = PLUGINDEF_VERSION;
	flags = 0;
	id = "wavefolder_synth";
	name = N_("Wavefolder Synth");
	groups = 0;
	description = ""; // description (tooltip)
	category = N_("Synth");       // category
	shortname = "";     // shortname
	mono_audio = compute_static;
	stereo_audio = 0;
	set_samplerate = init_static;
	activate_plugin = 0;
	register_params = register_params_static;
	load_ui = 0;
	clear_state = clear_state_f_static;
	delete_instance = del_instance;
}

Dsp::~Dsp() {
}

inline void Dsp::clear_state_f()
{
	for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec0[l0] = 0.0;
	for (int l1 = 0; l1 < 2; l1 = l1 + 1) fVec0[l1] = 0.0;
	for (int l2 = 0; l2 < 2; l2 = l2 + 1) fRec5[l2] = 0.0;
	for (int l3 = 0; l3 < 2; l3 = l3 + 1) fRec6[l3] = 0.0;
	for (int l4 = 0; l4 < 2; l4 = l4 + 1) fRec7[l4] = 0.0;
	for (int l5 = 0; l5 < 2; l5 = l5 + 1) fRec8[l5] = 0.0;
	for (int l6 = 0; l6 < 2; l6 = l6 + 1) fVec1[l6] = 0.0;
	for (int l7 = 0; l7 < 2; l7 = l7 + 1) fRec9[l7] = 0.0;
	for (int l8 = 0; l8 < 2; l8 = l8 + 1) fRec2[l8] = 0.0;
	for (int l9 = 0; l9 < 2; l9 = l9 + 1) fRec3[l9] = 0.0;
	for (int l10 = 0; l10 < 2; l10 = l10 + 1) fRec4[l10] = 0.0;
	for (int l11 = 0; l11 < 2; l11 = l11 + 1) fRec1[l11] = 0.0;
	for (int l12 = 0; l12 < 2; l12 = l12 + 1) fRec10[l12] = 0.0;
}

void Dsp::clear_state_f_static(PluginDef *p)
{
	static_cast<Dsp*>(p)->clear_state_f();
}

inline void Dsp::init(unsigned int sample_rate)
{
	fSampleRate = sample_rate;
	fConst0 = std::min<double>(1.92e+05, std::max<double>(1.0, static_cast<double>(fSampleRate)));
	fConst1 = 0.441 / fConst0;
	fConst2 = 44.1 / fConst0;
	fConst3 = 1.0 - fConst2;
	fConst4 = 3.141592653589793 / fConst0;
	fConst5 = 0.4 * fConst0;
	fConst6 = std::exp(-(6.666666666666667 / fConst0));
	fConst7 = std::exp(-(2e+02 / fConst0));
	fConst8 = log1p(0.0);
	clear_state_f();
}

void Dsp::init_static(unsigned int sample_rate, PluginDef *p)
{
	static_cast<Dsp*>(p)->init(sample_rate);
}

void always_inline Dsp::compute(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0)
{
	double fSlow0 = fConst1 * static_cast<double>(fHslider0);
	double fSlow1 = fConst2 * static_cast<double>(fHslider1);
	double fSlow2 = fConst2 * static_cast<double>(fHslider2);
	double fSlow3 = fConst2 * std::pow(1e+01, 0.05 * static_cast<double>(fHslider3));
	double fSlow4 = fConst2 * static_cast<double>(fHslider4);
	double fSlow5 = fConst2 * std::pow(1e+01, 0.05 * static_cast<double>(fHslider5));
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		fRec0[0] = fSlow0 + fConst3 * fRec0[1];
		double fTemp0 = static_cast<double>(input0[i0]);
		fVec0[0] = fTemp0;
		fRec5[0] = fSlow1 + fConst3 * fRec5[1];
		fRec6[0] = fSlow2 + fConst3 * fRec6[1];
		double fTemp1 = std::fabs(fTemp0);
		double fTemp2 = ((fTemp1 > fRec7[1]) ? fConst7 : fConst6);
		fRec7[0] = fTemp1 * (1.0 - fTemp2) + fRec7[1] * fTemp2;
		double fTemp3 = std::tan(fConst4 * std::max<double>(4e+01, std::min<double>(fConst5, fRec5[0] * std::pow(2.0, fRec6[0] * std::min<double>(1.0, 4.0 * fRec7[0])))));
		fRec8[0] = fSlow3 + fConst3 * fRec8[1];
		double fTemp4 = fTemp0 * fRec8[0];
		fVec1[0] = fTemp4;
		double fTemp5 = fTemp4 - fVec1[1];
		double fTemp6 = std::fabs(fTemp4);
		double fTemp7 = std::max<double>(0.0, fTemp6 + -0.5);
		double fTemp8 = std::floor(fTemp7);
		double fTemp9 = std::fabs(fVec1[1]);
		double fTemp10 = std::max<double>(0.0, fTemp9 + -0.5);
		double fTemp11 = std::floor(fTemp10);
		double fTemp12 = 0.5 * (fTemp4 + fVec1[1]);
		double fTemp13 = std::fabs(fTemp12);
		double fTemp14 = std::max<double>(0.0, fTemp13 + -0.5);
		double fTemp15 = std::floor(fTemp14);
		double fTemp16 = ((std::fabs(fTemp5) <= 0.001) ? ((fTemp13 <= 0.5) ? fTemp12 : 0.5 * static_cast<double>((fTemp12 > 0.0) - (fTemp12 < 0.0)) * (1.0 - 2.0 * std::fmod(fTemp15, 2.0)) * (1.0 - 2.0 * (fTemp14 - fTemp15))) : (((fTemp6 <= 0.5) ? 0.5 * mydsp_faustpower2_f(fTemp0) * mydsp_faustpower2_f(fRec8[0]) : 0.5 * (1.0 - 2.0 * std::fmod(fTemp8, 2.0)) * std::exp(-(fConst8 * (fTemp8 + 1.0))) * (fTemp7 - fTemp8) * (fTemp8 + (1.0 - fTemp7)) + 0.125) - ((fTemp9 <= 0.5) ? 0.5 * mydsp_faustpower2_f(fVec1[1]) : 0.5 * (1.0 - 2.0 * std::fmod(fTemp11, 2.0)) * std::exp(-(fConst8 * (fTemp11 + 1.0))) * (fTemp10 - fTemp11) * (fTemp11 + (1.0 - fTemp10)) + 0.125)) / fTemp5);
		fRec9[0] = fSlow4 + fConst3 * fRec9[1];
		double fTemp17 = std::max<double>(0.5, fRec9[0]);
		double fTemp18 = 1.0 / fTemp17 + fTemp3;
		double fTemp19 = fTemp16 - (fRec2[1] * fTemp18 + fRec3[1]);
		double fTemp20 = fTemp3 * fTemp18 + 1.0;
		double fTemp21 = fTemp3 * fTemp19 / fTemp20;
		fRec2[0] = fRec2[1] + 2.0 * fTemp21;
		double fTemp22 = fRec2[1] + fTemp21;
		double fTemp23 = fTemp3 * fTemp22;
		fRec3[0] = fRec3[1] + 2.0 * fTemp23;
		double fTemp24 = fRec3[1] + fTemp23;
		fRec4[0] = fTemp24;
		fRec1[0] = fRec4[0] + 0.995 * fRec1[1] - fRec4[1];
		fRec10[0] = fSlow5 + fConst3 * fRec10[1];
		output0[i0] = static_cast<FAUSTFLOAT>(0.5 * (1.0 - fRec0[0]) * (fTemp0 + fVec0[1]) + fRec0[0] * fRec1[0] * fRec10[0]);
		fRec0[1] = fRec0[0];
		fVec0[1] = fVec0[0];
		fRec5[1] = fRec5[0];
		fRec6[1] = fRec6[0];
		fRec7[1] = fRec7[0];
		fRec8[1] = fRec8[0];
		fVec1[1] = fVec1[0];
		fRec9[1] = fRec9[0];
		fRec2[1] = fRec2[0];
		fRec3[1] = fRec3[0];
		fRec4[1] = fRec4[0];
		fRec1[1] = fRec1[0];
		fRec10[1] = fRec10[0];
	}
}

void __rt_func Dsp::compute_static(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0, PluginDef *p)
{
	static_cast<Dsp*>(p)->compute(count, input0, output0);
}

int Dsp::register_par(const ParamReg& reg)
{
	reg.registerFloatVar("wavefolder_synth.envelope",N_("Envelope"),"S","",&fHslider2, 1.5, -3.0, 3.0, 0.05, 0);
	reg.registerFloatVar("wavefolder_synth.fold",N_("Fold"),"S","",&fHslider3, 18.0, 0.0, 36.0, 0.1, 0);
	reg.registerFloatVar("wavefolder_synth.level",N_("Level"),"S","",&fHslider5, -6.0, -24.0, 6.0, 0.1, 0);
	reg.registerFloatVar("wavefolder_synth.mix",N_("Mix"),"S","",&fHslider0, 1e+02, 0.0, 1e+02, 1.0, 0);
	reg.registerFloatVar("wavefolder_synth.resonance",N_("Resonance"),"S","",&fHslider4, 1.0, 0.5, 4.0, 0.05, 0);
	reg.registerFloatVar("wavefolder_synth.tone",N_("Cutoff"),"S","",&fHslider1, 1.2e+03, 1e+02, 8e+03, 1e+01, 0);
	return 0;
}

int Dsp::register_params_static(const ParamReg& reg)
{
	return static_cast<Dsp*>(reg.plugin)->register_par(reg);
}

PluginDef *plugin() {
	return new Dsp();
}

void Dsp::del_instance(PluginDef *p)
{
	delete static_cast<Dsp*>(p);
}

} // end namespace wavefolder_synth

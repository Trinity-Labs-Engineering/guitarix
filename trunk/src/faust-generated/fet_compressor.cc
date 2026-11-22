// generated from file '../src/faust/fet_compressor.dsp' by dsp2cc:
// Code generated with Faust (https://faust.grame.fr)


namespace fet_compressor {

class Dsp: public PluginDef {
private:
	int fSampleRate;
	double fVec0[2];
	int iVec1[2];
	FAUSTFLOAT fHslider0;
	double fConst0;
	double fConst1;
	double fRec0[2];
	FAUSTFLOAT fHslider1;
	double fRec1[2];
	FAUSTFLOAT fHslider2;
	double fRec3[2];
	FAUSTFLOAT fHslider3;
	double fRec5[2];
	double fConst2;
	FAUSTFLOAT fHslider4;
	double fRec7[2];
	FAUSTFLOAT fHslider5;
	double fRec8[2];
	double fRec6[2];
	double fRec4[2];
	double fRec2[2];
	double fVec2[2];
	FAUSTFLOAT fHslider6;
	double fRec9[2];

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
	id = "fet_compressor";
	name = N_("FET 76");
	groups = 0;
	description = N_("76-inspired feedback compression and adjustable harmonic colour; behavioural model."); // description (tooltip)
	category = N_("Guitar Effects");       // category
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
	for (int l0 = 0; l0 < 2; l0 = l0 + 1) fVec0[l0] = 0.0;
	for (int l1 = 0; l1 < 2; l1 = l1 + 1) iVec1[l1] = 0;
	for (int l2 = 0; l2 < 2; l2 = l2 + 1) fRec0[l2] = 0.0;
	for (int l3 = 0; l3 < 2; l3 = l3 + 1) fRec1[l3] = 0.0;
	for (int l4 = 0; l4 < 2; l4 = l4 + 1) fRec3[l4] = 0.0;
	for (int l5 = 0; l5 < 2; l5 = l5 + 1) fRec5[l5] = 0.0;
	for (int l6 = 0; l6 < 2; l6 = l6 + 1) fRec7[l6] = 0.0;
	for (int l7 = 0; l7 < 2; l7 = l7 + 1) fRec8[l7] = 0.0;
	for (int l8 = 0; l8 < 2; l8 = l8 + 1) fRec6[l8] = 0.0;
	for (int l9 = 0; l9 < 2; l9 = l9 + 1) fRec4[l9] = 0.0;
	for (int l10 = 0; l10 < 2; l10 = l10 + 1) fRec2[l10] = 0.0;
	for (int l11 = 0; l11 < 2; l11 = l11 + 1) fVec2[l11] = 0.0;
	for (int l12 = 0; l12 < 2; l12 = l12 + 1) fRec9[l12] = 0.0;
}

void Dsp::clear_state_f_static(PluginDef *p)
{
	static_cast<Dsp*>(p)->clear_state_f();
}

inline void Dsp::init(unsigned int sample_rate)
{
	fSampleRate = sample_rate;
	fConst0 = std::min<double>(1.92e+05, std::max<double>(1.0, static_cast<double>(fSampleRate)));
	fConst1 = std::exp(-(1e+02 / fConst0));
	fConst2 = 1.0 / fConst0;
	clear_state_f();
}

void Dsp::init_static(unsigned int sample_rate, PluginDef *p)
{
	static_cast<Dsp*>(p)->init(sample_rate);
}

void always_inline Dsp::compute(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0)
{
	double fSlow0 = static_cast<double>(fHslider0);
	double fSlow1 = static_cast<double>(fHslider1);
	double fSlow2 = static_cast<double>(fHslider2);
	double fSlow3 = static_cast<double>(fHslider3);
	double fSlow4 = static_cast<double>(fHslider4);
	int iSlow5 = std::min<int>(3, std::max<int>(0, static_cast<int>(static_cast<double>(fHslider5))));
	double fSlow6 = static_cast<double>(((iSlow5 >= 2) ? ((iSlow5 >= 3) ? 20 : 12) : ((iSlow5 >= 1) ? 8 : 4)));
	double fSlow7 = static_cast<double>(fHslider6);
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		double fTemp0 = static_cast<double>(input0[i0]);
		fVec0[0] = fTemp0;
		iVec1[0] = 1;
		double fTemp1 = 0.5 * (fTemp0 + fVec0[1]);
		double fTemp2 = static_cast<double>(iVec1[1]);
		double fTemp3 = 1.0 - fConst1 * fTemp2;
		fRec0[0] = fSlow0 * fTemp3 + fConst1 * fTemp2 * fRec0[1];
		fRec1[0] = fSlow1 * fTemp3 + fConst1 * fTemp2 * fRec1[1];
		fRec3[0] = fSlow2 * fTemp3 + fConst1 * fTemp2 * fRec3[1];
		fRec5[0] = fSlow3 * fTemp3 + fConst1 * fTemp2 * fRec5[1];
		double fTemp4 = 5e-07 * fRec5[0];
		int iTemp5 = std::fabs(fTemp4) < 2.220446049250313e-16;
		double fTemp6 = ((iTemp5) ? 0.0 : std::exp(-(fConst2 / ((iTemp5) ? 1.0 : fTemp4))));
		double fTemp7 = std::fabs(fRec2[1]);
		fRec7[0] = fSlow4 * fTemp3 + fConst1 * fTemp2 * fRec7[1];
		fRec8[0] = fSlow6 * fTemp3 + fConst1 * fTemp2 * fRec8[1];
		double fTemp8 = std::max<double>(fConst2, 0.001 * fRec7[0] * fRec8[0]);
		int iTemp9 = std::fabs(fTemp8) < 2.220446049250313e-16;
		double fTemp10 = std::max<double>(fConst2, 1e-06 * fRec5[0] * fRec8[0]);
		int iTemp11 = std::fabs(fTemp10) < 2.220446049250313e-16;
		double fTemp12 = ((fTemp7 > fRec6[1]) ? ((iTemp11) ? 0.0 : std::exp(-(fConst2 / ((iTemp11) ? 1.0 : fTemp10)))) : ((iTemp9) ? 0.0 : std::exp(-(fConst2 / ((iTemp9) ? 1.0 : fTemp8)))));
		fRec6[0] = fTemp7 * (1.0 - fTemp12) + fRec6[1] * fTemp12;
		double fTemp13 = 2e+01 * std::log10(std::max<double>(2.2250738585072014e-308, std::max<double>(1e-12, std::fabs(fRec6[0]))));
		double fTemp14 = 1.5 / fRec8[0];
		int iTemp15 = (fTemp13 > (-18.0 - fTemp14)) + (fTemp13 > (fTemp14 + -18.0));
		fRec4[0] = fTemp6 * fRec4[1] - (1.0 - fTemp6) * std::max<double>(0.0, ((iTemp15 == 0) ? 0.0 : ((iTemp15 == 1) ? 0.5 * (mydsp_faustpower2_f(fTemp13 + fTemp14 + 18.0) / std::max<double>(1e-06, 3.0 / fRec8[0])) : fTemp13 + 18.0))) * (fRec8[0] + -1.0);
		fRec2[0] = fTemp0 * std::pow(1e+01, 0.05 * fRec3[0]) * std::pow(1e+01, 0.05 * fRec4[0]);
		double fTemp16 = 0.03 * fRec1[0] + 1.0;
		double fTemp17 = fRec2[0] * fTemp16;
		fVec2[0] = fTemp17;
		fRec9[0] = fSlow7 * fTemp3 + fConst1 * fTemp2 * fRec9[1];
		output0[i0] = static_cast<FAUSTFLOAT>(fTemp1 + 0.01 * fRec0[0] * ((0.5 * (1.0 - 0.01 * fRec1[0]) * (fRec2[0] + fRec2[1]) + 0.01 * (fRec1[0] * (fTemp17 + fVec2[1]) / (fTemp16 * (std::sqrt(mydsp_faustpower2_f(fRec2[0]) * mydsp_faustpower2_f(fTemp16) + 1.0) + std::sqrt(mydsp_faustpower2_f(fVec2[1]) + 1.0))))) * std::pow(1e+01, 0.05 * fRec9[0]) - fTemp1));
		fVec0[1] = fVec0[0];
		iVec1[1] = iVec1[0];
		fRec0[1] = fRec0[0];
		fRec1[1] = fRec1[0];
		fRec3[1] = fRec3[0];
		fRec5[1] = fRec5[0];
		fRec7[1] = fRec7[0];
		fRec8[1] = fRec8[0];
		fRec6[1] = fRec6[0];
		fRec4[1] = fRec4[0];
		fRec2[1] = fRec2[0];
		fVec2[1] = fVec2[0];
		fRec9[1] = fRec9[0];
	}
}

void __rt_func Dsp::compute_static(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0, PluginDef *p)
{
	static_cast<Dsp*>(p)->compute(count, input0, output0);
}

int Dsp::register_par(const ParamReg& reg)
{
	reg.registerFloatVar("fet_compressor.attack",N_("Attack"),"S","",&fHslider3, 4e+02, 2e+01, 8e+02, 1.0, 0);
	reg.registerFloatVar("fet_compressor.colour",N_("Colour"),"S","",&fHslider1, 35.0, 0.0, 1e+02, 1.0, 0);
	reg.registerFloatVar("fet_compressor.input",N_("Input"),"S","",&fHslider2, 0.0, -12.0, 24.0, 0.1, 0);
	reg.registerFloatVar("fet_compressor.mix",N_("Mix"),"S","",&fHslider0, 1e+02, 0.0, 1e+02, 1.0, 0);
	reg.registerFloatVar("fet_compressor.output",N_("Output"),"S","",&fHslider6, 0.0, -24.0, 12.0, 0.1, 0);
	reg.registerFloatVar("fet_compressor.ratio",N_("Ratio"),"S","",&fHslider5, 0.0, 0.0, 3.0, 1.0, 0);
	reg.registerFloatVar("fet_compressor.release",N_("Release"),"S","",&fHslider4, 2e+02, 5e+01, 1.1e+03, 1.0, 0);
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

} // end namespace fet_compressor

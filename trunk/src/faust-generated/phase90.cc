// generated from file '../src/faust/phase90.dsp' by dsp2cc:
// Code generated with Faust (https://faust.grame.fr)


namespace phase90 {

class Dsp: public PluginDef {
private:
	int fSampleRate;
	double fConst0;
	double fConst1;
	double fConst2;
	double fConst3;
	double fConst4;
	double fConst5;
	double fConst6;
	double fConst7;
	double fConst8;
	double fVec0[2];
	int iVec1[2];
	double fConst9;
	double fRec1[2];
	double fConst10;
	double fConst11;
	double fConst12;
	FAUSTFLOAT fHslider0;
	double fConst13;
	double fRec15[2];
	double fRec14[2];
	double fRec11[2];
	double fRec12[2];
	double fRec8[2];
	double fRec9[2];
	double fRec5[2];
	double fRec6[2];
	double fRec2[2];
	double fRec3[2];
	double fVec2[2];
	double fConst14;
	double fRec0[2];
	FAUSTFLOAT fHslider1;
	double fRec16[2];

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
	id = "phase90";
	name = N_("Phase 90");
	groups = 0;
	description = ""; // description (tooltip)
	category = N_("Modulation");       // category
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
	for (int l2 = 0; l2 < 2; l2 = l2 + 1) fRec1[l2] = 0.0;
	for (int l3 = 0; l3 < 2; l3 = l3 + 1) fRec15[l3] = 0.0;
	for (int l4 = 0; l4 < 2; l4 = l4 + 1) fRec14[l4] = 0.0;
	for (int l5 = 0; l5 < 2; l5 = l5 + 1) fRec11[l5] = 0.0;
	for (int l6 = 0; l6 < 2; l6 = l6 + 1) fRec12[l6] = 0.0;
	for (int l7 = 0; l7 < 2; l7 = l7 + 1) fRec8[l7] = 0.0;
	for (int l8 = 0; l8 < 2; l8 = l8 + 1) fRec9[l8] = 0.0;
	for (int l9 = 0; l9 < 2; l9 = l9 + 1) fRec5[l9] = 0.0;
	for (int l10 = 0; l10 < 2; l10 = l10 + 1) fRec6[l10] = 0.0;
	for (int l11 = 0; l11 < 2; l11 = l11 + 1) fRec2[l11] = 0.0;
	for (int l12 = 0; l12 < 2; l12 = l12 + 1) fRec3[l12] = 0.0;
	for (int l13 = 0; l13 < 2; l13 = l13 + 1) fVec2[l13] = 0.0;
	for (int l14 = 0; l14 < 2; l14 = l14 + 1) fRec0[l14] = 0.0;
	for (int l15 = 0; l15 < 2; l15 = l15 + 1) fRec16[l15] = 0.0;
}

void Dsp::clear_state_f_static(PluginDef *p)
{
	static_cast<Dsp*>(p)->clear_state_f();
}

inline void Dsp::init(unsigned int sample_rate)
{
	fSampleRate = sample_rate;
	fConst0 = std::min<double>(1.92e+05, std::max<double>(1.0, static_cast<double>(fSampleRate)));
	fConst1 = std::tan(70.92198581560284 / fConst0);
	fConst2 = 1.0 / fConst1;
	fConst3 = 1.0 / (fConst2 + 1.0);
	fConst4 = 0.5 / fConst1;
	fConst5 = std::tan(104.16666666666666 / fConst0);
	fConst6 = 1.0 / fConst5;
	fConst7 = 1.0 / (fConst6 + 1.0);
	fConst8 = 0.9791666666666666 / fConst5;
	fConst9 = 1.0 - fConst6;
	fConst10 = 10638297.872340426 / fConst0;
	fConst11 = 1.0 / fConst0;
	fConst12 = 44.1 / fConst0;
	fConst13 = 1.0 - fConst12;
	fConst14 = 1.0 - fConst2;
	clear_state_f();
}

void Dsp::init_static(unsigned int sample_rate, PluginDef *p)
{
	static_cast<Dsp*>(p)->init(sample_rate);
}

void always_inline Dsp::compute(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0)
{
	double fSlow0 = fConst12 * static_cast<double>(fHslider0);
	double fSlow1 = fConst12 * std::pow(1e+01, 0.05 * static_cast<double>(fHslider1));
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		double fTemp0 = static_cast<double>(input0[i0]);
		fVec0[0] = fTemp0;
		iVec1[0] = 1;
		fRec1[0] = fConst7 * (fConst8 * (fTemp0 - fVec0[1]) - fConst9 * fRec1[1]);
		fRec15[0] = fSlow0 + fConst13 * fRec15[1];
		double fTemp1 = ((1 - iVec1[1]) ? 0.0 : fRec14[1] + fConst11 * fRec15[0]);
		fRec14[0] = fTemp1 - std::floor(fTemp1);
		double fTemp2 = 0.15 * ((fRec14[0] < 0.65) ? 3.0769230769230766 * fRec14[0] + -1.0 : 1.0 - 5.714285714285714 * (fRec14[0] + -0.65));
		double fTemp3 = fTemp2 + 0.17100000000000026;
		int iTemp4 = fTemp3 < 0.0;
		double fTemp5 = mydsp_faustpower2_f(0.4948045522018803 * (fTemp2 + -1.8499999999999996) + 1.0);
		double fTemp6 = std::tan(std::min<double>(fConst10 * (((iTemp4) ? 0.0 : ((fRec12[1] <= fTemp3) ? 0.0026280218027387348 * (fTemp2 + (0.17100000000000026 - 0.5 * fRec12[1])) : 0.005367 * (fTemp5 * (0.004 * fRec12[1] + 1.0) / std::max<double>(fRec12[1], 1e-09)))) + 4.1666666666666665e-05), 0.7853981633974483));
		double fTemp7 = (fRec1[0] * fTemp6 + fRec11[1]) / (fTemp6 + 1.0);
		double fTemp8 = 2.0 * fTemp7;
		fRec11[0] = fTemp8 - fRec11[1];
		fRec12[0] = fRec1[0] - fTemp7;
		double fRec13 = fRec1[0] - fTemp8;
		double fTemp9 = std::tan(std::min<double>(fConst10 * (((iTemp4) ? 0.0 : ((fRec9[1] <= fTemp3) ? 0.0026280218027387348 * (fTemp2 + (0.17100000000000026 - 0.5 * fRec9[1])) : 0.005367 * (fTemp5 * (0.004 * fRec9[1] + 1.0) / std::max<double>(fRec9[1], 1e-09)))) + 4.1666666666666665e-05), 0.7853981633974483));
		double fTemp10 = (fRec13 * fTemp9 + fRec8[1]) / (fTemp9 + 1.0);
		double fTemp11 = 2.0 * fTemp10;
		fRec8[0] = fTemp11 - fRec8[1];
		fRec9[0] = fRec13 - fTemp10;
		double fRec10 = fRec13 - fTemp11;
		double fTemp12 = std::tan(std::min<double>(fConst10 * (((iTemp4) ? 0.0 : ((fRec6[1] <= fTemp3) ? 0.0026280218027387348 * (fTemp2 + (0.17100000000000026 - 0.5 * fRec6[1])) : 0.005367 * (fTemp5 * (0.004 * fRec6[1] + 1.0) / std::max<double>(fRec6[1], 1e-09)))) + 4.1666666666666665e-05), 0.7853981633974483));
		double fTemp13 = (fRec10 * fTemp12 + fRec5[1]) / (fTemp12 + 1.0);
		double fTemp14 = 2.0 * fTemp13;
		fRec5[0] = fTemp14 - fRec5[1];
		fRec6[0] = fRec10 - fTemp13;
		double fRec7 = fRec10 - fTemp14;
		double fTemp15 = std::tan(std::min<double>(fConst10 * (((iTemp4) ? 0.0 : ((fRec3[1] <= fTemp3) ? 0.0026280218027387348 * (fTemp2 + (0.17100000000000026 - 0.5 * fRec3[1])) : 0.005367 * (fTemp5 * (0.004 * fRec3[1] + 1.0) / std::max<double>(fRec3[1], 1e-09)))) + 4.1666666666666665e-05), 0.7853981633974483));
		double fTemp16 = (fRec7 * fTemp15 + fRec2[1]) / (fTemp15 + 1.0);
		double fTemp17 = 2.0 * fTemp16;
		fRec2[0] = fTemp17 - fRec2[1];
		fRec3[0] = fRec7 - fTemp16;
		double fRec4 = fRec7 - fTemp17;
		double fTemp18 = fRec1[0] + fRec4;
		fVec2[0] = fTemp18;
		fRec0[0] = fConst3 * (fConst4 * (fTemp18 - fVec2[1]) - fConst14 * fRec0[1]);
		fRec16[0] = fSlow1 + fConst13 * fRec16[1];
		output0[i0] = static_cast<FAUSTFLOAT>(1.788 * fRec0[0] * fRec16[0]);
		fVec0[1] = fVec0[0];
		iVec1[1] = iVec1[0];
		fRec1[1] = fRec1[0];
		fRec15[1] = fRec15[0];
		fRec14[1] = fRec14[0];
		fRec11[1] = fRec11[0];
		fRec12[1] = fRec12[0];
		fRec8[1] = fRec8[0];
		fRec9[1] = fRec9[0];
		fRec5[1] = fRec5[0];
		fRec6[1] = fRec6[0];
		fRec2[1] = fRec2[0];
		fRec3[1] = fRec3[0];
		fVec2[1] = fVec2[0];
		fRec0[1] = fRec0[0];
		fRec16[1] = fRec16[0];
	}
}

void __rt_func Dsp::compute_static(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0, PluginDef *p)
{
	static_cast<Dsp*>(p)->compute(count, input0, output0);
}

int Dsp::register_par(const ParamReg& reg)
{
	reg.registerFloatVar("phase90.level",N_("Level"),"S","",&fHslider1, -5.0, -2e+01, 6.0, 0.1, 0);
	reg.registerFloatVar("phase90.rate",N_("Rate"),"S","",&fHslider0, 0.5, 0.1, 1e+01, 0.01, 0);
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

} // end namespace phase90

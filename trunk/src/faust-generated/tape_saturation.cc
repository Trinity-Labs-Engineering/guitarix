// generated from file '../src/faust/tape_saturation.dsp' by dsp2cc:
// Code generated with Faust (https://faust.grame.fr)


namespace tape_saturation {
class mydspSIG0 {
	
  private:
	
	int iVec2[2];
	int iRec11[2];
	
  public:
	
	int getNumInputsmydspSIG0() {
		return 0;
	}
	int getNumOutputsmydspSIG0() {
		return 1;
	}
	
	void instanceInitmydspSIG0(int sample_rate) {
		for (int l12 = 0; l12 < 2; l12 = l12 + 1) {
			iVec2[l12] = 0;
		}
		for (int l13 = 0; l13 < 2; l13 = l13 + 1) {
			iRec11[l13] = 0;
		}
	}
	
	void fillmydspSIG0(int count, double* table) {
		for (int i1 = 0; i1 < count; i1 = i1 + 1) {
			iVec2[0] = 1;
			iRec11[0] = (iVec2[1] + iRec11[1]) % 65536;
			table[i1] = std::sin(9.587379924285257e-05 * static_cast<double>(iRec11[0]));
			iVec2[1] = iVec2[0];
			iRec11[1] = iRec11[0];
		}
	}

};

static mydspSIG0* newmydspSIG0() { return (mydspSIG0*)new mydspSIG0(); }
static void deletemydspSIG0(mydspSIG0* dsp) { delete dsp; }

static double ftbl0mydspSIG0[65536];


class Dsp: public PluginDef {
private:
	int fSampleRate;
	double fConst0;
	double fConst1;
	FAUSTFLOAT fHslider0;
	double fConst2;
	int iVec0[2];
	double fRec0[2];
	double fConst3;
	double fConst4;
	FAUSTFLOAT fHslider1;
	double fRec4[2];
	double fConst5;
	double fConst6;
	double fConst7;
	double fConst8;
	double fConst9;
	FAUSTFLOAT fHslider2;
	double fRec9[2];
	double fVec1[3];
	double fRec8[2];
	double fRec5[2];
	double fConst10;
	double fRec6[2];
	double fConst11;
	double fRec1[2];
	double fRec2[2];
	int IOTA0;
	double fRec3[2048];
	double fConst12;
	FAUSTFLOAT fHslider3;
	double fRec10[2];
	double fConst13;
	double fRec12[2];
	FAUSTFLOAT fHslider4;
	double fRec13[2];
	double fConst14;
	double fRec14[2];

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
	id = "tape_saturation";
	name = N_("Tape Colour");
	groups = 0;
	description = ""; // description (tooltip)
	category = N_("Distortion");       // category
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
	for (int l0 = 0; l0 < 2; l0 = l0 + 1) iVec0[l0] = 0;
	for (int l1 = 0; l1 < 2; l1 = l1 + 1) fRec0[l1] = 0.0;
	for (int l2 = 0; l2 < 2; l2 = l2 + 1) fRec4[l2] = 0.0;
	for (int l3 = 0; l3 < 2; l3 = l3 + 1) fRec9[l3] = 0.0;
	for (int l4 = 0; l4 < 3; l4 = l4 + 1) fVec1[l4] = 0.0;
	for (int l5 = 0; l5 < 2; l5 = l5 + 1) fRec8[l5] = 0.0;
	for (int l6 = 0; l6 < 2; l6 = l6 + 1) fRec5[l6] = 0.0;
	for (int l7 = 0; l7 < 2; l7 = l7 + 1) fRec6[l7] = 0.0;
	for (int l8 = 0; l8 < 2; l8 = l8 + 1) fRec1[l8] = 0.0;
	for (int l9 = 0; l9 < 2; l9 = l9 + 1) fRec2[l9] = 0.0;
	for (int l10 = 0; l10 < 2048; l10 = l10 + 1) fRec3[l10] = 0.0;
	for (int l11 = 0; l11 < 2; l11 = l11 + 1) fRec10[l11] = 0.0;
	for (int l14 = 0; l14 < 2; l14 = l14 + 1) fRec12[l14] = 0.0;
	for (int l15 = 0; l15 < 2; l15 = l15 + 1) fRec13[l15] = 0.0;
	for (int l16 = 0; l16 < 2; l16 = l16 + 1) fRec14[l16] = 0.0;
}

void Dsp::clear_state_f_static(PluginDef *p)
{
	static_cast<Dsp*>(p)->clear_state_f();
}

inline void Dsp::init(unsigned int sample_rate)
{
	mydspSIG0* sig0 = newmydspSIG0();
	sig0->instanceInitmydspSIG0(sample_rate);
	sig0->fillmydspSIG0(65536, ftbl0mydspSIG0);
	deletemydspSIG0(sig0);
	fSampleRate = sample_rate;
	fConst0 = std::min<double>(1.92e+05, std::max<double>(1.0, static_cast<double>(fSampleRate)));
	fConst1 = 44.1 / fConst0;
	fConst2 = 1.0 - fConst1;
	fConst3 = 3.141592653589793 / fConst0;
	fConst4 = 0.4 * fConst0;
	fConst5 = std::tan(62.83185307179586 / fConst0);
	fConst6 = fConst5 + 1.4142271248762552;
	fConst7 = fConst5 * fConst6 + 1.0;
	fConst8 = fConst5 / fConst7;
	fConst9 = 2.0 * fConst8;
	fConst10 = 2.0 * fConst5;
	fConst11 = 1.0 / fConst7;
	fConst12 = 0.441 / fConst0;
	fConst13 = 0.55 / fConst0;
	fConst14 = 8.3 / fConst0;
	IOTA0 = 0;
	clear_state_f();
}

void Dsp::init_static(unsigned int sample_rate, PluginDef *p)
{
	static_cast<Dsp*>(p)->init(sample_rate);
}

void always_inline Dsp::compute(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0)
{
	double fSlow0 = fConst1 * std::pow(1e+01, 0.05 * static_cast<double>(fHslider0));
	double fSlow1 = fConst1 * static_cast<double>(fHslider1);
	double fSlow2 = fConst1 * std::pow(1e+01, 0.05 * static_cast<double>(fHslider2));
	double fSlow3 = fConst12 * static_cast<double>(fHslider3);
	double fSlow4 = fConst12 * static_cast<double>(fHslider4);
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		iVec0[0] = 1;
		fRec0[0] = fSlow0 + fConst2 * fRec0[1];
		fRec4[0] = fSlow1 + fConst2 * fRec4[1];
		double fTemp0 = std::tan(fConst3 * std::min<double>(fConst4, std::max<double>(1e+03, fRec4[0])));
		fRec9[0] = fSlow2 + fConst2 * fRec9[1];
		double fTemp1 = std::max<double>(1.0, fRec9[0]);
		double fTemp2 = tanh(0.125 * static_cast<double>(input0[i0]) * fTemp1);
		fVec1[0] = fTemp2;
		double fTemp3 = fVec1[1] - fVec1[2];
		double fTemp4 = 1.125 * fTemp3;
		double fTemp5 = fTemp2 - fVec1[1];
		double fTemp6 = fTemp4 + 0.875 * fTemp5;
		double fTemp7 = 0.375 * fTemp5;
		double fTemp8 = tanh(0.5277777777777778 * (fTemp4 + 6.75 * fVec1[1] + 1.25 * fTemp2 + 0.015 * fRec8[1] - fTemp7));
		double fTemp9 = 1.0 - mydsp_faustpower2_f(fTemp8);
		double fTemp10 = fTemp8 - fRec8[1];
		double fTemp11 = 3.0 * std::fabs(fTemp10) + 1.0;
		double fTemp12 = std::max<double>(-1.0, std::min<double>(1.0, fRec8[1] + fTemp6 * (0.31666666666666665 * fTemp9 + fTemp10 / (fTemp11 * (((fTemp6 >= 0.0) ? 1.0 : -1.0) + (0.001 - 0.015 * (fTemp10 / fTemp11))))) / (1.000000001 - 0.00475 * fTemp9)));
		double fTemp13 = fTemp7 + 1.75 * fTemp2 - (fTemp4 + fVec1[2] + 0.75 * fVec1[1]);
		double fTemp14 = tanh(0.5277777777777778 * (3.0 * fTemp2 + 6.0 * fVec1[1] + 0.015 * fTemp12 - fVec1[2]));
		double fTemp15 = 1.0 - mydsp_faustpower2_f(fTemp14);
		double fTemp16 = fTemp14 - fTemp12;
		double fTemp17 = 3.0 * std::fabs(fTemp16) + 1.0;
		double fTemp18 = std::max<double>(-1.0, std::min<double>(1.0, fTemp12 + fTemp13 * (0.31666666666666665 * fTemp15 + fTemp16 / (fTemp17 * (((fTemp13 >= 0.0) ? 1.0 : -1.0) + (0.001 - 0.015 * (fTemp16 / fTemp17))))) / (1.000000001 - 0.00475 * fTemp15)));
		double fTemp19 = 0.375 * fTemp3;
		double fTemp20 = 1.125 * fTemp5;
		double fTemp21 = fTemp19 + fVec1[2] + 3.75 * fTemp2 - (fTemp20 + 4.75 * fVec1[1]);
		double fTemp22 = tanh(0.5277777777777778 * (fTemp19 + 1.25 * fVec1[1] + 6.75 * fTemp2 + 0.015 * fTemp18 - fTemp20));
		double fTemp23 = 1.0 - mydsp_faustpower2_f(fTemp22);
		double fTemp24 = fTemp22 - fTemp18;
		double fTemp25 = 3.0 * std::fabs(fTemp24) + 1.0;
		double fTemp26 = std::max<double>(-1.0, std::min<double>(1.0, fTemp18 + fTemp21 * (0.31666666666666665 * fTemp23 + fTemp24 / (fTemp25 * (((fTemp21 >= 0.0) ? 1.0 : -1.0) + (0.001 - 0.015 * (fTemp24 / fTemp25))))) / (1.000000001 - 0.00475 * fTemp23)));
		double fTemp27 = 2.375 * fTemp5 - fTemp19;
		double fTemp28 = tanh(0.5277777777777778 * (8.0 * fTemp2 + 0.015 * fTemp26));
		double fTemp29 = 1.0 - mydsp_faustpower2_f(fTemp28);
		double fTemp30 = fTemp28 - fTemp26;
		double fTemp31 = 3.0 * std::fabs(fTemp30) + 1.0;
		fRec8[0] = std::max<double>(-1.0, std::min<double>(1.0, fTemp26 + fTemp27 * (0.31666666666666665 * fTemp29 + fTemp30 / (fTemp31 * (((fTemp27 >= 0.0) ? 1.0 : -1.0) + (0.001 - 0.015 * (fTemp30 / fTemp31))))) / (1.000000001 - 0.00475 * fTemp29)));
		double fTemp32 = 3.1578947368421053 * (fRec8[0] / std::sqrt(fTemp1));
		double fTemp33 = fTemp32 - (fConst6 * fRec5[1] + fRec6[1]);
		fRec5[0] = fRec5[1] + fConst9 * fTemp33;
		double fTemp34 = fRec5[1] + fConst8 * fTemp33;
		fRec6[0] = fRec6[1] + fConst10 * fTemp34;
		double fTemp35 = fConst11 * fTemp33;
		double fRec7 = fTemp35;
		double fTemp36 = fTemp0 + 1.4142271248762552;
		double fTemp37 = fRec7 - (fRec1[1] * fTemp36 + fRec2[1]);
		double fTemp38 = fTemp0 * fTemp36 + 1.0;
		double fTemp39 = fTemp0 * fTemp37 / fTemp38;
		fRec1[0] = fRec1[1] + 2.0 * fTemp39;
		double fTemp40 = fRec1[1] + fTemp39;
		double fTemp41 = fTemp0 * fTemp40;
		fRec2[0] = fRec2[1] + 2.0 * fTemp41;
		double fTemp42 = fRec2[1] + fTemp41;
		fRec3[IOTA0 & 2047] = fTemp42;
		fRec10[0] = fSlow3 + fConst2 * fRec10[1];
		int iTemp43 = 1 - iVec0[1];
		double fTemp44 = ((iTemp43) ? 0.0 : fConst13 + fRec12[1]);
		fRec12[0] = fTemp44 - std::floor(fTemp44);
		fRec13[0] = fSlow4 + fConst2 * fRec13[1];
		double fTemp45 = ((iTemp43) ? 0.0 : fConst14 + fRec14[1]);
		fRec14[0] = fTemp45 - std::floor(fTemp45);
		double fTemp46 = fConst0 * (0.0015 * fRec10[0] * ftbl0mydspSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0 * fRec12[0]), 65535))] + 0.00015 * fRec13[0] * ftbl0mydspSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0 * fRec14[0]), 65535))] + 0.002);
		int iTemp47 = static_cast<int>(fTemp46);
		double fTemp48 = std::floor(fTemp46);
		output0[i0] = static_cast<FAUSTFLOAT>(fRec0[0] * (fRec3[(IOTA0 - std::min<int>(1025, std::max<int>(0, iTemp47))) & 2047] * (fTemp48 + (1.0 - fTemp46)) + (fTemp46 - fTemp48) * fRec3[(IOTA0 - std::min<int>(1025, std::max<int>(0, iTemp47 + 1))) & 2047]));
		iVec0[1] = iVec0[0];
		fRec0[1] = fRec0[0];
		fRec4[1] = fRec4[0];
		fRec9[1] = fRec9[0];
		fVec1[2] = fVec1[1];
		fVec1[1] = fVec1[0];
		fRec8[1] = fRec8[0];
		fRec5[1] = fRec5[0];
		fRec6[1] = fRec6[0];
		fRec1[1] = fRec1[0];
		fRec2[1] = fRec2[0];
		IOTA0 = IOTA0 + 1;
		fRec10[1] = fRec10[0];
		fRec12[1] = fRec12[0];
		fRec13[1] = fRec13[0];
		fRec14[1] = fRec14[0];
	}
}

void __rt_func Dsp::compute_static(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0, PluginDef *p)
{
	static_cast<Dsp*>(p)->compute(count, input0, output0);
}

int Dsp::register_par(const ParamReg& reg)
{
	reg.registerFloatVar("tape_saturation.drive",N_("Drive"),"S","",&fHslider2, 12.0, 0.0, 3e+01, 0.1, 0);
	reg.registerFloatVar("tape_saturation.flutter",N_("Flutter"),"S","",&fHslider4, 5.0, 0.0, 1e+02, 1.0, 0);
	reg.registerFloatVar("tape_saturation.level",N_("Level"),"S","",&fHslider0, -3.0, -24.0, 6.0, 0.1, 0);
	reg.registerFloatVar("tape_saturation.tone",N_("Tone"),"S","",&fHslider1, 6.5e+03, 1e+03, 1.6e+04, 1e+01, 0);
	reg.registerFloatVar("tape_saturation.wow",N_("Wow"),"S","",&fHslider3, 1e+01, 0.0, 1e+02, 1.0, 0);
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

} // end namespace tape_saturation

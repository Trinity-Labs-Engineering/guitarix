// generated from file '../src/faust/plate_reverb.dsp' by dsp2cc:
// Code generated with Faust (https://faust.grame.fr)


namespace plate_reverb {

class Dsp: public PluginDef {
private:
	int fSampleRate;
	int IOTA0;
	double *fVec0;
	double fConst0;
	double fConst1;
	FAUSTFLOAT fHslider0;
	double fConst2;
	double fConst3;
	double fRec0[2];
	double fConst4;
	double fConst5;
	double fConst6;
	double fConst7;
	FAUSTFLOAT fHslider1;
	double fConst8;
	double fRec12[2];
	double fRec13[2];
	double fRec14[2];
	double fRec15[2];
	double fVec1[2];
	double fRec11[2];
	double fVec2[1024];
	int iConst9;
	double fRec9[2];
	double fVec3[1024];
	int iConst10;
	double fRec7[2];
	double fVec4[4096];
	int iConst11;
	double fRec5[2];
	double fVec5[2048];
	int iConst12;
	double fRec3[2];
	FAUSTFLOAT fHslider2;
	double fRec16[2];
	FAUSTFLOAT fHslider3;
	double fRec20[2];
	double *fVec6;
	int iConst13;
	double fRec21[2];
	double *fVec7;
	int iConst14;
	double fRec19[2];
	double *fVec8;
	int iConst15;
	double fRec17[2];
	double *fVec9;
	int iConst16;
	double fRec1[2];
	double *fVec10;
	int iConst17;
	double fRec26[2];
	double *fVec11;
	int iConst18;
	double fRec25[2];
	double *fVec12;
	int iConst19;
	double fRec23[2];
	double *fVec13;
	int iConst20;
	double fRec2[2];

	bool mem_allocated;
	unsigned int pending_sample_rate;
	void mem_alloc();
	void mem_free();
	void clear_state_f();
	int activate(bool start);
	void init(unsigned int sample_rate);
	void compute(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0);
	int register_par(const ParamReg& reg);

	static void clear_state_f_static(PluginDef*);
	static int activate_static(bool start, PluginDef*);
	static void init_static(unsigned int sample_rate, PluginDef*);
	static void compute_static(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0, PluginDef*);
	static int register_params_static(const ParamReg& reg);
	static void del_instance(PluginDef *p);
public:
	Dsp();
	~Dsp();
};



Dsp::Dsp()
	: PluginDef(),
	  fVec0(0),
	  fVec6(0),
	  fVec7(0),
	  fVec8(0),
	  fVec9(0),
	  fVec10(0),
	  fVec11(0),
	  fVec12(0),
	  fVec13(0),
	  mem_allocated(false),
	  pending_sample_rate(0) {
	version = PLUGINDEF_VERSION;
	flags = 0;
	id = "plate_reverb";
	name = N_("Dattorro Plate");
	groups = 0;
	description = ""; // description (tooltip)
	category = N_("Reverb");       // category
	shortname = "";     // shortname
	mono_audio = compute_static;
	stereo_audio = 0;
	set_samplerate = init_static;
	activate_plugin = activate_static;
	register_params = register_params_static;
	load_ui = 0;
	clear_state = clear_state_f_static;
	delete_instance = del_instance;
}

Dsp::~Dsp() {
	mem_free();
}

inline void Dsp::clear_state_f()
{
	if (!mem_allocated) return;
	for (int l0 = 0; l0 < 65536; l0 = l0 + 1) fVec0[l0] = 0.0;
	for (int l1 = 0; l1 < 2; l1 = l1 + 1) fRec0[l1] = 0.0;
	for (int l2 = 0; l2 < 2; l2 = l2 + 1) fRec12[l2] = 0.0;
	for (int l3 = 0; l3 < 2; l3 = l3 + 1) fRec13[l3] = 0.0;
	for (int l4 = 0; l4 < 2; l4 = l4 + 1) fRec14[l4] = 0.0;
	for (int l5 = 0; l5 < 2; l5 = l5 + 1) fRec15[l5] = 0.0;
	for (int l6 = 0; l6 < 2; l6 = l6 + 1) fVec1[l6] = 0.0;
	for (int l7 = 0; l7 < 2; l7 = l7 + 1) fRec11[l7] = 0.0;
	for (int l8 = 0; l8 < 1024; l8 = l8 + 1) fVec2[l8] = 0.0;
	for (int l9 = 0; l9 < 2; l9 = l9 + 1) fRec9[l9] = 0.0;
	for (int l10 = 0; l10 < 1024; l10 = l10 + 1) fVec3[l10] = 0.0;
	for (int l11 = 0; l11 < 2; l11 = l11 + 1) fRec7[l11] = 0.0;
	for (int l12 = 0; l12 < 4096; l12 = l12 + 1) fVec4[l12] = 0.0;
	for (int l13 = 0; l13 < 2; l13 = l13 + 1) fRec5[l13] = 0.0;
	for (int l14 = 0; l14 < 2048; l14 = l14 + 1) fVec5[l14] = 0.0;
	for (int l15 = 0; l15 < 2; l15 = l15 + 1) fRec3[l15] = 0.0;
	for (int l16 = 0; l16 < 2; l16 = l16 + 1) fRec16[l16] = 0.0;
	for (int l17 = 0; l17 < 2; l17 = l17 + 1) fRec20[l17] = 0.0;
	for (int l18 = 0; l18 < 8192; l18 = l18 + 1) fVec6[l18] = 0.0;
	for (int l19 = 0; l19 < 2; l19 = l19 + 1) fRec21[l19] = 0.0;
	for (int l20 = 0; l20 < 32768; l20 = l20 + 1) fVec7[l20] = 0.0;
	for (int l21 = 0; l21 < 2; l21 = l21 + 1) fRec19[l21] = 0.0;
	for (int l22 = 0; l22 < 32768; l22 = l22 + 1) fVec8[l22] = 0.0;
	for (int l23 = 0; l23 < 2; l23 = l23 + 1) fRec17[l23] = 0.0;
	for (int l24 = 0; l24 < 32768; l24 = l24 + 1) fVec9[l24] = 0.0;
	for (int l25 = 0; l25 < 2; l25 = l25 + 1) fRec1[l25] = 0.0;
	for (int l26 = 0; l26 < 8192; l26 = l26 + 1) fVec10[l26] = 0.0;
	for (int l27 = 0; l27 < 2; l27 = l27 + 1) fRec26[l27] = 0.0;
	for (int l28 = 0; l28 < 32768; l28 = l28 + 1) fVec11[l28] = 0.0;
	for (int l29 = 0; l29 < 2; l29 = l29 + 1) fRec25[l29] = 0.0;
	for (int l30 = 0; l30 < 16384; l30 = l30 + 1) fVec12[l30] = 0.0;
	for (int l31 = 0; l31 < 2; l31 = l31 + 1) fRec23[l31] = 0.0;
	for (int l32 = 0; l32 < 32768; l32 = l32 + 1) fVec13[l32] = 0.0;
	for (int l33 = 0; l33 < 2; l33 = l33 + 1) fRec2[l33] = 0.0;
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
	fConst4 = 1.0 / std::tan(3.141592653589793 * (std::min<double>(1.8e+04, 0.45 * fConst0) / fConst0));
	fConst5 = 1.0 / (fConst4 + 1.0);
	fConst6 = 1.0 - fConst4;
	fConst7 = 0.001 * fConst0;
	fConst8 = 5e+01 / fConst0;
	iConst9 = static_cast<int>(0.004771345048889486 * fConst0);
	iConst10 = static_cast<int>(0.0035953092974026412 * fConst0);
	iConst11 = static_cast<int>(0.01273478713752898 * fConst0);
	iConst12 = static_cast<int>(0.009307482947481604 * fConst0);
	iConst13 = static_cast<int>(0.030509727495715868 * fConst0);
	iConst14 = static_cast<int>(0.14169550754342933 * fConst0);
	iConst15 = static_cast<int>(0.08924431302711602 * fConst0);
	iConst16 = static_cast<int>(0.10628003091293975 * fConst0);
	iConst17 = static_cast<int>(0.022579886428547427 * fConst0);
	iConst18 = static_cast<int>(0.14962534861059776 * fConst0);
	iConst19 = static_cast<int>(0.060481838647894894 * fConst0);
	iConst20 = static_cast<int>(0.12499579987231611 * fConst0);
	IOTA0 = 0;
}

void Dsp::init_static(unsigned int sample_rate, PluginDef *p)
{
	Dsp *dsp = static_cast<Dsp*>(p);
	dsp->pending_sample_rate = sample_rate;
	if (dsp->mem_allocated) {
		dsp->init(sample_rate);
		dsp->clear_state_f();
	}
}

void Dsp::mem_alloc()
{
	if (!fVec0) fVec0 = new double[65536];
	if (!fVec6) fVec6 = new double[8192];
	if (!fVec7) fVec7 = new double[32768];
	if (!fVec8) fVec8 = new double[32768];
	if (!fVec9) fVec9 = new double[32768];
	if (!fVec10) fVec10 = new double[8192];
	if (!fVec11) fVec11 = new double[32768];
	if (!fVec12) fVec12 = new double[16384];
	if (!fVec13) fVec13 = new double[32768];
	mem_allocated = true;
}

void Dsp::mem_free()
{
	mem_allocated = false;
	if (fVec0) { delete[] fVec0; fVec0 = 0; }
	if (fVec6) { delete[] fVec6; fVec6 = 0; }
	if (fVec7) { delete[] fVec7; fVec7 = 0; }
	if (fVec8) { delete[] fVec8; fVec8 = 0; }
	if (fVec9) { delete[] fVec9; fVec9 = 0; }
	if (fVec10) { delete[] fVec10; fVec10 = 0; }
	if (fVec11) { delete[] fVec11; fVec11 = 0; }
	if (fVec12) { delete[] fVec12; fVec12 = 0; }
	if (fVec13) { delete[] fVec13; fVec13 = 0; }
}

int Dsp::activate(bool start)
{
	if (start) {
		if (!mem_allocated) {
			try {
				mem_alloc();
				if (pending_sample_rate) init(pending_sample_rate);
				clear_state_f();
			} catch (...) {
				mem_free();
				return -1;
			}
		}
	} else {
		mem_free();
	}
	return 0;
}

int Dsp::activate_static(bool start, PluginDef *p)
{
	return static_cast<Dsp*>(p)->activate(start);
}

void always_inline Dsp::compute(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0)
{
	double fSlow0 = fConst1 * static_cast<double>(fHslider0);
	double fSlow1 = fConst7 * static_cast<double>(fHslider1);
	double fSlow2 = fConst2 * static_cast<double>(fHslider2);
	double fSlow3 = fConst2 * static_cast<double>(fHslider3);
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		double fTemp0 = static_cast<double>(input0[i0]);
		fVec0[IOTA0 & 65535] = fTemp0;
		fRec0[0] = fSlow0 + fConst3 * fRec0[1];
		double fTemp1 = ((fRec12[1] != 0.0) ? (((fRec13[1] > 0.0) & (fRec13[1] < 1.0)) ? fRec12[1] : 0.0) : (((fRec13[1] == 0.0) & (fSlow1 != fRec14[1])) ? fConst8 : (((fRec13[1] == 1.0) & (fSlow1 != fRec15[1])) ? -fConst8 : 0.0)));
		fRec12[0] = fTemp1;
		fRec13[0] = std::max<double>(0.0, std::min<double>(1.0, fRec13[1] + fTemp1));
		fRec14[0] = (((fRec13[1] >= 1.0) & (fRec15[1] != fSlow1)) ? fSlow1 : fRec14[1]);
		fRec15[0] = (((fRec13[1] <= 0.0) & (fRec14[1] != fSlow1)) ? fSlow1 : fRec15[1]);
		double fTemp2 = fVec0[(IOTA0 - static_cast<int>(std::min<double>(32768.0, std::max<double>(0.0, fRec14[0])))) & 65535];
		double fTemp3 = fTemp2 + fRec13[0] * (fVec0[(IOTA0 - static_cast<int>(std::min<double>(32768.0, std::max<double>(0.0, fRec15[0])))) & 65535] - fTemp2);
		fVec1[0] = fTemp3;
		fRec11[0] = -(fConst5 * (fConst6 * fRec11[1] - (fTemp3 + fVec1[1])));
		double fTemp4 = fRec11[0] - 0.75 * fRec9[1];
		fVec2[IOTA0 & 1023] = fTemp4;
		fRec9[0] = fVec2[(IOTA0 - iConst9) & 1023];
		double fRec10 = 0.75 * fTemp4;
		double fTemp5 = fRec10 + fRec9[1] - 0.625 * fRec7[1];
		fVec3[IOTA0 & 1023] = fTemp5;
		fRec7[0] = fVec3[(IOTA0 - iConst10) & 1023];
		double fRec8 = 0.625 * fTemp5;
		double fTemp6 = fRec8 + fRec7[1] - 0.75 * fRec5[1];
		fVec4[IOTA0 & 4095] = fTemp6;
		fRec5[0] = fVec4[(IOTA0 - iConst11) & 4095];
		double fRec6 = 0.75 * fTemp6;
		double fTemp7 = fRec6 + fRec5[1] - 0.625 * fRec3[1];
		fVec5[IOTA0 & 2047] = fTemp7;
		fRec3[0] = fVec5[(IOTA0 - iConst12) & 2047];
		double fRec4 = 0.625 * fTemp7;
		fRec16[0] = fSlow2 + fConst3 * fRec16[1];
		fRec20[0] = fSlow3 + fConst3 * fRec20[1];
		double fTemp8 = 1.0 - fRec20[0];
		double fTemp9 = 0.7 * fRec21[1] + fRec2[1];
		fVec6[IOTA0 & 8191] = fTemp9;
		fRec21[0] = fVec6[(IOTA0 - iConst13) & 8191];
		double fRec22 = -(0.7 * fTemp9);
		fVec7[IOTA0 & 32767] = fRec22 + fRec21[1];
		fRec19[0] = fTemp8 * fVec7[(IOTA0 - iConst14) & 32767] + fRec20[0] * fRec19[1];
		double fTemp10 = fRec19[0] * fRec16[0] - 0.5 * fRec17[1];
		fVec8[IOTA0 & 32767] = fTemp10;
		fRec17[0] = fVec8[(IOTA0 - iConst15) & 32767];
		double fRec18 = 0.5 * fTemp10;
		fVec9[IOTA0 & 32767] = fRec18 + fRec17[1];
		fRec1[0] = fRec3[1] + fRec4 + fRec16[0] * fVec9[(IOTA0 - iConst16) & 32767];
		double fTemp11 = 0.7 * fRec26[1] + fRec1[1];
		fVec10[IOTA0 & 8191] = fTemp11;
		fRec26[0] = fVec10[(IOTA0 - iConst17) & 8191];
		double fRec27 = -(0.7 * fTemp11);
		fVec11[IOTA0 & 32767] = fRec27 + fRec26[1];
		fRec25[0] = fTemp8 * fVec11[(IOTA0 - iConst18) & 32767] + fRec20[0] * fRec25[1];
		double fTemp12 = fRec16[0] * fRec25[0] - 0.5 * fRec23[1];
		fVec12[IOTA0 & 16383] = fTemp12;
		fRec23[0] = fVec12[(IOTA0 - iConst19) & 16383];
		double fRec24 = 0.5 * fTemp12;
		fVec13[IOTA0 & 32767] = fRec24 + fRec23[1];
		fRec2[0] = fRec4 + fRec3[1] + fRec16[0] * fVec13[(IOTA0 - iConst20) & 32767];
		output0[i0] = static_cast<FAUSTFLOAT>(fTemp0 * (1.0 - fRec0[0]) + 0.5 * fRec0[0] * (fRec1[0] + fRec2[0]));
		IOTA0 = IOTA0 + 1;
		fRec0[1] = fRec0[0];
		fRec12[1] = fRec12[0];
		fRec13[1] = fRec13[0];
		fRec14[1] = fRec14[0];
		fRec15[1] = fRec15[0];
		fVec1[1] = fVec1[0];
		fRec11[1] = fRec11[0];
		fRec9[1] = fRec9[0];
		fRec7[1] = fRec7[0];
		fRec5[1] = fRec5[0];
		fRec3[1] = fRec3[0];
		fRec16[1] = fRec16[0];
		fRec20[1] = fRec20[0];
		fRec21[1] = fRec21[0];
		fRec19[1] = fRec19[0];
		fRec17[1] = fRec17[0];
		fRec1[1] = fRec1[0];
		fRec26[1] = fRec26[0];
		fRec25[1] = fRec25[0];
		fRec23[1] = fRec23[0];
		fRec2[1] = fRec2[0];
	}
}

void __rt_func Dsp::compute_static(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0, PluginDef *p)
{
	static_cast<Dsp*>(p)->compute(count, input0, output0);
}

int Dsp::register_par(const ParamReg& reg)
{
	reg.registerFloatVar("plate_reverb.damping",N_("Damping"),"S","",&fHslider3, 0.3, 0.0, 0.95, 0.01, 0);
	reg.registerFloatVar("plate_reverb.decay",N_("Decay"),"S","",&fHslider2, 0.65, 0.1, 0.95, 0.01, 0);
	reg.registerFloatVar("plate_reverb.mix",N_("Mix"),"S","",&fHslider0, 25.0, 0.0, 1e+02, 1.0, 0);
	reg.registerFloatVar("plate_reverb.predelay",N_("Predelay"),"S","",&fHslider1, 15.0, 0.0, 1e+02, 1.0, 0);
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

} // end namespace plate_reverb

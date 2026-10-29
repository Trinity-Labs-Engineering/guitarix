// generated from file '../src/faust/hall_reverb.dsp' by dsp2cc:
// Code generated with Faust (https://faust.grame.fr)


namespace hall_reverb {

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
	FAUSTFLOAT fHslider1;
	double fRec12[2];
	double fConst6;
	FAUSTFLOAT fHslider2;
	double fRec13[2];
	double fConst7;
	double fConst8;
	double fConst9;
	double fConst10;
	double fRec14[2];
	FAUSTFLOAT fHslider3;
	double fRec15[2];
	double fRec11[2];
	double *fVec1;
	double fConst11;
	int iConst12;
	double fConst13;
	FAUSTFLOAT fHslider4;
	double fConst14;
	double fRec16[2];
	double fRec17[2];
	double fRec18[2];
	double fRec19[2];
	double fVec2[4096];
	int iConst15;
	double fRec9[2];
	double fConst16;
	double fConst17;
	double fRec23[2];
	double fRec22[2];
	double *fVec3;
	double fConst18;
	int iConst19;
	double *fVec4;
	int iConst20;
	double fRec20[2];
	double fConst21;
	double fConst22;
	double fRec27[2];
	double fRec26[2];
	double *fVec5;
	double fConst23;
	int iConst24;
	double *fVec6;
	int iConst25;
	double fRec24[2];
	double fConst26;
	double fConst27;
	double fRec31[2];
	double fRec30[2];
	double *fVec7;
	double fConst28;
	int iConst29;
	double *fVec8;
	int iConst30;
	double fRec28[2];
	double fConst31;
	double fConst32;
	double fRec35[2];
	double fRec34[2];
	double *fVec9;
	double fConst33;
	int iConst34;
	double fVec10[4096];
	int iConst35;
	double fRec32[2];
	double fConst36;
	double fConst37;
	double fRec39[2];
	double fRec38[2];
	double *fVec11;
	double fConst38;
	int iConst39;
	double *fVec12;
	int iConst40;
	double fRec36[2];
	double fConst41;
	double fConst42;
	double fRec43[2];
	double fRec42[2];
	double *fVec13;
	double fConst43;
	int iConst44;
	double *fVec14;
	int iConst45;
	double fRec40[2];
	double fConst46;
	double fConst47;
	double fRec47[2];
	double fRec46[2];
	double *fVec15;
	double fConst48;
	int iConst49;
	double fVec16[4096];
	int iConst50;
	double fRec44[2];
	double fRec1[3];
	double fRec2[3];
	double fRec3[3];
	double fRec4[3];
	double fRec5[3];
	double fRec6[3];
	double fRec7[3];
	double fRec8[3];

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
	  fVec1(0),
	  fVec3(0),
	  fVec4(0),
	  fVec5(0),
	  fVec6(0),
	  fVec7(0),
	  fVec8(0),
	  fVec9(0),
	  fVec11(0),
	  fVec12(0),
	  fVec13(0),
	  fVec14(0),
	  fVec15(0),
	  mem_allocated(false),
	  pending_sample_rate(0) {
	version = PLUGINDEF_VERSION;
	flags = 0;
	id = "hall_reverb";
	name = N_("Zita Hall");
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
	for (int l6 = 0; l6 < 2; l6 = l6 + 1) fRec11[l6] = 0.0;
	for (int l7 = 0; l7 < 65536; l7 = l7 + 1) fVec1[l7] = 0.0;
	for (int l8 = 0; l8 < 2; l8 = l8 + 1) fRec16[l8] = 0.0;
	for (int l9 = 0; l9 < 2; l9 = l9 + 1) fRec17[l9] = 0.0;
	for (int l10 = 0; l10 < 2; l10 = l10 + 1) fRec18[l10] = 0.0;
	for (int l11 = 0; l11 < 2; l11 = l11 + 1) fRec19[l11] = 0.0;
	for (int l12 = 0; l12 < 4096; l12 = l12 + 1) fVec2[l12] = 0.0;
	for (int l13 = 0; l13 < 2; l13 = l13 + 1) fRec9[l13] = 0.0;
	for (int l14 = 0; l14 < 2; l14 = l14 + 1) fRec23[l14] = 0.0;
	for (int l15 = 0; l15 < 2; l15 = l15 + 1) fRec22[l15] = 0.0;
	for (int l16 = 0; l16 < 65536; l16 = l16 + 1) fVec3[l16] = 0.0;
	for (int l17 = 0; l17 < 8192; l17 = l17 + 1) fVec4[l17] = 0.0;
	for (int l18 = 0; l18 < 2; l18 = l18 + 1) fRec20[l18] = 0.0;
	for (int l19 = 0; l19 < 2; l19 = l19 + 1) fRec27[l19] = 0.0;
	for (int l20 = 0; l20 < 2; l20 = l20 + 1) fRec26[l20] = 0.0;
	for (int l21 = 0; l21 < 65536; l21 = l21 + 1) fVec5[l21] = 0.0;
	for (int l22 = 0; l22 < 8192; l22 = l22 + 1) fVec6[l22] = 0.0;
	for (int l23 = 0; l23 < 2; l23 = l23 + 1) fRec24[l23] = 0.0;
	for (int l24 = 0; l24 < 2; l24 = l24 + 1) fRec31[l24] = 0.0;
	for (int l25 = 0; l25 < 2; l25 = l25 + 1) fRec30[l25] = 0.0;
	for (int l26 = 0; l26 < 65536; l26 = l26 + 1) fVec7[l26] = 0.0;
	for (int l27 = 0; l27 < 8192; l27 = l27 + 1) fVec8[l27] = 0.0;
	for (int l28 = 0; l28 < 2; l28 = l28 + 1) fRec28[l28] = 0.0;
	for (int l29 = 0; l29 < 2; l29 = l29 + 1) fRec35[l29] = 0.0;
	for (int l30 = 0; l30 < 2; l30 = l30 + 1) fRec34[l30] = 0.0;
	for (int l31 = 0; l31 < 32768; l31 = l31 + 1) fVec9[l31] = 0.0;
	for (int l32 = 0; l32 < 4096; l32 = l32 + 1) fVec10[l32] = 0.0;
	for (int l33 = 0; l33 < 2; l33 = l33 + 1) fRec32[l33] = 0.0;
	for (int l34 = 0; l34 < 2; l34 = l34 + 1) fRec39[l34] = 0.0;
	for (int l35 = 0; l35 < 2; l35 = l35 + 1) fRec38[l35] = 0.0;
	for (int l36 = 0; l36 < 32768; l36 = l36 + 1) fVec11[l36] = 0.0;
	for (int l37 = 0; l37 < 8192; l37 = l37 + 1) fVec12[l37] = 0.0;
	for (int l38 = 0; l38 < 2; l38 = l38 + 1) fRec36[l38] = 0.0;
	for (int l39 = 0; l39 < 2; l39 = l39 + 1) fRec43[l39] = 0.0;
	for (int l40 = 0; l40 < 2; l40 = l40 + 1) fRec42[l40] = 0.0;
	for (int l41 = 0; l41 < 65536; l41 = l41 + 1) fVec13[l41] = 0.0;
	for (int l42 = 0; l42 < 8192; l42 = l42 + 1) fVec14[l42] = 0.0;
	for (int l43 = 0; l43 < 2; l43 = l43 + 1) fRec40[l43] = 0.0;
	for (int l44 = 0; l44 < 2; l44 = l44 + 1) fRec47[l44] = 0.0;
	for (int l45 = 0; l45 < 2; l45 = l45 + 1) fRec46[l45] = 0.0;
	for (int l46 = 0; l46 < 32768; l46 = l46 + 1) fVec15[l46] = 0.0;
	for (int l47 = 0; l47 < 4096; l47 = l47 + 1) fVec16[l47] = 0.0;
	for (int l48 = 0; l48 < 2; l48 = l48 + 1) fRec44[l48] = 0.0;
	for (int l49 = 0; l49 < 3; l49 = l49 + 1) fRec1[l49] = 0.0;
	for (int l50 = 0; l50 < 3; l50 = l50 + 1) fRec2[l50] = 0.0;
	for (int l51 = 0; l51 < 3; l51 = l51 + 1) fRec3[l51] = 0.0;
	for (int l52 = 0; l52 < 3; l52 = l52 + 1) fRec4[l52] = 0.0;
	for (int l53 = 0; l53 < 3; l53 = l53 + 1) fRec5[l53] = 0.0;
	for (int l54 = 0; l54 < 3; l54 = l54 + 1) fRec6[l54] = 0.0;
	for (int l55 = 0; l55 < 3; l55 = l55 + 1) fRec7[l55] = 0.0;
	for (int l56 = 0; l56 < 3; l56 = l56 + 1) fRec8[l56] = 0.0;
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
	fConst4 = std::floor(0.219991 * fConst0 + 0.5);
	fConst5 = 6.907755278982138 * (fConst4 / fConst0);
	fConst6 = 6.283185307179586 / fConst0;
	fConst7 = 0.45 * fConst0;
	fConst8 = 1.0 / std::tan(628.3185307179587 / fConst0);
	fConst9 = 1.0 / (fConst8 + 1.0);
	fConst10 = 1.0 - fConst8;
	fConst11 = std::floor(0.019123 * fConst0 + 0.5);
	iConst12 = static_cast<int>(std::min<double>(65536.0, std::max<double>(0.0, fConst4 - fConst11)));
	fConst13 = 0.001 * fConst0;
	fConst14 = 5e+01 / fConst0;
	iConst15 = static_cast<int>(std::min<double>(4096.0, std::max<double>(0.0, fConst11 + -1.0)));
	fConst16 = std::floor(0.256891 * fConst0 + 0.5);
	fConst17 = 6.907755278982138 * (fConst16 / fConst0);
	fConst18 = std::floor(0.027333 * fConst0 + 0.5);
	iConst19 = static_cast<int>(std::min<double>(65536.0, std::max<double>(0.0, fConst16 - fConst18)));
	iConst20 = static_cast<int>(std::min<double>(8192.0, std::max<double>(0.0, fConst18 + -1.0)));
	fConst21 = std::floor(0.192303 * fConst0 + 0.5);
	fConst22 = 6.907755278982138 * (fConst21 / fConst0);
	fConst23 = std::floor(0.029291 * fConst0 + 0.5);
	iConst24 = static_cast<int>(std::min<double>(32768.0, std::max<double>(0.0, fConst21 - fConst23)));
	iConst25 = static_cast<int>(std::min<double>(8192.0, std::max<double>(0.0, fConst23 + -1.0)));
	fConst26 = std::floor(0.210389 * fConst0 + 0.5);
	fConst27 = 6.907755278982138 * (fConst26 / fConst0);
	fConst28 = std::floor(0.024421 * fConst0 + 0.5);
	iConst29 = static_cast<int>(std::min<double>(65536.0, std::max<double>(0.0, fConst26 - fConst28)));
	iConst30 = static_cast<int>(std::min<double>(8192.0, std::max<double>(0.0, fConst28 + -1.0)));
	fConst31 = std::floor(0.125 * fConst0 + 0.5);
	fConst32 = 6.907755278982138 * (fConst31 / fConst0);
	fConst33 = std::floor(0.013458 * fConst0 + 0.5);
	iConst34 = static_cast<int>(std::min<double>(32768.0, std::max<double>(0.0, fConst31 - fConst33)));
	iConst35 = static_cast<int>(std::min<double>(4096.0, std::max<double>(0.0, fConst33 + -1.0)));
	fConst36 = std::floor(0.127837 * fConst0 + 0.5);
	fConst37 = 6.907755278982138 * (fConst36 / fConst0);
	fConst38 = std::floor(0.031604 * fConst0 + 0.5);
	iConst39 = static_cast<int>(std::min<double>(32768.0, std::max<double>(0.0, fConst36 - fConst38)));
	iConst40 = static_cast<int>(std::min<double>(8192.0, std::max<double>(0.0, fConst38 + -1.0)));
	fConst41 = std::floor(0.174713 * fConst0 + 0.5);
	fConst42 = 6.907755278982138 * (fConst41 / fConst0);
	fConst43 = std::floor(0.022904 * fConst0 + 0.5);
	iConst44 = static_cast<int>(std::min<double>(32768.0, std::max<double>(0.0, fConst41 - fConst43)));
	iConst45 = static_cast<int>(std::min<double>(8192.0, std::max<double>(0.0, fConst43 + -1.0)));
	fConst46 = std::floor(0.153129 * fConst0 + 0.5);
	fConst47 = 6.907755278982138 * (fConst46 / fConst0);
	fConst48 = std::floor(0.020346 * fConst0 + 0.5);
	iConst49 = static_cast<int>(std::min<double>(32768.0, std::max<double>(0.0, fConst46 - fConst48)));
	iConst50 = static_cast<int>(std::min<double>(4096.0, std::max<double>(0.0, fConst48 + -1.0)));
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
	if (!fVec1) fVec1 = new double[65536];
	if (!fVec3) fVec3 = new double[65536];
	if (!fVec4) fVec4 = new double[8192];
	if (!fVec5) fVec5 = new double[65536];
	if (!fVec6) fVec6 = new double[8192];
	if (!fVec7) fVec7 = new double[65536];
	if (!fVec8) fVec8 = new double[8192];
	if (!fVec9) fVec9 = new double[32768];
	if (!fVec11) fVec11 = new double[32768];
	if (!fVec12) fVec12 = new double[8192];
	if (!fVec13) fVec13 = new double[65536];
	if (!fVec14) fVec14 = new double[8192];
	if (!fVec15) fVec15 = new double[32768];
	mem_allocated = true;
}

void Dsp::mem_free()
{
	mem_allocated = false;
	if (fVec0) { delete[] fVec0; fVec0 = 0; }
	if (fVec1) { delete[] fVec1; fVec1 = 0; }
	if (fVec3) { delete[] fVec3; fVec3 = 0; }
	if (fVec4) { delete[] fVec4; fVec4 = 0; }
	if (fVec5) { delete[] fVec5; fVec5 = 0; }
	if (fVec6) { delete[] fVec6; fVec6 = 0; }
	if (fVec7) { delete[] fVec7; fVec7 = 0; }
	if (fVec8) { delete[] fVec8; fVec8 = 0; }
	if (fVec9) { delete[] fVec9; fVec9 = 0; }
	if (fVec11) { delete[] fVec11; fVec11 = 0; }
	if (fVec12) { delete[] fVec12; fVec12 = 0; }
	if (fVec13) { delete[] fVec13; fVec13 = 0; }
	if (fVec14) { delete[] fVec14; fVec14 = 0; }
	if (fVec15) { delete[] fVec15; fVec15 = 0; }
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
	double fSlow1 = fConst2 * static_cast<double>(fHslider1);
	double fSlow2 = fConst2 * static_cast<double>(fHslider2);
	double fSlow3 = fConst2 * static_cast<double>(fHslider3);
	double fSlow4 = fConst13 * static_cast<double>(fHslider4);
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		double fTemp0 = static_cast<double>(input0[i0]);
		fVec0[IOTA0 & 65535] = fTemp0;
		fRec0[0] = fSlow0 + fConst3 * fRec0[1];
		fRec12[0] = fSlow1 + fConst3 * fRec12[1];
		double fTemp1 = std::max<double>(0.2, fRec12[0]);
		double fTemp2 = std::exp(-(fConst5 / fTemp1));
		fRec13[0] = fSlow2 + fConst3 * fRec13[1];
		double fTemp3 = std::cos(fConst6 * std::min<double>(std::max<double>(1.5e+03, fRec13[0]), fConst7));
		double fTemp4 = mydsp_faustpower2_f(fTemp2);
		double fTemp5 = 1.0 - fTemp3 * fTemp4;
		double fTemp6 = 1.0 - fTemp4;
		double fTemp7 = std::sqrt(std::max<double>(0.0, mydsp_faustpower2_f(fTemp5) / mydsp_faustpower2_f(fTemp6) + -1.0));
		double fTemp8 = fTemp5 / fTemp6;
		fRec14[0] = -(fConst9 * (fConst10 * fRec14[1] - (fRec8[1] + fRec8[2])));
		fRec15[0] = fSlow3 + fConst3 * fRec15[1];
		double fTemp9 = fTemp1 * std::max<double>(0.5, fRec15[0]);
		fRec11[0] = fTemp2 * (fTemp7 + (1.0 - fTemp8)) * (fRec8[1] + fRec14[0] * (std::exp(-(fConst5 / fTemp9)) / fTemp2 + -1.0)) + (fTemp8 - fTemp7) * fRec11[1];
		fVec1[IOTA0 & 65535] = 0.35355339059327373 * fRec11[0] + 1e-20;
		double fTemp10 = 0.6 * fRec9[1] + fVec1[(IOTA0 - iConst12) & 65535];
		double fTemp11 = ((fRec16[1] != 0.0) ? (((fRec17[1] > 0.0) & (fRec17[1] < 1.0)) ? fRec16[1] : 0.0) : (((fRec17[1] == 0.0) & (fSlow4 != fRec18[1])) ? fConst14 : (((fRec17[1] == 1.0) & (fSlow4 != fRec19[1])) ? -fConst14 : 0.0)));
		fRec16[0] = fTemp11;
		fRec17[0] = std::max<double>(0.0, std::min<double>(1.0, fRec17[1] + fTemp11));
		fRec18[0] = (((fRec17[1] >= 1.0) & (fRec19[1] != fSlow4)) ? fSlow4 : fRec18[1]);
		fRec19[0] = (((fRec17[1] <= 0.0) & (fRec18[1] != fSlow4)) ? fSlow4 : fRec19[1]);
		double fTemp12 = fVec0[(IOTA0 - static_cast<int>(std::min<double>(32768.0, std::max<double>(0.0, fRec18[0])))) & 65535];
		double fTemp13 = 0.3 * (fTemp12 + fRec17[0] * (fVec0[(IOTA0 - static_cast<int>(std::min<double>(32768.0, std::max<double>(0.0, fRec19[0])))) & 65535] - fTemp12));
		fVec2[IOTA0 & 4095] = fTemp10 - fTemp13;
		fRec9[0] = fVec2[(IOTA0 - iConst15) & 4095];
		double fRec10 = 0.6 * (fTemp13 - fTemp10);
		double fTemp14 = std::exp(-(fConst17 / fTemp1));
		double fTemp15 = mydsp_faustpower2_f(fTemp14);
		double fTemp16 = 1.0 - fTemp3 * fTemp15;
		double fTemp17 = 1.0 - fTemp15;
		double fTemp18 = std::sqrt(std::max<double>(0.0, mydsp_faustpower2_f(fTemp16) / mydsp_faustpower2_f(fTemp17) + -1.0));
		double fTemp19 = fTemp16 / fTemp17;
		fRec23[0] = -(fConst9 * (fConst10 * fRec23[1] - (fRec4[1] + fRec4[2])));
		fRec22[0] = fTemp14 * (fTemp18 + (1.0 - fTemp19)) * (fRec4[1] + fRec23[0] * (std::exp(-(fConst17 / fTemp9)) / fTemp14 + -1.0)) + (fTemp19 - fTemp18) * fRec22[1];
		fVec3[IOTA0 & 65535] = 0.35355339059327373 * fRec22[0] + 1e-20;
		double fTemp20 = 0.6 * fRec20[1] + fVec3[(IOTA0 - iConst19) & 65535];
		fVec4[IOTA0 & 8191] = fTemp20 - fTemp13;
		fRec20[0] = fVec4[(IOTA0 - iConst20) & 8191];
		double fRec21 = 0.6 * (fTemp13 - fTemp20);
		double fTemp21 = std::exp(-(fConst22 / fTemp1));
		double fTemp22 = mydsp_faustpower2_f(fTemp21);
		double fTemp23 = 1.0 - fTemp3 * fTemp22;
		double fTemp24 = 1.0 - fTemp22;
		double fTemp25 = std::sqrt(std::max<double>(0.0, mydsp_faustpower2_f(fTemp23) / mydsp_faustpower2_f(fTemp24) + -1.0));
		double fTemp26 = fTemp23 / fTemp24;
		fRec27[0] = -(fConst9 * (fConst10 * fRec27[1] - (fRec6[1] + fRec6[2])));
		fRec26[0] = fTemp21 * (fTemp25 + (1.0 - fTemp26)) * (fRec6[1] + fRec27[0] * (std::exp(-(fConst22 / fTemp9)) / fTemp21 + -1.0)) + (fTemp26 - fTemp25) * fRec26[1];
		fVec5[IOTA0 & 65535] = 0.35355339059327373 * fRec26[0] + 1e-20;
		double fTemp27 = fVec5[(IOTA0 - iConst24) & 65535] + fTemp13 + 0.6 * fRec24[1];
		fVec6[IOTA0 & 8191] = fTemp27;
		fRec24[0] = fVec6[(IOTA0 - iConst25) & 8191];
		double fRec25 = -(0.6 * fTemp27);
		double fTemp28 = std::exp(-(fConst27 / fTemp1));
		double fTemp29 = mydsp_faustpower2_f(fTemp28);
		double fTemp30 = 1.0 - fTemp3 * fTemp29;
		double fTemp31 = 1.0 - fTemp29;
		double fTemp32 = std::sqrt(std::max<double>(0.0, mydsp_faustpower2_f(fTemp30) / mydsp_faustpower2_f(fTemp31) + -1.0));
		double fTemp33 = fTemp30 / fTemp31;
		fRec31[0] = -(fConst9 * (fConst10 * fRec31[1] - (fRec2[1] + fRec2[2])));
		fRec30[0] = fTemp28 * (fTemp32 + (1.0 - fTemp33)) * (fRec2[1] + fRec31[0] * (std::exp(-(fConst27 / fTemp9)) / fTemp28 + -1.0)) + (fTemp33 - fTemp32) * fRec30[1];
		fVec7[IOTA0 & 65535] = 0.35355339059327373 * fRec30[0] + 1e-20;
		double fTemp34 = fVec7[(IOTA0 - iConst29) & 65535] + fTemp13 + 0.6 * fRec28[1];
		fVec8[IOTA0 & 8191] = fTemp34;
		fRec28[0] = fVec8[(IOTA0 - iConst30) & 8191];
		double fRec29 = -(0.6 * fTemp34);
		double fTemp35 = std::exp(-(fConst32 / fTemp1));
		double fTemp36 = mydsp_faustpower2_f(fTemp35);
		double fTemp37 = 1.0 - fTemp3 * fTemp36;
		double fTemp38 = 1.0 - fTemp36;
		double fTemp39 = std::sqrt(std::max<double>(0.0, mydsp_faustpower2_f(fTemp37) / mydsp_faustpower2_f(fTemp38) + -1.0));
		double fTemp40 = fTemp37 / fTemp38;
		fRec35[0] = -(fConst9 * (fConst10 * fRec35[1] - (fRec7[1] + fRec7[2])));
		fRec34[0] = fTemp35 * (fTemp39 + (1.0 - fTemp40)) * (fRec7[1] + fRec35[0] * (std::exp(-(fConst32 / fTemp9)) / fTemp35 + -1.0)) + (fTemp40 - fTemp39) * fRec34[1];
		fVec9[IOTA0 & 32767] = 0.35355339059327373 * fRec34[0] + 1e-20;
		double fTemp41 = fVec9[(IOTA0 - iConst34) & 32767] - (fTemp13 + 0.6 * fRec32[1]);
		fVec10[IOTA0 & 4095] = fTemp41;
		fRec32[0] = fVec10[(IOTA0 - iConst35) & 4095];
		double fRec33 = 0.6 * fTemp41;
		double fTemp42 = std::exp(-(fConst37 / fTemp1));
		double fTemp43 = mydsp_faustpower2_f(fTemp42);
		double fTemp44 = 1.0 - fTemp3 * fTemp43;
		double fTemp45 = 1.0 - fTemp43;
		double fTemp46 = std::sqrt(std::max<double>(0.0, mydsp_faustpower2_f(fTemp44) / mydsp_faustpower2_f(fTemp45) + -1.0));
		double fTemp47 = fTemp44 / fTemp45;
		fRec39[0] = -(fConst9 * (fConst10 * fRec39[1] - (fRec3[1] + fRec3[2])));
		fRec38[0] = fTemp42 * (fTemp46 + (1.0 - fTemp47)) * (fRec3[1] + fRec39[0] * (std::exp(-(fConst37 / fTemp9)) / fTemp42 + -1.0)) + (fTemp47 - fTemp46) * fRec38[1];
		fVec11[IOTA0 & 32767] = 0.35355339059327373 * fRec38[0] + 1e-20;
		double fTemp48 = fVec11[(IOTA0 - iConst39) & 32767] - (fTemp13 + 0.6 * fRec36[1]);
		fVec12[IOTA0 & 8191] = fTemp48;
		fRec36[0] = fVec12[(IOTA0 - iConst40) & 8191];
		double fRec37 = 0.6 * fTemp48;
		double fTemp49 = std::exp(-(fConst42 / fTemp1));
		double fTemp50 = mydsp_faustpower2_f(fTemp49);
		double fTemp51 = 1.0 - fTemp3 * fTemp50;
		double fTemp52 = 1.0 - fTemp50;
		double fTemp53 = std::sqrt(std::max<double>(0.0, mydsp_faustpower2_f(fTemp51) / mydsp_faustpower2_f(fTemp52) + -1.0));
		double fTemp54 = fTemp51 / fTemp52;
		fRec43[0] = -(fConst9 * (fConst10 * fRec43[1] - (fRec5[1] + fRec5[2])));
		fRec42[0] = fTemp49 * (fTemp53 + (1.0 - fTemp54)) * (fRec5[1] + fRec43[0] * (std::exp(-(fConst42 / fTemp9)) / fTemp49 + -1.0)) + (fTemp54 - fTemp53) * fRec42[1];
		fVec13[IOTA0 & 65535] = 0.35355339059327373 * fRec42[0] + 1e-20;
		double fTemp55 = fTemp13 + fVec13[(IOTA0 - iConst44) & 65535] - 0.6 * fRec40[1];
		fVec14[IOTA0 & 8191] = fTemp55;
		fRec40[0] = fVec14[(IOTA0 - iConst45) & 8191];
		double fRec41 = 0.6 * fTemp55;
		double fTemp56 = std::exp(-(fConst47 / fTemp1));
		double fTemp57 = mydsp_faustpower2_f(fTemp56);
		double fTemp58 = 1.0 - fTemp57 * fTemp3;
		double fTemp59 = 1.0 - fTemp57;
		double fTemp60 = std::sqrt(std::max<double>(0.0, mydsp_faustpower2_f(fTemp58) / mydsp_faustpower2_f(fTemp59) + -1.0));
		double fTemp61 = fTemp58 / fTemp59;
		fRec47[0] = -(fConst9 * (fConst10 * fRec47[1] - (fRec1[1] + fRec1[2])));
		fRec46[0] = fTemp56 * (fTemp60 + (1.0 - fTemp61)) * (fRec1[1] + fRec47[0] * (std::exp(-(fConst47 / fTemp9)) / fTemp56 + -1.0)) + (fTemp61 - fTemp60) * fRec46[1];
		fVec15[IOTA0 & 32767] = 0.35355339059327373 * fRec46[0] + 1e-20;
		double fTemp62 = fVec15[(IOTA0 - iConst49) & 32767] + fTemp13 - 0.6 * fRec44[1];
		fVec16[IOTA0 & 4095] = fTemp62;
		fRec44[0] = fVec16[(IOTA0 - iConst50) & 4095];
		double fRec45 = 0.6 * fTemp62;
		double fTemp63 = fRec45 + fRec41;
		double fTemp64 = fRec33 + fRec37 + fTemp63;
		fRec1[0] = fRec9[1] + fRec20[1] + fRec24[1] + fRec28[1] + fRec32[1] + fRec36[1] + fRec40[1] + fRec44[1] + fRec10 + fRec21 + fRec25 + fRec29 + fTemp64;
		fRec2[0] = fRec32[1] + fRec36[1] + fRec40[1] + fRec44[1] + fTemp64 - (fRec9[1] + fRec20[1] + fRec24[1] + fRec28[1] + fRec10 + fRec21 + fRec29 + fRec25);
		double fTemp65 = fRec37 + fRec33;
		fRec3[0] = fRec24[1] + fRec28[1] + fRec40[1] + fRec44[1] + fRec25 + fRec29 + fTemp63 - (fRec9[1] + fRec20[1] + fRec32[1] + fRec36[1] + fRec10 + fRec21 + fTemp65);
		fRec4[0] = fRec9[1] + fRec20[1] + fRec40[1] + fRec44[1] + fRec10 + fRec21 + fTemp63 - (fRec24[1] + fRec28[1] + fRec32[1] + fRec36[1] + fRec25 + fRec29 + fTemp65);
		double fTemp66 = fRec45 + fRec37;
		double fTemp67 = fRec41 + fRec33;
		fRec5[0] = fRec20[1] + fRec28[1] + fRec36[1] + fRec44[1] + fRec21 + fRec29 + fTemp66 - (fRec9[1] + fRec24[1] + fRec32[1] + fRec40[1] + fRec10 + fRec25 + fTemp67);
		fRec6[0] = fRec9[1] + fRec24[1] + fRec36[1] + fRec44[1] + fRec10 + fRec25 + fTemp66 - (fRec20[1] + fRec28[1] + fRec32[1] + fRec40[1] + fRec21 + fRec29 + fTemp67);
		double fTemp68 = fRec45 + fRec33;
		double fTemp69 = fRec41 + fRec37;
		fRec7[0] = fRec9[1] + fRec28[1] + fRec32[1] + fRec44[1] + fRec10 + fRec29 + fTemp68 - (fRec20[1] + fRec24[1] + fRec36[1] + fRec40[1] + fRec21 + fRec25 + fTemp69);
		fRec8[0] = fRec20[1] + fRec24[1] + fRec32[1] + fRec44[1] + fRec21 + fRec25 + fTemp68 - (fRec9[1] + fRec28[1] + fRec36[1] + fRec40[1] + fRec10 + fRec29 + fTemp69);
		output0[i0] = static_cast<FAUSTFLOAT>(fTemp0 * (1.0 - fRec0[0]) + 0.185 * fRec0[0] * (fRec2[0] + fRec3[0] + (fRec2[0] - fRec3[0])));
		IOTA0 = IOTA0 + 1;
		fRec0[1] = fRec0[0];
		fRec12[1] = fRec12[0];
		fRec13[1] = fRec13[0];
		fRec14[1] = fRec14[0];
		fRec15[1] = fRec15[0];
		fRec11[1] = fRec11[0];
		fRec16[1] = fRec16[0];
		fRec17[1] = fRec17[0];
		fRec18[1] = fRec18[0];
		fRec19[1] = fRec19[0];
		fRec9[1] = fRec9[0];
		fRec23[1] = fRec23[0];
		fRec22[1] = fRec22[0];
		fRec20[1] = fRec20[0];
		fRec27[1] = fRec27[0];
		fRec26[1] = fRec26[0];
		fRec24[1] = fRec24[0];
		fRec31[1] = fRec31[0];
		fRec30[1] = fRec30[0];
		fRec28[1] = fRec28[0];
		fRec35[1] = fRec35[0];
		fRec34[1] = fRec34[0];
		fRec32[1] = fRec32[0];
		fRec39[1] = fRec39[0];
		fRec38[1] = fRec38[0];
		fRec36[1] = fRec36[0];
		fRec43[1] = fRec43[0];
		fRec42[1] = fRec42[0];
		fRec40[1] = fRec40[0];
		fRec47[1] = fRec47[0];
		fRec46[1] = fRec46[0];
		fRec44[1] = fRec44[0];
		fRec1[2] = fRec1[1];
		fRec1[1] = fRec1[0];
		fRec2[2] = fRec2[1];
		fRec2[1] = fRec2[0];
		fRec3[2] = fRec3[1];
		fRec3[1] = fRec3[0];
		fRec4[2] = fRec4[1];
		fRec4[1] = fRec4[0];
		fRec5[2] = fRec5[1];
		fRec5[1] = fRec5[0];
		fRec6[2] = fRec6[1];
		fRec6[1] = fRec6[0];
		fRec7[2] = fRec7[1];
		fRec7[1] = fRec7[0];
		fRec8[2] = fRec8[1];
		fRec8[1] = fRec8[0];
	}
}

void __rt_func Dsp::compute_static(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0, PluginDef *p)
{
	static_cast<Dsp*>(p)->compute(count, input0, output0);
}

int Dsp::register_par(const ParamReg& reg)
{
	reg.registerFloatVar("hall_reverb.bass",N_("Bass Decay"),"S","",&fHslider3, 1.25, 0.5, 2.0, 0.05, 0);
	reg.registerFloatVar("hall_reverb.damping",N_("HF Damping"),"S","",&fHslider2, 6e+03, 1.5e+03, 1.6e+04, 1e+02, 0);
	reg.registerFloatVar("hall_reverb.decay",N_("Decay"),"S","",&fHslider1, 2.5, 0.2, 1e+01, 0.1, 0);
	reg.registerFloatVar("hall_reverb.mix",N_("Mix"),"S","",&fHslider0, 25.0, 0.0, 1e+02, 1.0, 0);
	reg.registerFloatVar("hall_reverb.predelay",N_("Predelay"),"S","",&fHslider4, 2e+01, 0.0, 1e+02, 1.0, 0);
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

} // end namespace hall_reverb

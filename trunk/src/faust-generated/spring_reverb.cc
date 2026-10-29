// generated from file '../src/faust/spring_reverb.dsp' by dsp2cc:
// Code generated with Faust (https://faust.grame.fr)


namespace spring_reverb {

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
	double fConst8;
	FAUSTFLOAT fHslider1;
	double fRec11[2];
	FAUSTFLOAT fHslider2;
	double fRec12[2];
	double fConst9;
	int iConst10;
	int iConst11;
	int iConst12;
	int iConst13;
	int iConst14;
	int iConst15;
	int iConst16;
	int iConst17;
	double fVec1[2048];
	int iConst18;
	double fVec2[4096];
	int iConst19;
	double fVec3[4096];
	int iConst20;
	double *fVec4;
	int iConst21;
	double fVec5[4096];
	int iConst22;
	double *fVec6;
	int iConst23;
	double fVec7[4096];
	int iConst24;
	double *fVec8;
	int iConst25;
	double fVec9[2048];
	int iConst26;
	double fVec10[4096];
	int iConst27;
	double fVec11[4096];
	int iConst28;
	double fVec12[4096];
	int iConst29;
	double fVec13[4096];
	int iConst30;
	double fVec14[4096];
	int iConst31;
	double fVec15[4096];
	int iConst32;
	double *fVec16;
	int iConst33;
	double fVec17[2048];
	double *fVec18;
	double fConst34;
	FAUSTFLOAT fHslider3;
	FAUSTFLOAT fHslider4;
	double fRec13[2];
	double fVec19[2];
	double fRec10[2];
	double fRec2[2];
	double fVec20[4096];
	double *fVec21;
	double fVec22[2];
	double fRec14[2];
	double fRec3[2];
	double fVec23[4096];
	int iConst35;
	double *fVec24;
	double fVec25[2];
	double fRec15[2];
	double fRec4[2];
	double fVec26[4096];
	int iConst36;
	double *fVec27;
	double fVec28[2];
	double fRec16[2];
	double fRec5[2];
	double fVec29[4096];
	int iConst37;
	double *fVec30;
	double fVec31[2];
	double fRec17[2];
	double fRec6[2];
	double *fVec32;
	int iConst38;
	double *fVec33;
	double fVec34[2];
	double fRec18[2];
	double fRec7[2];
	double *fVec35;
	int iConst39;
	double *fVec36;
	double fVec37[2];
	double fRec19[2];
	double fRec8[2];
	double *fVec38;
	int iConst40;
	double *fVec39;
	double fVec40[2];
	double fRec20[2];
	double fRec9[2];
	double fVec41[2];
	double fRec1[2];

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
	  fVec4(0),
	  fVec6(0),
	  fVec8(0),
	  fVec16(0),
	  fVec18(0),
	  fVec21(0),
	  fVec24(0),
	  fVec27(0),
	  fVec30(0),
	  fVec32(0),
	  fVec33(0),
	  fVec35(0),
	  fVec36(0),
	  fVec38(0),
	  fVec39(0),
	  mem_allocated(false),
	  pending_sample_rate(0) {
	version = PLUGINDEF_VERSION;
	flags = 0;
	id = "spring_reverb";
	name = N_("Spring Reverb");
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
	for (int l0 = 0; l0 < 16384; l0 = l0 + 1) fVec0[l0] = 0.0;
	for (int l1 = 0; l1 < 2; l1 = l1 + 1) fRec0[l1] = 0.0;
	for (int l2 = 0; l2 < 2; l2 = l2 + 1) fRec11[l2] = 0.0;
	for (int l3 = 0; l3 < 2; l3 = l3 + 1) fRec12[l3] = 0.0;
	for (int l4 = 0; l4 < 2048; l4 = l4 + 1) fVec1[l4] = 0.0;
	for (int l5 = 0; l5 < 4096; l5 = l5 + 1) fVec2[l5] = 0.0;
	for (int l6 = 0; l6 < 4096; l6 = l6 + 1) fVec3[l6] = 0.0;
	for (int l7 = 0; l7 < 8192; l7 = l7 + 1) fVec4[l7] = 0.0;
	for (int l8 = 0; l8 < 4096; l8 = l8 + 1) fVec5[l8] = 0.0;
	for (int l9 = 0; l9 < 8192; l9 = l9 + 1) fVec6[l9] = 0.0;
	for (int l10 = 0; l10 < 4096; l10 = l10 + 1) fVec7[l10] = 0.0;
	for (int l11 = 0; l11 < 8192; l11 = l11 + 1) fVec8[l11] = 0.0;
	for (int l12 = 0; l12 < 2048; l12 = l12 + 1) fVec9[l12] = 0.0;
	for (int l13 = 0; l13 < 4096; l13 = l13 + 1) fVec10[l13] = 0.0;
	for (int l14 = 0; l14 < 4096; l14 = l14 + 1) fVec11[l14] = 0.0;
	for (int l15 = 0; l15 < 4096; l15 = l15 + 1) fVec12[l15] = 0.0;
	for (int l16 = 0; l16 < 4096; l16 = l16 + 1) fVec13[l16] = 0.0;
	for (int l17 = 0; l17 < 4096; l17 = l17 + 1) fVec14[l17] = 0.0;
	for (int l18 = 0; l18 < 4096; l18 = l18 + 1) fVec15[l18] = 0.0;
	for (int l19 = 0; l19 < 8192; l19 = l19 + 1) fVec16[l19] = 0.0;
	for (int l20 = 0; l20 < 2048; l20 = l20 + 1) fVec17[l20] = 0.0;
	for (int l21 = 0; l21 < 16384; l21 = l21 + 1) fVec18[l21] = 0.0;
	for (int l22 = 0; l22 < 2; l22 = l22 + 1) fRec13[l22] = 0.0;
	for (int l23 = 0; l23 < 2; l23 = l23 + 1) fVec19[l23] = 0.0;
	for (int l24 = 0; l24 < 2; l24 = l24 + 1) fRec10[l24] = 0.0;
	for (int l25 = 0; l25 < 2; l25 = l25 + 1) fRec2[l25] = 0.0;
	for (int l26 = 0; l26 < 4096; l26 = l26 + 1) fVec20[l26] = 0.0;
	for (int l27 = 0; l27 < 16384; l27 = l27 + 1) fVec21[l27] = 0.0;
	for (int l28 = 0; l28 < 2; l28 = l28 + 1) fVec22[l28] = 0.0;
	for (int l29 = 0; l29 < 2; l29 = l29 + 1) fRec14[l29] = 0.0;
	for (int l30 = 0; l30 < 2; l30 = l30 + 1) fRec3[l30] = 0.0;
	for (int l31 = 0; l31 < 4096; l31 = l31 + 1) fVec23[l31] = 0.0;
	for (int l32 = 0; l32 < 16384; l32 = l32 + 1) fVec24[l32] = 0.0;
	for (int l33 = 0; l33 < 2; l33 = l33 + 1) fVec25[l33] = 0.0;
	for (int l34 = 0; l34 < 2; l34 = l34 + 1) fRec15[l34] = 0.0;
	for (int l35 = 0; l35 < 2; l35 = l35 + 1) fRec4[l35] = 0.0;
	for (int l36 = 0; l36 < 4096; l36 = l36 + 1) fVec26[l36] = 0.0;
	for (int l37 = 0; l37 < 16384; l37 = l37 + 1) fVec27[l37] = 0.0;
	for (int l38 = 0; l38 < 2; l38 = l38 + 1) fVec28[l38] = 0.0;
	for (int l39 = 0; l39 < 2; l39 = l39 + 1) fRec16[l39] = 0.0;
	for (int l40 = 0; l40 < 2; l40 = l40 + 1) fRec5[l40] = 0.0;
	for (int l41 = 0; l41 < 4096; l41 = l41 + 1) fVec29[l41] = 0.0;
	for (int l42 = 0; l42 < 16384; l42 = l42 + 1) fVec30[l42] = 0.0;
	for (int l43 = 0; l43 < 2; l43 = l43 + 1) fVec31[l43] = 0.0;
	for (int l44 = 0; l44 < 2; l44 = l44 + 1) fRec17[l44] = 0.0;
	for (int l45 = 0; l45 < 2; l45 = l45 + 1) fRec6[l45] = 0.0;
	for (int l46 = 0; l46 < 8192; l46 = l46 + 1) fVec32[l46] = 0.0;
	for (int l47 = 0; l47 < 16384; l47 = l47 + 1) fVec33[l47] = 0.0;
	for (int l48 = 0; l48 < 2; l48 = l48 + 1) fVec34[l48] = 0.0;
	for (int l49 = 0; l49 < 2; l49 = l49 + 1) fRec18[l49] = 0.0;
	for (int l50 = 0; l50 < 2; l50 = l50 + 1) fRec7[l50] = 0.0;
	for (int l51 = 0; l51 < 8192; l51 = l51 + 1) fVec35[l51] = 0.0;
	for (int l52 = 0; l52 < 16384; l52 = l52 + 1) fVec36[l52] = 0.0;
	for (int l53 = 0; l53 < 2; l53 = l53 + 1) fVec37[l53] = 0.0;
	for (int l54 = 0; l54 < 2; l54 = l54 + 1) fRec19[l54] = 0.0;
	for (int l55 = 0; l55 < 2; l55 = l55 + 1) fRec8[l55] = 0.0;
	for (int l56 = 0; l56 < 8192; l56 = l56 + 1) fVec38[l56] = 0.0;
	for (int l57 = 0; l57 < 16384; l57 = l57 + 1) fVec39[l57] = 0.0;
	for (int l58 = 0; l58 < 2; l58 = l58 + 1) fVec40[l58] = 0.0;
	for (int l59 = 0; l59 < 2; l59 = l59 + 1) fRec20[l59] = 0.0;
	for (int l60 = 0; l60 < 2; l60 = l60 + 1) fRec9[l60] = 0.0;
	for (int l61 = 0; l61 < 2; l61 = l61 + 1) fVec41[l61] = 0.0;
	for (int l62 = 0; l62 < 2; l62 = l62 + 1) fRec1[l62] = 0.0;
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
	fConst4 = 1.0 / std::tan(471.23889803846896 / fConst0);
	fConst5 = 1.0 / (fConst4 + 1.0);
	fConst6 = 1.0 - fConst4;
	fConst7 = 3.141592653589793 / fConst0;
	fConst8 = 0.45 * fConst0;
	fConst9 = std::round(0.05 * fConst0);
	iConst10 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, fConst9)));
	iConst11 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.035 * fConst0))));
	iConst12 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.0425 * fConst0))));
	iConst13 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.0275 * fConst0))));
	iConst14 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.04625 * fConst0))));
	iConst15 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.03125 * fConst0))));
	iConst16 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.03875 * fConst0))));
	iConst17 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.02375 * fConst0))));
	iConst18 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.009 * fConst0))));
	iConst19 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.019499999999999997 * fConst0))));
	iConst20 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.014249999999999999 * fConst0))));
	iConst21 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.02475 * fConst0))));
	iConst22 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.011625 * fConst0))));
	iConst23 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.022125 * fConst0))));
	iConst24 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.016875 * fConst0))));
	iConst25 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.027374999999999997 * fConst0))));
	iConst26 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.01 * fConst0))));
	iConst27 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.0175 * fConst0))));
	iConst28 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.01375 * fConst0))));
	iConst29 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.02125 * fConst0))));
	iConst30 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.011875 * fConst0))));
	iConst31 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.019375000000000003 * fConst0))));
	iConst32 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.015625 * fConst0))));
	iConst33 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.023125 * fConst0))));
	fConst34 = std::round(0.08 * fConst0);
	iConst35 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.01475 * fConst0))));
	iConst36 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.017625000000000002 * fConst0))));
	iConst37 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.020499999999999997 * fConst0))));
	iConst38 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.023375 * fConst0))));
	iConst39 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.026250000000000002 * fConst0))));
	iConst40 = static_cast<int>(std::min<double>(fConst9, std::max<double>(0.0, std::round(0.029124999999999998 * fConst0))));
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
	if (!fVec0) fVec0 = new double[16384];
	if (!fVec4) fVec4 = new double[8192];
	if (!fVec6) fVec6 = new double[8192];
	if (!fVec8) fVec8 = new double[8192];
	if (!fVec16) fVec16 = new double[8192];
	if (!fVec18) fVec18 = new double[16384];
	if (!fVec21) fVec21 = new double[16384];
	if (!fVec24) fVec24 = new double[16384];
	if (!fVec27) fVec27 = new double[16384];
	if (!fVec30) fVec30 = new double[16384];
	if (!fVec32) fVec32 = new double[8192];
	if (!fVec33) fVec33 = new double[16384];
	if (!fVec35) fVec35 = new double[8192];
	if (!fVec36) fVec36 = new double[16384];
	if (!fVec38) fVec38 = new double[8192];
	if (!fVec39) fVec39 = new double[16384];
	mem_allocated = true;
}

void Dsp::mem_free()
{
	mem_allocated = false;
	if (fVec0) { delete[] fVec0; fVec0 = 0; }
	if (fVec4) { delete[] fVec4; fVec4 = 0; }
	if (fVec6) { delete[] fVec6; fVec6 = 0; }
	if (fVec8) { delete[] fVec8; fVec8 = 0; }
	if (fVec16) { delete[] fVec16; fVec16 = 0; }
	if (fVec18) { delete[] fVec18; fVec18 = 0; }
	if (fVec21) { delete[] fVec21; fVec21 = 0; }
	if (fVec24) { delete[] fVec24; fVec24 = 0; }
	if (fVec27) { delete[] fVec27; fVec27 = 0; }
	if (fVec30) { delete[] fVec30; fVec30 = 0; }
	if (fVec32) { delete[] fVec32; fVec32 = 0; }
	if (fVec33) { delete[] fVec33; fVec33 = 0; }
	if (fVec35) { delete[] fVec35; fVec35 = 0; }
	if (fVec36) { delete[] fVec36; fVec36 = 0; }
	if (fVec38) { delete[] fVec38; fVec38 = 0; }
	if (fVec39) { delete[] fVec39; fVec39 = 0; }
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
	double fSlow3 = std::max<double>(0.0, std::min<double>(2.0, std::round(static_cast<double>(fHslider3))));
	double fSlow4 = ((fSlow3 == 0.0) ? 2e-06 : ((fSlow3 == 1.0) ? 0.0005 : 2.8e-05));
	double fSlow5 = fConst2 * static_cast<double>(fHslider4);
	double fSlow6 = 2.0 * fSlow4;
	double fSlow7 = 3.0 * fSlow4;
	double fSlow8 = 5.0 * fSlow4;
	double fSlow9 = 7.0 * fSlow4;
	double fSlow10 = 11.0 * fSlow4;
	double fSlow11 = 13.0 * fSlow4;
	double fSlow12 = 17.0 * fSlow4;
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		double fTemp0 = static_cast<double>(input0[i0]);
		fVec0[IOTA0 & 16383] = fTemp0;
		fRec0[0] = fSlow0 + fConst3 * fRec0[1];
		fRec11[0] = fSlow1 + fConst3 * fRec11[1];
		double fTemp1 = std::max<double>(0.0, std::min<double>(1e+01, 1e+01 * fRec11[0]));
		double fTemp2 = std::min<double>(fConst8, fTemp1 * (1.9e+02 * fTemp1 + 5e+01) + 1.5e+03);
		double fTemp3 = 1.0 / std::tan(fConst7 * fTemp2);
		double fTemp4 = 1.0 - fTemp3;
		double fTemp5 = fRec2[1] + fRec6[1];
		double fTemp6 = fRec8[1] + fTemp5 + fRec4[1];
		fRec12[0] = fSlow2 + fConst3 * fRec12[1];
		double fTemp7 = std::max<double>(0.0, std::min<double>(1e+01, 1e+01 * fRec12[0]));
		double fTemp8 = fTemp7 * (0.013 - 0.0006 * fTemp7) + 0.26;
		double fTemp9 = fVec0[(IOTA0 - iConst10) & 16383];
		double fTemp10 = fVec0[(IOTA0 - iConst11) & 16383];
		double fTemp11 = fTemp9 + fTemp10;
		double fTemp12 = fVec0[(IOTA0 - iConst12) & 16383];
		double fTemp13 = fVec0[(IOTA0 - iConst13) & 16383];
		double fTemp14 = fTemp12 + fTemp13;
		double fTemp15 = fTemp11 + fTemp14;
		double fTemp16 = fVec0[(IOTA0 - iConst14) & 16383];
		double fTemp17 = fVec0[(IOTA0 - iConst15) & 16383];
		double fTemp18 = fTemp16 + fTemp17;
		double fTemp19 = fVec0[(IOTA0 - iConst16) & 16383];
		double fTemp20 = fVec0[(IOTA0 - iConst17) & 16383];
		double fTemp21 = fTemp19 + fTemp20;
		double fTemp22 = fTemp18 + fTemp21;
		fVec1[IOTA0 & 2047] = fTemp15 + fTemp22;
		double fTemp23 = fVec1[(IOTA0 - iConst18) & 2047];
		double fTemp24 = fTemp9 - fTemp10;
		double fTemp25 = fTemp12 - fTemp13;
		double fTemp26 = fTemp24 + fTemp25;
		double fTemp27 = fTemp16 - fTemp17;
		double fTemp28 = fTemp19 - fTemp20;
		double fTemp29 = fTemp27 + fTemp28;
		fVec2[IOTA0 & 4095] = fTemp26 + fTemp29;
		double fTemp30 = fVec2[(IOTA0 - iConst19) & 4095];
		double fTemp31 = fTemp23 - fTemp30;
		double fTemp32 = fTemp11 - fTemp14;
		double fTemp33 = fTemp18 - fTemp21;
		fVec3[IOTA0 & 4095] = fTemp32 + fTemp33;
		double fTemp34 = fVec3[(IOTA0 - iConst20) & 4095];
		double fTemp35 = fTemp24 - fTemp25;
		double fTemp36 = fTemp27 - fTemp28;
		fVec4[IOTA0 & 8191] = fTemp35 + fTemp36;
		double fTemp37 = fVec4[(IOTA0 - iConst21) & 8191];
		double fTemp38 = fTemp34 - fTemp37;
		double fTemp39 = fTemp31 + fTemp38;
		fVec5[IOTA0 & 4095] = fTemp15 - fTemp22;
		double fTemp40 = fVec5[(IOTA0 - iConst22) & 4095];
		fVec6[IOTA0 & 8191] = fTemp26 - fTemp29;
		double fTemp41 = fVec6[(IOTA0 - iConst23) & 8191];
		double fTemp42 = fTemp40 - fTemp41;
		fVec7[IOTA0 & 4095] = fTemp32 - fTemp33;
		double fTemp43 = fVec7[(IOTA0 - iConst24) & 4095];
		fVec8[IOTA0 & 8191] = fTemp35 - fTemp36;
		double fTemp44 = fVec8[(IOTA0 - iConst25) & 8191];
		double fTemp45 = fTemp43 - fTemp44;
		double fTemp46 = fTemp42 - fTemp45;
		fVec9[IOTA0 & 2047] = fTemp39 - fTemp46;
		double fTemp47 = fVec9[(IOTA0 - iConst26) & 2047];
		double fTemp48 = fTemp23 + fTemp30;
		double fTemp49 = fTemp34 + fTemp37;
		double fTemp50 = fTemp48 + fTemp49;
		double fTemp51 = fTemp40 + fTemp41;
		double fTemp52 = fTemp43 + fTemp44;
		double fTemp53 = fTemp51 - fTemp52;
		fVec10[IOTA0 & 4095] = fTemp50 - fTemp53;
		double fTemp54 = fVec10[(IOTA0 - iConst27) & 4095];
		double fTemp55 = fTemp47 - fTemp54;
		double fTemp56 = fTemp31 - fTemp38;
		double fTemp57 = fTemp42 + fTemp45;
		fVec11[IOTA0 & 4095] = fTemp56 - fTemp57;
		double fTemp58 = fVec11[(IOTA0 - iConst28) & 4095];
		double fTemp59 = fTemp48 - fTemp49;
		double fTemp60 = fTemp51 + fTemp52;
		fVec12[IOTA0 & 4095] = fTemp59 - fTemp60;
		double fTemp61 = fVec12[(IOTA0 - iConst29) & 4095];
		double fTemp62 = fTemp58 + fTemp61;
		double fTemp63 = fTemp55 - fTemp62;
		fVec13[IOTA0 & 4095] = fTemp39 + fTemp46;
		double fTemp64 = fVec13[(IOTA0 - iConst30) & 4095];
		fVec14[IOTA0 & 4095] = fTemp50 + fTemp53;
		double fTemp65 = fVec14[(IOTA0 - iConst31) & 4095];
		double fTemp66 = fTemp64 + fTemp65;
		fVec15[IOTA0 & 4095] = fTemp56 + fTemp57;
		double fTemp67 = fVec15[(IOTA0 - iConst32) & 4095];
		fVec16[IOTA0 & 8191] = fTemp59 + fTemp60;
		double fTemp68 = fVec16[(IOTA0 - iConst33) & 8191];
		double fTemp69 = fTemp67 + fTemp68;
		double fTemp70 = fTemp66 - fTemp69;
		fVec17[IOTA0 & 2047] = fTemp63 - fTemp70;
		fVec18[IOTA0 & 16383] = (fRec9[1] + fRec5[1] + fRec7[1] + fTemp6 + fRec3[1]) * fTemp8 - 0.01 * fVec17[(IOTA0 - iConst18) & 2047];
		fRec13[0] = fSlow5 + fConst3 * fRec13[1];
		double fTemp71 = std::max<double>(0.0, std::min<double>(1e+01, 1e+01 * fRec13[0]));
		double fTemp72 = fTemp71 * (0.00032 * fTemp71 + -0.0072);
		double fTemp73 = fVec18[(IOTA0 - static_cast<int>(std::min<double>(fConst34, std::max<double>(0.0, std::round(fConst0 * (fSlow4 + fTemp72 + 0.07)))))) & 16383];
		fVec19[0] = fTemp73;
		double fTemp74 = fTemp3 + 1.0;
		fRec10[0] = -((fRec10[1] * fTemp4 - (fTemp73 + fVec19[1])) / fTemp74);
		double fTemp75 = fRec10[0] - fRec10[1];
		fRec2[0] = ((std::fabs(fTemp75) <= 0.001) ? std::max<double>(-1.0, std::min<double>(1.0, 0.5 * (fRec10[0] + fRec10[1]))) : ((((fRec10[0] <= 1.0) & (fRec10[0] >= -1.0)) ? 0.5 * mydsp_faustpower2_f(fRec10[0]) : fRec10[0] * static_cast<double>((fRec10[0] > 0.0) - (fRec10[0] < 0.0)) + -0.5) - (((fRec10[1] <= 1.0) & (fRec10[1] >= -1.0)) ? 0.5 * mydsp_faustpower2_f(fRec10[1]) : fRec10[1] * static_cast<double>((fRec10[1] > 0.0) - (fRec10[1] < 0.0)) + -0.5)) / fTemp75);
		fVec20[IOTA0 & 4095] = fTemp63 + fTemp70;
		fVec21[IOTA0 & 16383] = fTemp8 * (fTemp6 - (fRec9[1] + fRec3[1] + fRec7[1] + fRec5[1])) + 0.01 * fVec20[(IOTA0 - iConst30) & 4095];
		double fTemp76 = fVec21[(IOTA0 - static_cast<int>(std::min<double>(fConst34, std::max<double>(0.0, std::round(fConst0 * (fSlow6 + fTemp72 + 0.07)))))) & 16383];
		fVec22[0] = fTemp76;
		fRec14[0] = -((fTemp4 * fRec14[1] - (fTemp76 + fVec22[1])) / fTemp74);
		double fTemp77 = fRec14[0] - fRec14[1];
		fRec3[0] = ((std::fabs(fTemp77) <= 0.001) ? std::max<double>(-1.0, std::min<double>(1.0, 0.5 * (fRec14[0] + fRec14[1]))) : ((((fRec14[0] <= 1.0) & (fRec14[0] >= -1.0)) ? 0.5 * mydsp_faustpower2_f(fRec14[0]) : fRec14[0] * static_cast<double>((fRec14[0] > 0.0) - (fRec14[0] < 0.0)) + -0.5) - (((fRec14[1] <= 1.0) & (fRec14[1] >= -1.0)) ? 0.5 * mydsp_faustpower2_f(fRec14[1]) : fRec14[1] * static_cast<double>((fRec14[1] > 0.0) - (fRec14[1] < 0.0)) + -0.5)) / fTemp77);
		double fTemp78 = fRec4[1] + fRec8[1];
		double fTemp79 = fTemp55 + fTemp62;
		double fTemp80 = fTemp66 + fTemp69;
		fVec23[IOTA0 & 4095] = fTemp79 - fTemp80;
		fVec24[IOTA0 & 16383] = fTemp8 * (fRec7[1] + fTemp5 + fRec3[1] - (fRec9[1] + fTemp78 + fRec5[1])) + 0.01 * fVec23[(IOTA0 - iConst35) & 4095];
		double fTemp81 = fVec24[(IOTA0 - static_cast<int>(std::min<double>(fConst34, std::max<double>(0.0, std::round(fConst0 * (fSlow7 + fTemp72 + 0.07)))))) & 16383];
		fVec25[0] = fTemp81;
		fRec15[0] = -((fTemp4 * fRec15[1] - (fTemp81 + fVec25[1])) / fTemp74);
		double fTemp82 = fRec15[0] - fRec15[1];
		fRec4[0] = ((std::fabs(fTemp82) <= 0.001) ? std::max<double>(-1.0, std::min<double>(1.0, 0.5 * (fRec15[0] + fRec15[1]))) : ((((fRec15[0] <= 1.0) & (fRec15[0] >= -1.0)) ? 0.5 * mydsp_faustpower2_f(fRec15[0]) : fRec15[0] * static_cast<double>((fRec15[0] > 0.0) - (fRec15[0] < 0.0)) + -0.5) - (((fRec15[1] <= 1.0) & (fRec15[1] >= -1.0)) ? 0.5 * mydsp_faustpower2_f(fRec15[1]) : fRec15[1] * static_cast<double>((fRec15[1] > 0.0) - (fRec15[1] < 0.0)) + -0.5)) / fTemp82);
		fVec26[IOTA0 & 4095] = fTemp79 + fTemp80;
		fVec27[IOTA0 & 16383] = fTemp8 * (fRec9[1] + fTemp5 + fRec5[1] - (fRec7[1] + fTemp78 + fRec3[1])) - 0.01 * fVec26[(IOTA0 - iConst36) & 4095];
		double fTemp83 = fVec27[(IOTA0 - static_cast<int>(std::min<double>(fConst34, std::max<double>(0.0, std::round(fConst0 * (fSlow8 + fTemp72 + 0.07)))))) & 16383];
		fVec28[0] = fTemp83;
		fRec16[0] = -((fTemp4 * fRec16[1] - (fTemp83 + fVec28[1])) / fTemp74);
		double fTemp84 = fRec16[0] - fRec16[1];
		fRec5[0] = ((std::fabs(fTemp84) <= 0.001) ? std::max<double>(-1.0, std::min<double>(1.0, 0.5 * (fRec16[0] + fRec16[1]))) : ((((fRec16[0] <= 1.0) & (fRec16[0] >= -1.0)) ? 0.5 * mydsp_faustpower2_f(fRec16[0]) : fRec16[0] * static_cast<double>((fRec16[0] > 0.0) - (fRec16[0] < 0.0)) + -0.5) - (((fRec16[1] <= 1.0) & (fRec16[1] >= -1.0)) ? 0.5 * mydsp_faustpower2_f(fRec16[1]) : fRec16[1] * static_cast<double>((fRec16[1] > 0.0) - (fRec16[1] < 0.0)) + -0.5)) / fTemp84);
		double fTemp85 = fRec2[1] + fRec4[1];
		double fTemp86 = fRec6[1] + fRec8[1];
		double fTemp87 = fTemp47 + fTemp54;
		double fTemp88 = fTemp58 - fTemp61;
		double fTemp89 = fTemp87 - fTemp88;
		double fTemp90 = fTemp64 - fTemp65;
		double fTemp91 = fTemp67 - fTemp68;
		double fTemp92 = fTemp90 - fTemp91;
		fVec29[IOTA0 & 4095] = fTemp89 - fTemp92;
		fVec30[IOTA0 & 16383] = fTemp8 * (fRec5[1] + fRec3[1] + fTemp85 - (fRec9[1] + fRec7[1] + fTemp86)) - 0.01 * fVec29[(IOTA0 - iConst37) & 4095];
		double fTemp93 = fVec30[(IOTA0 - static_cast<int>(std::min<double>(fConst34, std::max<double>(0.0, std::round(fConst0 * (fSlow9 + fTemp72 + 0.07)))))) & 16383];
		fVec31[0] = fTemp93;
		fRec17[0] = -((fTemp4 * fRec17[1] - (fTemp93 + fVec31[1])) / fTemp74);
		double fTemp94 = fRec17[0] - fRec17[1];
		fRec6[0] = ((std::fabs(fTemp94) <= 0.001) ? std::max<double>(-1.0, std::min<double>(1.0, 0.5 * (fRec17[0] + fRec17[1]))) : ((((fRec17[0] <= 1.0) & (fRec17[0] >= -1.0)) ? 0.5 * mydsp_faustpower2_f(fRec17[0]) : fRec17[0] * static_cast<double>((fRec17[0] > 0.0) - (fRec17[0] < 0.0)) + -0.5) - (((fRec17[1] <= 1.0) & (fRec17[1] >= -1.0)) ? 0.5 * mydsp_faustpower2_f(fRec17[1]) : fRec17[1] * static_cast<double>((fRec17[1] > 0.0) - (fRec17[1] < 0.0)) + -0.5)) / fTemp94);
		fVec32[IOTA0 & 8191] = fTemp89 + fTemp92;
		fVec33[IOTA0 & 16383] = fTemp8 * (fRec9[1] + fRec7[1] + fTemp85 - (fRec5[1] + fRec3[1] + fTemp86)) - 0.01 * fVec32[(IOTA0 - iConst38) & 8191];
		double fTemp95 = fVec33[(IOTA0 - static_cast<int>(std::min<double>(fConst34, std::max<double>(0.0, std::round(fConst0 * (fSlow10 + fTemp72 + 0.07)))))) & 16383];
		fVec34[0] = fTemp95;
		fRec18[0] = -((fTemp4 * fRec18[1] - (fTemp95 + fVec34[1])) / fTemp74);
		double fTemp96 = fRec18[0] - fRec18[1];
		fRec7[0] = ((std::fabs(fTemp96) <= 0.001) ? std::max<double>(-1.0, std::min<double>(1.0, 0.5 * (fRec18[0] + fRec18[1]))) : ((((fRec18[0] <= 1.0) & (fRec18[0] >= -1.0)) ? 0.5 * mydsp_faustpower2_f(fRec18[0]) : fRec18[0] * static_cast<double>((fRec18[0] > 0.0) - (fRec18[0] < 0.0)) + -0.5) - (((fRec18[1] <= 1.0) & (fRec18[1] >= -1.0)) ? 0.5 * mydsp_faustpower2_f(fRec18[1]) : fRec18[1] * static_cast<double>((fRec18[1] > 0.0) - (fRec18[1] < 0.0)) + -0.5)) / fTemp96);
		double fTemp97 = fRec2[1] + fRec8[1];
		double fTemp98 = fRec6[1] + fRec4[1];
		double fTemp99 = fTemp87 + fTemp88;
		double fTemp100 = fTemp90 + fTemp91;
		fVec35[IOTA0 & 8191] = fTemp99 - fTemp100;
		fVec36[IOTA0 & 16383] = fTemp8 * (fRec9[1] + fRec3[1] + fTemp97 - (fRec5[1] + fRec7[1] + fTemp98)) + 0.01 * fVec35[(IOTA0 - iConst39) & 8191];
		double fTemp101 = fVec36[(IOTA0 - static_cast<int>(std::min<double>(fConst34, std::max<double>(0.0, std::round(fConst0 * (fSlow11 + fTemp72 + 0.07)))))) & 16383];
		fVec37[0] = fTemp101;
		fRec19[0] = -((fTemp4 * fRec19[1] - (fTemp101 + fVec37[1])) / fTemp74);
		double fTemp102 = fRec19[0] - fRec19[1];
		fRec8[0] = ((std::fabs(fTemp102) <= 0.001) ? std::max<double>(-1.0, std::min<double>(1.0, 0.5 * (fRec19[0] + fRec19[1]))) : ((((fRec19[0] <= 1.0) & (fRec19[0] >= -1.0)) ? 0.5 * mydsp_faustpower2_f(fRec19[0]) : fRec19[0] * static_cast<double>((fRec19[0] > 0.0) - (fRec19[0] < 0.0)) + -0.5) - (((fRec19[1] <= 1.0) & (fRec19[1] >= -1.0)) ? 0.5 * mydsp_faustpower2_f(fRec19[1]) : fRec19[1] * static_cast<double>((fRec19[1] > 0.0) - (fRec19[1] < 0.0)) + -0.5)) / fTemp102);
		fVec38[IOTA0 & 8191] = fTemp99 + fTemp100;
		fVec39[IOTA0 & 16383] = fTemp8 * (fRec5[1] + fRec7[1] + fTemp97 - (fRec9[1] + fRec3[1] + fTemp98)) - 0.01 * fVec38[(IOTA0 - iConst40) & 8191];
		double fTemp103 = fVec39[(IOTA0 - static_cast<int>(std::min<double>(fConst34, std::max<double>(0.0, std::round(fConst0 * (fSlow12 + fTemp72 + 0.07)))))) & 16383];
		fVec40[0] = fTemp103;
		fRec20[0] = -((fTemp4 * fRec20[1] - (fTemp103 + fVec40[1])) / fTemp74);
		double fTemp104 = fRec20[0] - fRec20[1];
		fRec9[0] = ((std::fabs(fTemp104) <= 0.001) ? std::max<double>(-1.0, std::min<double>(1.0, 0.5 * (fRec20[0] + fRec20[1]))) : ((((fRec20[0] <= 1.0) & (fRec20[0] >= -1.0)) ? 0.5 * mydsp_faustpower2_f(fRec20[0]) : fRec20[0] * static_cast<double>((fRec20[0] > 0.0) - (fRec20[0] < 0.0)) + -0.5) - (((fRec20[1] <= 1.0) & (fRec20[1] >= -1.0)) ? 0.5 * mydsp_faustpower2_f(fRec20[1]) : fRec20[1] * static_cast<double>((fRec20[1] > 0.0) - (fRec20[1] < 0.0)) + -0.5)) / fTemp104);
		double fTemp105 = fRec2[0] + fRec3[0] + fRec4[0] + fRec5[0] + fRec6[0] + fRec7[0] + fRec8[0] + fRec9[0];
		fVec41[0] = fTemp105;
		fRec1[0] = -(fConst5 * (fConst6 * fRec1[1] - fConst4 * (fTemp105 - fVec41[1])));
		output0[i0] = static_cast<FAUSTFLOAT>(fTemp0 * (1.0 - fRec0[0]) + 0.4 * fRec0[0] * fRec1[0] * std::min<double>(5.0, 2e+03 / fTemp2 + 1.0));
		IOTA0 = IOTA0 + 1;
		fRec0[1] = fRec0[0];
		fRec11[1] = fRec11[0];
		fRec12[1] = fRec12[0];
		fRec13[1] = fRec13[0];
		fVec19[1] = fVec19[0];
		fRec10[1] = fRec10[0];
		fRec2[1] = fRec2[0];
		fVec22[1] = fVec22[0];
		fRec14[1] = fRec14[0];
		fRec3[1] = fRec3[0];
		fVec25[1] = fVec25[0];
		fRec15[1] = fRec15[0];
		fRec4[1] = fRec4[0];
		fVec28[1] = fVec28[0];
		fRec16[1] = fRec16[0];
		fRec5[1] = fRec5[0];
		fVec31[1] = fVec31[0];
		fRec17[1] = fRec17[0];
		fRec6[1] = fRec6[0];
		fVec34[1] = fVec34[0];
		fRec18[1] = fRec18[0];
		fRec7[1] = fRec7[0];
		fVec37[1] = fVec37[0];
		fRec19[1] = fRec19[0];
		fRec8[1] = fRec8[0];
		fVec40[1] = fVec40[0];
		fRec20[1] = fRec20[0];
		fRec9[1] = fRec9[0];
		fVec41[1] = fVec41[0];
		fRec1[1] = fRec1[0];
	}
}

void __rt_func Dsp::compute_static(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0, PluginDef *p)
{
	static_cast<Dsp*>(p)->compute(count, input0, output0);
}

int Dsp::register_par(const ParamReg& reg)
{
	static const value_pair fHslider3_values[] = {{"Tight"},{"Wide"},{"Balanced"},{0}};
	reg.registerFloatVar("spring_reverb.character",N_("Character"),"S","",&fHslider3, 2.0, 0.0, 2.0, 1.0, fHslider3_values);
	reg.registerFloatVar("spring_reverb.dwell",N_("Dwell"),"S","",&fHslider2, 0.5, 0.0, 1.0, 0.01, 0);
	reg.registerFloatVar("spring_reverb.mix",N_("Mix"),"S","",&fHslider0, 3e+01, 0.0, 1e+02, 1.0, 0);
	reg.registerFloatVar("spring_reverb.tension",N_("Tension"),"S","",&fHslider4, 0.5, 0.0, 1.0, 0.01, 0);
	reg.registerFloatVar("spring_reverb.tone",N_("Tone"),"S","",&fHslider1, 0.5, 0.0, 1.0, 0.01, 0);
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

} // end namespace spring_reverb

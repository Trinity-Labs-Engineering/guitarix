// generated from file '../src/faust/granular_cloud.dsp' by dsp2cc:
// Code generated with Faust (https://faust.grame.fr)


namespace granular_cloud {
class mydspSIG0 {
	
  private:
	
	
  public:
	
	int getNumInputsmydspSIG0() {
		return 0;
	}
	int getNumOutputsmydspSIG0() {
		return 1;
	}
	
	void instanceInitmydspSIG0(int sample_rate) {
	}
	
	void fillmydspSIG0(int count, double* table) {
		for (int i1 = 0; i1 < count; i1 = i1 + 1) {
			table[i1] = 0.0;
		}
	}

};

static mydspSIG0* newmydspSIG0() { return (mydspSIG0*)new mydspSIG0(); }
static void deletemydspSIG0(mydspSIG0* dsp) { delete dsp; }



class Dsp: public PluginDef {
private:
	int fSampleRate;
	double fConst0;
	double fConst1;
	FAUSTFLOAT fHslider0;
	double fConst2;
	double fConst3;
	int iVec0[2];
	double fRec0[2];
	double *ftbl0;
	FAUSTFLOAT fHslider1;
	int iRec2[2];
	double fConst4;
	double fConst5;
	FAUSTFLOAT fHslider2;
	double fRec5[2];
	double fRec4[2];
	double fVec1[2];
	FAUSTFLOAT fHslider3;
	double fRec6[2];
	FAUSTFLOAT fHslider4;
	double fRec7[2];
	double fRec8[2];
	FAUSTFLOAT fHslider5;
	double fRec9[2];
	double fRec3[2];
	double fRec10[2];
	double fRec11[2];
	int iRec12[2];
	double fVec2[2];
	double fRec13[2];
	double fRec14[2];
	double fRec15[2];
	double fVec3[2];
	double fRec16[2];
	double fRec17[2];
	double fRec18[2];
	double fVec4[2];
	double fRec19[2];
	double fRec20[2];
	double fRec21[2];
	double fVec5[2];
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
	  ftbl0(0),
	  mem_allocated(false),
	  pending_sample_rate(0) {
	version = PLUGINDEF_VERSION;
	flags = 0;
	id = "granular_cloud";
	name = N_("Granular Cloud");
	groups = 0;
	description = ""; // description (tooltip)
	category = N_("Delay");       // category
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
	for (int l0 = 0; l0 < 2; l0 = l0 + 1) iVec0[l0] = 0;
	for (int l1 = 0; l1 < 2; l1 = l1 + 1) fRec0[l1] = 0.0;
	for (int l2 = 0; l2 < 2; l2 = l2 + 1) iRec2[l2] = 0;
	for (int l3 = 0; l3 < 2; l3 = l3 + 1) fRec5[l3] = 0.0;
	for (int l4 = 0; l4 < 2; l4 = l4 + 1) fRec4[l4] = 0.0;
	for (int l5 = 0; l5 < 2; l5 = l5 + 1) fVec1[l5] = 0.0;
	for (int l6 = 0; l6 < 2; l6 = l6 + 1) fRec6[l6] = 0.0;
	for (int l7 = 0; l7 < 2; l7 = l7 + 1) fRec7[l7] = 0.0;
	for (int l8 = 0; l8 < 2; l8 = l8 + 1) fRec8[l8] = 0.0;
	for (int l9 = 0; l9 < 2; l9 = l9 + 1) fRec9[l9] = 0.0;
	for (int l10 = 0; l10 < 2; l10 = l10 + 1) fRec3[l10] = 0.0;
	for (int l11 = 0; l11 < 2; l11 = l11 + 1) fRec10[l11] = 0.0;
	for (int l12 = 0; l12 < 2; l12 = l12 + 1) fRec11[l12] = 0.0;
	for (int l13 = 0; l13 < 2; l13 = l13 + 1) iRec12[l13] = 0;
	for (int l14 = 0; l14 < 2; l14 = l14 + 1) fVec2[l14] = 0.0;
	for (int l15 = 0; l15 < 2; l15 = l15 + 1) fRec13[l15] = 0.0;
	for (int l16 = 0; l16 < 2; l16 = l16 + 1) fRec14[l16] = 0.0;
	for (int l17 = 0; l17 < 2; l17 = l17 + 1) fRec15[l17] = 0.0;
	for (int l18 = 0; l18 < 2; l18 = l18 + 1) fVec3[l18] = 0.0;
	for (int l19 = 0; l19 < 2; l19 = l19 + 1) fRec16[l19] = 0.0;
	for (int l20 = 0; l20 < 2; l20 = l20 + 1) fRec17[l20] = 0.0;
	for (int l21 = 0; l21 < 2; l21 = l21 + 1) fRec18[l21] = 0.0;
	for (int l22 = 0; l22 < 2; l22 = l22 + 1) fVec4[l22] = 0.0;
	for (int l23 = 0; l23 < 2; l23 = l23 + 1) fRec19[l23] = 0.0;
	for (int l24 = 0; l24 < 2; l24 = l24 + 1) fRec20[l24] = 0.0;
	for (int l25 = 0; l25 < 2; l25 = l25 + 1) fRec21[l25] = 0.0;
	for (int l26 = 0; l26 < 2; l26 = l26 + 1) fVec5[l26] = 0.0;
	for (int l27 = 0; l27 < 2; l27 = l27 + 1) fRec1[l27] = 0.0;
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
	mydspSIG0* sig0 = newmydspSIG0();
	sig0->instanceInitmydspSIG0(sample_rate);
	sig0->fillmydspSIG0(524289, ftbl0);
	fConst4 = 1.0 / fConst0;
	fConst5 = 0.0441 / fConst0;
	deletemydspSIG0(sig0);
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
	if (!ftbl0) ftbl0 = new double[524289];
	mem_allocated = true;
}

void Dsp::mem_free()
{
	mem_allocated = false;
	if (ftbl0) { delete[] ftbl0; ftbl0 = 0; }
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
	int iSlow1 = static_cast<double>(fHslider1) >= 0.5;
	double fSlow2 = fConst5 * static_cast<double>(fHslider2);
	double fSlow3 = fConst5 * static_cast<double>(fHslider3);
	double fSlow4 = fConst5 * static_cast<double>(fHslider4);
	double fSlow5 = fConst2 * static_cast<double>(fHslider5);
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		double fTemp0 = static_cast<double>(input0[i0]);
		iVec0[0] = 1;
		fRec0[0] = fSlow0 + fConst3 * fRec0[1];
		iRec2[0] = (iRec2[1] + 1 - iSlow1) & 524287;
		ftbl0[((iSlow1) ? 524288 : iRec2[0])] = fTemp0;
		fRec5[0] = fSlow2 + fConst3 * fRec5[1];
		double fTemp1 = std::max<double>(0.04, fRec5[0]);
		double fTemp2 = ((1 - iVec0[1]) ? 0.0 : fRec4[1] + fConst4 / std::max<double>(0.04, fTemp1));
		fRec4[0] = fTemp2 - std::floor(fTemp2);
		double fTemp3 = fRec4[0] - std::floor(fRec4[0]);
		fVec1[0] = fTemp3;
		int iTemp4 = fTemp3 < fVec1[1];
		fRec6[0] = fSlow3 + fConst3 * fRec6[1];
		fRec7[0] = fSlow4 + fConst3 * fRec7[1];
		fRec8[0] = std::fmod(48271.0 * (fRec8[1] + 1.0), 2147483647.0);
		fRec9[0] = fSlow5 + fConst3 * fRec9[1];
		double fTemp5 = std::pow(2.0, 0.08333333333333333 * fRec9[0]);
		double fTemp6 = static_cast<double>(iRec2[0]) + (-2.0 - fConst0 * (fRec6[0] + 4.656612875245797e-10 * fRec7[0] * fRec8[0] + fTemp1 * std::max<double>(1.0, fTemp5)));
		fRec3[0] = ((iTemp4) ? fTemp6 : fRec3[1]);
		fRec10[0] = ((iTemp4) ? fTemp5 : fRec10[1]);
		double fTemp7 = fConst0 * fTemp1;
		fRec11[0] = ((iTemp4) ? fTemp7 : fRec11[1]);
		double fTemp8 = fRec3[0] + fRec10[0] * fRec11[0] * fTemp3;
		double fTemp9 = std::floor(fTemp8);
		int iTemp10 = static_cast<int>(fTemp9) & 524287;
		iRec12[0] = std::min<int>(524288, iRec12[1] + 1 - iSlow1);
		int iTemp11 = static_cast<int>(fTemp9 + 1.0) & 524287;
		double fTemp12 = fRec4[0] + (0.25 - std::floor(fRec4[0] + 0.25));
		fVec2[0] = fTemp12;
		int iTemp13 = fTemp12 < fVec2[1];
		fRec13[0] = ((iTemp13) ? fTemp6 : fRec13[1]);
		fRec14[0] = ((iTemp13) ? fTemp5 : fRec14[1]);
		fRec15[0] = ((iTemp13) ? fTemp7 : fRec15[1]);
		double fTemp14 = fRec13[0] + fRec14[0] * fRec15[0] * fTemp12;
		double fTemp15 = std::floor(fTemp14);
		int iTemp16 = static_cast<int>(fTemp15) & 524287;
		int iTemp17 = static_cast<int>(fTemp15 + 1.0) & 524287;
		double fTemp18 = fRec4[0] + (0.5 - std::floor(fRec4[0] + 0.5));
		fVec3[0] = fTemp18;
		int iTemp19 = fTemp18 < fVec3[1];
		fRec16[0] = ((iTemp19) ? fTemp6 : fRec16[1]);
		fRec17[0] = ((iTemp19) ? fTemp5 : fRec17[1]);
		fRec18[0] = ((iTemp19) ? fTemp7 : fRec18[1]);
		double fTemp20 = fRec16[0] + fRec17[0] * fRec18[0] * fTemp18;
		double fTemp21 = std::floor(fTemp20);
		int iTemp22 = static_cast<int>(fTemp21) & 524287;
		int iTemp23 = static_cast<int>(fTemp21 + 1.0) & 524287;
		double fTemp24 = fRec4[0] + (0.75 - std::floor(fRec4[0] + 0.75));
		fVec4[0] = fTemp24;
		int iTemp25 = fTemp24 < fVec4[1];
		fRec19[0] = ((iTemp25) ? fTemp6 : fRec19[1]);
		fRec20[0] = ((iTemp25) ? fTemp5 : fRec20[1]);
		fRec21[0] = ((iTemp25) ? fTemp7 : fRec21[1]);
		double fTemp26 = fRec19[0] + fRec20[0] * fRec21[0] * fTemp24;
		double fTemp27 = std::floor(fTemp26);
		int iTemp28 = static_cast<int>(fTemp27) & 524287;
		int iTemp29 = static_cast<int>(fTemp27 + 1.0) & 524287;
		double fTemp30 = (ftbl0[iTemp10] * static_cast<double>(((iRec2[0] - iTemp10) & 524287) < iRec12[0]) * (fTemp9 + (1.0 - fTemp8)) + (fTemp8 - fTemp9) * ftbl0[iTemp11] * static_cast<double>(((iRec2[0] - iTemp11) & 524287) < iRec12[0])) * (1.0 - std::cos(6.283185307179586 * fTemp3)) + (ftbl0[iTemp16] * static_cast<double>(((iRec2[0] - iTemp16) & 524287) < iRec12[0]) * (fTemp15 + (1.0 - fTemp14)) + (fTemp14 - fTemp15) * ftbl0[iTemp17] * static_cast<double>(((iRec2[0] - iTemp17) & 524287) < iRec12[0])) * (1.0 - std::cos(6.283185307179586 * fTemp12)) + (ftbl0[iTemp22] * static_cast<double>(((iRec2[0] - iTemp22) & 524287) < iRec12[0]) * (fTemp21 + (1.0 - fTemp20)) + (fTemp20 - fTemp21) * ftbl0[iTemp23] * static_cast<double>(((iRec2[0] - iTemp23) & 524287) < iRec12[0])) * (1.0 - std::cos(6.283185307179586 * fTemp18)) + (ftbl0[iTemp28] * static_cast<double>(((iRec2[0] - iTemp28) & 524287) < iRec12[0]) * (fTemp27 + (1.0 - fTemp26)) + (fTemp26 - fTemp27) * ftbl0[iTemp29] * static_cast<double>(((iRec2[0] - iTemp29) & 524287) < iRec12[0])) * (1.0 - std::cos(6.283185307179586 * fTemp24));
		fVec5[0] = fTemp30;
		fRec1[0] = 0.995 * fRec1[1] + 0.25 * (fTemp30 - fVec5[1]);
		output0[i0] = static_cast<FAUSTFLOAT>(fTemp0 * (1.0 - fRec0[0]) + fRec0[0] * fRec1[0]);
		iVec0[1] = iVec0[0];
		fRec0[1] = fRec0[0];
		iRec2[1] = iRec2[0];
		fRec5[1] = fRec5[0];
		fRec4[1] = fRec4[0];
		fVec1[1] = fVec1[0];
		fRec6[1] = fRec6[0];
		fRec7[1] = fRec7[0];
		fRec8[1] = fRec8[0];
		fRec9[1] = fRec9[0];
		fRec3[1] = fRec3[0];
		fRec10[1] = fRec10[0];
		fRec11[1] = fRec11[0];
		iRec12[1] = iRec12[0];
		fVec2[1] = fVec2[0];
		fRec13[1] = fRec13[0];
		fRec14[1] = fRec14[0];
		fRec15[1] = fRec15[0];
		fVec3[1] = fVec3[0];
		fRec16[1] = fRec16[0];
		fRec17[1] = fRec17[0];
		fRec18[1] = fRec18[0];
		fVec4[1] = fVec4[0];
		fRec19[1] = fRec19[0];
		fRec20[1] = fRec20[0];
		fRec21[1] = fRec21[0];
		fVec5[1] = fVec5[0];
		fRec1[1] = fRec1[0];
	}
}

void __rt_func Dsp::compute_static(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0, PluginDef *p)
{
	static_cast<Dsp*>(p)->compute(count, input0, output0);
}

int Dsp::register_par(const ParamReg& reg)
{
	reg.registerFloatVar("granular_cloud.freeze",N_("Freeze"),"S","",&fHslider1, 0.0, 0.0, 1.0, 1.0, 0);
	reg.registerFloatVar("granular_cloud.mix",N_("Mix"),"S","",&fHslider0, 5e+01, 0.0, 1e+02, 1.0, 0);
	reg.registerFloatVar("granular_cloud.pitch",N_("Pitch"),"S","",&fHslider5, 0.0, -12.0, 12.0, 1.0, 0);
	reg.registerFloatVar("granular_cloud.position",N_("Position"),"S","",&fHslider3, 1e+02, 0.0, 5e+02, 1.0, 0);
	reg.registerFloatVar("granular_cloud.scatter",N_("Scatter"),"S","",&fHslider4, 4e+01, 0.0, 2e+02, 1.0, 0);
	reg.registerFloatVar("granular_cloud.size",N_("Grain Size"),"S","",&fHslider2, 1.2e+02, 4e+01, 2.5e+02, 1.0, 0);
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

} // end namespace granular_cloud

// generated from file 'src/faust/digital_delay.dsp' by dsp2cc:
// Code generated with Faust (https://faust.grame.fr)

#include "beat.h"

namespace digital_delay {

class Dsp: public PluginDef {
private:
	int fSampleRate;
	FAUSTFLOAT fVslider0;
	FAUSTFLOAT fCheckbox0;
	FAUSTFLOAT fVslider1;
	FAUSTFLOAT fHslider0;
	float fConst0;
	float fConst1;
	FAUSTFLOAT fHslider1;
	FAUSTFLOAT fHslider2;
	FAUSTFLOAT fVslider2;
	float fRec2[3];
	float fRec1[3];
	float fVec0[2];
	float fConst2;
	float fConst3;
	float fConst4;
	float fConst5;
	float fConst6;
	float fConst7;
	float fConst8;
	float fConst9;
	float fConst10;
	float fConst11;
	float fConst12;
	float fConst13;
	float fConst14;
	float fConst15;
	float fConst16;
	float fConst17;
	float fConst18;
	float fConst19;
	float fConst20;
	float fConst21;
	float fConst22;
	float fConst23;
	float fConst24;
	float fConst25;
	float fConst26;
	float fConst27;
	float fConst28;
	float fConst29;
	float fConst30;
	float fConst31;
	float fConst32;
	float fConst33;
	float fConst34;
	float fConst35;
	float fConst36;
	float fConst37;
	float fConst38;
	float fConst39;
	float fConst40;
	float fConst41;
	float fConst42;
	float fConst43;
	float fConst44;
	float fConst45;
	float fConst46;
	float fRec11[2];
	float fConst47;
	float fConst48;
	float fConst49;
	float fRec10[3];
	float fConst50;
	float fRec9[3];
	float fConst51;
	float fConst52;
	float fConst53;
	float fConst54;
	float fRec8[3];
	float fConst55;
	float fConst56;
	float fConst57;
	float fConst58;
	float fRec7[3];
	float fConst59;
	float fConst60;
	float fConst61;
	float fConst62;
	float fRec6[3];
	float fConst63;
	float fConst64;
	float fConst65;
	float fConst66;
	float fRec5[3];
	float fConst67;
	float fConst68;
	float fConst69;
	float fVec1[2];
	float fRec4[2];
	float fConst70;
	float fConst71;
	float fConst72;
	float fRec3[3];
	float fRec18[2];
	float fRec16[2];
	float fRec14[2];
	float fRec12[2];
	float fRec26[2];
	float fRec24[2];
	float fRec22[2];
	float fRec20[2];
	int IOTA0;
	float *fVec2;
	FAUSTFLOAT fHslider3;
	float fConst73;
	FAUSTFLOAT fHslider4;
	float fConst74;
	float fRec28[2];
	float fRec29[2];
	float fRec30[2];
	float fRec31[2];
	float fRec0[2];

	bool mem_allocated;
	void mem_alloc();
	void mem_free();
	void clear_state_f();
	int activate(bool start);
	int load_ui_f(const UiBuilder& b, int form);
	static const char *glade_def;
	void init(unsigned int sample_rate);
	void compute(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0);
	int register_par(const ParamReg& reg);

	static void clear_state_f_static(PluginDef*);
	static int activate_static(bool start, PluginDef*);
	static int load_ui_f_static(const UiBuilder& b, int form);
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
	  fVec2(0),
	  mem_allocated(false) {
	version = PLUGINDEF_VERSION;
	flags = 0;
	id = "dide";
	name = N_("Digital Delay");
	groups = 0;
	description = N_("Digital Delay"); // description (tooltip)
	category = N_("Echo / Delay");       // category
	shortname = N_("Digi Delay");     // shortname
	mono_audio = compute_static;
	stereo_audio = 0;
	set_samplerate = init_static;
	activate_plugin = activate_static;
	register_params = register_params_static;
	load_ui = load_ui_f_static;
	clear_state = clear_state_f_static;
	delete_instance = del_instance;
}

Dsp::~Dsp() {
}

inline void Dsp::clear_state_f()
{
	for (int l0 = 0; l0 < 3; l0 = l0 + 1) fRec2[l0] = 0.0f;
	for (int l1 = 0; l1 < 3; l1 = l1 + 1) fRec1[l1] = 0.0f;
	for (int l2 = 0; l2 < 2; l2 = l2 + 1) fVec0[l2] = 0.0f;
	for (int l3 = 0; l3 < 2; l3 = l3 + 1) fRec11[l3] = 0.0f;
	for (int l4 = 0; l4 < 3; l4 = l4 + 1) fRec10[l4] = 0.0f;
	for (int l5 = 0; l5 < 3; l5 = l5 + 1) fRec9[l5] = 0.0f;
	for (int l6 = 0; l6 < 3; l6 = l6 + 1) fRec8[l6] = 0.0f;
	for (int l7 = 0; l7 < 3; l7 = l7 + 1) fRec7[l7] = 0.0f;
	for (int l8 = 0; l8 < 3; l8 = l8 + 1) fRec6[l8] = 0.0f;
	for (int l9 = 0; l9 < 3; l9 = l9 + 1) fRec5[l9] = 0.0f;
	for (int l10 = 0; l10 < 2; l10 = l10 + 1) fVec1[l10] = 0.0f;
	for (int l11 = 0; l11 < 2; l11 = l11 + 1) fRec4[l11] = 0.0f;
	for (int l12 = 0; l12 < 3; l12 = l12 + 1) fRec3[l12] = 0.0f;
	for (int l13 = 0; l13 < 2; l13 = l13 + 1) fRec18[l13] = 0.0f;
	for (int l14 = 0; l14 < 2; l14 = l14 + 1) fRec16[l14] = 0.0f;
	for (int l15 = 0; l15 < 2; l15 = l15 + 1) fRec14[l15] = 0.0f;
	for (int l16 = 0; l16 < 2; l16 = l16 + 1) fRec12[l16] = 0.0f;
	for (int l17 = 0; l17 < 2; l17 = l17 + 1) fRec26[l17] = 0.0f;
	for (int l18 = 0; l18 < 2; l18 = l18 + 1) fRec24[l18] = 0.0f;
	for (int l19 = 0; l19 < 2; l19 = l19 + 1) fRec22[l19] = 0.0f;
	for (int l20 = 0; l20 < 2; l20 = l20 + 1) fRec20[l20] = 0.0f;
	for (int l21 = 0; l21 < 1048576; l21 = l21 + 1) fVec2[l21] = 0.0f;
	for (int l22 = 0; l22 < 2; l22 = l22 + 1) fRec28[l22] = 0.0f;
	for (int l23 = 0; l23 < 2; l23 = l23 + 1) fRec29[l23] = 0.0f;
	for (int l24 = 0; l24 < 2; l24 = l24 + 1) fRec30[l24] = 0.0f;
	for (int l25 = 0; l25 < 2; l25 = l25 + 1) fRec31[l25] = 0.0f;
	for (int l26 = 0; l26 < 2; l26 = l26 + 1) fRec0[l26] = 0.0f;
}

void Dsp::clear_state_f_static(PluginDef *p)
{
	static_cast<Dsp*>(p)->clear_state_f();
}

inline void Dsp::init(unsigned int sample_rate)
{
	fSampleRate = sample_rate;
	fConst0 = std::min<float>(1.92e+05f, std::max<float>(1.0f, static_cast<float>(fSampleRate)));
	fConst1 = 3.1415927f / fConst0;
	fConst2 = std::tan(37699.113f / fConst0);
	fConst3 = 1.0f / fConst2;
	fConst4 = (fConst3 + 1.0f) / fConst2 + 1.0f;
	fConst5 = 0.8f / fConst4;
	fConst6 = 1.0f / (fConst3 + 1.0f);
	fConst7 = 1.0f - fConst3;
	fConst8 = 25132.742f / fConst0;
	fConst9 = std::tan(fConst8);
	fConst10 = 1.0f / fConst9;
	fConst11 = fConst0 * std::sin(50265.484f / fConst0);
	fConst12 = 3141.5928f / fConst11;
	fConst13 = 1.0f / ((fConst10 + fConst12) / fConst9 + 1.0f);
	fConst14 = 2.0f * (1.0f - 1.0f / mydsp_faustpower2_f(fConst9));
	fConst15 = 12566.371f / fConst0;
	fConst16 = std::tan(fConst15);
	fConst17 = 1.0f / fConst16;
	fConst18 = fConst0 * std::sin(fConst8);
	fConst19 = 6268.3013f / fConst18;
	fConst20 = 1.0f / ((fConst17 + fConst19) / fConst16 + 1.0f);
	fConst21 = 2.0f * (1.0f - 1.0f / mydsp_faustpower2_f(fConst16));
	fConst22 = std::tan(6283.1855f / fConst0);
	fConst23 = 1.0f / fConst22;
	fConst24 = fConst0 * std::sin(fConst15);
	fConst25 = 1570.7964f / fConst24;
	fConst26 = 1.0f / ((fConst23 + fConst25) / fConst22 + 1.0f);
	fConst27 = 2.0f * (1.0f - 1.0f / mydsp_faustpower2_f(fConst22));
	fConst28 = std::tan(1178.0973f / fConst0);
	fConst29 = 1.0f / fConst28;
	fConst30 = fConst0 * std::sin(2356.1946f / fConst0);
	fConst31 = 392.6991f / fConst30;
	fConst32 = 1.0f / ((fConst29 + fConst31) / fConst28 + 1.0f);
	fConst33 = 2.0f * (1.0f - 1.0f / mydsp_faustpower2_f(fConst28));
	fConst34 = std::tan(628.31854f / fConst0);
	fConst35 = 1.0f / fConst34;
	fConst36 = fConst0 * std::sin(1256.6371f / fConst0);
	fConst37 = 221.88087f / fConst36;
	fConst38 = 1.0f / ((fConst35 + fConst37) / fConst34 + 1.0f);
	fConst39 = 2.0f * (1.0f - 1.0f / mydsp_faustpower2_f(fConst34));
	fConst40 = std::tan(251.32741f / fConst0);
	fConst41 = mydsp_faustpower2_f(fConst40);
	fConst42 = 1.0f / fConst40;
	fConst43 = (fConst42 + 1.0f) / fConst40 + 1.0f;
	fConst44 = 1.0f / (fConst41 * fConst43);
	fConst45 = 1.0f / (fConst42 + 1.0f);
	fConst46 = 1.0f - fConst42;
	fConst47 = 1.0f / fConst43;
	fConst48 = (fConst42 + -1.0f) / fConst40 + 1.0f;
	fConst49 = 2.0f * (1.0f - 1.0f / fConst41);
	fConst50 = (fConst35 - fConst37) / fConst34 + 1.0f;
	fConst51 = 157.07964f / fConst36;
	fConst52 = (fConst35 + fConst51) / fConst34 + 1.0f;
	fConst53 = (fConst35 - fConst51) / fConst34 + 1.0f;
	fConst54 = (fConst29 - fConst31) / fConst28 + 1.0f;
	fConst55 = 466.72372f / fConst30;
	fConst56 = (fConst29 + fConst55) / fConst28 + 1.0f;
	fConst57 = (fConst29 - fConst55) / fConst28 + 1.0f;
	fConst58 = (fConst23 - fConst25) / fConst22 + 1.0f;
	fConst59 = 2218.8088f / fConst24;
	fConst60 = (fConst23 + fConst59) / fConst22 + 1.0f;
	fConst61 = (fConst23 - fConst59) / fConst22 + 1.0f;
	fConst62 = (fConst17 - fConst19) / fConst16 + 1.0f;
	fConst63 = 3141.5928f / fConst18;
	fConst64 = (fConst17 + fConst63) / fConst16 + 1.0f;
	fConst65 = (fConst17 - fConst63) / fConst16 + 1.0f;
	fConst66 = (fConst10 - fConst12) / fConst9 + 1.0f;
	fConst67 = 3955.0308f / fConst11;
	fConst68 = (fConst10 + fConst67) / fConst9 + 1.0f;
	fConst69 = (fConst10 - fConst67) / fConst9 + 1.0f;
	fConst70 = 1.0f / fConst4;
	fConst71 = (fConst3 + -1.0f) / fConst2 + 1.0f;
	fConst72 = 2.0f * (1.0f - 1.0f / mydsp_faustpower2_f(fConst2));
	fConst73 = 6e+01f * fConst0;
	fConst74 = 1e+01f / fConst0;
	IOTA0 = 0;
}

void Dsp::init_static(unsigned int sample_rate, PluginDef *p)
{
	static_cast<Dsp*>(p)->init(sample_rate);
}

void Dsp::mem_alloc()
{
	if (!fVec2) fVec2 = new float[1048576];
	mem_allocated = true;
}

void Dsp::mem_free()
{
	mem_allocated = false;
	if (fVec2) { delete[] fVec2; fVec2 = 0; }
}

int Dsp::activate(bool start)
{
	if (start) {
		if (!mem_allocated) {
			mem_alloc();
			clear_state_f();
		}
	} else if (mem_allocated) {
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
	float fSlow0 = 0.01f * static_cast<float>(fVslider0);
	float fSlow1 = static_cast<float>(fCheckbox0);
	int iSlow2 = static_cast<int>(fSlow1);
	float fSlow3 = 0.01f * static_cast<float>(fVslider1);
	float fSlow4 = static_cast<float>(fHslider0);
	float fSlow5 = std::min<float>(2.0f, std::max<float>(0.0f, fSlow4 + -2.0f));
	int iSlow6 = fSlow5 == 0.0f;
	int iSlow7 = fSlow5 == 1.0f;
	float fSlow8 = std::min<float>(2.0f, std::max<float>(0.0f, fSlow4 + -1.0f));
	int iSlow9 = fSlow8 == 0.0f;
	int iSlow10 = fSlow8 == 1.0f;
	float fSlow11 = std::min<float>(2.0f, std::max<float>(0.0f, fSlow4));
	int iSlow12 = fSlow11 == 0.0f;
	int iSlow13 = fSlow11 == 1.0f;
	float fSlow14 = std::tan(fConst1 * static_cast<float>(fHslider1));
	float fSlow15 = 1.0f / fSlow14;
	float fSlow16 = 1.0f / ((fSlow15 + 1.4142135f) / fSlow14 + 1.0f);
	float fSlow17 = std::tan(fConst1 * static_cast<float>(fHslider2));
	float fSlow18 = mydsp_faustpower2_f(fSlow17);
	float fSlow19 = 1.0f / fSlow17;
	float fSlow20 = (fSlow19 + 1.4142135f) / fSlow17 + 1.0f;
	float fSlow21 = 1.0f / (fSlow18 * fSlow20);
	float fSlow22 = 0.01f * static_cast<float>(fVslider2);
	float fSlow23 = 1.0f / fSlow20;
	float fSlow24 = (fSlow19 + -1.4142135f) / fSlow17 + 1.0f;
	float fSlow25 = 2.0f * (1.0f - 1.0f / fSlow18);
	float fSlow26 = (fSlow15 + -1.4142135f) / fSlow14 + 1.0f;
	float fSlow27 = 2.0f * (1.0f - 1.0f / mydsp_faustpower2_f(fSlow14));
	float fSlow28 = 1.0f - fSlow1;
	float fSlow29 = std::min<float>(524288.0f, B2N(static_cast<int>(static_cast<float>(fHslider3)), fConst73 / static_cast<float>(fHslider4)));
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		float fTemp0 = static_cast<float>(input0[i0]);
		fRec2[0] = fSlow22 * fRec0[1] - fSlow23 * (fSlow24 * fRec2[2] + fSlow25 * fRec2[1]);
		fRec1[0] = fSlow21 * (fRec2[2] + (fRec2[0] - 2.0f * fRec2[1])) - fSlow16 * (fSlow26 * fRec1[2] + fSlow27 * fRec1[1]);
		float fTemp1 = fSlow16 * (fRec1[2] + fRec1[0] + 2.0f * fRec1[1]);
		fVec0[0] = fTemp1;
		fRec11[0] = -(fConst45 * (fConst46 * fRec11[1] - fConst42 * (fTemp1 - fVec0[1])));
		fRec10[0] = fRec11[0] - fConst47 * (fConst48 * fRec10[2] + fConst49 * fRec10[1]);
		float fTemp2 = fConst39 * fRec9[1];
		fRec9[0] = fConst44 * (fRec10[2] + (fRec10[0] - 2.0f * fRec10[1])) - fConst38 * (fConst50 * fRec9[2] + fTemp2);
		float fTemp3 = fConst33 * fRec8[1];
		fRec8[0] = fConst38 * (fTemp2 + fConst52 * fRec9[0] + fConst53 * fRec9[2]) - fConst32 * (fConst54 * fRec8[2] + fTemp3);
		float fTemp4 = fConst27 * fRec7[1];
		fRec7[0] = fConst32 * (fTemp3 + fConst56 * fRec8[0] + fConst57 * fRec8[2]) - fConst26 * (fConst58 * fRec7[2] + fTemp4);
		float fTemp5 = fConst21 * fRec6[1];
		fRec6[0] = fConst26 * (fTemp4 + fConst60 * fRec7[0] + fConst61 * fRec7[2]) - fConst20 * (fConst62 * fRec6[2] + fTemp5);
		float fTemp6 = fConst14 * fRec5[1];
		fRec5[0] = fConst20 * (fTemp5 + fConst64 * fRec6[0] + fConst65 * fRec6[2]) - fConst13 * (fConst66 * fRec5[2] + fTemp6);
		float fTemp7 = fTemp6 + fConst68 * fRec5[0] + fConst69 * fRec5[2];
		fVec1[0] = fTemp7;
		fRec4[0] = -(fConst6 * (fConst7 * fRec4[1] - fConst13 * (fTemp7 + fVec1[1])));
		fRec3[0] = fRec4[0] - fConst70 * (fConst71 * fRec3[2] + fConst72 * fRec3[1]);
		float fTemp8 = ((iSlow12) ? fTemp1 : ((iSlow13) ? fConst5 * (fRec3[2] + fRec3[0] + 2.0f * fRec3[1]) : fTemp1));
		float fTemp9 = 0.2f * fRec14[1];
		float fTemp10 = fTemp9 + fTemp8 + 0.6f * fRec18[1];
		float fTemp11 = 0.1f * fRec12[1];
		float fTemp12 = 0.4f * fRec16[1] + fTemp11;
		fRec18[0] = fTemp10 - fTemp12;
		float fRec19 = 0.6f * (fTemp12 - fTemp10);
		fRec16[0] = fRec19 + fRec18[1];
		float fTemp13 = fTemp8 + fTemp9;
		float fRec17 = 0.4f * (fTemp13 - fTemp12);
		fRec14[0] = fRec17 + fRec16[1];
		float fRec15 = 0.2f * (fTemp11 - fTemp13);
		fRec12[0] = fRec15 + fRec14[1];
		float fRec13 = 0.1f * (fTemp8 - fTemp11);
		float fTemp14 = ((iSlow9) ? fTemp8 : ((iSlow10) ? fRec13 + fRec12[1] : fTemp8));
		float fTemp15 = 0.3f * fRec24[1];
		float fTemp16 = fTemp15 + fTemp14 + 0.2f * fRec26[1];
		float fTemp17 = 0.5f * fRec20[1];
		float fTemp18 = 0.4f * fRec22[1] + fTemp17;
		fRec26[0] = fTemp16 - fTemp18;
		float fRec27 = 0.2f * (fTemp18 - fTemp16);
		fRec24[0] = fRec27 + fRec26[1];
		float fRec25 = 0.3f * (fTemp18 - (fTemp14 + fTemp15));
		fRec22[0] = fRec25 + fRec24[1];
		float fRec23 = 0.4f * (fTemp14 - fTemp18);
		fRec20[0] = fRec23 + fRec22[1];
		float fRec21 = 0.5f * (fTemp14 - fTemp17);
		float fTemp19 = ((iSlow2) ? fRec0[1] : ((iSlow6) ? fTemp14 : ((iSlow7) ? fRec21 + fRec20[1] : fTemp14))) + fSlow28 * fTemp0;
		fVec2[IOTA0 & 1048575] = fTemp19;
		float fTemp20 = ((fRec28[1] != 0.0f) ? (((fRec29[1] > 0.0f) & (fRec29[1] < 1.0f)) ? fRec28[1] : 0.0f) : (((fRec29[1] == 0.0f) & (fSlow29 != fRec30[1])) ? fConst74 : (((fRec29[1] == 1.0f) & (fSlow29 != fRec31[1])) ? -fConst74 : 0.0f)));
		fRec28[0] = fTemp20;
		fRec29[0] = std::max<float>(0.0f, std::min<float>(1.0f, fRec29[1] + fTemp20));
		fRec30[0] = (((fRec29[1] >= 1.0f) & (fRec31[1] != fSlow29)) ? fSlow29 : fRec30[1]);
		fRec31[0] = (((fRec29[1] <= 0.0f) & (fRec30[1] != fSlow29)) ? fSlow29 : fRec31[1]);
		float fTemp21 = fVec2[(IOTA0 - static_cast<int>(std::min<float>(524288.0f, std::max<float>(0.0f, fRec30[0])))) & 1048575];
		fRec0[0] = fTemp21 + fRec29[0] * (fVec2[(IOTA0 - static_cast<int>(std::min<float>(524288.0f, std::max<float>(0.0f, fRec31[0])))) & 1048575] - fTemp21);
		output0[i0] = static_cast<FAUSTFLOAT>(fTemp0 + fSlow0 * ((iSlow2) ? fRec0[0] : fSlow3 * fRec0[0]));
		fRec2[2] = fRec2[1];
		fRec2[1] = fRec2[0];
		fRec1[2] = fRec1[1];
		fRec1[1] = fRec1[0];
		fVec0[1] = fVec0[0];
		fRec11[1] = fRec11[0];
		fRec10[2] = fRec10[1];
		fRec10[1] = fRec10[0];
		fRec9[2] = fRec9[1];
		fRec9[1] = fRec9[0];
		fRec8[2] = fRec8[1];
		fRec8[1] = fRec8[0];
		fRec7[2] = fRec7[1];
		fRec7[1] = fRec7[0];
		fRec6[2] = fRec6[1];
		fRec6[1] = fRec6[0];
		fRec5[2] = fRec5[1];
		fRec5[1] = fRec5[0];
		fVec1[1] = fVec1[0];
		fRec4[1] = fRec4[0];
		fRec3[2] = fRec3[1];
		fRec3[1] = fRec3[0];
		fRec18[1] = fRec18[0];
		fRec16[1] = fRec16[0];
		fRec14[1] = fRec14[0];
		fRec12[1] = fRec12[0];
		fRec26[1] = fRec26[0];
		fRec24[1] = fRec24[0];
		fRec22[1] = fRec22[0];
		fRec20[1] = fRec20[0];
		IOTA0 = IOTA0 + 1;
		fRec28[1] = fRec28[0];
		fRec29[1] = fRec29[0];
		fRec30[1] = fRec30[0];
		fRec31[1] = fRec31[0];
		fRec0[1] = fRec0[0];
	}
}

void __rt_func Dsp::compute_static(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0, PluginDef *p)
{
	static_cast<Dsp*>(p)->compute(count, input0, output0);
}

int Dsp::register_par(const ParamReg& reg)
{
	reg.registerFloatVar("dide.Freeze",N_("Freeze"),"B",N_("Freeze the current delay"),&fCheckbox0, 0.0, 0.0, 1.0, 1.0, 0);
	reg.registerFloatVar("dide.bpm",N_("BPM"),"S",N_("Delay in Beats per Minute"),&fHslider4, 1.2e+02f, 24.0f, 3.6e+02f, 1.0f, 0);
	reg.registerFloatVar("dide.feedback",N_("Feedback"),"S",N_("Percentage of the feedback level in the de.delay loop"),&fVslider2, 5e+01f, 1.0f, 1e+02f, 1.0f, 0);
	reg.registerFloatVar("dide.gain",N_("Gain"),"S",N_("Overall gain of the de.delay line in percent"),&fVslider0, 1e+02f, 0.0f, 1.2e+02f, 1.0f, 0);
	reg.registerFloatVar("dide.highpass",N_("Hipass"),"S",N_("Highpass filter frequency in the feedback loop"),&fHslider2, 1.2e+02f, 2e+01f, 2e+04f, 1.0f, 0);
	reg.registerFloatVar("dide.howpass",N_("Lopass"),"S",N_("Lowpass filter frequency in the feedback loop"),&fHslider1, 1.2e+04f, 2e+01f, 2e+04f, 1.0f, 0);
	reg.registerFloatVar("dide.level",N_("Level"),"S",N_("Percentage of the de.delay gain level"),&fVslider1, 5e+01f, 1.0f, 1e+02f, 1.0f, 0);
	static const value_pair fHslider0_values[] = {{"plain"},{"presence"},{"tape"},{"tape2"},{0}};
	reg.registerFloatVar("dide.mode",N_("Mode"),"S","",&fHslider0, 0.0f, 0.0f, 3.0f, 1.0f, fHslider0_values);
	static const value_pair fHslider3_values[] = {{"Dotted 1/2 note"},{"1/2 note"},{"1/2 note triplets"},{" Dotted 1/4 note"},{"1/4 note"},{"1/4 note triplets"},{"Dotted 1/8 note"},{"1/8 note"},{"1/8 note triplets"},{" Dotted 1/16 note"},{"1/16 note"},{"1/16 note triplets"},{"Dotted 1/32 note"},{"1/32 note"},{"1/32 note triplets"},{" Dotted 1/64 note"},{"1/64 note"},{"1/64 note triplets"},{0}};
	reg.registerFloatVar("dide.notes",N_("Notes"),"S",N_("Note setting for bpm"),&fHslider3, 4.0f, 0.0f, 17.0f, 1.0f, fHslider3_values);
	return 0;
}

int Dsp::register_params_static(const ParamReg& reg)
{
	return static_cast<Dsp*>(reg.plugin)->register_par(reg);
}

const char *Dsp::glade_def = "\
<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<!-- Generated with glade 3.22.1 -->\n\
<interface>\n\
  <requires lib=\"gtk+\" version=\"3.0\"/>\n\
  <requires lib=\"gxwidgets\" version=\"0.0\"/>\n\
  <object class=\"GtkWindow\" id=\"window1\">\n\
    <property name=\"can_focus\">False</property>\n\
    <child>\n\
      <placeholder/>\n\
    </child>\n\
    <child>\n\
      <object class=\"GtkBox\" id=\"vbox1\">\n\
        <property name=\"visible\">True</property>\n\
        <property name=\"can_focus\">False</property>\n\
        <property name=\"orientation\">vertical</property>\n\
        <child>\n\
          <object class=\"GtkBox\" id=\"rackbox\">\n\
            <property name=\"visible\">True</property>\n\
            <property name=\"can_focus\">False</property>\n\
            <child>\n\
              <object class=\"GtkBox\" id=\"vbox2\">\n\
                <property name=\"visible\">True</property>\n\
                <property name=\"can_focus\">False</property>\n\
                <property name=\"orientation\">vertical</property>\n\
                <child>\n\
                  <object class=\"GtkLabel\" id=\"label1\">\n\
                    <property name=\"visible\">True</property>\n\
                    <property name=\"can_focus\">False</property>\n\
                  </object>\n\
                  <packing>\n\
                    <property name=\"expand\">False</property>\n\
                    <property name=\"fill\">False</property>\n\
                    <property name=\"position\">0</property>\n\
                  </packing>\n\
                </child>\n\
                <child>\n\
                  <object class=\"GtkLabel\" id=\"label1:rack_label\">\n\
                    <property name=\"visible\">True</property>\n\
                    <property name=\"can_focus\">False</property>\n\
                    <property name=\"label\" translatable=\"yes\">label</property>\n\
                  </object>\n\
                  <packing>\n\
                    <property name=\"expand\">False</property>\n\
                    <property name=\"fill\">False</property>\n\
                    <property name=\"position\">1</property>\n\
                  </packing>\n\
                </child>\n\
                <child>\n\
                  <object class=\"GxMidKnob\" id=\"GxMidKnob1\">\n\
                    <property name=\"visible\">True</property>\n\
                    <property name=\"can_focus\">True</property>\n\
                    <property name=\"receives_default\">True</property>\n\
                    <property name=\"var_id\">dide.bpm</property>\n\
                    <property name=\"label_ref\">label1:rack_label</property>\n\
                  </object>\n\
                  <packing>\n\
                    <property name=\"expand\">True</property>\n\
                    <property name=\"fill\">True</property>\n\
                    <property name=\"position\">2</property>\n\
                  </packing>\n\
                </child>\n\
                <child>\n\
                  <object class=\"GtkLabel\" id=\"label2\">\n\
                    <property name=\"visible\">True</property>\n\
                    <property name=\"can_focus\">False</property>\n\
                  </object>\n\
                  <packing>\n\
                    <property name=\"expand\">False</property>\n\
                    <property name=\"fill\">False</property>\n\
                    <property name=\"position\">3</property>\n\
                  </packing>\n\
                </child>\n\
              </object>\n\
              <packing>\n\
                <property name=\"expand\">True</property>\n\
                <property name=\"fill\">True</property>\n\
                <property name=\"position\">0</property>\n\
              </packing>\n\
            </child>\n\
            <child>\n\
              <object class=\"GtkBox\" id=\"vbox8\">\n\
                <property name=\"visible\">True</property>\n\
                <property name=\"can_focus\">False</property>\n\
                <property name=\"orientation\">vertical</property>\n\
                <property name=\"spacing\">10</property>\n\
                <child>\n\
                  <object class=\"GtkBox\" id=\"hbox1\">\n\
                    <property name=\"visible\">True</property>\n\
                    <property name=\"can_focus\">False</property>\n\
                    <child>\n\
                      <object class=\"GtkBox\" id=\"vbox3\">\n\
                        <property name=\"visible\">True</property>\n\
                        <property name=\"can_focus\">False</property>\n\
                        <property name=\"orientation\">vertical</property>\n\
                        <child>\n\
                          <object class=\"GtkLabel\" id=\"label2:rack_label\">\n\
                            <property name=\"visible\">True</property>\n\
                            <property name=\"can_focus\">False</property>\n\
                            <property name=\"label\" translatable=\"yes\">label</property>\n\
                          </object>\n\
                          <packing>\n\
                            <property name=\"expand\">False</property>\n\
                            <property name=\"fill\">False</property>\n\
                            <property name=\"position\">0</property>\n\
                          </packing>\n\
                        </child>\n\
                        <child>\n\
                          <object class=\"GxSmallKnobR\" id=\"GxMidKnob2\">\n\
                            <property name=\"visible\">True</property>\n\
                            <property name=\"can_focus\">True</property>\n\
                            <property name=\"receives_default\">True</property>\n\
                            <property name=\"var_id\">dide.feedback</property>\n\
                            <property name=\"label_ref\">label2:rack_label</property>\n\
                          </object>\n\
                          <packing>\n\
                            <property name=\"expand\">False</property>\n\
                            <property name=\"fill\">False</property>\n\
                            <property name=\"position\">1</property>\n\
                          </packing>\n\
                        </child>\n\
                      </object>\n\
                      <packing>\n\
                        <property name=\"expand\">True</property>\n\
                        <property name=\"fill\">True</property>\n\
                        <property name=\"position\">0</property>\n\
                      </packing>\n\
                    </child>\n\
                    <child>\n\
                      <object class=\"GtkBox\" id=\"vbox4\">\n\
                        <property name=\"visible\">True</property>\n\
                        <property name=\"can_focus\">False</property>\n\
                        <property name=\"orientation\">vertical</property>\n\
                        <child>\n\
                          <object class=\"GtkLabel\" id=\"label3:rack_label\">\n\
                            <property name=\"visible\">True</property>\n\
                            <property name=\"can_focus\">False</property>\n\
                            <property name=\"label\" translatable=\"yes\">label</property>\n\
                          </object>\n\
                          <packing>\n\
                            <property name=\"expand\">False</property>\n\
                            <property name=\"fill\">False</property>\n\
                            <property name=\"position\">0</property>\n\
                          </packing>\n\
                        </child>\n\
                        <child>\n\
                          <object class=\"GxSmallKnobR\" id=\"GxMidKnob3\">\n\
                            <property name=\"visible\">True</property>\n\
                            <property name=\"can_focus\">True</property>\n\
                            <property name=\"receives_default\">True</property>\n\
                            <property name=\"var_id\">dide.level</property>\n\
                            <property name=\"label_ref\">label3:rack_label</property>\n\
                          </object>\n\
                          <packing>\n\
                            <property name=\"expand\">False</property>\n\
                            <property name=\"fill\">False</property>\n\
                            <property name=\"position\">1</property>\n\
                          </packing>\n\
                        </child>\n\
                      </object>\n\
                      <packing>\n\
                        <property name=\"expand\">True</property>\n\
                        <property name=\"fill\">True</property>\n\
                        <property name=\"position\">1</property>\n\
                      </packing>\n\
                    </child>\n\
                    <child>\n\
                      <object class=\"GtkBox\" id=\"vbox5\">\n\
                        <property name=\"visible\">True</property>\n\
                        <property name=\"can_focus\">False</property>\n\
                        <property name=\"orientation\">vertical</property>\n\
                        <child>\n\
                          <object class=\"GtkLabel\" id=\"label4:rack_label\">\n\
                            <property name=\"visible\">True</property>\n\
                            <property name=\"can_focus\">False</property>\n\
                            <property name=\"label\" translatable=\"yes\">label</property>\n\
                          </object>\n\
                          <packing>\n\
                            <property name=\"expand\">False</property>\n\
                            <property name=\"fill\">False</property>\n\
                            <property name=\"position\">0</property>\n\
                          </packing>\n\
                        </child>\n\
                        <child>\n\
                          <object class=\"GxSmallKnobR\" id=\"GxMidKnob4\">\n\
                            <property name=\"visible\">True</property>\n\
                            <property name=\"can_focus\">True</property>\n\
                            <property name=\"receives_default\">True</property>\n\
                            <property name=\"var_id\">dide.highpass</property>\n\
                            <property name=\"label_ref\">label4:rack_label</property>\n\
                          </object>\n\
                          <packing>\n\
                            <property name=\"expand\">False</property>\n\
                            <property name=\"fill\">False</property>\n\
                            <property name=\"position\">1</property>\n\
                          </packing>\n\
                        </child>\n\
                      </object>\n\
                      <packing>\n\
                        <property name=\"expand\">True</property>\n\
                        <property name=\"fill\">True</property>\n\
                        <property name=\"position\">2</property>\n\
                      </packing>\n\
                    </child>\n\
                    <child>\n\
                      <object class=\"GtkBox\" id=\"vbox6\">\n\
                        <property name=\"visible\">True</property>\n\
                        <property name=\"can_focus\">False</property>\n\
                        <property name=\"orientation\">vertical</property>\n\
                        <child>\n\
                          <object class=\"GtkLabel\" id=\"label5:rack_label\">\n\
                            <property name=\"visible\">True</property>\n\
                            <property name=\"can_focus\">False</property>\n\
                            <property name=\"label\" translatable=\"yes\">label</property>\n\
                          </object>\n\
                          <packing>\n\
                            <property name=\"expand\">False</property>\n\
                            <property name=\"fill\">False</property>\n\
                            <property name=\"position\">0</property>\n\
                          </packing>\n\
                        </child>\n\
                        <child>\n\
                          <object class=\"GxSmallKnobR\" id=\"GxMidKnob5\">\n\
                            <property name=\"visible\">True</property>\n\
                            <property name=\"can_focus\">True</property>\n\
                            <property name=\"receives_default\">True</property>\n\
                            <property name=\"var_id\">dide.howpass</property>\n\
                            <property name=\"label_ref\">label5:rack_label</property>\n\
                          </object>\n\
                          <packing>\n\
                            <property name=\"expand\">False</property>\n\
                            <property name=\"fill\">False</property>\n\
                            <property name=\"position\">1</property>\n\
                          </packing>\n\
                        </child>\n\
                      </object>\n\
                      <packing>\n\
                        <property name=\"expand\">True</property>\n\
                        <property name=\"fill\">True</property>\n\
                        <property name=\"position\">3</property>\n\
                      </packing>\n\
                    </child>\n\
                  </object>\n\
                  <packing>\n\
                    <property name=\"expand\">True</property>\n\
                    <property name=\"fill\">True</property>\n\
                    <property name=\"position\">0</property>\n\
                  </packing>\n\
                </child>\n\
                <child>\n\
                  <object class=\"GtkGrid\" id=\"table1\">\n\
                    <property name=\"visible\">True</property>\n\
                    <property name=\"can_focus\">False</property>\n\
                    <property name=\"row_spacing\">3</property>\n\
                    <property name=\"column_spacing\">6</property>\n\
                    <child>\n\
                      <object class=\"GxSelector\" id=\"gxselector2\">\n\
                        <property name=\"visible\">True</property>\n\
                        <property name=\"can_focus\">True</property>\n\
                        <property name=\"receives_default\">True</property>\n\
                        <property name=\"hexpand\">True</property>\n\
                        <property name=\"var_id\">dide.mode</property>\n\
                        <property name=\"label_ref\">label21:rack_label_inverse</property>\n\
                      </object>\n\
                      <packing>\n\
                        <property name=\"left_attach\">1</property>\n\
                        <property name=\"top_attach\">1</property>\n\
                      </packing>\n\
                    </child>\n\
                    <child>\n\
                      <object class=\"GtkLabel\" id=\"label11:rack_label_inverse\">\n\
                        <property name=\"visible\">True</property>\n\
                        <property name=\"can_focus\">False</property>\n\
                        <property name=\"label\" translatable=\"yes\">label</property>\n\
                        <property name=\"xalign\">1</property>\n\
                      </object>\n\
                      <packing>\n\
                        <property name=\"left_attach\">0</property>\n\
                        <property name=\"top_attach\">0</property>\n\
                      </packing>\n\
                    </child>\n\
                    <child>\n\
                      <object class=\"GtkLabel\" id=\"label21:rack_label_inverse\">\n\
                        <property name=\"visible\">True</property>\n\
                        <property name=\"can_focus\">False</property>\n\
                        <property name=\"label\" translatable=\"yes\">label</property>\n\
                        <property name=\"xalign\">1</property>\n\
                      </object>\n\
                      <packing>\n\
                        <property name=\"left_attach\">0</property>\n\
                        <property name=\"top_attach\">1</property>\n\
                      </packing>\n\
                    </child>\n\
                    <child>\n\
                      <object class=\"GxSelector\" id=\"gxselector1\">\n\
                        <property name=\"visible\">True</property>\n\
                        <property name=\"can_focus\">True</property>\n\
                        <property name=\"receives_default\">True</property>\n\
                        <property name=\"hexpand\">True</property>\n\
                        <property name=\"var_id\">dide.notes</property>\n\
                        <property name=\"label_ref\">label11:rack_label_inverse</property>\n\
                      </object>\n\
                      <packing>\n\
                        <property name=\"left_attach\">1</property>\n\
                        <property name=\"top_attach\">0</property>\n\
                        <property name=\"width\">3</property>\n\
                      </packing>\n\
                    </child>\n\
                    <child>\n\
                      <object class=\"GtkLabel\" id=\"label31:rack_label_inverse\">\n\
                        <property name=\"visible\">True</property>\n\
                        <property name=\"can_focus\">False</property>\n\
                        <property name=\"label\" translatable=\"yes\">label</property>\n\
                        <property name=\"xalign\">1</property>\n\
                      </object>\n\
                      <packing>\n\
                        <property name=\"left_attach\">2</property>\n\
                        <property name=\"top_attach\">1</property>\n\
                      </packing>\n\
                    </child>\n\
                    <child>\n\
                      <object class=\"GxSwitch\" id=\"gxswitch1\">\n\
                        <property name=\"visible\">True</property>\n\
                        <property name=\"can_focus\">True</property>\n\
                        <property name=\"receives_default\">True</property>\n\
                        <property name=\"var_id\">dide.Freeze</property>\n\
                        <property name=\"label_ref\">label31:rack_label_inverse</property>\n\
                        <property name=\"base_name\">frbutton</property>\n\
                      </object>\n\
                      <packing>\n\
                        <property name=\"left_attach\">3</property>\n\
                        <property name=\"top_attach\">1</property>\n\
                      </packing>\n\
                    </child>\n\
                  </object>\n\
                  <packing>\n\
                    <property name=\"expand\">False</property>\n\
                    <property name=\"fill\">False</property>\n\
                    <property name=\"position\">1</property>\n\
                  </packing>\n\
                </child>\n\
              </object>\n\
              <packing>\n\
                <property name=\"expand\">True</property>\n\
                <property name=\"fill\">True</property>\n\
                <property name=\"position\">1</property>\n\
              </packing>\n\
            </child>\n\
            <child>\n\
              <object class=\"GtkBox\" id=\"vbox7\">\n\
                <property name=\"visible\">True</property>\n\
                <property name=\"can_focus\">False</property>\n\
                <property name=\"orientation\">vertical</property>\n\
                <child>\n\
                  <object class=\"GtkLabel\" id=\"label6:rack_label1\">\n\
                    <property name=\"visible\">True</property>\n\
                    <property name=\"can_focus\">False</property>\n\
                  </object>\n\
                  <packing>\n\
                    <property name=\"expand\">False</property>\n\
                    <property name=\"fill\">False</property>\n\
                    <property name=\"position\">0</property>\n\
                  </packing>\n\
                </child>\n\
                <child>\n\
                  <object class=\"GtkLabel\" id=\"label6:rack_label\">\n\
                    <property name=\"visible\">True</property>\n\
                    <property name=\"can_focus\">False</property>\n\
                    <property name=\"label\" translatable=\"yes\">label</property>\n\
                  </object>\n\
                  <packing>\n\
                    <property name=\"expand\">False</property>\n\
                    <property name=\"fill\">False</property>\n\
                    <property name=\"position\">1</property>\n\
                  </packing>\n\
                </child>\n\
                <child>\n\
                  <object class=\"GxMidKnob\" id=\"GxMidKnob6\">\n\
                    <property name=\"visible\">True</property>\n\
                    <property name=\"can_focus\">True</property>\n\
                    <property name=\"receives_default\">True</property>\n\
                    <property name=\"var_id\">dide.gain</property>\n\
                    <property name=\"label_ref\">label6:rack_label</property>\n\
                  </object>\n\
                  <packing>\n\
                    <property name=\"expand\">True</property>\n\
                    <property name=\"fill\">True</property>\n\
                    <property name=\"position\">2</property>\n\
                  </packing>\n\
                </child>\n\
                <child>\n\
                  <object class=\"GtkLabel\" id=\"label6:rack_label2\">\n\
                    <property name=\"visible\">True</property>\n\
                    <property name=\"can_focus\">False</property>\n\
                  </object>\n\
                  <packing>\n\
                    <property name=\"expand\">False</property>\n\
                    <property name=\"fill\">False</property>\n\
                    <property name=\"position\">3</property>\n\
                  </packing>\n\
                </child>\n\
              </object>\n\
              <packing>\n\
                <property name=\"expand\">True</property>\n\
                <property name=\"fill\">True</property>\n\
                <property name=\"position\">2</property>\n\
              </packing>\n\
            </child>\n\
          </object>\n\
          <packing>\n\
            <property name=\"expand\">True</property>\n\
            <property name=\"fill\">False</property>\n\
            <property name=\"position\">0</property>\n\
          </packing>\n\
        </child>\n\
        <child>\n\
          <object class=\"GtkBox\" id=\"minibox\">\n\
            <property name=\"visible\">True</property>\n\
            <property name=\"can_focus\">False</property>\n\
            <property name=\"spacing\">4</property>\n\
            <child>\n\
              <object class=\"GxHSlider\" id=\"gxhslider1\">\n\
                <property name=\"visible\">True</property>\n\
                <property name=\"can_focus\">True</property>\n\
                <property name=\"receives_default\">True</property>\n\
                <property name=\"round_digits\">0</property>\n\
                <property name=\"var_id\">dide.gain</property>\n\
                <property name=\"show_value\">False</property>\n\
                <property name=\"value_position\">right</property>\n\
                <property name=\"value_xalign\">0.52000000000000002</property>\n\
                <property name=\"label_ref\">label0:rack_label</property>\n\
              </object>\n\
              <packing>\n\
                <property name=\"expand\">False</property>\n\
                <property name=\"fill\">False</property>\n\
                <property name=\"position\">0</property>\n\
              </packing>\n\
            </child>\n\
            <child>\n\
              <object class=\"GtkLabel\" id=\"label0:rack_label\">\n\
                <property name=\"visible\">True</property>\n\
                <property name=\"can_focus\">False</property>\n\
                <property name=\"label\" translatable=\"yes\">Amount</property>\n\
                <property name=\"xalign\">0</property>\n\
              </object>\n\
              <packing>\n\
                <property name=\"expand\">False</property>\n\
                <property name=\"fill\">False</property>\n\
                <property name=\"position\">1</property>\n\
              </packing>\n\
            </child>\n\
          </object>\n\
          <packing>\n\
            <property name=\"expand\">True</property>\n\
            <property name=\"fill\">True</property>\n\
            <property name=\"position\">1</property>\n\
          </packing>\n\
        </child>\n\
      </object>\n\
    </child>\n\
  </object>\n\
</interface>\n\
";

inline int Dsp::load_ui_f(const UiBuilder& b, int form)
{
    if (form & UI_FORM_GLADE) {
        b.load_glade(glade_def);
        return 0;
    }
    if (form & UI_FORM_STACK) {
#define PARAM(p) ("dide" "." p)
// -----delay
b.openHorizontalhideBox("");
b.create_master_slider(PARAM("bpm"), _(" delay (bpm)"));
b.closeBox();
b.openVerticalBox("");
{
    b.openHorizontalBox("");
    {
        b.openVerticalBox("");
        {
            b.create_selector(PARAM("notes"), "tact");
            b.set_next_flags(UI_NUM_SHOW_ALWAYS|UI_NUM_TOP);
            b.create_small_rackknob(PARAM("bpm"), _(" delay (bpm)"));
        }
        b.closeBox();
        b.openVerticalBox("");
        {
            b.openHorizontalBox("");
            b.openFrameBox("");
            b.closeBox();
            b.create_selector(PARAM("mode"), "mode");
            b.closeBox();
            b.openHorizontalBox("");
            {
                b.set_next_flags(UI_NUM_SHOW_ALWAYS|UI_NUM_TOP);
                b.create_small_rackknob(PARAM("highpass"), _("highpass(hz)"));
                b.set_next_flags(UI_NUM_SHOW_ALWAYS|UI_NUM_TOP);
                b.create_small_rackknob(PARAM("howpass"), _("lowpass(hz)"));
            }
            b.closeBox();
        }
        b.closeBox();
        b.openVerticalBox("");
        {
            b.create_small_rackknob(PARAM("level"), _("level"));
            b.create_small_rackknob(PARAM("feedback"), _("feedback"));
        }
        b.closeBox();
        b.openVerticalBox("");
        {
            b.create_small_rackknobr(PARAM("gain"), _("amount"));
            b.openFrameBox("");
            b.closeBox();
            b.create_switch(sw_frbutton,PARAM("Freeze"), _("freeze"));
        }
        b.closeBox();
    }
    b.closeBox();
}
b.closeBox();

#undef PARAM
        return 0;
    }
	return -1;
}

int Dsp::load_ui_f_static(const UiBuilder& b, int form)
{
	return static_cast<Dsp*>(b.plugin)->load_ui_f(b, form);
}
PluginDef *plugin() {
	return new Dsp();
}

void Dsp::del_instance(PluginDef *p)
{
	delete static_cast<Dsp*>(p);
}

} // end namespace digital_delay

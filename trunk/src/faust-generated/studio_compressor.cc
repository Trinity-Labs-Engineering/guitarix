// generated from file '../src/faust/studio_compressor.dsp' by dsp2cc:
// Code generated with Faust (https://faust.grame.fr)


namespace studio_compressor {

class Dsp: public PluginDef {
private:
	int fSampleRate;
	FAUSTFLOAT fHslider0;
	double fConst0;
	double fConst1;
	int iVec0[2];
	double fRec0[2];
	FAUSTFLOAT fHslider1;
	double fRec2[2];
	FAUSTFLOAT fHslider2;
	double fRec3[2];
	FAUSTFLOAT fHslider3;
	double fRec4[2];
	double fConst2;
	FAUSTFLOAT fHslider4;
	double fRec5[2];
	FAUSTFLOAT fHslider5;
	double fRec6[2];
	double fRec1[2];
	FAUSTFLOAT fHslider6;
	double fRec7[2];

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
	id = "studio_compressor";
	name = N_("Studio Compressor");
	groups = 0;
	description = N_("Transparent feed-forward compression with a soft knee and parallel mix."); // description (tooltip)
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
	for (int l0 = 0; l0 < 2; l0 = l0 + 1) iVec0[l0] = 0;
	for (int l1 = 0; l1 < 2; l1 = l1 + 1) fRec0[l1] = 0.0;
	for (int l2 = 0; l2 < 2; l2 = l2 + 1) fRec2[l2] = 0.0;
	for (int l3 = 0; l3 < 2; l3 = l3 + 1) fRec3[l3] = 0.0;
	for (int l4 = 0; l4 < 2; l4 = l4 + 1) fRec4[l4] = 0.0;
	for (int l5 = 0; l5 < 2; l5 = l5 + 1) fRec5[l5] = 0.0;
	for (int l6 = 0; l6 < 2; l6 = l6 + 1) fRec6[l6] = 0.0;
	for (int l7 = 0; l7 < 2; l7 = l7 + 1) fRec1[l7] = 0.0;
	for (int l8 = 0; l8 < 2; l8 = l8 + 1) fRec7[l8] = 0.0;
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
	double fSlow5 = static_cast<double>(fHslider5);
	double fSlow6 = static_cast<double>(fHslider6);
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		double fTemp0 = static_cast<double>(input0[i0]);
		iVec0[0] = 1;
		double fTemp1 = static_cast<double>(iVec0[1]);
		double fTemp2 = 1.0 - fConst1 * fTemp1;
		fRec0[0] = fSlow0 * fTemp2 + fConst1 * fTemp1 * fRec0[1];
		fRec2[0] = fSlow1 * fTemp2 + fConst1 * fTemp1 * fRec2[1];
		double fTemp3 = 2e+01 * std::log10(std::max<double>(2.2250738585072014e-308, std::max<double>(1e-12, std::fabs(fTemp0))));
		fRec3[0] = fSlow2 * fTemp2 + fConst1 * fTemp1 * fRec3[1];
		fRec4[0] = fSlow3 * fTemp2 + fConst1 * fTemp1 * fRec4[1];
		double fTemp4 = 0.5 * fRec4[0];
		int iTemp5 = (fTemp3 > (fRec3[0] - fTemp4)) + (fTemp3 > (fRec3[0] + fTemp4));
		double fTemp6 = (1.0 - 1.0 / fRec2[0]) * std::max<double>(0.0, ((iTemp5 == 0) ? 0.0 : ((iTemp5 == 1) ? 0.5 * (mydsp_faustpower2_f(fTemp3 + fTemp4 - fRec3[0]) / std::max<double>(1e-06, fRec4[0])) : fTemp3 - fRec3[0])));
		fRec5[0] = fSlow4 * fTemp2 + fConst1 * fTemp1 * fRec5[1];
		double fTemp7 = std::max<double>(fConst2, 0.001 * fRec5[0]);
		int iTemp8 = std::fabs(fTemp7) < 2.220446049250313e-16;
		fRec6[0] = fSlow5 * fTemp2 + fConst1 * fTemp1 * fRec6[1];
		double fTemp9 = std::max<double>(fConst2, 0.001 * fRec6[0]);
		int iTemp10 = std::fabs(fTemp9) < 2.220446049250313e-16;
		double fTemp11 = ((-fTemp6 > fRec1[1]) ? ((iTemp10) ? 0.0 : std::exp(-(fConst2 / ((iTemp10) ? 1.0 : fTemp9)))) : ((iTemp8) ? 0.0 : std::exp(-(fConst2 / ((iTemp8) ? 1.0 : fTemp7)))));
		fRec1[0] = fRec1[1] * fTemp11 - fTemp6 * (1.0 - fTemp11);
		fRec7[0] = fSlow6 * fTemp2 + fConst1 * fTemp1 * fRec7[1];
		output0[i0] = static_cast<FAUSTFLOAT>(fTemp0 * (1.0 - 0.01 * fRec0[0] * (1.0 - std::pow(1e+01, 0.05 * fRec1[0]) * std::pow(1e+01, 0.05 * fRec7[0]))));
		iVec0[1] = iVec0[0];
		fRec0[1] = fRec0[0];
		fRec2[1] = fRec2[0];
		fRec3[1] = fRec3[0];
		fRec4[1] = fRec4[0];
		fRec5[1] = fRec5[0];
		fRec6[1] = fRec6[0];
		fRec1[1] = fRec1[0];
		fRec7[1] = fRec7[0];
	}
}

void __rt_func Dsp::compute_static(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0, PluginDef *p)
{
	static_cast<Dsp*>(p)->compute(count, input0, output0);
}

int Dsp::register_par(const ParamReg& reg)
{
	reg.registerFloatVar("studio_compressor.attack",N_("Attack"),"S","",&fHslider4, 1e+01, 0.1, 1e+02, 0.1, 0);
	reg.registerFloatVar("studio_compressor.knee",N_("Knee"),"S","",&fHslider3, 6.0, 0.0, 24.0, 0.1, 0);
	reg.registerFloatVar("studio_compressor.makeup",N_("Makeup"),"S","",&fHslider6, 0.0, -12.0, 24.0, 0.1, 0);
	reg.registerFloatVar("studio_compressor.mix",N_("Mix"),"S","",&fHslider0, 1e+02, 0.0, 1e+02, 1.0, 0);
	reg.registerFloatVar("studio_compressor.ratio",N_("Ratio"),"S","",&fHslider1, 3.0, 1.0, 2e+01, 0.1, 0);
	reg.registerFloatVar("studio_compressor.release",N_("Release"),"S","",&fHslider5, 1.5e+02, 2e+01, 2e+03, 1.0, 0);
	reg.registerFloatVar("studio_compressor.threshold",N_("Threshold"),"S","",&fHslider2, -24.0, -6e+01, 0.0, 0.1, 0);
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

} // end namespace studio_compressor

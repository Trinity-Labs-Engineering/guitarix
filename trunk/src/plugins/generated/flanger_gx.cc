// generated from file '../src/plugins/flanger_gx.dsp' by dsp2cc:
// Code generated with Faust (https://faust.grame.fr)

#include "gx_faust_support.h"
#include "gx_plugin.h"

namespace pluginlib {
namespace flanger_gx {

class Dsp: public PluginDef {
private:
	int fSampleRate;
	FAUSTFLOAT fHslider0;
	double fConst0;
	double fConst1;
	int iVec0[2];
	double fRec0[2];
	FAUSTFLOAT fHslider1;
	double fRec1[2];
	int IOTA0;
	double fVec1[2048];
	double fConst2;
	FAUSTFLOAT fHslider2;
	double fRec3[2];
	FAUSTFLOAT fHslider3;
	double fRec4[2];
	double fConst3;
	FAUSTFLOAT fHslider4;
	double fRec7[2];
	double fRec8[2];
	double fRec6[2];
	double fRec5[2];
	FAUSTFLOAT fHslider5;
	double fRec9[2];
	double fVec2[2048];
	double fRec2[2];
	double fVec3[4096];
	double fRec10[2];

	void clear_state_f();
	int load_ui_f(const UiBuilder& b, int form);
	static const char *glade_def;
	void init(unsigned int sample_rate);
	void compute(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0);
	int register_par(const ParamReg& reg);

	static void clear_state_f_static(PluginDef*);
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
	: PluginDef() {
	version = PLUGINDEF_VERSION;
	flags = 0;
	id = "flanger_mono_gx";
	name = N_("Flanger GX");
	groups = 0;
	description = ""; // description (tooltip)
	category = N_("Modulation");       // category
	shortname = "";     // shortname
	mono_audio = compute_static;
	stereo_audio = 0;
	set_samplerate = init_static;
	activate_plugin = 0;
	register_params = register_params_static;
	load_ui = load_ui_f_static;
	clear_state = clear_state_f_static;
	delete_instance = del_instance;
}

Dsp::~Dsp() {
}

inline void Dsp::clear_state_f()
{
	for (int l0 = 0; l0 < 2; l0 = l0 + 1) iVec0[l0] = 0;
	for (int l1 = 0; l1 < 2; l1 = l1 + 1) fRec0[l1] = 0.0;
	for (int l2 = 0; l2 < 2; l2 = l2 + 1) fRec1[l2] = 0.0;
	for (int l3 = 0; l3 < 2048; l3 = l3 + 1) fVec1[l3] = 0.0;
	for (int l4 = 0; l4 < 2; l4 = l4 + 1) fRec3[l4] = 0.0;
	for (int l5 = 0; l5 < 2; l5 = l5 + 1) fRec4[l5] = 0.0;
	for (int l6 = 0; l6 < 2; l6 = l6 + 1) fRec7[l6] = 0.0;
	for (int l7 = 0; l7 < 2; l7 = l7 + 1) fRec8[l7] = 0.0;
	for (int l8 = 0; l8 < 2; l8 = l8 + 1) fRec6[l8] = 0.0;
	for (int l9 = 0; l9 < 2; l9 = l9 + 1) fRec5[l9] = 0.0;
	for (int l10 = 0; l10 < 2; l10 = l10 + 1) fRec9[l10] = 0.0;
	for (int l11 = 0; l11 < 2048; l11 = l11 + 1) fVec2[l11] = 0.0;
	for (int l12 = 0; l12 < 2; l12 = l12 + 1) fRec2[l12] = 0.0;
	for (int l13 = 0; l13 < 4096; l13 = l13 + 1) fVec3[l13] = 0.0;
	for (int l14 = 0; l14 < 2; l14 = l14 + 1) fRec10[l14] = 0.0;
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
	fConst2 = 0.5 * fConst0;
	fConst3 = 0.10471975511965977 / fConst0;
	IOTA0 = 0;
	clear_state_f();
}

void Dsp::init_static(unsigned int sample_rate, PluginDef *p)
{
	static_cast<Dsp*>(p)->init(sample_rate);
}

void always_inline Dsp::compute(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0)
{
	double fSlow0 = 0.01 * static_cast<double>(fHslider0);
	double fSlow1 = static_cast<double>(fHslider1);
	double fSlow2 = static_cast<double>(fHslider2);
	double fSlow3 = static_cast<double>(fHslider3);
	double fSlow4 = static_cast<double>(fHslider4);
	double fSlow5 = std::min<double>(0.95, std::max<double>(-0.95, static_cast<double>(fHslider5)));
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		iVec0[0] = 1;
		double fTemp0 = static_cast<double>(iVec0[1]);
		double fTemp1 = 1.0 - fConst1 * fTemp0;
		fRec0[0] = fSlow0 * fTemp1 + fConst1 * fTemp0 * fRec0[1];
		fRec1[0] = fSlow1 * fTemp1 + fConst1 * fTemp0 * fRec1[1];
		double fTemp2 = std::min<double>(1.0, 1.0 - fRec1[0]);
		double fTemp3 = static_cast<double>(input0[i0]);
		fVec1[IOTA0 & 2047] = fTemp3;
		double fTemp4 = std::min<double>(1.0, fRec1[0] + 1.0);
		fRec3[0] = fSlow2 * fTemp1 + fConst1 * fTemp0 * fRec3[1];
		fRec4[0] = fSlow3 * fTemp1 + fConst1 * fTemp0 * fRec4[1];
		fRec7[0] = fSlow4 * fTemp1 + fConst1 * fTemp0 * fRec7[1];
		fRec8[0] = fRec8[1] - fConst3 * fRec7[0] * fRec5[1];
		fRec6[0] = fConst3 * fRec7[0] * fRec8[0] + static_cast<double>(1 - iVec0[1]) + fRec6[1];
		fRec5[0] = fRec6[0];
		double fTemp5 = 0.001 * fRec3[0] + 0.0005 * fRec4[0] * (fRec5[0] + 1.0);
		double fTemp6 = std::max<double>(2.0, std::min<double>(2044.0, fConst2 * fTemp5));
		double fTemp7 = fTemp6 + -0.999995;
		double fTemp8 = std::floor(fTemp7);
		double fTemp9 = fTemp6 + (-3.0 - fTemp8);
		double fTemp10 = fTemp6 + (-2.0 - fTemp8);
		double fTemp11 = fTemp6 - fTemp8;
		double fTemp12 = fTemp6 + (-1.0 - fTemp8);
		double fTemp13 = fTemp11 * fTemp12;
		double fTemp14 = fTemp13 * fTemp10;
		int iTemp15 = static_cast<int>(fTemp7);
		int iTemp16 = std::min<int>(2046, std::max<int>(0, iTemp15 + 3));
		fRec9[0] = fSlow5 * fTemp1 + fConst1 * fTemp0 * fRec9[1];
		double fTemp17 = std::fabs(fRec1[0]) + 2.0;
		int iTemp18 = std::min<int>(2046, std::max<int>(0, iTemp15 + 1));
		int iTemp19 = std::min<int>(2046, std::max<int>(0, iTemp15));
		int iTemp20 = std::min<int>(2046, std::max<int>(0, iTemp15 + 2));
		double fTemp21 = 0.16666666666666666 * fTemp14 * fVec1[(IOTA0 - iTemp16) & 2047] + 0.3333333333333333 * fRec9[0] * fRec2[1] * fTemp17 + fTemp9 * (fTemp10 * (0.5 * fTemp11 * fVec1[(IOTA0 - iTemp18) & 2047] - 0.16666666666666666 * fVec1[(IOTA0 - iTemp19) & 2047] * fTemp12) - 0.5 * fTemp13 * fVec1[(IOTA0 - iTemp20) & 2047]);
		fVec2[IOTA0 & 2047] = fTemp21;
		fRec2[0] = fTemp9 * (fTemp10 * (0.5 * fTemp11 * fVec2[(IOTA0 - iTemp18) & 2047] - 0.16666666666666666 * fTemp12 * fVec2[(IOTA0 - iTemp19) & 2047]) - 0.5 * fTemp13 * fVec2[(IOTA0 - iTemp20) & 2047]) + 0.16666666666666666 * fTemp14 * fVec2[(IOTA0 - iTemp16) & 2047];
		double fTemp22 = fTemp3 * (2.0 - fRec0[0] * fTemp4) + fRec0[0] * fRec2[0] * fTemp4;
		double fTemp23 = std::max<double>(2.0, std::min<double>(4.09e+03, fConst0 * fTemp5));
		double fTemp24 = fTemp23 + -0.999995;
		double fTemp25 = std::floor(fTemp24);
		double fTemp26 = fTemp23 + (-2.0 - fTemp25);
		double fTemp27 = fTemp23 - fTemp25;
		double fTemp28 = fTemp22 - 0.3333333333333333 * fRec9[0] * fTemp17 * fRec10[1];
		fVec3[IOTA0 & 4095] = fTemp28;
		int iTemp29 = static_cast<int>(fTemp24);
		double fTemp30 = fTemp23 + (-1.0 - fTemp25);
		double fTemp31 = fTemp27 * fTemp30;
		fRec10[0] = (fTemp23 + (-3.0 - fTemp25)) * (fTemp26 * (0.5 * fTemp27 * fVec3[(IOTA0 - std::min<int>(4092, std::max<int>(0, iTemp29 + 1))) & 4095] - 0.16666666666666666 * fVec3[(IOTA0 - std::min<int>(4092, std::max<int>(0, iTemp29))) & 4095] * fTemp30) - 0.5 * fTemp31 * fVec3[(IOTA0 - std::min<int>(4092, std::max<int>(0, iTemp29 + 2))) & 4095]) + 0.16666666666666666 * fTemp31 * fTemp26 * fVec3[(IOTA0 - std::min<int>(4092, std::max<int>(0, iTemp29 + 3))) & 4095];
		output0[i0] = static_cast<FAUSTFLOAT>(0.25 * ((2.0 - fRec0[0] * fTemp2) * fTemp22 + fRec0[0] * fRec10[0] * fTemp2));
		iVec0[1] = iVec0[0];
		fRec0[1] = fRec0[0];
		fRec1[1] = fRec1[0];
		IOTA0 = IOTA0 + 1;
		fRec3[1] = fRec3[0];
		fRec4[1] = fRec4[0];
		fRec7[1] = fRec7[0];
		fRec8[1] = fRec8[0];
		fRec6[1] = fRec6[0];
		fRec5[1] = fRec5[0];
		fRec9[1] = fRec9[0];
		fRec2[1] = fRec2[0];
		fRec10[1] = fRec10[0];
	}
}

void __rt_func Dsp::compute_static(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0, PluginDef *p)
{
	static_cast<Dsp*>(p)->compute(count, input0, output0);
}

int Dsp::register_par(const ParamReg& reg)
{
	reg.registerFloatVar("flanger_mono_gx.bpm",N_("BPM"),"S","",&fHslider4, 6.0, 3.0, 3.6e+02, 0.1, 0);
	reg.registerFloatVar("flanger_mono_gx.depth",N_("Depth"),"S","",&fHslider2, 0.5, 0.0, 5.0, 0.01, 0);
	reg.registerFloatVar("flanger_mono_gx.feedback",N_("Feedback"),"S","",&fHslider5, -0.707, -1.0, 1.0, 0.01, 0);
	reg.registerFloatVar("flanger_mono_gx.mix",N_("Mix"),"S","",&fHslider1, 0.0, -1.0, 1.0, 0.1, 0);
	reg.registerFloatVar("flanger_mono_gx.wet",N_("Wet"),"S","",&fHslider0, 1e+02, 0.0, 1e+02, 1.0, 0);
	reg.registerFloatVar("flanger_mono_gx.width",N_("Width"),"S","",&fHslider3, 5.0, 0.0, 1e+01, 0.01, 0);
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
  <requires lib=\"gtk+\" version=\"3.20\"/>\n\
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
              <object class=\"GtkBox\" id=\"hbox1\">\n\
                <property name=\"visible\">True</property>\n\
                <property name=\"can_focus\">False</property>\n\
                <property name=\"spacing\">4</property>\n\
                <child>\n\
                  <object class=\"GtkBox\" id=\"vbox2\">\n\
                    <property name=\"visible\">True</property>\n\
                    <property name=\"can_focus\">False</property>\n\
                    <property name=\"orientation\">vertical</property>\n\
                    <child>\n\
                      <object class=\"GtkLabel\" id=\"label1:rack_label\">\n\
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
                      <object class=\"GxSmallKnobR\" id=\"gxbigknob1\">\n\
                        <property name=\"visible\">True</property>\n\
                        <property name=\"can_focus\">True</property>\n\
                        <property name=\"receives_default\">True</property>\n\
                        <property name=\"var_id\">flanger_mono_gx.bpm</property>\n\
                        <property name=\"label_ref\">label1:rack_label</property>\n\
                      </object>\n\
                      <packing>\n\
                        <property name=\"expand\">False</property>\n\
                        <property name=\"fill\">False</property>\n\
                        <property name=\"position\">1</property>\n\
                      </packing>\n\
                    </child>\n\
                  </object>\n\
                  <packing>\n\
                    <property name=\"expand\">False</property>\n\
                    <property name=\"fill\">False</property>\n\
                    <property name=\"position\">0</property>\n\
                  </packing>\n\
                </child>\n\
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
                      <object class=\"GxSmallKnobR\" id=\"gxbigknob2\">\n\
                        <property name=\"visible\">True</property>\n\
                        <property name=\"can_focus\">True</property>\n\
                        <property name=\"receives_default\">True</property>\n\
                        <property name=\"var_id\">flanger_mono_gx.depth</property>\n\
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
                    <property name=\"expand\">False</property>\n\
                    <property name=\"fill\">False</property>\n\
                    <property name=\"position\">1</property>\n\
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
                      <object class=\"GxSmallKnobR\" id=\"gxbigknob3\">\n\
                        <property name=\"visible\">True</property>\n\
                        <property name=\"can_focus\">True</property>\n\
                        <property name=\"receives_default\">True</property>\n\
                        <property name=\"var_id\">flanger_mono_gx.width</property>\n\
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
                    <property name=\"expand\">False</property>\n\
                    <property name=\"fill\">False</property>\n\
                    <property name=\"position\">2</property>\n\
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
                      <object class=\"GxSmallKnobR\" id=\"gxbigknob4\">\n\
                        <property name=\"visible\">True</property>\n\
                        <property name=\"can_focus\">True</property>\n\
                        <property name=\"receives_default\">True</property>\n\
                        <property name=\"var_id\">flanger_mono_gx.feedback</property>\n\
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
                    <property name=\"expand\">False</property>\n\
                    <property name=\"fill\">False</property>\n\
                    <property name=\"position\">3</property>\n\
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
                      <object class=\"GxSmallKnobR\" id=\"gxbigknob5\">\n\
                        <property name=\"visible\">True</property>\n\
                        <property name=\"can_focus\">True</property>\n\
                        <property name=\"receives_default\">True</property>\n\
                        <property name=\"var_id\">flanger_mono_gx.mix</property>\n\
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
                    <property name=\"expand\">False</property>\n\
                    <property name=\"fill\">False</property>\n\
                    <property name=\"position\">4</property>\n\
                  </packing>\n\
                </child>\n\
                <child>\n\
                  <object class=\"GtkBox\" id=\"vbox7\">\n\
                    <property name=\"visible\">True</property>\n\
                    <property name=\"can_focus\">False</property>\n\
                    <property name=\"orientation\">vertical</property>\n\
                    <child>\n\
                      <object class=\"GtkLabel\" id=\"label6:rack_label\">\n\
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
                      <object class=\"GxSmallKnobR\" id=\"gxbigknob6\">\n\
                        <property name=\"visible\">True</property>\n\
                        <property name=\"can_focus\">True</property>\n\
                        <property name=\"receives_default\">True</property>\n\
                        <property name=\"var_id\">flanger_mono_gx.wet</property>\n\
                        <property name=\"label_ref\">label6:rack_label</property>\n\
                      </object>\n\
                      <packing>\n\
                        <property name=\"expand\">False</property>\n\
                        <property name=\"fill\">False</property>\n\
                        <property name=\"position\">1</property>\n\
                      </packing>\n\
                    </child>\n\
                  </object>\n\
                  <packing>\n\
                    <property name=\"expand\">False</property>\n\
                    <property name=\"fill\">False</property>\n\
                    <property name=\"position\">5</property>\n\
                  </packing>\n\
                </child>\n\
              </object>\n\
              <packing>\n\
                <property name=\"expand\">True</property>\n\
                <property name=\"fill\">False</property>\n\
                <property name=\"pack_type\">end</property>\n\
                <property name=\"position\">0</property>\n\
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
                <property name=\"var_id\">flanger_mono_gx.wet</property>\n\
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
                <property name=\"label\" translatable=\"yes\">Dry/Wet</property>\n\
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
#define PARAM(p) ("flanger_mono_gx" "." p)
b.openHorizontalhideBox("");
b.create_master_slider(PARAM("wet"), "Dry/Wet");
b.closeBox();

b.openHorizontalBox("");
b.create_small_rackknob(PARAM("bpm"), "freq");
b.create_small_rackknob(PARAM("depth"), "depth");
b.create_small_rackknob(PARAM("width"), "width");
b.create_small_rackknob(PARAM("feedback"), "feedback");
b.create_small_rackknob(PARAM("mix"), "mix");
b.create_small_rackknob(PARAM("wet"), "wet");
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

} // end namespace flanger_gx
} // end namespace pluginlib

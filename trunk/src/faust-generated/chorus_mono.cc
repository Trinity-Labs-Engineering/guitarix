// generated from file '../src/faust/chorus_mono.dsp' by dsp2cc:
// Code generated with Faust (https://faust.grame.fr)


namespace chorus_mono {

class Dsp: public PluginDef {
private:
	int fSampleRate;
	int IOTA0;
	float fVec0[4096];
	FAUSTFLOAT fVslider0;
	float fConst0;
	float fConst1;
	int iVec1[2];
	float fRec0[2];
	FAUSTFLOAT fHslider0;
	float fRec1[2];
	float fConst2;
	FAUSTFLOAT fHslider1;
	float fRec4[2];
	float fRec5[2];
	float fRec3[2];
	float fRec2[2];

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
	id = "chorus_mono";
	name = N_("Chorus Mono");
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
	for (int l0 = 0; l0 < 4096; l0 = l0 + 1) fVec0[l0] = 0.0f;
	for (int l1 = 0; l1 < 2; l1 = l1 + 1) iVec1[l1] = 0;
	for (int l2 = 0; l2 < 2; l2 = l2 + 1) fRec0[l2] = 0.0f;
	for (int l3 = 0; l3 < 2; l3 = l3 + 1) fRec1[l3] = 0.0f;
	for (int l4 = 0; l4 < 2; l4 = l4 + 1) fRec4[l4] = 0.0f;
	for (int l5 = 0; l5 < 2; l5 = l5 + 1) fRec5[l5] = 0.0f;
	for (int l6 = 0; l6 < 2; l6 = l6 + 1) fRec3[l6] = 0.0f;
	for (int l7 = 0; l7 < 2; l7 = l7 + 1) fRec2[l7] = 0.0f;
}

void Dsp::clear_state_f_static(PluginDef *p)
{
	static_cast<Dsp*>(p)->clear_state_f();
}

inline void Dsp::init(unsigned int sample_rate)
{
	fSampleRate = sample_rate;
	fConst0 = std::min<float>(1.92e+05f, std::max<float>(1.0f, static_cast<float>(fSampleRate)));
	fConst1 = std::exp(-(1e+02f / fConst0));
	fConst2 = 0.10471976f / fConst0;
	IOTA0 = 0;
	clear_state_f();
}

void Dsp::init_static(unsigned int sample_rate, PluginDef *p)
{
	static_cast<Dsp*>(p)->init(sample_rate);
}

void always_inline Dsp::compute(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0)
{
	float fSlow0 = 0.01f * static_cast<float>(fVslider0);
	float fSlow1 = static_cast<float>(fHslider0);
	float fSlow2 = static_cast<float>(fHslider1);
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		float fTemp0 = static_cast<float>(input0[i0]);
		fVec0[IOTA0 & 4095] = fTemp0;
		iVec1[0] = 1;
		float fTemp1 = static_cast<float>(iVec1[1]);
		float fTemp2 = 1.0f - fConst1 * fTemp1;
		fRec0[0] = fSlow0 * fTemp2 + fConst1 * fTemp1 * fRec0[1];
		fRec1[0] = fSlow1 * fTemp2 + fConst1 * fTemp1 * fRec1[1];
		fRec4[0] = fSlow2 * fTemp2 + fConst1 * fTemp1 * fRec4[1];
		fRec5[0] = fRec5[1] - fConst2 * fRec4[0] * fRec2[1];
		fRec3[0] = fConst2 * fRec4[0] * fRec5[0] + static_cast<float>(1 - iVec1[1]) + fRec3[1];
		fRec2[0] = fRec3[0];
		float fTemp3 = std::max<float>(2.0f, std::min<float>(4.09e+03f, fConst0 * (0.002f * fRec2[0] + 0.01f)));
		float fTemp4 = fTemp3 + -0.999995f;
		float fTemp5 = std::floor(fTemp4);
		float fTemp6 = fTemp3 + (-2.0f - fTemp5);
		float fTemp7 = fTemp3 - fTemp5;
		int iTemp8 = static_cast<int>(fTemp4);
		float fTemp9 = fTemp3 + (-1.0f - fTemp5);
		float fTemp10 = fTemp7 * fTemp9;
		output0[i0] = static_cast<FAUSTFLOAT>(fTemp0 + fRec0[0] * fRec1[0] * ((fTemp3 + (-3.0f - fTemp5)) * (fTemp6 * (0.5f * fTemp7 * fVec0[(IOTA0 - std::min<int>(4092, std::max<int>(0, iTemp8 + 1))) & 4095] - 0.16666667f * fVec0[(IOTA0 - std::min<int>(4092, std::max<int>(0, iTemp8))) & 4095] * fTemp9) - 0.5f * fTemp10 * fVec0[(IOTA0 - std::min<int>(4092, std::max<int>(0, iTemp8 + 2))) & 4095]) + 0.16666667f * fTemp10 * fTemp6 * fVec0[(IOTA0 - std::min<int>(4092, std::max<int>(0, iTemp8 + 3))) & 4095]));
		IOTA0 = IOTA0 + 1;
		iVec1[1] = iVec1[0];
		fRec0[1] = fRec0[0];
		fRec1[1] = fRec1[0];
		fRec4[1] = fRec4[0];
		fRec5[1] = fRec5[0];
		fRec3[1] = fRec3[0];
		fRec2[1] = fRec2[0];
	}
}

void __rt_func Dsp::compute_static(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0, PluginDef *p)
{
	static_cast<Dsp*>(p)->compute(count, input0, output0);
}

int Dsp::register_par(const ParamReg& reg)
{
	reg.registerFloatVar("chorus_mono.bpm",N_("BPM"),"S","",&fHslider1, 1.2e+02f, 6.0f, 3.6e+02f, 0.1f, 0);
	reg.registerFloatVar("chorus_mono.level",N_("Level"),"S","",&fHslider0, 0.7f, 0.0f, 2.0f, 0.01f, 0);
	reg.registerFloatVar("chorus_mono.wet_dry",N_("Dry/Wet"),"S",N_("percentage of processed signal in output signal"),&fVslider0, 1e+02f, 0.0f, 1e+02f, 1.0f, 0);
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
            <property name=\"spacing\">4</property>\n\
            <child>\n\
              <object class=\"GtkBox\" id=\"hbox1\">\n\
                <property name=\"visible\">True</property>\n\
                <property name=\"can_focus\">False</property>\n\
                <property name=\"spacing\">10</property>\n\
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
                        <property name=\"var_id\">chorus_mono.bpm</property>\n\
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
                      <object class=\"GxMidKnob\" id=\"gxbigknob2\">\n\
                        <property name=\"visible\">True</property>\n\
                        <property name=\"can_focus\">True</property>\n\
                        <property name=\"receives_default\">True</property>\n\
                        <property name=\"var_id\">chorus_mono.level</property>\n\
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
                        <property name=\"var_id\">chorus_mono.wet_dry</property>\n\
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
                <property name=\"var_id\">chorus_mono.level</property>\n\
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
                <property name=\"label\" translatable=\"yes\">Level</property>\n\
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
#define PARAM(p) ("chorus_mono" "." p)
b.openHorizontalhideBox("");
b.create_master_slider(PARAM("level"), _("level"));
b.closeBox();
b.openVerticalBox("");
{
    b.openHorizontalBox("");
    {
	b.create_small_rackknobr(PARAM("level"), _("level"));
	b.create_small_rackknob(PARAM("bpm"), _("BPM"));
	b.create_small_rackknob(PARAM("wet_dry"), _("dry/wet"));
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

} // end namespace chorus_mono

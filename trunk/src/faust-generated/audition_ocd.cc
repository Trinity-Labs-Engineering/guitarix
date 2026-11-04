// generated from file '../src/faust/audition_ocd.dsp' by dsp2cc:
// Code generated with Faust (https://faust.grame.fr)


namespace audition_ocd {

class Dsp: public PluginDef {
private:
	int fSampleRate;
	double fConst0;
	double fConst1;
	double fConst2;
	double fConst3;
	double fConst4;
	FAUSTFLOAT fHslider0;
	double fConst5;
	double fRec7[2];
	FAUSTFLOAT fHslider1;
	double fRec8[2];
	double fConst6;
	double fConst7;
	double fConst8;
	double fConst9;
	double fConst10;
	double fConst11;
	double fConst12;
	double fConst13;
	double fConst14;
	double fConst15;
	double fConst16;
	double fConst17;
	double fConst18;
	double fConst19;
	double fConst20;
	double fConst21;
	double fConst22;
	double fConst23;
	double fConst24;
	double fConst25;
	double fConst26;
	double fConst27;
	double fConst28;
	double fConst29;
	double fConst30;
	double fConst31;
	double fConst32;
	double fConst33;
	double fConst34;
	double fConst35;
	double fConst36;
	double fConst37;
	double fConst38;
	double fConst39;
	double fConst40;
	double fConst41;
	double fConst42;
	double fConst43;
	double fConst44;
	double fConst45;
	double fConst46;
	double fConst47;
	double fConst48;
	double fConst49;
	double fConst50;
	double fConst51;
	double fConst52;
	double fConst53;
	double fConst54;
	double fConst55;
	double fConst56;
	double fConst57;
	double fConst58;
	double fConst59;
	double fConst60;
	double fConst61;
	double fConst62;
	double fConst63;
	double fConst64;
	double fConst65;
	double fConst66;
	double fConst67;
	double fConst68;
	double fConst69;
	double fConst70;
	double fConst71;
	double fConst72;
	double fConst73;
	double fConst74;
	double fConst75;
	double fConst76;
	double fConst77;
	double fVec0[2];
	double fConst78;
	double fRec16[2];
	double fConst79;
	FAUSTFLOAT fHslider2;
	double fRec17[2];
	double fRec15[3];
	double fVec1[12];
	double fConst80;
	double fRec10[2];
	double fRec11[12];
	double fRec12[12];
	double fRec13[12];
	double fRec14[12];
	double fConst81;
	double fConst82;
	double fRec9[3];
	double fConst83;
	double fConst84;
	double fRec1[2];
	int iRec2[2];
	int iRec3[2];
	double fRec4[2];
	double fRec5[2];
	int iRec6[2];
	double fConst85;
	double fRec0[2];
	FAUSTFLOAT fHslider3;
	double fRec18[2];

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
	id = "audition_ocd";
	name = N_("OCD Drive");
	groups = 0;
	description = ""; // description (tooltip)
	category = N_("Audition");       // category
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
	for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec7[l0] = 0.0;
	for (int l1 = 0; l1 < 2; l1 = l1 + 1) fRec8[l1] = 0.0;
	for (int l2 = 0; l2 < 2; l2 = l2 + 1) fVec0[l2] = 0.0;
	for (int l3 = 0; l3 < 2; l3 = l3 + 1) fRec16[l3] = 0.0;
	for (int l4 = 0; l4 < 2; l4 = l4 + 1) fRec17[l4] = 0.0;
	for (int l5 = 0; l5 < 3; l5 = l5 + 1) fRec15[l5] = 0.0;
	for (int l6 = 0; l6 < 12; l6 = l6 + 1) fVec1[l6] = 0.0;
	for (int l7 = 0; l7 < 2; l7 = l7 + 1) fRec10[l7] = 0.0;
	for (int l8 = 0; l8 < 12; l8 = l8 + 1) fRec11[l8] = 0.0;
	for (int l9 = 0; l9 < 12; l9 = l9 + 1) fRec12[l9] = 0.0;
	for (int l10 = 0; l10 < 12; l10 = l10 + 1) fRec13[l10] = 0.0;
	for (int l11 = 0; l11 < 12; l11 = l11 + 1) fRec14[l11] = 0.0;
	for (int l12 = 0; l12 < 3; l12 = l12 + 1) fRec9[l12] = 0.0;
	for (int l13 = 0; l13 < 2; l13 = l13 + 1) fRec1[l13] = 0.0;
	for (int l14 = 0; l14 < 2; l14 = l14 + 1) iRec2[l14] = 0;
	for (int l15 = 0; l15 < 2; l15 = l15 + 1) iRec3[l15] = 0;
	for (int l16 = 0; l16 < 2; l16 = l16 + 1) fRec4[l16] = 0.0;
	for (int l17 = 0; l17 < 2; l17 = l17 + 1) fRec5[l17] = 0.0;
	for (int l18 = 0; l18 < 2; l18 = l18 + 1) iRec6[l18] = 0;
	for (int l19 = 0; l19 < 2; l19 = l19 + 1) fRec0[l19] = 0.0;
	for (int l20 = 0; l20 < 2; l20 = l20 + 1) fRec18[l20] = 0.0;
}

void Dsp::clear_state_f_static(PluginDef *p)
{
	static_cast<Dsp*>(p)->clear_state_f();
}

inline void Dsp::init(unsigned int sample_rate)
{
	fSampleRate = sample_rate;
	fConst0 = std::min<double>(1.92e+05, std::max<double>(1.0, static_cast<double>(fSampleRate)));
	fConst1 = 15.707963267948966 / fConst0;
	fConst2 = 1.0 / (fConst1 + 1.0);
	fConst3 = 10638297.872340426 / fConst0;
	fConst4 = 44.1 / fConst0;
	fConst5 = 1.0 - fConst4;
	fConst6 = 49999.99999999999 / fConst0;
	fConst7 = std::tan(3141.592653589793 / fConst0);
	fConst8 = 1.0 / fConst7;
	fConst9 = (fConst8 + 4.863686023157921) / fConst7 + 0.19681659604183724;
	fConst10 = 2.0 / fConst9;
	fConst11 = 1.0 / mydsp_faustpower2_f(fConst7);
	fConst12 = 2.0 * (0.19681659604183724 - fConst11);
	fConst13 = 8e-08 * fConst0;
	fConst14 = fConst13 + 0.00010434782608695653;
	fConst15 = 0.008379572631713933 / fConst14;
	fConst16 = (0.16834212000000015 - fConst15) / (fConst15 + 0.16834212000000015);
	fConst17 = 0.003365821158762255 / fConst14;
	fConst18 = (0.08928117999999996 - fConst17) / (fConst17 + 0.08928117999999996);
	fConst19 = fConst16 + fConst18;
	fConst20 = 0.008331621144446522 / fConst14;
	fConst21 = (0.18640057999999993 - fConst20) / (fConst20 + 0.18640057999999993);
	fConst22 = fConst21 - fConst16;
	fConst23 = 0.0074110485024928945 / fConst14;
	fConst24 = (0.18817092000000013 - fConst23) / (fConst23 + 0.18817092000000013);
	fConst25 = fConst24 - fConst21;
	fConst26 = 0.00647386924018694 / fConst14;
	fConst27 = (0.18999239999999995 - fConst26) / (fConst26 + 0.18999239999999995);
	fConst28 = fConst27 - fConst24;
	fConst29 = 0.005519369830639822 / fConst14;
	fConst30 = (0.19186755 - fConst29) / (fConst29 + 0.19186755);
	fConst31 = fConst30 - fConst27;
	fConst32 = 0.00454678888102043 / fConst14;
	fConst33 = (0.1937990599999999 - fConst32) / (fConst32 + 0.1937990599999999);
	fConst34 = fConst33 - fConst30;
	fConst35 = 0.003555313997705983 / fConst14;
	fConst36 = (0.19578982999999983 - fConst35) / (fConst35 + 0.19578982999999983);
	fConst37 = fConst36 - fConst33;
	fConst38 = 0.0025440760932679924 / fConst14;
	fConst39 = (0.19784294000000013 - fConst38) / (fConst38 + 0.19784294000000013);
	fConst40 = fConst39 - fConst36;
	fConst41 = 0.001512145573902368 / fConst14;
	fConst42 = (0.19996174999999994 - fConst41) / (fConst41 + 0.19996174999999994);
	fConst43 = fConst42 - fConst39;
	fConst44 = 0.00046119410462311914 / fConst14;
	fConst45 = (0.26385236999999995 - fConst44) / (fConst44 + 0.26385236999999995);
	fConst46 = fConst45 - fConst42;
	fConst47 = 2.4911411742190235e-06 / fConst14;
	fConst48 = (4.16587867 - fConst47) / (fConst47 + 4.16587867);
	fConst49 = fConst48 - fConst45;
	fConst50 = 0.000827782245839755 / fConst14;
	fConst51 = (0.4499385199999999 - fConst50) / (fConst50 + 0.4499385199999999);
	fConst52 = fConst51 - fConst48;
	fConst53 = 0.0018993135786224502 / fConst14;
	fConst54 = (0.25327153000000013 - fConst53) / (fConst53 + 0.25327153000000013);
	fConst55 = fConst54 - fConst51;
	fConst56 = 0.002924522544757472 / fConst14;
	fConst57 = (0.22919418999999985 - fConst56) / (fConst56 + 0.22919418999999985);
	fConst58 = fConst57 - fConst54;
	fConst59 = 0.003928652379414453 / fConst14;
	fConst60 = (0.21797082000000012 - fConst59) / (fConst59 + 0.21797082000000012);
	fConst61 = fConst60 - fConst57;
	fConst62 = 0.0049137938954473305 / fConst14;
	fConst63 = (0.21092719999999998 - fConst62) / (fConst62 + 0.21092719999999998);
	fConst64 = fConst63 - fConst60;
	fConst65 = 0.005880822079480015 / fConst14;
	fConst66 = (0.20579537000000014 - fConst65) / (fConst65 + 0.20579537000000014);
	fConst67 = fConst66 - fConst63;
	fConst68 = 0.006830427723456045 / fConst14;
	fConst69 = (0.2017125799999997 - fConst68) / (fConst68 + 0.2017125799999997);
	fConst70 = fConst69 - fConst66;
	fConst71 = fConst18 - fConst69;
	fConst72 = (-4.0 - 0.0487362103139599 / fConst14) * fConst19 + fConst22 * std::fabs(-0.16834212000000015 - fConst15) + fConst25 * std::fabs(-0.3547427000000001 - 0.016711193776160455 / fConst14) + fConst28 * std::fabs(-0.5429136200000002 - 0.02412224227865335 / fConst14) + fConst31 * std::fabs(-0.7329060200000002 - 0.03059611151884029 / fConst14) + fConst34 * std::fabs(-0.9247735700000002 - 0.03611548134948011 / fConst14) + fConst37 * std::fabs(-1.11857263 - 0.04066227023050054 / fConst14) + fConst40 * std::fabs(-1.31436246 - 0.04421758422820653 / fConst14) + fConst43 * std::fabs(-1.5122054 - 0.04676166032147452 / fConst14) + fConst46 * std::fabs(-1.71216715 - 0.04827380589537689 / fConst14) + fConst49 * std::fabs(-1.97601952 - 0.048735 / fConst14) + fConst52 * std::fabs(-6.14189819 - 0.048737491141174226 / fConst14) + fConst55 * std::fabs(-6.59183671 - 0.049565273387013976 / fConst14) + fConst58 * std::fabs(-6.84510824 - 0.051464586965636425 / fConst14) + fConst61 * std::fabs(-7.0743024299999995 - 0.0543891095103939 / fConst14) + fConst64 * std::fabs(-7.29227325 - 0.058317761889808355 / fConst14) + fConst67 * std::fabs(-7.50320045 - 0.06323155578525569 / fConst14) + fConst70 * std::fabs(-7.70899582 - 0.0691123778647357 / fConst14) + fConst71 * std::fabs(-7.9107084 - 0.07594280558819175 / fConst14);
	fConst73 = 1.0 / fConst14;
	fConst74 = std::tan(47.34848484848485 / fConst0);
	fConst75 = 1.0 / fConst74;
	fConst76 = 1.0 / (fConst75 + 1.0);
	fConst77 = 0.9791666666666666 / fConst74;
	fConst78 = 1.0 - fConst75;
	fConst79 = 3288325270.4937057 / fConst7;
	fConst80 = 0.5 * fConst19;
	fConst81 = 1.0 / fConst9;
	fConst82 = (fConst8 + -4.863686023157921) / fConst7 + 0.19681659604183724;
	fConst83 = (fConst8 + 23.413213190045493) / fConst7 + 0.19681659604183724;
	fConst84 = (fConst8 + -23.413213190045493) / fConst7 + 0.19681659604183724;
	fConst85 = 1.0 - fConst1;
	clear_state_f();
}

void Dsp::init_static(unsigned int sample_rate, PluginDef *p)
{
	static_cast<Dsp*>(p)->init(sample_rate);
}

void always_inline Dsp::compute(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0)
{
	double fSlow0 = fConst4 * static_cast<double>(fHslider0);
	double fSlow1 = fConst4 * static_cast<double>(fHslider1);
	double fSlow2 = fConst4 * static_cast<double>(fHslider2);
	double fSlow3 = fConst4 * static_cast<double>(fHslider3);
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		fRec7[0] = fSlow0 + fConst5 * fRec7[1];
		double fTemp0 = std::max<double>(1.0, 1e+04 * std::max<double>(0.0, std::min<double>(1.0, fRec7[0])));
		double fTemp1 = fConst3 + fTemp0;
		double fTemp2 = 1.0 / fTemp1 + 2e-06;
		fRec8[0] = fSlow1 + fConst5 * fRec8[1];
		double fTemp3 = 3.3e+04 - 1.98e+04 * std::max<double>(0.0, std::min<double>(1.0, fRec8[0]));
		double fTemp4 = 1.0 / fTemp2 + fTemp3;
		double fTemp5 = static_cast<double>(-iRec2[1]);
		double fTemp6 = static_cast<double>(input0[i0]);
		fVec0[0] = fTemp6;
		fRec16[0] = fConst76 * (fConst77 * (fTemp6 - fVec0[1]) - fConst78 * fRec16[1]);
		fRec17[0] = fSlow2 + fConst5 * fRec17[1];
		double fTemp7 = 5e+05 * mydsp_faustpower2_f(std::max<double>(0.0, std::min<double>(1.0, fRec17[0]))) + 1.8e+04;
		double fTemp8 = fConst79 * (2.2e-10 * fTemp7 + 0.00021999999999999998);
		double fTemp9 = 2.0 * fRec15[1] * (523353.22129306715 / fTemp7 - fConst11);
		double fTemp10 = fConst11 + (fTemp8 + 523353.22129306715) / fTemp7;
		fRec15[0] = fRec16[0] - (fRec15[2] * (fConst11 + (523353.22129306715 - fTemp8) / fTemp7) + fTemp9) / fTemp10;
		double fTemp11 = fConst79 * (1.0022e-07 * fTemp7 + 0.00021999999999999998);
		double fTemp12 = (fTemp9 + fRec15[0] * (fConst11 + (fTemp11 + 523353.22129306715) / fTemp7) + fRec15[2] * (fConst11 + (523353.22129306715 - fTemp11) / fTemp7)) / fTemp10;
		fVec1[0] = fTemp12;
		double fTemp13 = fConst13 * fRec10[1] + 0.0001 * (0.0037884237976 * fVec1[1] + 0.04674778664 * fVec1[3] + 0.20949881212 * fVec1[5] + 0.8799857828 * fVec1[6] + 0.021123350532 * fVec1[9] + 0.0023888566484 * fVec1[11] - (0.00025234506108 * fTemp12 + 0.016762448672 * fVec1[2] + 0.1006560798 * fVec1[4] + 0.009853003852 * fVec1[7] + 0.026253322116 * fVec1[8] + 0.009654191464 * fVec1[10]));
		double fTemp14 = fConst72 - (fConst22 * std::fabs(fConst73 * (fTemp13 + 0.04035663768224597) + 3.83165788) + fConst25 * std::fabs(fConst73 * (fTemp13 + 0.03202501653779945) + 3.6452573) + fConst28 * std::fabs(fConst73 * (fTemp13 + 0.024613968035306553) + 3.45708638) + fConst31 * std::fabs(fConst73 * (fTemp13 + 0.018140098795119613) + 3.26709398) + fConst34 * std::fabs(fConst73 * (fTemp13 + 0.01262072896447979) + 3.07522643) + fConst37 * std::fabs(fConst73 * (fTemp13 + 0.008073940083459361) + 2.88142737) + fConst40 * std::fabs(fConst73 * (fTemp13 + 0.004518626085753378) + 2.68563754) + fConst43 * std::fabs(fConst73 * (fTemp13 + 0.0019745499924853855) + 2.4877946) + fConst46 * std::fabs(fConst73 * (fTemp13 + 0.00046240441858301745) + 2.28783285) + fConst49 * std::fabs(fConst73 * (fTemp13 + 1.2103139598983047e-06) + 2.02398048) + fConst52 * std::fabs(fConst73 * (fTemp13 + -1.2808272143207188e-06) + -2.14189819) + fConst55 * std::fabs(fConst73 * (fTemp13 + -0.0008290630730540757) + -2.59183671) + fConst58 * std::fabs(fConst73 * (fTemp13 + -0.002728376651676526) + -2.84510824) + fConst61 * std::fabs(fConst73 * (fTemp13 + -0.005652899196433998) + -3.07430243) + fConst64 * std::fabs(fConst73 * (fTemp13 + -0.009581551575848451) + -3.29227325) + fConst67 * std::fabs(fConst73 * (fTemp13 + -0.014495345471295782) + -3.50320045) + fConst70 * std::fabs(fConst73 * (fTemp13 + -0.020376167550775797) + -3.70899582) + fConst71 * std::fabs(fConst73 * (fTemp13 + -0.027206595274231842) + -3.9107084));
		double fTemp15 = fTemp13 + fConst80 * fTemp13 + 0.0487362103139599;
		double fTemp16 = fConst73 * fTemp15;
		double fTemp17 = 0.5 * fTemp14;
		double fTemp18 = fConst13 * (fTemp16 + (-4.0 - (fRec10[1] + fTemp17))) + 0.0001 * (0.0029681430836 * fVec1[1] + 0.05581795852 * fVec1[3] + 0.4814269576 * fVec1[5] + 0.7307934196 * fVec1[6] + 0.032341594504 * fVec1[8] + 0.0007282737012 * fVec1[11] - (0.00015138522672 * fTemp12 + 0.016454924 * fVec1[2] + 0.15112621884 * fVec1[4] + 0.13127642356 * fVec1[7] + 0.0034562122476 * fVec1[9] + 0.001712804902 * fVec1[10]));
		double fTemp19 = fConst72 - (fConst22 * std::fabs(fConst73 * (fTemp18 + 0.04035663768224597) + 3.83165788) + fConst25 * std::fabs(fConst73 * (fTemp18 + 0.03202501653779945) + 3.6452573) + fConst28 * std::fabs(fConst73 * (fTemp18 + 0.024613968035306553) + 3.45708638) + fConst31 * std::fabs(fConst73 * (fTemp18 + 0.018140098795119613) + 3.26709398) + fConst34 * std::fabs(fConst73 * (fTemp18 + 0.01262072896447979) + 3.07522643) + fConst37 * std::fabs(fConst73 * (fTemp18 + 0.008073940083459361) + 2.88142737) + fConst40 * std::fabs(fConst73 * (fTemp18 + 0.004518626085753378) + 2.68563754) + fConst43 * std::fabs(fConst73 * (fTemp18 + 0.0019745499924853855) + 2.4877946) + fConst46 * std::fabs(fConst73 * (fTemp18 + 0.00046240441858301745) + 2.28783285) + fConst49 * std::fabs(fConst73 * (fTemp18 + 1.2103139598983047e-06) + 2.02398048) + fConst52 * std::fabs(fConst73 * (fTemp18 + -1.2808272143207188e-06) + -2.14189819) + fConst55 * std::fabs(fConst73 * (fTemp18 + -0.0008290630730540757) + -2.59183671) + fConst58 * std::fabs(fConst73 * (fTemp18 + -0.002728376651676526) + -2.84510824) + fConst61 * std::fabs(fConst73 * (fTemp18 + -0.005652899196433998) + -3.07430243) + fConst64 * std::fabs(fConst73 * (fTemp18 + -0.009581551575848451) + -3.29227325) + fConst67 * std::fabs(fConst73 * (fTemp18 + -0.014495345471295782) + -3.50320045) + fConst70 * std::fabs(fConst73 * (fTemp18 + -0.020376167550775797) + -3.70899582) + fConst71 * std::fabs(fConst73 * (fTemp18 + -0.027206595274231842) + -3.9107084));
		double fTemp20 = fTemp14 - fTemp19;
		double fTemp21 = fTemp18 + fConst80 * fTemp18 + 0.0487362103139599;
		double fTemp22 = fTemp15 - fTemp21;
		double fTemp23 = fConst13 * (fRec10[1] + 0.5 * fTemp20 - fConst73 * fTemp22) + 0.0001 * (0.0007282737012 * fTemp12 + 0.032341594504 * fVec1[3] + 0.7307934196 * fVec1[5] + 0.4814269576 * fVec1[6] + 0.05581795852 * fVec1[8] + 0.0029681430836 * fVec1[10] - (0.001712804902 * fVec1[1] + 0.0034562122476 * fVec1[2] + 0.13127642356 * fVec1[4] + 0.15112621884 * fVec1[7] + 0.016454924 * fVec1[9] + 0.00015138522672 * fVec1[11]));
		double fTemp24 = fConst72 - (fConst22 * std::fabs(fConst73 * (fTemp23 + 0.04035663768224597) + 3.83165788) + fConst25 * std::fabs(fConst73 * (fTemp23 + 0.03202501653779945) + 3.6452573) + fConst28 * std::fabs(fConst73 * (fTemp23 + 0.024613968035306553) + 3.45708638) + fConst31 * std::fabs(fConst73 * (fTemp23 + 0.018140098795119613) + 3.26709398) + fConst34 * std::fabs(fConst73 * (fTemp23 + 0.01262072896447979) + 3.07522643) + fConst37 * std::fabs(fConst73 * (fTemp23 + 0.008073940083459361) + 2.88142737) + fConst40 * std::fabs(fConst73 * (fTemp23 + 0.004518626085753378) + 2.68563754) + fConst43 * std::fabs(fConst73 * (fTemp23 + 0.0019745499924853855) + 2.4877946) + fConst46 * std::fabs(fConst73 * (fTemp23 + 0.00046240441858301745) + 2.28783285) + fConst49 * std::fabs(fConst73 * (fTemp23 + 1.2103139598983047e-06) + 2.02398048) + fConst52 * std::fabs(fConst73 * (fTemp23 + -1.2808272143207188e-06) + -2.14189819) + fConst55 * std::fabs(fConst73 * (fTemp23 + -0.0008290630730540757) + -2.59183671) + fConst58 * std::fabs(fConst73 * (fTemp23 + -0.002728376651676526) + -2.84510824) + fConst61 * std::fabs(fConst73 * (fTemp23 + -0.005652899196433998) + -3.07430243) + fConst64 * std::fabs(fConst73 * (fTemp23 + -0.009581551575848451) + -3.29227325) + fConst67 * std::fabs(fConst73 * (fTemp23 + -0.014495345471295782) + -3.50320045) + fConst70 * std::fabs(fConst73 * (fTemp23 + -0.020376167550775797) + -3.70899582) + fConst71 * std::fabs(fConst73 * (fTemp23 + -0.027206595274231842) + -3.9107084));
		double fTemp25 = fTemp20 + fTemp24;
		double fTemp26 = fTemp23 + fConst80 * fTemp23 + 0.0487362103139599;
		double fTemp27 = fTemp22 + fTemp26;
		double fTemp28 = fConst13 * (fConst73 * fTemp27 + (-4.0 - (fRec10[1] + 0.5 * fTemp25))) + 0.0001 * (0.0023888566484 * fTemp12 + 0.021123350532 * fVec1[2] + 0.8799857828 * fVec1[5] + 0.20949881212 * fVec1[6] + 0.04674778664 * fVec1[8] + 0.0037884237976 * fVec1[10] - (0.009654191464 * fVec1[1] + 0.026253322116 * fVec1[3] + 0.009853003852 * fVec1[4] + 0.1006560798 * fVec1[7] + 0.016762448672 * fVec1[9] + 0.00025234506108 * fVec1[11]));
		double fTemp29 = fConst72 - (fConst22 * std::fabs(fConst73 * (fTemp28 + 0.04035663768224597) + 3.83165788) + fConst25 * std::fabs(fConst73 * (fTemp28 + 0.03202501653779945) + 3.6452573) + fConst28 * std::fabs(fConst73 * (fTemp28 + 0.024613968035306553) + 3.45708638) + fConst31 * std::fabs(fConst73 * (fTemp28 + 0.018140098795119613) + 3.26709398) + fConst34 * std::fabs(fConst73 * (fTemp28 + 0.01262072896447979) + 3.07522643) + fConst37 * std::fabs(fConst73 * (fTemp28 + 0.008073940083459361) + 2.88142737) + fConst40 * std::fabs(fConst73 * (fTemp28 + 0.004518626085753378) + 2.68563754) + fConst43 * std::fabs(fConst73 * (fTemp28 + 0.0019745499924853855) + 2.4877946) + fConst46 * std::fabs(fConst73 * (fTemp28 + 0.00046240441858301745) + 2.28783285) + fConst49 * std::fabs(fConst73 * (fTemp28 + 1.2103139598983047e-06) + 2.02398048) + fConst52 * std::fabs(fConst73 * (fTemp28 + -1.2808272143207188e-06) + -2.14189819) + fConst55 * std::fabs(fConst73 * (fTemp28 + -0.0008290630730540757) + -2.59183671) + fConst58 * std::fabs(fConst73 * (fTemp28 + -0.002728376651676526) + -2.84510824) + fConst61 * std::fabs(fConst73 * (fTemp28 + -0.005652899196433998) + -3.07430243) + fConst64 * std::fabs(fConst73 * (fTemp28 + -0.009581551575848451) + -3.29227325) + fConst67 * std::fabs(fConst73 * (fTemp28 + -0.014495345471295782) + -3.50320045) + fConst70 * std::fabs(fConst73 * (fTemp28 + -0.020376167550775797) + -3.70899582) + fConst71 * std::fabs(fConst73 * (fTemp28 + -0.027206595274231842) + -3.9107084));
		double fTemp30 = fTemp28 + fConst80 * fTemp28 + 0.0487362103139599;
		fRec10[0] = fRec10[1] + 0.5 * (fTemp25 - fTemp29) - fConst73 * (fTemp27 - fTemp30);
		fRec11[0] = 0.5 * (fConst73 * fTemp30 + (-4.0 - 0.5 * fTemp29));
		fRec12[0] = 0.5 * (fConst73 * fTemp26 + (-4.0 - 0.5 * fTemp24));
		fRec13[0] = 0.5 * (fConst73 * fTemp21 + (-4.0 - 0.5 * fTemp19));
		fRec14[0] = 0.5 * (fTemp16 + (-4.0 - fTemp17));
		double fTemp31 = fConst12 * fRec9[1];
		fRec9[0] = 0.9565217391304348 * (0.005280837633 * (fRec11[9] + fRec14[2]) + 0.2199964457 * (fRec11[6] + fRec14[5]) + 0.05237470303 * (fRec11[5] + fRec14[6]) + 0.01168694666 * (fRec11[3] + fRec14[8]) + 0.0009471059494 * (fRec11[1] + fRec14[10]) + 0.0005972141621 * (fRec14[0] + fRec11[11]) + 0.008085398626 * (fRec12[8] + fRec13[3]) + 0.1826983549 * (fRec12[6] + fRec13[5]) + 0.1203567394 * (fRec12[5] + fRec13[6]) + 0.01395448963 * (fRec12[3] + fRec13[8]) + 0.0001820684253 * (fRec13[0] + fRec12[11]) + 0.0007420357709 * (fRec12[1] + fRec13[10]) - (0.0004282012255 * (fRec12[10] + fRec13[1]) + 0.0008640530619 * (fRec12[9] + fRec13[2]) + 0.03281910589 * (fRec12[7] + fRec13[4]) + 0.03778155471 * (fRec12[4] + fRec13[7]) + 3.784630668e-05 * (fRec12[0] + fRec13[11]) + 0.004113731 * (fRec12[2] + fRec13[9]) + 6.308626527e-05 * (fRec11[0] + fRec14[11]) + 0.004190612168 * (fRec11[2] + fRec14[9]) + 0.02516401995 * (fRec11[4] + fRec14[7]) + 0.002463250963 * (fRec11[7] + fRec14[4]) + 0.006563330529 * (fRec11[8] + fRec14[3]) + 0.002413547866 * (fRec11[10] + fRec14[1]))) - fConst81 * (fConst82 * fRec9[2] + fTemp31);
		double fTemp32 = static_cast<double>(iRec3[1]);
		double fTemp33 = static_cast<double>(iRec6[1]);
		double fTemp34 = fRec5[1] + fTemp33;
		double fTemp35 = fTemp32 * fTemp1 - 5e+05 * fTemp34;
		double fTemp36 = fConst3 + fTemp0 + 5e+05;
		double fTemp37 = fTemp35 / fTemp36;
		double fTemp38 = fTemp5 + fConst10 * (fTemp31 + fConst83 * fRec9[0] + fConst84 * fRec9[2]) + fRec1[1] - fTemp37;
		double fTemp39 = fConst6 * (fTemp5 - fTemp37);
		double fTemp40 = fConst6 + fTemp4;
		fRec1[0] = (fRec1[1] * fTemp4 - fConst6 * fTemp38 - fTemp39) / fTemp40;
		iRec2[0] = 0;
		iRec3[0] = 0;
		double fTemp41 = (((fTemp39 - fTemp4 * (fRec1[1] + fTemp38)) / fTemp40 + static_cast<double>(iRec2[1])) / fTemp2 - fTemp35 * fTemp3 / fTemp36) / fTemp4;
		fRec4[0] = -(0.5 * (5e+05 * (fTemp34 / fTemp36) + fTemp41 + 5e+05 * (fTemp32 / fTemp36)));
		fRec5[0] = (fTemp0 * fRec5[1] - fConst3 * (fTemp1 * (fTemp32 + fTemp34) / fTemp36 - fTemp41) - fConst3 * fTemp33) / fTemp1;
		iRec6[0] = 0;
		fRec0[0] = fConst2 * (fRec4[0] - fRec4[1] + fConst85 * fRec0[1]);
		fRec18[0] = fSlow3 + fConst5 * fRec18[1];
		output0[i0] = static_cast<FAUSTFLOAT>(0.035 * fRec0[0] * std::pow(1e+01, 0.05 * fRec18[0]));
		fRec7[1] = fRec7[0];
		fRec8[1] = fRec8[0];
		fVec0[1] = fVec0[0];
		fRec16[1] = fRec16[0];
		fRec17[1] = fRec17[0];
		fRec15[2] = fRec15[1];
		fRec15[1] = fRec15[0];
		for (int j0 = 11; j0 > 0; j0 = j0 - 1) {
			fVec1[j0] = fVec1[j0 - 1];
		}
		fRec10[1] = fRec10[0];
		for (int j1 = 11; j1 > 0; j1 = j1 - 1) {
			fRec11[j1] = fRec11[j1 - 1];
		}
		for (int j2 = 11; j2 > 0; j2 = j2 - 1) {
			fRec12[j2] = fRec12[j2 - 1];
		}
		for (int j3 = 11; j3 > 0; j3 = j3 - 1) {
			fRec13[j3] = fRec13[j3 - 1];
		}
		for (int j4 = 11; j4 > 0; j4 = j4 - 1) {
			fRec14[j4] = fRec14[j4 - 1];
		}
		fRec9[2] = fRec9[1];
		fRec9[1] = fRec9[0];
		fRec1[1] = fRec1[0];
		iRec2[1] = iRec2[0];
		iRec3[1] = iRec3[0];
		fRec4[1] = fRec4[0];
		fRec5[1] = fRec5[0];
		iRec6[1] = iRec6[0];
		fRec0[1] = fRec0[0];
		fRec18[1] = fRec18[0];
	}
}

void __rt_func Dsp::compute_static(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0, PluginDef *p)
{
	static_cast<Dsp*>(p)->compute(count, input0, output0);
}

int Dsp::register_par(const ParamReg& reg)
{
	reg.registerFloatVar("audition_ocd.drive",N_("Drive"),"S","",&fHslider2, 0.4, 0.0, 1.0, 0.01, 0);
	reg.registerFloatVar("audition_ocd.hp",N_("HP Mode"),"S","",&fHslider1, 1.0, 0.0, 1.0, 1.0, 0);
	reg.registerFloatVar("audition_ocd.level",N_("Level"),"S","",&fHslider3, 0.0, -24.0, 12.0, 0.1, 0);
	reg.registerFloatVar("audition_ocd.tone",N_("Tone"),"S","",&fHslider0, 0.5, 0.0, 1.0, 0.01, 0);
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

} // end namespace audition_ocd

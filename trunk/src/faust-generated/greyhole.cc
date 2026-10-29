// generated from file '../src/faust/greyhole.dsp' by dsp2cc:
// Code generated with Faust (https://faust.grame.fr)


namespace greyhole {
const static int imydspSIG0Wave0[2048] = {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,199,211,223,227,229,233,239,241,251,257,263,269,271,277,281,283,293,307,311,313,317,331,337,347,349,353,359,367,373,379,383,389,397,401,409,419,421,431,433,439,443,449,457,461,463,467,479,487,491,499,503,509,521,523,541,547,557,563,569,571,577,587,593,599,601,607,613,617,619,631,641,643,647,653,659,661,673,677,683,691,701,709,719,727,733,739,743,751,757,761,769,773,787,797,809,811,821,823,827,829,839,853,857,859,863,877,881,883,887,907,911,919,929,937,941,947,953,967,971,977,983,991,997,1009,1013,1019,1021,1031,1033,1039,1049,1051,1061,1063,1069,1087,1091,1093,1097,1103,1109,1117,1123,1129,1151,1153,1163,1171,1181,1187,1193,1201,1213,1217,1223,1229,1231,1237,1249,1259,1277,1279,1283,1289,1291,1297,1301,1303,1307,1319,1321,1327,1361,1367,1373,1381,1399,1409,1423,1427,1429,1433,1439,1447,1451,1453,1459,1471,1481,1483,1487,1489,1493,1499,1511,1523,1531,1543,1549,1553,1559,1567,1571,1579,1583,1597,1601,1607,1609,1613,1619,1621,1627,1637,1657,1663,1667,1669,1693,1697,1699,1709,1721,1723,1733,1741,1747,1753,1759,1777,1783,1787,1789,1801,1811,1823,1831,1847,1861,1867,1871,1873,1877,1879,1889,1901,1907,1913,1931,1933,1949,1951,1973,1979,1987,1993,1997,1999,2003,2011,2017,2027,2029,2039,2053,2063,2069,2081,2083,2087,2089,2099,2111,2113,2129,2131,2137,2141,2143,2153,2161,2179,2203,2207,2213,2221,2237,2239,2243,2251,2267,2269,2273,2281,2287,2293,2297,2309,2311,2333,2339,2341,2347,2351,2357,2371,2377,2381,2383,2389,2393,2399,2411,2417,2423,2437,2441,2447,2459,2467,2473,2477,2503,2521,2531,2539,2543,2549,2551,2557,2579,2591,2593,2609,2617,2621,2633,2647,2657,2659,2663,2671,2677,2683,2687,2689,2693,2699,2707,2711,2713,2719,2729,2731,2741,2749,2753,2767,2777,2789,2791,2797,2801,2803,2819,2833,2837,2843,2851,2857,2861,2879,2887,2897,2903,2909,2917,2927,2939,2953,2957,2963,2969,2971,2999,3001,3011,3019,3023,3037,3041,3049,3061,3067,3079,3083,3089,3109,3119,3121,3137,3163,3167,3169,3181,3187,3191,3203,3209,3217,3221,3229,3251,3253,3257,3259,3271,3299,3301,3307,3313,3319,3323,3329,3331,3343,3347,3359,3361,3371,3373,3389,3391,3407,3413,3433,3449,3457,3461,3463,3467,3469,3491,3499,3511,3517,3527,3529,3533,3539,3541,3547,3557,3559,3571,3581,3583,3593,3607,3613,3617,3623,3631,3637,3643,3659,3671,3673,3677,3691,3697,3701,3709,3719,3727,3733,3739,3761,3767,3769,3779,3793,3797,3803,3821,3823,3833,3847,3851,3853,3863,3877,3881,3889,3907,3911,3917,3919,3923,3929,3931,3943,3947,3967,3989,4001,4003,4007,4013,4019,4021,4027,4049,4051,4057,4073,4079,4091,4093,4099,4111,4127,4129,4133,4139,4153,4157,4159,4177,4201,4211,4217,4219,4229,4231,4241,4243,4253,4259,4261,4271,4273,4283,4289,4297,4327,4337,4339,4349,4357,4363,4373,4391,4397,4409,4421,4423,4441,4447,4451,4457,4463,4481,4483,4493,4507,4513,4517,4519,4523,4547,4549,4561,4567,4583,4591,4597,4603,4621,4637,4639,4643,4649,4651,4657,4663,4673,4679,4691,4703,4721,4723,4729,4733,4751,4759,4783,4787,4789,4793,4799,4801,4813,4817,4831,4861,4871,4877,4889,4903,4909,4919,4931,4933,4937,4943,4951,4957,4967,4969,4973,4987,4993,4999,5003,5009,5011,5021,5023,5039,5051,5059,5077,5081,5087,5099,5101,5107,5113,5119,5147,5153,5167,5171,5179,5189,5197,5209,5227,5231,5233,5237,5261,5273,5279,5281,5297,5303,5309,5323,5333,5347,5351,5381,5387,5393,5399,5407,5413,5417,5419,5431,5437,5441,5443,5449,5471,5477,5479,5483,5501,5503,5507,5519,5521,5527,5531,5557,5563,5569,5573,5581,5591,5623,5639,5641,5647,5651,5653,5657,5659,5669,5683,5689,5693,5701,5711,5717,5737,5741,5743,5749,5779,5783,5791,5801,5807,5813,5821,5827,5839,5843,5849,5851,5857,5861,5867,5869,5879,5881,5897,5903,5923,5927,5939,5953,5981,5987,6007,6011,6029,6037,6043,6047,6053,6067,6073,6079,6089,6091,6101,6113,6121,6131,6133,6143,6151,6163,6173,6197,6199,6203,6211,6217,6221,6229,6247,6257,6263,6269,6271,6277,6287,6299,6301,6311,6317,6323,6329,6337,6343,6353,6359,6361,6367,6373,6379,6389,6397,6421,6427,6449,6451,6469,6473,6481,6491,6521,6529,6547,6551,6553,6563,6569,6571,6577,6581,6599,6607,6619,6637,6653,6659,6661,6673,6679,6689,6691,6701,6703,6709,6719,6733,6737,6761,6763,6779,6781,6791,6793,6803,6823,6827,6829,6833,6841,6857,6863,6869,6871,6883,6899,6907,6911,6917,6947,6949,6959,6961,6967,6971,6977,6983,6991,6997,7001,7013,7019,7027,7039,7043,7057,7069,7079,7103,7109,7121,7127,7129,7151,7159,7177,7187,7193,7207,7211,7213,7219,7229,7237,7243,7247,7253,7283,7297,7307,7309,7321,7331,7333,7349,7351,7369,7393,7411,7417,7433,7451,7457,7459,7477,7481,7487,7489,7499,7507,7517,7523,7529,7537,7541,7547,7549,7559,7561,7573,7577,7583,7589,7591,7603,7607,7621,7639,7643,7649,7669,7673,7681,7687,7691,7699,7703,7717,7723,7727,7741,7753,7757,7759,7789,7793,7817,7823,7829,7841,7853,7867,7873,7877,7879,7883,7901,7907,7919,7927,7933,7937,7949,7951,7963,7993,8009,8011,8017,8039,8053,8059,8069,8081,8087,8089,8093,8101,8111,8117,8123,8147,8161,8167,8171,8179,8191,8209,8219,8221,8231,8233,8237,8243,8263,8269,8273,8287,8291,8293,8297,8311,8317,8329,8353,8363,8369,8377,8387,8389,8419,8423,8429,8431,8443,8447,8461,8467,8501,8513,8521,8527,8537,8539,8543,8563,8573,8581,8597,8599,8609,8623,8627,8629,8641,8647,8663,8669,8677,8681,8689,8693,8699,8707,8713,8719,8731,8737,8741,8747,8753,8761,8779,8783,8803,8807,8819,8821,8831,8837,8839,8849,8861,8863,8867,8887,8893,8923,8929,8933,8941,8951,8963,8969,8971,8999,9001,9007,9011,9013,9029,9041,9043,9049,9059,9067,9091,9103,9109,9127,9133,9137,9151,9157,9161,9173,9181,9187,9199,9203,9209,9221,9227,9239,9241,9257,9277,9281,9283,9293,9311,9319,9323,9337,9341,9343,9349,9371,9377,9391,9397,9403,9413,9419,9421,9431,9433,9437,9439,9461,9463,9467,9473,9479,9491,9497,9511,9521,9533,9539,9547,9551,9587,9601,9613,9619,9623,9629,9631,9643,9649,9661,9677,9679,9689,9697,9719,9721,9733,9739,9743,9749,9767,9769,9781,9787,9791,9803,9811,9817,9829,9833,9839,9851,9857,9859,9871,9883,9887,9901,9907,9923,9929,9931,9941,9949,9967,9973,10007,10009,10037,10039,10061,10067,10069,10079,10091,10093,10099,10103,10111,10133,10139,10141,10151,10159,10163,10169,10177,10181,10193,10211,10223,10243,10247,10253,10259,10267,10271,10273,10289,10301,10303,10313,10321,10331,10333,10337,10343,10357,10369,10391,10399,10427,10429,10433,10453,10457,10459,10463,10477,10487,10499,10501,10513,10529,10531,10559,10567,10589,10597,10601,10607,10613,10627,10631,10639,10651,10657,10663,10667,10687,10691,10709,10711,10723,10729,10733,10739,10753,10771,10781,10789,10799,10831,10837,10847,10853,10859,10861,10867,10883,10889,10891,10903,10909,10937,10939,10949,10957,10973,10979,10987,10993,11003,11027,11047,11057,11059,11069,11071,11083,11087,11093,11113,11117,11119,11131,11149,11159,11161,11171,11173,11177,11197,11213,11239,11243,11251,11257,11261,11273,11279,11287,11299,11311,11317,11321,11329,11351,11353,11369,11383,11393,11399,11411,11423,11437,11443,11447,11467,11471,11483,11489,11491,11497,11503,11519,11527,11549,11551,11579,11587,11593,11597,11617,11621,11633,11657,11677,11681,11689,11699,11701,11717,11719,11731,11743,11777,11779,11783,11789,11801,11807,11813,11821,11827,11831,11833,11839,11863,11867,11887,11897,11903,11909,11923,11927,11933,11939,11941,11953,11959,11969,11971,11981,11987,12007,12011,12037,12041,12043,12049,12071,12073,12097,12101,12107,12109,12113,12119,12143,12149,12157,12161,12163,12197,12203,12211,12227,12239,12241,12251,12253,12263,12269,12277,12281,12289,12301,12323,12329,12343,12347,12373,12377,12379,12391,12401,12409,12413,12421,12433,12437,12451,12457,12473,12479,12487,12491,12497,12503,12511,12517,12527,12539,12541,12547,12553,12569,12577,12583,12589,12601,12611,12613,12619,12637,12641,12647,12653,12659,12671,12689,12697,12703,12713,12721,12739,12743,12757,12763,12781,12791,12799,12809,12821,12823,12829,12841,12853,12889,12893,12899,12907,12911,12917,12919,12923,12941,12953,12959,12967,12973,12979,12983,13001,13003,13007,13009,13033,13037,13043,13049,13063,13093,13099,13103,13109,13121,13127,13147,13151,13159,13163,13171,13177,13183,13187,13217,13219,13229,13241,13249,13259,13267,13291,13297,13309,13313,13327,13331,13337,13339,13367,13381,13397,13399,13411,13417,13421,13441,13451,13457,13463,13469,13477,13487,13499,13513,13523,13537,13553,13567,13577,13591,13597,13613,13619,13627,13633,13649,13669,13679,13681,13687,13691,13693,13697,13709,13711,13721,13723,13729,13751,13757,13759,13763,13781,13789,13799,13807,13829,13831,13841,13859,13873,13877,13879,13883,13901,13903,13907,13913,13921,13931,13933,13963,13967,13997,13999,14009,14011,14029,14033,14051,14057,14071,14081,14083,14087,14107,14143,14149,14153,14159,14173,14177,14197,14207,14221,14243,14249,14251,14281,14293,14303,14321,14323,14327,14341,14347,14369,14387,14389,14401,14407,14411,14419,14423,14431,14437,14447,14449,14461,14479,14489,14503,14519,14533,14537,14543,14549,14551,14557,14561,14563,14591,14593,14621,14627,14629,14633,14639,14653,14657,14669,14683,14699,14713,14717,14723,14731,14737,14741,14747,14753,14759,14767,14771,14779,14783,14797,14813,14821,14827,14831,14843,14851,14867,14869,14879,14887,14891,14897,14923,14929,14939,14947,14951,14957,14969,14983,15013,15017,15031,15053,15061,15073,15077,15083,15091,15101,15107,15121,15131,15137,15139,15149,15161,15173,15187,15193,15199,15217,15227,15233,15241,15259,15263,15269,15271,15277,15287,15289,15299,15307,15313,15319,15329,15331,15349,15359,15361,15373,15377,15383,15391,15401,15413,15427,15439,15443,15451,15461,15467,15473,15493,15497,15511,15527,15541,15551,15559,15569,15581,15583,15601,15607,15619,15629,15641,15643,15647,15649,15661,15667,15671,15679,15683,15727,15731,15733,15737,15739,15749,15761,15767,15773,15787,15791,15797,15803,15809,15817,15823,15859,15877,15881,15887,15889,15901,15907,15913,15919,15923,15937,15959,15971,15973,15991,16001,16007,16033,16057,16061,16063,16067,16069,16073,16087,16091,16097,16103,16111,16127,16139,16141,16183,16187,16189,16193,16217,16223,16229,16231,16249,16253,16267,16273,16301,16319,16333,16339,16349,16361,16363,16369,16381,16411,16417,16421,16427,16433,16447,16451,16453,16477,16481,16487,16493,16519,16529,16547,16553,16561,16567,16573,16603,16607,16619,16631,16633,16649,16651,16657,16661,16673,16691,16693,16699,16703,16729,16741,16747,16759,16763,16787,16811,16823,16829,16831,16843,16871,16879,16883,16889,16901,16903,16921,16927,16931,16937,16943,16963,16979,16981,16987,16993,17011,17021,17027,17029,17033,17041,17047,17053,17077,17093,17099,17107,17117,17123,17137,17159,17167,17183,17189,17191,17203,17207,17209,17231,17239,17257,17291,17293,17299,17317,17321,17327,17333,17341,17351,17359,17377,17383,17387,17389,17393,17401,17417,17419,17431,17443,17449,17467,17471,17477,17483,17489,17491,17497,17509,17519,17539,17551,17569,17573,17579,17581,17597,17599,17609,17623,17627,17657,17659,17669,17681,17683,17707,17713,17729,17737,17747,17749,17761,17783,17789,17791,17807,17827,17837,17839,17851,17863};
class mydspSIG0 {
	
  private:
	
	int imydspSIG0Wave0_idx;
	
  public:
	
	int getNumInputsmydspSIG0() {
		return 0;
	}
	int getNumOutputsmydspSIG0() {
		return 1;
	}
	
	void instanceInitmydspSIG0(int sample_rate) {
		imydspSIG0Wave0_idx = 0;
	}
	
	void fillmydspSIG0(int count, int* table) {
		for (int i1 = 0; i1 < count; i1 = i1 + 1) {
			table[i1] = imydspSIG0Wave0[imydspSIG0Wave0_idx];
			imydspSIG0Wave0_idx = (1 + imydspSIG0Wave0_idx) % 2048;
		}
	}

};

static mydspSIG0* newmydspSIG0() { return (mydspSIG0*)new mydspSIG0(); }
static void deletemydspSIG0(mydspSIG0* dsp) { delete dsp; }

static int itbl0mydspSIG0[2048];


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
	FAUSTFLOAT fHslider1;
	double fRec4[2];
	FAUSTFLOAT fHslider2;
	double fRec8[2];
	double fVec1[2];
	double fRec12[2];
	double fRec16[2];
	double fRec23[2];
	double fRec27[2];
	double fRec31[2];
	double fRec38[2];
	double fRec42[2];
	double fRec46[2];
	FAUSTFLOAT fHslider3;
	double fRec50[2];
	double fConst4;
	FAUSTFLOAT fHslider4;
	double fRec51[2];
	double fConst5;
	double fVec2[2];
	double fRec52[2];
	double fRec53[2];
	int IOTA0;
	double *fVec3;
	double fConst6;
	FAUSTFLOAT fHslider5;
	double fConst7;
	double fRec54[2];
	double fRec55[2];
	double fRec56[2];
	double fRec57[2];
	double *fVec4;
	double fRec58[2];
	double fVec5[2];
	double fRec49[2];
	double fRec47[2];
	double *fVec6;
	double *fVec7;
	double fRec60[2];
	double fVec8[2];
	double fRec59[2];
	double fRec48[2];
	double *fVec9;
	double fVec10[2];
	double fRec45[2];
	double fRec43[2];
	double *fVec11;
	double fRec62[2];
	double fVec12[2];
	double fRec61[2];
	double fRec44[2];
	double *fVec13;
	double fVec14[2];
	double fRec41[2];
	double fRec39[2];
	double *fVec15;
	double fRec64[2];
	double fVec16[2];
	double fRec63[2];
	double fRec40[2];
	double *fVec17;
	double fVec18[2];
	double fRec37[2];
	double fRec35[2];
	double *fVec19;
	double fRec66[2];
	double fVec20[2];
	double fRec65[2];
	double fRec36[2];
	double *fVec21;
	double fRec67[2];
	double fVec22[2];
	double fRec34[2];
	double fRec32[2];
	double *fVec23;
	double fRec69[2];
	double fVec24[2];
	double fRec68[2];
	double fRec33[2];
	double *fVec25;
	double fVec26[2];
	double fRec30[2];
	double fRec28[2];
	double *fVec27;
	double fRec71[2];
	double fVec28[2];
	double fRec70[2];
	double fRec29[2];
	double *fVec29;
	double fVec30[2];
	double fRec26[2];
	double fRec24[2];
	double *fVec31;
	double fRec73[2];
	double fVec32[2];
	double fRec72[2];
	double fRec25[2];
	double *fVec33;
	double fVec34[2];
	double fRec22[2];
	double fRec20[2];
	double *fVec35;
	double fRec75[2];
	double fVec36[2];
	double fRec74[2];
	double fRec21[2];
	double *fVec37;
	double fRec76[2];
	double fVec38[2];
	double fRec19[2];
	double fRec17[2];
	double *fVec39;
	double fRec78[2];
	double fVec40[2];
	double fRec77[2];
	double fRec18[2];
	double *fVec41;
	double fVec42[2];
	double fRec15[2];
	double fRec13[2];
	double *fVec43;
	double fRec80[2];
	double fVec44[2];
	double fRec79[2];
	double fRec14[2];
	double *fVec45;
	double fVec46[2];
	double fRec11[2];
	double fRec9[2];
	double *fVec47;
	double fRec82[2];
	double fVec48[2];
	double fRec81[2];
	double fRec10[2];
	double *fVec49;
	double fVec50[2];
	double fRec7[2];
	double fRec5[2];
	double *fVec51;
	double fRec84[2];
	double fVec52[2];
	double fRec83[2];
	double fRec6[2];
	double fRec3[2];
	double fRec1[1024];
	double fRec85[2];
	double fRec2[1024];

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
	  fVec3(0),
	  fVec4(0),
	  fVec6(0),
	  fVec7(0),
	  fVec9(0),
	  fVec11(0),
	  fVec13(0),
	  fVec15(0),
	  fVec17(0),
	  fVec19(0),
	  fVec21(0),
	  fVec23(0),
	  fVec25(0),
	  fVec27(0),
	  fVec29(0),
	  fVec31(0),
	  fVec33(0),
	  fVec35(0),
	  fVec37(0),
	  fVec39(0),
	  fVec41(0),
	  fVec43(0),
	  fVec45(0),
	  fVec47(0),
	  fVec49(0),
	  fVec51(0),
	  mem_allocated(false),
	  pending_sample_rate(0) {
	version = PLUGINDEF_VERSION;
	flags = 0;
	id = "greyhole";
	name = N_("Greyhole");
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
	for (int l2 = 0; l2 < 2; l2 = l2 + 1) fRec4[l2] = 0.0;
	for (int l3 = 0; l3 < 2; l3 = l3 + 1) fRec8[l3] = 0.0;
	for (int l4 = 0; l4 < 2; l4 = l4 + 1) fVec1[l4] = 0.0;
	for (int l5 = 0; l5 < 2; l5 = l5 + 1) fRec12[l5] = 0.0;
	for (int l6 = 0; l6 < 2; l6 = l6 + 1) fRec16[l6] = 0.0;
	for (int l7 = 0; l7 < 2; l7 = l7 + 1) fRec23[l7] = 0.0;
	for (int l8 = 0; l8 < 2; l8 = l8 + 1) fRec27[l8] = 0.0;
	for (int l9 = 0; l9 < 2; l9 = l9 + 1) fRec31[l9] = 0.0;
	for (int l10 = 0; l10 < 2; l10 = l10 + 1) fRec38[l10] = 0.0;
	for (int l11 = 0; l11 < 2; l11 = l11 + 1) fRec42[l11] = 0.0;
	for (int l12 = 0; l12 < 2; l12 = l12 + 1) fRec46[l12] = 0.0;
	for (int l13 = 0; l13 < 2; l13 = l13 + 1) fRec50[l13] = 0.0;
	for (int l14 = 0; l14 < 2; l14 = l14 + 1) fRec51[l14] = 0.0;
	for (int l15 = 0; l15 < 2; l15 = l15 + 1) fVec2[l15] = 0.0;
	for (int l16 = 0; l16 < 2; l16 = l16 + 1) fRec52[l16] = 0.0;
	for (int l17 = 0; l17 < 2; l17 = l17 + 1) fRec53[l17] = 0.0;
	for (int l18 = 0; l18 < 1048576; l18 = l18 + 1) fVec3[l18] = 0.0;
	for (int l19 = 0; l19 < 2; l19 = l19 + 1) fRec54[l19] = 0.0;
	for (int l20 = 0; l20 < 2; l20 = l20 + 1) fRec55[l20] = 0.0;
	for (int l21 = 0; l21 < 2; l21 = l21 + 1) fRec56[l21] = 0.0;
	for (int l22 = 0; l22 < 2; l22 = l22 + 1) fRec57[l22] = 0.0;
	for (int l23 = 0; l23 < 16384; l23 = l23 + 1) fVec4[l23] = 0.0;
	for (int l24 = 0; l24 < 2; l24 = l24 + 1) fRec58[l24] = 0.0;
	for (int l25 = 0; l25 < 2; l25 = l25 + 1) fVec5[l25] = 0.0;
	for (int l26 = 0; l26 < 2; l26 = l26 + 1) fRec49[l26] = 0.0;
	for (int l27 = 0; l27 < 2; l27 = l27 + 1) fRec47[l27] = 0.0;
	for (int l28 = 0; l28 < 1048576; l28 = l28 + 1) fVec6[l28] = 0.0;
	for (int l29 = 0; l29 < 16384; l29 = l29 + 1) fVec7[l29] = 0.0;
	for (int l30 = 0; l30 < 2; l30 = l30 + 1) fRec60[l30] = 0.0;
	for (int l31 = 0; l31 < 2; l31 = l31 + 1) fVec8[l31] = 0.0;
	for (int l32 = 0; l32 < 2; l32 = l32 + 1) fRec59[l32] = 0.0;
	for (int l33 = 0; l33 < 2; l33 = l33 + 1) fRec48[l33] = 0.0;
	for (int l34 = 0; l34 < 16384; l34 = l34 + 1) fVec9[l34] = 0.0;
	for (int l35 = 0; l35 < 2; l35 = l35 + 1) fVec10[l35] = 0.0;
	for (int l36 = 0; l36 < 2; l36 = l36 + 1) fRec45[l36] = 0.0;
	for (int l37 = 0; l37 < 2; l37 = l37 + 1) fRec43[l37] = 0.0;
	for (int l38 = 0; l38 < 16384; l38 = l38 + 1) fVec11[l38] = 0.0;
	for (int l39 = 0; l39 < 2; l39 = l39 + 1) fRec62[l39] = 0.0;
	for (int l40 = 0; l40 < 2; l40 = l40 + 1) fVec12[l40] = 0.0;
	for (int l41 = 0; l41 < 2; l41 = l41 + 1) fRec61[l41] = 0.0;
	for (int l42 = 0; l42 < 2; l42 = l42 + 1) fRec44[l42] = 0.0;
	for (int l43 = 0; l43 < 16384; l43 = l43 + 1) fVec13[l43] = 0.0;
	for (int l44 = 0; l44 < 2; l44 = l44 + 1) fVec14[l44] = 0.0;
	for (int l45 = 0; l45 < 2; l45 = l45 + 1) fRec41[l45] = 0.0;
	for (int l46 = 0; l46 < 2; l46 = l46 + 1) fRec39[l46] = 0.0;
	for (int l47 = 0; l47 < 16384; l47 = l47 + 1) fVec15[l47] = 0.0;
	for (int l48 = 0; l48 < 2; l48 = l48 + 1) fRec64[l48] = 0.0;
	for (int l49 = 0; l49 < 2; l49 = l49 + 1) fVec16[l49] = 0.0;
	for (int l50 = 0; l50 < 2; l50 = l50 + 1) fRec63[l50] = 0.0;
	for (int l51 = 0; l51 < 2; l51 = l51 + 1) fRec40[l51] = 0.0;
	for (int l52 = 0; l52 < 16384; l52 = l52 + 1) fVec17[l52] = 0.0;
	for (int l53 = 0; l53 < 2; l53 = l53 + 1) fVec18[l53] = 0.0;
	for (int l54 = 0; l54 < 2; l54 = l54 + 1) fRec37[l54] = 0.0;
	for (int l55 = 0; l55 < 2; l55 = l55 + 1) fRec35[l55] = 0.0;
	for (int l56 = 0; l56 < 16384; l56 = l56 + 1) fVec19[l56] = 0.0;
	for (int l57 = 0; l57 < 2; l57 = l57 + 1) fRec66[l57] = 0.0;
	for (int l58 = 0; l58 < 2; l58 = l58 + 1) fVec20[l58] = 0.0;
	for (int l59 = 0; l59 < 2; l59 = l59 + 1) fRec65[l59] = 0.0;
	for (int l60 = 0; l60 < 2; l60 = l60 + 1) fRec36[l60] = 0.0;
	for (int l61 = 0; l61 < 16384; l61 = l61 + 1) fVec21[l61] = 0.0;
	for (int l62 = 0; l62 < 2; l62 = l62 + 1) fRec67[l62] = 0.0;
	for (int l63 = 0; l63 < 2; l63 = l63 + 1) fVec22[l63] = 0.0;
	for (int l64 = 0; l64 < 2; l64 = l64 + 1) fRec34[l64] = 0.0;
	for (int l65 = 0; l65 < 2; l65 = l65 + 1) fRec32[l65] = 0.0;
	for (int l66 = 0; l66 < 16384; l66 = l66 + 1) fVec23[l66] = 0.0;
	for (int l67 = 0; l67 < 2; l67 = l67 + 1) fRec69[l67] = 0.0;
	for (int l68 = 0; l68 < 2; l68 = l68 + 1) fVec24[l68] = 0.0;
	for (int l69 = 0; l69 < 2; l69 = l69 + 1) fRec68[l69] = 0.0;
	for (int l70 = 0; l70 < 2; l70 = l70 + 1) fRec33[l70] = 0.0;
	for (int l71 = 0; l71 < 16384; l71 = l71 + 1) fVec25[l71] = 0.0;
	for (int l72 = 0; l72 < 2; l72 = l72 + 1) fVec26[l72] = 0.0;
	for (int l73 = 0; l73 < 2; l73 = l73 + 1) fRec30[l73] = 0.0;
	for (int l74 = 0; l74 < 2; l74 = l74 + 1) fRec28[l74] = 0.0;
	for (int l75 = 0; l75 < 16384; l75 = l75 + 1) fVec27[l75] = 0.0;
	for (int l76 = 0; l76 < 2; l76 = l76 + 1) fRec71[l76] = 0.0;
	for (int l77 = 0; l77 < 2; l77 = l77 + 1) fVec28[l77] = 0.0;
	for (int l78 = 0; l78 < 2; l78 = l78 + 1) fRec70[l78] = 0.0;
	for (int l79 = 0; l79 < 2; l79 = l79 + 1) fRec29[l79] = 0.0;
	for (int l80 = 0; l80 < 16384; l80 = l80 + 1) fVec29[l80] = 0.0;
	for (int l81 = 0; l81 < 2; l81 = l81 + 1) fVec30[l81] = 0.0;
	for (int l82 = 0; l82 < 2; l82 = l82 + 1) fRec26[l82] = 0.0;
	for (int l83 = 0; l83 < 2; l83 = l83 + 1) fRec24[l83] = 0.0;
	for (int l84 = 0; l84 < 16384; l84 = l84 + 1) fVec31[l84] = 0.0;
	for (int l85 = 0; l85 < 2; l85 = l85 + 1) fRec73[l85] = 0.0;
	for (int l86 = 0; l86 < 2; l86 = l86 + 1) fVec32[l86] = 0.0;
	for (int l87 = 0; l87 < 2; l87 = l87 + 1) fRec72[l87] = 0.0;
	for (int l88 = 0; l88 < 2; l88 = l88 + 1) fRec25[l88] = 0.0;
	for (int l89 = 0; l89 < 16384; l89 = l89 + 1) fVec33[l89] = 0.0;
	for (int l90 = 0; l90 < 2; l90 = l90 + 1) fVec34[l90] = 0.0;
	for (int l91 = 0; l91 < 2; l91 = l91 + 1) fRec22[l91] = 0.0;
	for (int l92 = 0; l92 < 2; l92 = l92 + 1) fRec20[l92] = 0.0;
	for (int l93 = 0; l93 < 16384; l93 = l93 + 1) fVec35[l93] = 0.0;
	for (int l94 = 0; l94 < 2; l94 = l94 + 1) fRec75[l94] = 0.0;
	for (int l95 = 0; l95 < 2; l95 = l95 + 1) fVec36[l95] = 0.0;
	for (int l96 = 0; l96 < 2; l96 = l96 + 1) fRec74[l96] = 0.0;
	for (int l97 = 0; l97 < 2; l97 = l97 + 1) fRec21[l97] = 0.0;
	for (int l98 = 0; l98 < 16384; l98 = l98 + 1) fVec37[l98] = 0.0;
	for (int l99 = 0; l99 < 2; l99 = l99 + 1) fRec76[l99] = 0.0;
	for (int l100 = 0; l100 < 2; l100 = l100 + 1) fVec38[l100] = 0.0;
	for (int l101 = 0; l101 < 2; l101 = l101 + 1) fRec19[l101] = 0.0;
	for (int l102 = 0; l102 < 2; l102 = l102 + 1) fRec17[l102] = 0.0;
	for (int l103 = 0; l103 < 16384; l103 = l103 + 1) fVec39[l103] = 0.0;
	for (int l104 = 0; l104 < 2; l104 = l104 + 1) fRec78[l104] = 0.0;
	for (int l105 = 0; l105 < 2; l105 = l105 + 1) fVec40[l105] = 0.0;
	for (int l106 = 0; l106 < 2; l106 = l106 + 1) fRec77[l106] = 0.0;
	for (int l107 = 0; l107 < 2; l107 = l107 + 1) fRec18[l107] = 0.0;
	for (int l108 = 0; l108 < 16384; l108 = l108 + 1) fVec41[l108] = 0.0;
	for (int l109 = 0; l109 < 2; l109 = l109 + 1) fVec42[l109] = 0.0;
	for (int l110 = 0; l110 < 2; l110 = l110 + 1) fRec15[l110] = 0.0;
	for (int l111 = 0; l111 < 2; l111 = l111 + 1) fRec13[l111] = 0.0;
	for (int l112 = 0; l112 < 16384; l112 = l112 + 1) fVec43[l112] = 0.0;
	for (int l113 = 0; l113 < 2; l113 = l113 + 1) fRec80[l113] = 0.0;
	for (int l114 = 0; l114 < 2; l114 = l114 + 1) fVec44[l114] = 0.0;
	for (int l115 = 0; l115 < 2; l115 = l115 + 1) fRec79[l115] = 0.0;
	for (int l116 = 0; l116 < 2; l116 = l116 + 1) fRec14[l116] = 0.0;
	for (int l117 = 0; l117 < 16384; l117 = l117 + 1) fVec45[l117] = 0.0;
	for (int l118 = 0; l118 < 2; l118 = l118 + 1) fVec46[l118] = 0.0;
	for (int l119 = 0; l119 < 2; l119 = l119 + 1) fRec11[l119] = 0.0;
	for (int l120 = 0; l120 < 2; l120 = l120 + 1) fRec9[l120] = 0.0;
	for (int l121 = 0; l121 < 16384; l121 = l121 + 1) fVec47[l121] = 0.0;
	for (int l122 = 0; l122 < 2; l122 = l122 + 1) fRec82[l122] = 0.0;
	for (int l123 = 0; l123 < 2; l123 = l123 + 1) fVec48[l123] = 0.0;
	for (int l124 = 0; l124 < 2; l124 = l124 + 1) fRec81[l124] = 0.0;
	for (int l125 = 0; l125 < 2; l125 = l125 + 1) fRec10[l125] = 0.0;
	for (int l126 = 0; l126 < 16384; l126 = l126 + 1) fVec49[l126] = 0.0;
	for (int l127 = 0; l127 < 2; l127 = l127 + 1) fVec50[l127] = 0.0;
	for (int l128 = 0; l128 < 2; l128 = l128 + 1) fRec7[l128] = 0.0;
	for (int l129 = 0; l129 < 2; l129 = l129 + 1) fRec5[l129] = 0.0;
	for (int l130 = 0; l130 < 16384; l130 = l130 + 1) fVec51[l130] = 0.0;
	for (int l131 = 0; l131 < 2; l131 = l131 + 1) fRec84[l131] = 0.0;
	for (int l132 = 0; l132 < 2; l132 = l132 + 1) fVec52[l132] = 0.0;
	for (int l133 = 0; l133 < 2; l133 = l133 + 1) fRec83[l133] = 0.0;
	for (int l134 = 0; l134 < 2; l134 = l134 + 1) fRec6[l134] = 0.0;
	for (int l135 = 0; l135 < 2; l135 = l135 + 1) fRec3[l135] = 0.0;
	for (int l136 = 0; l136 < 1024; l136 = l136 + 1) fRec1[l136] = 0.0;
	for (int l137 = 0; l137 < 2; l137 = l137 + 1) fRec85[l137] = 0.0;
	for (int l138 = 0; l138 < 1024; l138 = l138 + 1) fRec2[l138] = 0.0;
}

void Dsp::clear_state_f_static(PluginDef *p)
{
	static_cast<Dsp*>(p)->clear_state_f();
}

inline void Dsp::init(unsigned int sample_rate)
{
	mydspSIG0* sig0 = newmydspSIG0();
	sig0->instanceInitmydspSIG0(sample_rate);
	sig0->fillmydspSIG0(2048, itbl0mydspSIG0);
	deletemydspSIG0(sig0);
	fSampleRate = sample_rate;
	fConst0 = std::min<double>(1.92e+05, std::max<double>(1.0, static_cast<double>(fSampleRate)));
	fConst1 = 0.441 / fConst0;
	fConst2 = 44.1 / fConst0;
	fConst3 = 1.0 - fConst2;
	fConst4 = 0.0005668934240362812 * fConst0;
	fConst5 = 3.141592653589793 / fConst0;
	fConst6 = 0.001 * fConst0;
	fConst7 = 1.0 / static_cast<double>(static_cast<int>(0.05 * fConst0));
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
	if (!fVec3) fVec3 = new double[1048576];
	if (!fVec4) fVec4 = new double[16384];
	if (!fVec6) fVec6 = new double[1048576];
	if (!fVec7) fVec7 = new double[16384];
	if (!fVec9) fVec9 = new double[16384];
	if (!fVec11) fVec11 = new double[16384];
	if (!fVec13) fVec13 = new double[16384];
	if (!fVec15) fVec15 = new double[16384];
	if (!fVec17) fVec17 = new double[16384];
	if (!fVec19) fVec19 = new double[16384];
	if (!fVec21) fVec21 = new double[16384];
	if (!fVec23) fVec23 = new double[16384];
	if (!fVec25) fVec25 = new double[16384];
	if (!fVec27) fVec27 = new double[16384];
	if (!fVec29) fVec29 = new double[16384];
	if (!fVec31) fVec31 = new double[16384];
	if (!fVec33) fVec33 = new double[16384];
	if (!fVec35) fVec35 = new double[16384];
	if (!fVec37) fVec37 = new double[16384];
	if (!fVec39) fVec39 = new double[16384];
	if (!fVec41) fVec41 = new double[16384];
	if (!fVec43) fVec43 = new double[16384];
	if (!fVec45) fVec45 = new double[16384];
	if (!fVec47) fVec47 = new double[16384];
	if (!fVec49) fVec49 = new double[16384];
	if (!fVec51) fVec51 = new double[16384];
	mem_allocated = true;
}

void Dsp::mem_free()
{
	mem_allocated = false;
	if (fVec3) { delete[] fVec3; fVec3 = 0; }
	if (fVec4) { delete[] fVec4; fVec4 = 0; }
	if (fVec6) { delete[] fVec6; fVec6 = 0; }
	if (fVec7) { delete[] fVec7; fVec7 = 0; }
	if (fVec9) { delete[] fVec9; fVec9 = 0; }
	if (fVec11) { delete[] fVec11; fVec11 = 0; }
	if (fVec13) { delete[] fVec13; fVec13 = 0; }
	if (fVec15) { delete[] fVec15; fVec15 = 0; }
	if (fVec17) { delete[] fVec17; fVec17 = 0; }
	if (fVec19) { delete[] fVec19; fVec19 = 0; }
	if (fVec21) { delete[] fVec21; fVec21 = 0; }
	if (fVec23) { delete[] fVec23; fVec23 = 0; }
	if (fVec25) { delete[] fVec25; fVec25 = 0; }
	if (fVec27) { delete[] fVec27; fVec27 = 0; }
	if (fVec29) { delete[] fVec29; fVec29 = 0; }
	if (fVec31) { delete[] fVec31; fVec31 = 0; }
	if (fVec33) { delete[] fVec33; fVec33 = 0; }
	if (fVec35) { delete[] fVec35; fVec35 = 0; }
	if (fVec37) { delete[] fVec37; fVec37 = 0; }
	if (fVec39) { delete[] fVec39; fVec39 = 0; }
	if (fVec41) { delete[] fVec41; fVec41 = 0; }
	if (fVec43) { delete[] fVec43; fVec43 = 0; }
	if (fVec45) { delete[] fVec45; fVec45 = 0; }
	if (fVec47) { delete[] fVec47; fVec47 = 0; }
	if (fVec49) { delete[] fVec49; fVec49 = 0; }
	if (fVec51) { delete[] fVec51; fVec51 = 0; }
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
	double fSlow2 = static_cast<double>(fHslider2);
	int iSlow3 = itbl0mydspSIG0[static_cast<int>(48.0 * fSlow2)];
	double fSlow4 = 9.999999999998899e-05 * static_cast<double>(iSlow3);
	int iSlow5 = itbl0mydspSIG0[static_cast<int>(61.0 * fSlow2)];
	double fSlow6 = 9.999999999998899e-05 * static_cast<double>(iSlow5);
	int iSlow7 = itbl0mydspSIG0[static_cast<int>(74.0 * fSlow2)];
	double fSlow8 = 9.999999999998899e-05 * static_cast<double>(iSlow7);
	int iSlow9 = itbl0mydspSIG0[static_cast<int>(29.0 * fSlow2)];
	double fSlow10 = 9.999999999998899e-05 * static_cast<double>(iSlow9);
	int iSlow11 = itbl0mydspSIG0[static_cast<int>(42.0 * fSlow2)];
	double fSlow12 = 9.999999999998899e-05 * static_cast<double>(iSlow11);
	int iSlow13 = itbl0mydspSIG0[static_cast<int>(55.0 * fSlow2)];
	double fSlow14 = 9.999999999998899e-05 * static_cast<double>(iSlow13);
	int iSlow15 = itbl0mydspSIG0[static_cast<int>(1e+01 * fSlow2)];
	double fSlow16 = 9.999999999998899e-05 * static_cast<double>(iSlow15);
	int iSlow17 = itbl0mydspSIG0[static_cast<int>(23.0 * fSlow2)];
	double fSlow18 = 9.999999999998899e-05 * static_cast<double>(iSlow17);
	int iSlow19 = itbl0mydspSIG0[static_cast<int>(36.0 * fSlow2)];
	double fSlow20 = 9.999999999998899e-05 * static_cast<double>(iSlow19);
	double fSlow21 = fConst2 * static_cast<double>(fHslider3);
	double fSlow22 = fConst2 * static_cast<double>(fHslider4);
	double fSlow23 = std::floor(std::min<double>(524285.0, fConst6 * static_cast<double>(fHslider5)));
	int iSlow24 = itbl0mydspSIG0[static_cast<int>(49.0 * fSlow2)];
	double fSlow25 = 9.999999999998899e-05 * static_cast<double>(iSlow24);
	int iSlow26 = itbl0mydspSIG0[static_cast<int>(59.0 * fSlow2)];
	double fSlow27 = 9.999999999998899e-05 * static_cast<double>(iSlow26);
	int iSlow28 = itbl0mydspSIG0[static_cast<int>(46.0 * fSlow2)];
	double fSlow29 = 9.999999999998899e-05 * static_cast<double>(iSlow28);
	int iSlow30 = itbl0mydspSIG0[static_cast<int>(33.0 * fSlow2)];
	double fSlow31 = 9.999999999998899e-05 * static_cast<double>(iSlow30);
	int iSlow32 = itbl0mydspSIG0[static_cast<int>(2e+01 * fSlow2)];
	double fSlow33 = 9.999999999998899e-05 * static_cast<double>(iSlow32);
	int iSlow34 = itbl0mydspSIG0[static_cast<int>(68.0 * fSlow2)];
	double fSlow35 = 9.999999999998899e-05 * static_cast<double>(iSlow34);
	int iSlow36 = itbl0mydspSIG0[static_cast<int>(78.0 * fSlow2)];
	double fSlow37 = 9.999999999998899e-05 * static_cast<double>(iSlow36);
	int iSlow38 = itbl0mydspSIG0[static_cast<int>(65.0 * fSlow2)];
	double fSlow39 = 9.999999999998899e-05 * static_cast<double>(iSlow38);
	int iSlow40 = itbl0mydspSIG0[static_cast<int>(52.0 * fSlow2)];
	double fSlow41 = 9.999999999998899e-05 * static_cast<double>(iSlow40);
	int iSlow42 = itbl0mydspSIG0[static_cast<int>(39.0 * fSlow2)];
	double fSlow43 = 9.999999999998899e-05 * static_cast<double>(iSlow42);
	int iSlow44 = itbl0mydspSIG0[static_cast<int>(87.0 * fSlow2)];
	double fSlow45 = 9.999999999998899e-05 * static_cast<double>(iSlow44);
	int iSlow46 = itbl0mydspSIG0[static_cast<int>(97.0 * fSlow2)];
	double fSlow47 = 9.999999999998899e-05 * static_cast<double>(iSlow46);
	int iSlow48 = itbl0mydspSIG0[static_cast<int>(84.0 * fSlow2)];
	double fSlow49 = 9.999999999998899e-05 * static_cast<double>(iSlow48);
	int iSlow50 = itbl0mydspSIG0[static_cast<int>(71.0 * fSlow2)];
	double fSlow51 = 9.999999999998899e-05 * static_cast<double>(iSlow50);
	int iSlow52 = itbl0mydspSIG0[static_cast<int>(58.0 * fSlow2)];
	double fSlow53 = 9.999999999998899e-05 * static_cast<double>(iSlow52);
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		double fTemp0 = static_cast<double>(input0[i0]);
		iVec0[0] = 1;
		fRec0[0] = fSlow0 + fConst3 * fRec0[1];
		fRec4[0] = fSlow1 + fConst3 * fRec4[1];
		double fTemp1 = fRec4[0] + fRec4[1];
		double fTemp2 = 1.0 - 0.5 * fTemp1;
		int iTemp3 = 1 - iVec0[1];
		fRec8[0] = 0.9999 * (fRec8[1] + static_cast<double>(iTemp3 * iSlow3)) + fSlow4;
		double fTemp4 = fRec8[0] + -1.49999;
		double fTemp5 = std::floor(fTemp4);
		double fTemp6 = fTemp5 + (2.0 - fRec8[0]);
		double fTemp7 = fRec8[0] - fTemp5;
		fVec1[0] = 0.65;
		double fTemp8 = fVec1[1] + 0.65;
		double fTemp9 = 0.5 * fTemp8;
		double fTemp10 = std::cos(fTemp9);
		fRec12[0] = 0.9999 * (fRec12[1] + static_cast<double>(iTemp3 * iSlow5)) + fSlow6;
		double fTemp11 = fRec12[0] + -1.49999;
		double fTemp12 = std::floor(fTemp11);
		double fTemp13 = fTemp12 + (2.0 - fRec12[0]);
		double fTemp14 = fRec12[0] - fTemp12;
		fRec16[0] = 0.9999 * (fRec16[1] + static_cast<double>(iTemp3 * iSlow7)) + fSlow8;
		double fTemp15 = fRec16[0] + -1.49999;
		double fTemp16 = std::floor(fTemp15);
		double fTemp17 = fTemp16 + (2.0 - fRec16[0]);
		double fTemp18 = fRec16[0] - fTemp16;
		double fTemp19 = std::sin(fTemp9);
		double fTemp20 = -0.5 * fTemp8;
		double fTemp21 = std::cos(fTemp20);
		fRec23[0] = 0.9999 * (fRec23[1] + static_cast<double>(iTemp3 * iSlow9)) + fSlow10;
		double fTemp22 = fRec23[0] + -1.49999;
		double fTemp23 = std::floor(fTemp22);
		double fTemp24 = fTemp23 + (2.0 - fRec23[0]);
		double fTemp25 = fRec23[0] - fTemp23;
		fRec27[0] = 0.9999 * (fRec27[1] + static_cast<double>(iTemp3 * iSlow11)) + fSlow12;
		double fTemp26 = fRec27[0] + -1.49999;
		double fTemp27 = std::floor(fTemp26);
		double fTemp28 = fTemp27 + (2.0 - fRec27[0]);
		double fTemp29 = fRec27[0] - fTemp27;
		fRec31[0] = 0.9999 * (fRec31[1] + static_cast<double>(iTemp3 * iSlow13)) + fSlow14;
		double fTemp30 = fRec31[0] + -1.49999;
		double fTemp31 = std::floor(fTemp30);
		double fTemp32 = fTemp31 + (2.0 - fRec31[0]);
		double fTemp33 = fRec31[0] - fTemp31;
		double fTemp34 = std::sin(fTemp20);
		fRec38[0] = 0.9999 * (fRec38[1] + static_cast<double>(iTemp3 * iSlow15)) + fSlow16;
		double fTemp35 = fRec38[0] + -1.49999;
		double fTemp36 = std::floor(fTemp35);
		double fTemp37 = fTemp36 + (2.0 - fRec38[0]);
		double fTemp38 = fRec38[0] - fTemp36;
		fRec42[0] = 0.9999 * (fRec42[1] + static_cast<double>(iTemp3 * iSlow17)) + fSlow18;
		double fTemp39 = fRec42[0] + -1.49999;
		double fTemp40 = std::floor(fTemp39);
		double fTemp41 = fTemp40 + (2.0 - fRec42[0]);
		double fTemp42 = fRec42[0] - fTemp40;
		fRec46[0] = 0.9999 * (fRec46[1] + static_cast<double>(iTemp3 * iSlow19)) + fSlow20;
		double fTemp43 = fRec46[0] + -1.49999;
		double fTemp44 = std::floor(fTemp43);
		double fTemp45 = fTemp44 + (2.0 - fRec46[0]);
		double fTemp46 = fRec46[0] - fTemp44;
		fRec50[0] = fSlow21 + fConst3 * fRec50[1];
		double fTemp47 = fRec50[0] + fRec50[1];
		fRec51[0] = fSlow22 + fConst3 * fRec51[1];
		double fTemp48 = fRec51[0] + fRec51[1];
		fVec2[0] = 0.3;
		double fTemp49 = fConst5 * (fVec2[1] + 0.3);
		double fTemp50 = std::sin(fTemp49);
		double fTemp51 = std::cos(fTemp49);
		fRec52[0] = fRec53[1] * fTemp50 + fRec52[1] * fTemp51;
		fRec53[0] = static_cast<double>(iTemp3) + fRec53[1] * fTemp51 - fTemp50 * fRec52[1];
		double fTemp52 = fConst4 * fTemp48 * (fRec52[0] + 1.0);
		double fTemp53 = fTemp52 + 8.500005;
		double fTemp54 = std::floor(fTemp53);
		double fTemp55 = fTemp52 + (7.0 - fTemp54);
		double fTemp56 = fTemp52 + (8.0 - fTemp54);
		int iTemp57 = static_cast<int>(fTemp53);
		double fTemp58 = fTemp52 + (9.0 - fTemp54);
		double fTemp59 = fTemp52 + (1e+01 - fTemp54);
		double fTemp60 = fTemp59 * fTemp58;
		double fTemp61 = fTemp60 * fTemp56;
		double fTemp62 = (fTemp52 + (6.0 - fTemp54)) * (fTemp55 * (fTemp56 * (0.041666666666666664 * fRec2[(IOTA0 - (std::min<int>(512, std::max<int>(0, iTemp57)) + 1)) & 1023] * fTemp58 - 0.16666666666666666 * fTemp59 * fRec2[(IOTA0 - (std::min<int>(512, std::max<int>(0, iTemp57 + 1)) + 1)) & 1023]) + 0.25 * fTemp60 * fRec2[(IOTA0 - (std::min<int>(512, std::max<int>(0, iTemp57 + 2)) + 1)) & 1023]) - 0.16666666666666666 * fTemp61 * fRec2[(IOTA0 - (std::min<int>(512, std::max<int>(0, iTemp57 + 3)) + 1)) & 1023]) + 0.041666666666666664 * fTemp61 * fTemp55 * fRec2[(IOTA0 - (std::min<int>(512, std::max<int>(0, iTemp57 + 4)) + 1)) & 1023];
		fVec3[IOTA0 & 1048575] = fTemp62;
		double fTemp63 = ((fRec54[1] != 0.0) ? (((fRec55[1] > 0.0) & (fRec55[1] < 1.0)) ? fRec54[1] : 0.0) : (((fRec55[1] == 0.0) & (fSlow23 != fRec56[1])) ? fConst7 : (((fRec55[1] == 1.0) & (fSlow23 != fRec57[1])) ? -fConst7 : 0.0)));
		fRec54[0] = fTemp63;
		fRec55[0] = std::max<double>(0.0, std::min<double>(1.0, fRec55[1] + fTemp63));
		fRec56[0] = (((fRec55[1] >= 1.0) & (fRec57[1] != fSlow23)) ? fSlow23 : fRec56[1]);
		fRec57[0] = (((fRec55[1] <= 0.0) & (fRec56[1] != fSlow23)) ? fSlow23 : fRec57[1]);
		int iTemp64 = static_cast<int>(std::min<double>(524288.0, std::max<double>(0.0, fRec56[0])));
		double fTemp65 = fVec3[(IOTA0 - iTemp64) & 1048575];
		int iTemp66 = static_cast<int>(std::min<double>(524288.0, std::max<double>(0.0, fRec57[0])));
		double fTemp67 = fTemp0 + 0.5 * fTemp47 * (fTemp65 + fRec55[0] * (fVec3[(IOTA0 - iTemp66) & 1048575] - fTemp65));
		double fTemp68 = fTemp10 * fTemp67 - fTemp19 * fRec36[1];
		double fTemp69 = fTemp10 * fTemp68 - fTemp19 * fRec40[1];
		double fTemp70 = fTemp10 * fTemp69 - fTemp19 * fRec44[1];
		fVec4[IOTA0 & 16383] = fTemp19 * fRec48[1] - fTemp10 * fTemp70;
		fRec58[0] = 0.9999 * (fRec58[1] + static_cast<double>(iTemp3 * iSlow24)) + fSlow25;
		double fTemp71 = fRec58[0] + -1.49999;
		double fTemp72 = fVec4[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp71)))) & 16383];
		fVec5[0] = fTemp72;
		double fTemp73 = std::floor(fTemp71);
		fRec49[0] = fVec5[1] - (fTemp73 + (2.0 - fRec58[0])) * (fRec49[1] - fTemp72) / (fRec58[0] - fTemp73);
		fRec47[0] = fRec49[0];
		double fTemp74 = fConst4 * fTemp48 * (fRec53[0] + 1.0);
		double fTemp75 = fTemp74 + 8.500005;
		double fTemp76 = std::floor(fTemp75);
		double fTemp77 = fTemp74 + (7.0 - fTemp76);
		double fTemp78 = fTemp74 + (8.0 - fTemp76);
		int iTemp79 = static_cast<int>(fTemp75);
		double fTemp80 = fTemp74 + (9.0 - fTemp76);
		double fTemp81 = fTemp74 + (1e+01 - fTemp76);
		double fTemp82 = fTemp81 * fTemp80;
		double fTemp83 = fTemp82 * fTemp78;
		double fTemp84 = (fTemp74 + (6.0 - fTemp76)) * (fTemp77 * (fTemp78 * (0.041666666666666664 * fRec1[(IOTA0 - (std::min<int>(512, std::max<int>(0, iTemp79)) + 1)) & 1023] * fTemp80 - 0.16666666666666666 * fTemp81 * fRec1[(IOTA0 - (std::min<int>(512, std::max<int>(0, iTemp79 + 1)) + 1)) & 1023]) + 0.25 * fTemp82 * fRec1[(IOTA0 - (std::min<int>(512, std::max<int>(0, iTemp79 + 2)) + 1)) & 1023]) - 0.16666666666666666 * fTemp83 * fRec1[(IOTA0 - (std::min<int>(512, std::max<int>(0, iTemp79 + 3)) + 1)) & 1023]) + 0.041666666666666664 * fTemp83 * fTemp77 * fRec1[(IOTA0 - (std::min<int>(512, std::max<int>(0, iTemp79 + 4)) + 1)) & 1023];
		fVec6[IOTA0 & 1048575] = fTemp84;
		double fTemp85 = fVec6[(IOTA0 - iTemp64) & 1048575];
		double fTemp86 = fTemp0 + 0.5 * (fTemp85 + fRec55[0] * (fVec6[(IOTA0 - iTemp66) & 1048575] - fTemp85)) * fTemp47;
		double fTemp87 = fTemp86 * fTemp10 - fTemp19 * fRec35[1];
		double fTemp88 = fTemp10 * fTemp87 - fTemp19 * fRec39[1];
		double fTemp89 = fTemp10 * fTemp88 - fTemp19 * fRec43[1];
		fVec7[IOTA0 & 16383] = fTemp10 * fTemp89 - fTemp19 * fRec47[1];
		fRec60[0] = 0.9999 * (fRec60[1] + static_cast<double>(iTemp3 * iSlow26)) + fSlow27;
		double fTemp90 = fRec60[0] + -1.49999;
		double fTemp91 = fVec7[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp90)))) & 16383];
		fVec8[0] = fTemp91;
		double fTemp92 = std::floor(fTemp90);
		fRec59[0] = fVec8[1] - (fTemp92 + (2.0 - fRec60[0])) * (fRec59[1] - fTemp91) / (fRec60[0] - fTemp92);
		fRec48[0] = fRec59[0];
		fVec9[IOTA0 & 16383] = fTemp10 * fRec48[1] + fTemp19 * fTemp70;
		double fTemp93 = fVec9[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp43)))) & 16383];
		fVec10[0] = fTemp93;
		fRec45[0] = -(fRec45[1] * fTemp45 / fTemp46 + fTemp45 * fTemp93 / fTemp46 + fVec10[1]);
		fRec43[0] = fRec45[0];
		fVec11[IOTA0 & 16383] = fRec47[1] * fTemp10 + fTemp19 * fTemp89;
		fRec62[0] = 0.9999 * (fRec62[1] + static_cast<double>(iTemp3 * iSlow28)) + fSlow29;
		double fTemp94 = fRec62[0] + -1.49999;
		double fTemp95 = fVec11[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp94)))) & 16383];
		fVec12[0] = fTemp95;
		double fTemp96 = std::floor(fTemp94);
		fRec61[0] = fVec12[1] - (fTemp96 + (2.0 - fRec62[0])) * (fRec61[1] - fTemp95) / (fRec62[0] - fTemp96);
		fRec44[0] = fRec61[0];
		fVec13[IOTA0 & 16383] = fTemp10 * fRec44[1] + fTemp19 * fTemp69;
		double fTemp97 = fVec13[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp39)))) & 16383];
		fVec14[0] = fTemp97;
		fRec41[0] = -(fRec41[1] * fTemp41 / fTemp42 + fTemp41 * fTemp97 / fTemp42 + fVec14[1]);
		fRec39[0] = fRec41[0];
		fVec15[IOTA0 & 16383] = fRec43[1] * fTemp10 + fTemp19 * fTemp88;
		fRec64[0] = 0.9999 * (fRec64[1] + static_cast<double>(iTemp3 * iSlow30)) + fSlow31;
		double fTemp98 = fRec64[0] + -1.49999;
		double fTemp99 = fVec15[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp98)))) & 16383];
		fVec16[0] = fTemp99;
		double fTemp100 = std::floor(fTemp98);
		fRec63[0] = fVec16[1] - (fTemp100 + (2.0 - fRec64[0])) * (fRec63[1] - fTemp99) / (fRec64[0] - fTemp100);
		fRec40[0] = fRec63[0];
		fVec17[IOTA0 & 16383] = fTemp10 * fRec40[1] + fTemp19 * fTemp68;
		double fTemp101 = fVec17[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp35)))) & 16383];
		fVec18[0] = fTemp101;
		fRec37[0] = -(fRec37[1] * fTemp37 / fTemp38 + fTemp37 * fTemp101 / fTemp38 + fVec18[1]);
		fRec35[0] = fRec37[0];
		fVec19[IOTA0 & 16383] = fRec39[1] * fTemp10 + fTemp19 * fTemp87;
		fRec66[0] = 0.9999 * (fRec66[1] + static_cast<double>(iTemp3 * iSlow32)) + fSlow33;
		double fTemp102 = fRec66[0] + -1.49999;
		double fTemp103 = fVec19[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp102)))) & 16383];
		fVec20[0] = fTemp103;
		double fTemp104 = std::floor(fTemp102);
		fRec65[0] = fVec20[1] - (fTemp104 + (2.0 - fRec66[0])) * (fRec65[1] - fTemp103) / (fRec66[0] - fTemp104);
		fRec36[0] = fRec65[0];
		double fTemp105 = fTemp10 * fRec36[1] + fTemp19 * fTemp67;
		double fTemp106 = fTemp21 * fTemp105 - fTemp34 * fRec21[1];
		double fTemp107 = fTemp21 * fTemp106 - fTemp34 * fRec25[1];
		double fTemp108 = fTemp21 * fTemp107 - fTemp34 * fRec29[1];
		fVec21[IOTA0 & 16383] = fTemp34 * fRec33[1] - fTemp21 * fTemp108;
		fRec67[0] = 0.9999 * (fRec67[1] + static_cast<double>(iTemp3 * iSlow34)) + fSlow35;
		double fTemp109 = fRec67[0] + -1.49999;
		double fTemp110 = fVec21[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp109)))) & 16383];
		fVec22[0] = fTemp110;
		double fTemp111 = std::floor(fTemp109);
		fRec34[0] = fVec22[1] - (fTemp111 + (2.0 - fRec67[0])) * (fRec34[1] - fTemp110) / (fRec67[0] - fTemp111);
		fRec32[0] = fRec34[0];
		double fTemp112 = fRec35[1] * fTemp10 + fTemp19 * fTemp86;
		double fTemp113 = fTemp112 * fTemp21 - fTemp34 * fRec20[1];
		double fTemp114 = fTemp21 * fTemp113 - fTemp34 * fRec24[1];
		double fTemp115 = fTemp21 * fTemp114 - fTemp34 * fRec28[1];
		fVec23[IOTA0 & 16383] = fTemp21 * fTemp115 - fRec32[1] * fTemp34;
		fRec69[0] = 0.9999 * (fRec69[1] + static_cast<double>(iTemp3 * iSlow36)) + fSlow37;
		double fTemp116 = fRec69[0] + -1.49999;
		double fTemp117 = fVec23[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp116)))) & 16383];
		fVec24[0] = fTemp117;
		double fTemp118 = std::floor(fTemp116);
		fRec68[0] = fVec24[1] - (fTemp118 + (2.0 - fRec69[0])) * (fRec68[1] - fTemp117) / (fRec69[0] - fTemp118);
		fRec33[0] = fRec68[0];
		fVec25[IOTA0 & 16383] = fTemp21 * fRec33[1] + fTemp34 * fTemp108;
		double fTemp119 = fVec25[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp30)))) & 16383];
		fVec26[0] = fTemp119;
		fRec30[0] = -(fRec30[1] * fTemp32 / fTemp33 + fTemp32 * fTemp119 / fTemp33 + fVec26[1]);
		fRec28[0] = fRec30[0];
		fVec27[IOTA0 & 16383] = fRec32[1] * fTemp21 + fTemp34 * fTemp115;
		fRec71[0] = 0.9999 * (fRec71[1] + static_cast<double>(iTemp3 * iSlow38)) + fSlow39;
		double fTemp120 = fRec71[0] + -1.49999;
		double fTemp121 = fVec27[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp120)))) & 16383];
		fVec28[0] = fTemp121;
		double fTemp122 = std::floor(fTemp120);
		fRec70[0] = fVec28[1] - (fTemp122 + (2.0 - fRec71[0])) * (fRec70[1] - fTemp121) / (fRec71[0] - fTemp122);
		fRec29[0] = fRec70[0];
		fVec29[IOTA0 & 16383] = fTemp21 * fRec29[1] + fTemp34 * fTemp107;
		double fTemp123 = fVec29[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp26)))) & 16383];
		fVec30[0] = fTemp123;
		fRec26[0] = -(fRec26[1] * fTemp28 / fTemp29 + fTemp28 * fTemp123 / fTemp29 + fVec30[1]);
		fRec24[0] = fRec26[0];
		fVec31[IOTA0 & 16383] = fRec28[1] * fTemp21 + fTemp34 * fTemp114;
		fRec73[0] = 0.9999 * (fRec73[1] + static_cast<double>(iTemp3 * iSlow40)) + fSlow41;
		double fTemp124 = fRec73[0] + -1.49999;
		double fTemp125 = fVec31[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp124)))) & 16383];
		fVec32[0] = fTemp125;
		double fTemp126 = std::floor(fTemp124);
		fRec72[0] = fVec32[1] - (fTemp126 + (2.0 - fRec73[0])) * (fRec72[1] - fTemp125) / (fRec73[0] - fTemp126);
		fRec25[0] = fRec72[0];
		fVec33[IOTA0 & 16383] = fTemp21 * fRec25[1] + fTemp34 * fTemp106;
		double fTemp127 = fVec33[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp22)))) & 16383];
		fVec34[0] = fTemp127;
		fRec22[0] = -(fRec22[1] * fTemp24 / fTemp25 + fTemp24 * fTemp127 / fTemp25 + fVec34[1]);
		fRec20[0] = fRec22[0];
		fVec35[IOTA0 & 16383] = fRec24[1] * fTemp21 + fTemp34 * fTemp113;
		fRec75[0] = 0.9999 * (fRec75[1] + static_cast<double>(iTemp3 * iSlow42)) + fSlow43;
		double fTemp128 = fRec75[0] + -1.49999;
		double fTemp129 = fVec35[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp128)))) & 16383];
		fVec36[0] = fTemp129;
		double fTemp130 = std::floor(fTemp128);
		fRec74[0] = fVec36[1] - (fTemp130 + (2.0 - fRec75[0])) * (fRec74[1] - fTemp129) / (fRec75[0] - fTemp130);
		fRec21[0] = fRec74[0];
		double fTemp131 = fTemp21 * fRec21[1] + fTemp34 * fTemp105;
		double fTemp132 = fTemp10 * fTemp131 - fTemp19 * fRec6[1];
		double fTemp133 = fTemp10 * fTemp132 - fTemp19 * fRec10[1];
		double fTemp134 = fTemp10 * fTemp133 - fTemp19 * fRec14[1];
		fVec37[IOTA0 & 16383] = fTemp19 * fRec18[1] - fTemp10 * fTemp134;
		fRec76[0] = 0.9999 * (fRec76[1] + static_cast<double>(iTemp3 * iSlow44)) + fSlow45;
		double fTemp135 = fRec76[0] + -1.49999;
		double fTemp136 = fVec37[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp135)))) & 16383];
		fVec38[0] = fTemp136;
		double fTemp137 = std::floor(fTemp135);
		fRec19[0] = fVec38[1] - (fTemp137 + (2.0 - fRec76[0])) * (fRec19[1] - fTemp136) / (fRec76[0] - fTemp137);
		fRec17[0] = fRec19[0];
		double fTemp138 = fRec20[1] * fTemp21 + fTemp34 * fTemp112;
		double fTemp139 = fTemp10 * fTemp138 - fTemp19 * fRec5[1];
		double fTemp140 = fTemp10 * fTemp139 - fTemp19 * fRec9[1];
		double fTemp141 = fTemp10 * fTemp140 - fTemp19 * fRec13[1];
		fVec39[IOTA0 & 16383] = fTemp10 * fTemp141 - fRec17[1] * fTemp19;
		fRec78[0] = 0.9999 * (fRec78[1] + static_cast<double>(iTemp3 * iSlow46)) + fSlow47;
		double fTemp142 = fRec78[0] + -1.49999;
		double fTemp143 = fVec39[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp142)))) & 16383];
		fVec40[0] = fTemp143;
		double fTemp144 = std::floor(fTemp142);
		fRec77[0] = fVec40[1] - (fTemp144 + (2.0 - fRec78[0])) * (fRec77[1] - fTemp143) / (fRec78[0] - fTemp144);
		fRec18[0] = fRec77[0];
		fVec41[IOTA0 & 16383] = fTemp10 * fRec18[1] + fTemp19 * fTemp134;
		double fTemp145 = fVec41[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp15)))) & 16383];
		fVec42[0] = fTemp145;
		fRec15[0] = -(fRec15[1] * fTemp17 / fTemp18 + fTemp17 * fTemp145 / fTemp18 + fVec42[1]);
		fRec13[0] = fRec15[0];
		fVec43[IOTA0 & 16383] = fRec17[1] * fTemp10 + fTemp19 * fTemp141;
		fRec80[0] = 0.9999 * (fRec80[1] + static_cast<double>(iTemp3 * iSlow48)) + fSlow49;
		double fTemp146 = fRec80[0] + -1.49999;
		double fTemp147 = fVec43[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp146)))) & 16383];
		fVec44[0] = fTemp147;
		double fTemp148 = std::floor(fTemp146);
		fRec79[0] = fVec44[1] - (fTemp148 + (2.0 - fRec80[0])) * (fRec79[1] - fTemp147) / (fRec80[0] - fTemp148);
		fRec14[0] = fRec79[0];
		fVec45[IOTA0 & 16383] = fTemp10 * fRec14[1] + fTemp19 * fTemp133;
		double fTemp149 = fVec45[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp11)))) & 16383];
		fVec46[0] = fTemp149;
		fRec11[0] = -(fRec11[1] * fTemp13 / fTemp14 + fTemp13 * fTemp149 / fTemp14 + fVec46[1]);
		fRec9[0] = fRec11[0];
		fVec47[IOTA0 & 16383] = fRec13[1] * fTemp10 + fTemp19 * fTemp140;
		fRec82[0] = 0.9999 * (fRec82[1] + static_cast<double>(iTemp3 * iSlow50)) + fSlow51;
		double fTemp150 = fRec82[0] + -1.49999;
		double fTemp151 = fVec47[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp150)))) & 16383];
		fVec48[0] = fTemp151;
		double fTemp152 = std::floor(fTemp150);
		fRec81[0] = fVec48[1] - (fTemp152 + (2.0 - fRec82[0])) * (fRec81[1] - fTemp151) / (fRec82[0] - fTemp152);
		fRec10[0] = fRec81[0];
		fVec49[IOTA0 & 16383] = fTemp10 * fRec10[1] + fTemp19 * fTemp132;
		double fTemp153 = fVec49[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp4)))) & 16383];
		fVec50[0] = fTemp153;
		fRec7[0] = -(fRec7[1] * fTemp6 / fTemp7 + fTemp6 * fTemp153 / fTemp7 + fVec50[1]);
		fRec5[0] = fRec7[0];
		fVec51[IOTA0 & 16383] = fRec9[1] * fTemp10 + fTemp19 * fTemp139;
		fRec84[0] = 0.9999 * (fRec84[1] + static_cast<double>(iTemp3 * iSlow52)) + fSlow53;
		double fTemp154 = fRec84[0] + -1.49999;
		double fTemp155 = fVec51[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp154)))) & 16383];
		fVec52[0] = fTemp155;
		double fTemp156 = std::floor(fTemp154);
		fRec83[0] = fVec52[1] - (fTemp156 + (2.0 - fRec84[0])) * (fRec83[1] - fTemp155) / (fRec84[0] - fTemp156);
		fRec6[0] = fRec83[0];
		fRec3[0] = fTemp2 * (fRec5[1] * fTemp10 + fTemp19 * fTemp138) + 0.5 * fTemp1 * fRec3[1];
		fRec1[IOTA0 & 1023] = fRec3[0];
		fRec85[0] = fTemp2 * (fTemp10 * fRec6[1] + fTemp19 * fTemp131) + 0.5 * fTemp1 * fRec85[1];
		fRec2[IOTA0 & 1023] = fRec85[0];
		output0[i0] = static_cast<FAUSTFLOAT>(fTemp0 * (1.0 - fRec0[0]) + 0.5 * fRec0[0] * (fRec1[IOTA0 & 1023] + fRec2[IOTA0 & 1023]));
		iVec0[1] = iVec0[0];
		fRec0[1] = fRec0[0];
		fRec4[1] = fRec4[0];
		fRec8[1] = fRec8[0];
		fVec1[1] = fVec1[0];
		fRec12[1] = fRec12[0];
		fRec16[1] = fRec16[0];
		fRec23[1] = fRec23[0];
		fRec27[1] = fRec27[0];
		fRec31[1] = fRec31[0];
		fRec38[1] = fRec38[0];
		fRec42[1] = fRec42[0];
		fRec46[1] = fRec46[0];
		fRec50[1] = fRec50[0];
		fRec51[1] = fRec51[0];
		fVec2[1] = fVec2[0];
		fRec52[1] = fRec52[0];
		fRec53[1] = fRec53[0];
		IOTA0 = IOTA0 + 1;
		fRec54[1] = fRec54[0];
		fRec55[1] = fRec55[0];
		fRec56[1] = fRec56[0];
		fRec57[1] = fRec57[0];
		fRec58[1] = fRec58[0];
		fVec5[1] = fVec5[0];
		fRec49[1] = fRec49[0];
		fRec47[1] = fRec47[0];
		fRec60[1] = fRec60[0];
		fVec8[1] = fVec8[0];
		fRec59[1] = fRec59[0];
		fRec48[1] = fRec48[0];
		fVec10[1] = fVec10[0];
		fRec45[1] = fRec45[0];
		fRec43[1] = fRec43[0];
		fRec62[1] = fRec62[0];
		fVec12[1] = fVec12[0];
		fRec61[1] = fRec61[0];
		fRec44[1] = fRec44[0];
		fVec14[1] = fVec14[0];
		fRec41[1] = fRec41[0];
		fRec39[1] = fRec39[0];
		fRec64[1] = fRec64[0];
		fVec16[1] = fVec16[0];
		fRec63[1] = fRec63[0];
		fRec40[1] = fRec40[0];
		fVec18[1] = fVec18[0];
		fRec37[1] = fRec37[0];
		fRec35[1] = fRec35[0];
		fRec66[1] = fRec66[0];
		fVec20[1] = fVec20[0];
		fRec65[1] = fRec65[0];
		fRec36[1] = fRec36[0];
		fRec67[1] = fRec67[0];
		fVec22[1] = fVec22[0];
		fRec34[1] = fRec34[0];
		fRec32[1] = fRec32[0];
		fRec69[1] = fRec69[0];
		fVec24[1] = fVec24[0];
		fRec68[1] = fRec68[0];
		fRec33[1] = fRec33[0];
		fVec26[1] = fVec26[0];
		fRec30[1] = fRec30[0];
		fRec28[1] = fRec28[0];
		fRec71[1] = fRec71[0];
		fVec28[1] = fVec28[0];
		fRec70[1] = fRec70[0];
		fRec29[1] = fRec29[0];
		fVec30[1] = fVec30[0];
		fRec26[1] = fRec26[0];
		fRec24[1] = fRec24[0];
		fRec73[1] = fRec73[0];
		fVec32[1] = fVec32[0];
		fRec72[1] = fRec72[0];
		fRec25[1] = fRec25[0];
		fVec34[1] = fVec34[0];
		fRec22[1] = fRec22[0];
		fRec20[1] = fRec20[0];
		fRec75[1] = fRec75[0];
		fVec36[1] = fVec36[0];
		fRec74[1] = fRec74[0];
		fRec21[1] = fRec21[0];
		fRec76[1] = fRec76[0];
		fVec38[1] = fVec38[0];
		fRec19[1] = fRec19[0];
		fRec17[1] = fRec17[0];
		fRec78[1] = fRec78[0];
		fVec40[1] = fVec40[0];
		fRec77[1] = fRec77[0];
		fRec18[1] = fRec18[0];
		fVec42[1] = fVec42[0];
		fRec15[1] = fRec15[0];
		fRec13[1] = fRec13[0];
		fRec80[1] = fRec80[0];
		fVec44[1] = fVec44[0];
		fRec79[1] = fRec79[0];
		fRec14[1] = fRec14[0];
		fVec46[1] = fVec46[0];
		fRec11[1] = fRec11[0];
		fRec9[1] = fRec9[0];
		fRec82[1] = fRec82[0];
		fVec48[1] = fVec48[0];
		fRec81[1] = fRec81[0];
		fRec10[1] = fRec10[0];
		fVec50[1] = fVec50[0];
		fRec7[1] = fRec7[0];
		fRec5[1] = fRec5[0];
		fRec84[1] = fRec84[0];
		fVec52[1] = fVec52[0];
		fRec83[1] = fRec83[0];
		fRec6[1] = fRec6[0];
		fRec3[1] = fRec3[0];
		fRec85[1] = fRec85[0];
	}
}

void __rt_func Dsp::compute_static(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0, PluginDef *p)
{
	static_cast<Dsp*>(p)->compute(count, input0, output0);
}

int Dsp::register_par(const ParamReg& reg)
{
	reg.registerFloatVar("greyhole.damping",N_("Damping"),"S","",&fHslider1, 0.35, 0.0, 0.95, 0.01, 0);
	reg.registerFloatVar("greyhole.delay",N_("Delay"),"S","",&fHslider5, 4e+02, 2e+01, 1.5e+03, 1.0, 0);
	reg.registerFloatVar("greyhole.feedback",N_("Feedback"),"S","",&fHslider3, 0.55, 0.0, 0.92, 0.01, 0);
	reg.registerFloatVar("greyhole.mix",N_("Mix"),"S","",&fHslider0, 3e+01, 0.0, 1e+02, 1.0, 0);
	reg.registerFloatVar("greyhole.modulation",N_("Modulation"),"S","",&fHslider4, 0.3, 0.0, 1.0, 0.01, 0);
	reg.registerFloatVar("greyhole.size",N_("Size"),"S","",&fHslider2, 1.0, 0.5, 3.0, 0.05, 0);
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

} // end namespace greyhole

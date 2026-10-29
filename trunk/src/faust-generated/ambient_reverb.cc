// generated from file '../src/faust/ambient_reverb.dsp' by dsp2cc:
// Code generated with Faust (https://faust.grame.fr)


namespace ambient_reverb {
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
	double fRec16[2];
	FAUSTFLOAT fHslider2;
	double fRec17[2];
	double fConst4;
	FAUSTFLOAT fHslider3;
	double fRec20[2];
	double fRec18[2];
	double fRec19[2];
	double fConst5;
	double fConst6;
	double fConst7;
	double fConst8;
	double fConst9;
	double fConst10;
	double fConst11;
	double fConst12;
	double fConst13;
	int IOTA0;
	double *fVec1;
	FAUSTFLOAT fHslider4;
	double fRec57[2];
	double fVec2[2];
	double fRec56[2];
	double fRec54[2];
	double *fVec3;
	double fRec59[2];
	double fVec4[2];
	double fRec58[2];
	double fRec55[2];
	double *fVec5;
	double fRec60[2];
	double fVec6[2];
	double fRec53[2];
	double fRec51[2];
	double *fVec7;
	double fRec62[2];
	double fVec8[2];
	double fRec61[2];
	double fRec52[2];
	double *fVec9;
	double fRec63[2];
	double fVec10[2];
	double fRec50[2];
	double fRec48[2];
	double *fVec11;
	double fRec65[2];
	double fVec12[2];
	double fRec64[2];
	double fRec49[2];
	double *fVec13;
	double fRec66[2];
	double fVec14[2];
	double fRec47[2];
	double fRec45[2];
	double *fVec15;
	double fRec68[2];
	double fVec16[2];
	double fRec67[2];
	double fRec46[2];
	double *fVec17;
	double fRec69[2];
	double fVec18[2];
	double fRec44[2];
	double fRec42[2];
	double *fVec19;
	double fRec71[2];
	double fVec20[2];
	double fRec70[2];
	double fRec43[2];
	double fVec21[1024];
	double *fVec22;
	double fRec72[2];
	double fVec23[2];
	double fRec41[2];
	double fVec24[1024];
	double *fVec25;
	double fRec74[2];
	double fVec26[2];
	double fRec73[2];
	double *fVec27;
	double fRec75[2];
	double fVec28[2];
	double fRec40[2];
	double fRec38[2];
	double *fVec29;
	double fRec77[2];
	double fVec30[2];
	double fRec76[2];
	double fRec39[2];
	double *fVec31;
	double fRec78[2];
	double fVec32[2];
	double fRec37[2];
	double fRec35[2];
	double *fVec33;
	double fRec80[2];
	double fVec34[2];
	double fRec79[2];
	double fRec36[2];
	double *fVec35;
	double fRec81[2];
	double fVec36[2];
	double fRec34[2];
	double fRec32[2];
	double *fVec37;
	double fRec83[2];
	double fVec38[2];
	double fRec82[2];
	double fRec33[2];
	double *fVec39;
	double fRec84[2];
	double fVec40[2];
	double fRec31[2];
	double fRec29[2];
	double *fVec41;
	double fRec86[2];
	double fVec42[2];
	double fRec85[2];
	double fRec30[2];
	double *fVec43;
	double fRec87[2];
	double fVec44[2];
	double fRec28[2];
	double fRec26[2];
	double *fVec45;
	double fRec89[2];
	double fVec46[2];
	double fRec88[2];
	double fRec27[2];
	double *fVec47;
	double *fVec48;
	double fRec90[2];
	double fVec49[2];
	double fRec25[2];
	double fRec24[2];
	double fConst14;
	double fConst15;
	double fConst16;
	double fRec23[3];
	double fConst17;
	double fConst18;
	double fRec22[3];
	double fConst19;
	double fConst20;
	double fConst21;
	double fConst22;
	double fConst23;
	double fConst24;
	double fConst25;
	double fRec21[3];
	double fConst26;
	double fConst27;
	double fConst28;
	double fConst29;
	double fConst30;
	double fRec96[2];
	double fRec95[3];
	double fRec94[3];
	double fVec50[2];
	double fRec93[2];
	double fConst31;
	double fRec92[3];
	double fConst32;
	double fRec91[3];
	double fConst33;
	double fConst34;
	double fRec99[2];
	double fRec98[3];
	double fRec97[3];
	FAUSTFLOAT fHslider5;
	double fRec100[2];
	double fVec51[1024];
	double fRec15[2];
	double *fVec52;
	double *fVec53;
	double fRec107[2];
	double fVec54[2];
	double fRec106[2];
	double fRec105[2];
	double fRec104[3];
	double fRec103[3];
	double fRec102[3];
	double fRec113[2];
	double fRec112[3];
	double fRec111[3];
	double fVec55[2];
	double fRec110[2];
	double fRec109[3];
	double fRec108[3];
	double fRec116[2];
	double fRec115[3];
	double fRec114[3];
	double fVec56[1024];
	double fRec101[2];
	double *fVec57;
	double fVec58[2];
	double fRec14[2];
	double fRec12[2];
	double *fVec59;
	double fRec118[2];
	double fVec60[2];
	double fRec117[2];
	double fRec13[2];
	double *fVec61;
	double fVec62[2];
	double fRec11[2];
	double fRec9[2];
	double *fVec63;
	double fVec64[2];
	double fRec119[2];
	double fRec10[2];
	double *fVec65;
	double fVec66[2];
	double fRec8[2];
	double fRec6[2];
	double *fVec67;
	double fRec121[2];
	double fVec68[2];
	double fRec120[2];
	double fRec7[2];
	double *fVec69;
	double fRec122[2];
	double fVec70[2];
	double fRec5[2];
	double fRec3[2];
	double *fVec71;
	double fVec72[2];
	double fRec123[2];
	double fRec4[2];
	double fRec1[2];
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
	  fVec1(0),
	  fVec3(0),
	  fVec5(0),
	  fVec7(0),
	  fVec9(0),
	  fVec11(0),
	  fVec13(0),
	  fVec15(0),
	  fVec17(0),
	  fVec19(0),
	  fVec22(0),
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
	  fVec48(0),
	  fVec52(0),
	  fVec53(0),
	  fVec57(0),
	  fVec59(0),
	  fVec61(0),
	  fVec63(0),
	  fVec65(0),
	  fVec67(0),
	  fVec69(0),
	  fVec71(0),
	  mem_allocated(false),
	  pending_sample_rate(0) {
	version = PLUGINDEF_VERSION;
	flags = 0;
	id = "ambient_reverb";
	name = N_("JP Ambient");
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
	for (int l0 = 0; l0 < 2; l0 = l0 + 1) iVec0[l0] = 0;
	for (int l1 = 0; l1 < 2; l1 = l1 + 1) fRec0[l1] = 0.0;
	for (int l2 = 0; l2 < 2; l2 = l2 + 1) fRec16[l2] = 0.0;
	for (int l3 = 0; l3 < 2; l3 = l3 + 1) fRec17[l3] = 0.0;
	for (int l4 = 0; l4 < 2; l4 = l4 + 1) fRec20[l4] = 0.0;
	for (int l5 = 0; l5 < 2; l5 = l5 + 1) fRec18[l5] = 0.0;
	for (int l6 = 0; l6 < 2; l6 = l6 + 1) fRec19[l6] = 0.0;
	for (int l7 = 0; l7 < 16384; l7 = l7 + 1) fVec1[l7] = 0.0;
	for (int l8 = 0; l8 < 2; l8 = l8 + 1) fRec57[l8] = 0.0;
	for (int l9 = 0; l9 < 2; l9 = l9 + 1) fVec2[l9] = 0.0;
	for (int l10 = 0; l10 < 2; l10 = l10 + 1) fRec56[l10] = 0.0;
	for (int l11 = 0; l11 < 2; l11 = l11 + 1) fRec54[l11] = 0.0;
	for (int l12 = 0; l12 < 16384; l12 = l12 + 1) fVec3[l12] = 0.0;
	for (int l13 = 0; l13 < 2; l13 = l13 + 1) fRec59[l13] = 0.0;
	for (int l14 = 0; l14 < 2; l14 = l14 + 1) fVec4[l14] = 0.0;
	for (int l15 = 0; l15 < 2; l15 = l15 + 1) fRec58[l15] = 0.0;
	for (int l16 = 0; l16 < 2; l16 = l16 + 1) fRec55[l16] = 0.0;
	for (int l17 = 0; l17 < 16384; l17 = l17 + 1) fVec5[l17] = 0.0;
	for (int l18 = 0; l18 < 2; l18 = l18 + 1) fRec60[l18] = 0.0;
	for (int l19 = 0; l19 < 2; l19 = l19 + 1) fVec6[l19] = 0.0;
	for (int l20 = 0; l20 < 2; l20 = l20 + 1) fRec53[l20] = 0.0;
	for (int l21 = 0; l21 < 2; l21 = l21 + 1) fRec51[l21] = 0.0;
	for (int l22 = 0; l22 < 16384; l22 = l22 + 1) fVec7[l22] = 0.0;
	for (int l23 = 0; l23 < 2; l23 = l23 + 1) fRec62[l23] = 0.0;
	for (int l24 = 0; l24 < 2; l24 = l24 + 1) fVec8[l24] = 0.0;
	for (int l25 = 0; l25 < 2; l25 = l25 + 1) fRec61[l25] = 0.0;
	for (int l26 = 0; l26 < 2; l26 = l26 + 1) fRec52[l26] = 0.0;
	for (int l27 = 0; l27 < 16384; l27 = l27 + 1) fVec9[l27] = 0.0;
	for (int l28 = 0; l28 < 2; l28 = l28 + 1) fRec63[l28] = 0.0;
	for (int l29 = 0; l29 < 2; l29 = l29 + 1) fVec10[l29] = 0.0;
	for (int l30 = 0; l30 < 2; l30 = l30 + 1) fRec50[l30] = 0.0;
	for (int l31 = 0; l31 < 2; l31 = l31 + 1) fRec48[l31] = 0.0;
	for (int l32 = 0; l32 < 16384; l32 = l32 + 1) fVec11[l32] = 0.0;
	for (int l33 = 0; l33 < 2; l33 = l33 + 1) fRec65[l33] = 0.0;
	for (int l34 = 0; l34 < 2; l34 = l34 + 1) fVec12[l34] = 0.0;
	for (int l35 = 0; l35 < 2; l35 = l35 + 1) fRec64[l35] = 0.0;
	for (int l36 = 0; l36 < 2; l36 = l36 + 1) fRec49[l36] = 0.0;
	for (int l37 = 0; l37 < 16384; l37 = l37 + 1) fVec13[l37] = 0.0;
	for (int l38 = 0; l38 < 2; l38 = l38 + 1) fRec66[l38] = 0.0;
	for (int l39 = 0; l39 < 2; l39 = l39 + 1) fVec14[l39] = 0.0;
	for (int l40 = 0; l40 < 2; l40 = l40 + 1) fRec47[l40] = 0.0;
	for (int l41 = 0; l41 < 2; l41 = l41 + 1) fRec45[l41] = 0.0;
	for (int l42 = 0; l42 < 16384; l42 = l42 + 1) fVec15[l42] = 0.0;
	for (int l43 = 0; l43 < 2; l43 = l43 + 1) fRec68[l43] = 0.0;
	for (int l44 = 0; l44 < 2; l44 = l44 + 1) fVec16[l44] = 0.0;
	for (int l45 = 0; l45 < 2; l45 = l45 + 1) fRec67[l45] = 0.0;
	for (int l46 = 0; l46 < 2; l46 = l46 + 1) fRec46[l46] = 0.0;
	for (int l47 = 0; l47 < 16384; l47 = l47 + 1) fVec17[l47] = 0.0;
	for (int l48 = 0; l48 < 2; l48 = l48 + 1) fRec69[l48] = 0.0;
	for (int l49 = 0; l49 < 2; l49 = l49 + 1) fVec18[l49] = 0.0;
	for (int l50 = 0; l50 < 2; l50 = l50 + 1) fRec44[l50] = 0.0;
	for (int l51 = 0; l51 < 2; l51 = l51 + 1) fRec42[l51] = 0.0;
	for (int l52 = 0; l52 < 16384; l52 = l52 + 1) fVec19[l52] = 0.0;
	for (int l53 = 0; l53 < 2; l53 = l53 + 1) fRec71[l53] = 0.0;
	for (int l54 = 0; l54 < 2; l54 = l54 + 1) fVec20[l54] = 0.0;
	for (int l55 = 0; l55 < 2; l55 = l55 + 1) fRec70[l55] = 0.0;
	for (int l56 = 0; l56 < 2; l56 = l56 + 1) fRec43[l56] = 0.0;
	for (int l57 = 0; l57 < 1024; l57 = l57 + 1) fVec21[l57] = 0.0;
	for (int l58 = 0; l58 < 16384; l58 = l58 + 1) fVec22[l58] = 0.0;
	for (int l59 = 0; l59 < 2; l59 = l59 + 1) fRec72[l59] = 0.0;
	for (int l60 = 0; l60 < 2; l60 = l60 + 1) fVec23[l60] = 0.0;
	for (int l61 = 0; l61 < 2; l61 = l61 + 1) fRec41[l61] = 0.0;
	for (int l62 = 0; l62 < 1024; l62 = l62 + 1) fVec24[l62] = 0.0;
	for (int l63 = 0; l63 < 16384; l63 = l63 + 1) fVec25[l63] = 0.0;
	for (int l64 = 0; l64 < 2; l64 = l64 + 1) fRec74[l64] = 0.0;
	for (int l65 = 0; l65 < 2; l65 = l65 + 1) fVec26[l65] = 0.0;
	for (int l66 = 0; l66 < 2; l66 = l66 + 1) fRec73[l66] = 0.0;
	for (int l67 = 0; l67 < 16384; l67 = l67 + 1) fVec27[l67] = 0.0;
	for (int l68 = 0; l68 < 2; l68 = l68 + 1) fRec75[l68] = 0.0;
	for (int l69 = 0; l69 < 2; l69 = l69 + 1) fVec28[l69] = 0.0;
	for (int l70 = 0; l70 < 2; l70 = l70 + 1) fRec40[l70] = 0.0;
	for (int l71 = 0; l71 < 2; l71 = l71 + 1) fRec38[l71] = 0.0;
	for (int l72 = 0; l72 < 16384; l72 = l72 + 1) fVec29[l72] = 0.0;
	for (int l73 = 0; l73 < 2; l73 = l73 + 1) fRec77[l73] = 0.0;
	for (int l74 = 0; l74 < 2; l74 = l74 + 1) fVec30[l74] = 0.0;
	for (int l75 = 0; l75 < 2; l75 = l75 + 1) fRec76[l75] = 0.0;
	for (int l76 = 0; l76 < 2; l76 = l76 + 1) fRec39[l76] = 0.0;
	for (int l77 = 0; l77 < 16384; l77 = l77 + 1) fVec31[l77] = 0.0;
	for (int l78 = 0; l78 < 2; l78 = l78 + 1) fRec78[l78] = 0.0;
	for (int l79 = 0; l79 < 2; l79 = l79 + 1) fVec32[l79] = 0.0;
	for (int l80 = 0; l80 < 2; l80 = l80 + 1) fRec37[l80] = 0.0;
	for (int l81 = 0; l81 < 2; l81 = l81 + 1) fRec35[l81] = 0.0;
	for (int l82 = 0; l82 < 16384; l82 = l82 + 1) fVec33[l82] = 0.0;
	for (int l83 = 0; l83 < 2; l83 = l83 + 1) fRec80[l83] = 0.0;
	for (int l84 = 0; l84 < 2; l84 = l84 + 1) fVec34[l84] = 0.0;
	for (int l85 = 0; l85 < 2; l85 = l85 + 1) fRec79[l85] = 0.0;
	for (int l86 = 0; l86 < 2; l86 = l86 + 1) fRec36[l86] = 0.0;
	for (int l87 = 0; l87 < 16384; l87 = l87 + 1) fVec35[l87] = 0.0;
	for (int l88 = 0; l88 < 2; l88 = l88 + 1) fRec81[l88] = 0.0;
	for (int l89 = 0; l89 < 2; l89 = l89 + 1) fVec36[l89] = 0.0;
	for (int l90 = 0; l90 < 2; l90 = l90 + 1) fRec34[l90] = 0.0;
	for (int l91 = 0; l91 < 2; l91 = l91 + 1) fRec32[l91] = 0.0;
	for (int l92 = 0; l92 < 16384; l92 = l92 + 1) fVec37[l92] = 0.0;
	for (int l93 = 0; l93 < 2; l93 = l93 + 1) fRec83[l93] = 0.0;
	for (int l94 = 0; l94 < 2; l94 = l94 + 1) fVec38[l94] = 0.0;
	for (int l95 = 0; l95 < 2; l95 = l95 + 1) fRec82[l95] = 0.0;
	for (int l96 = 0; l96 < 2; l96 = l96 + 1) fRec33[l96] = 0.0;
	for (int l97 = 0; l97 < 16384; l97 = l97 + 1) fVec39[l97] = 0.0;
	for (int l98 = 0; l98 < 2; l98 = l98 + 1) fRec84[l98] = 0.0;
	for (int l99 = 0; l99 < 2; l99 = l99 + 1) fVec40[l99] = 0.0;
	for (int l100 = 0; l100 < 2; l100 = l100 + 1) fRec31[l100] = 0.0;
	for (int l101 = 0; l101 < 2; l101 = l101 + 1) fRec29[l101] = 0.0;
	for (int l102 = 0; l102 < 16384; l102 = l102 + 1) fVec41[l102] = 0.0;
	for (int l103 = 0; l103 < 2; l103 = l103 + 1) fRec86[l103] = 0.0;
	for (int l104 = 0; l104 < 2; l104 = l104 + 1) fVec42[l104] = 0.0;
	for (int l105 = 0; l105 < 2; l105 = l105 + 1) fRec85[l105] = 0.0;
	for (int l106 = 0; l106 < 2; l106 = l106 + 1) fRec30[l106] = 0.0;
	for (int l107 = 0; l107 < 16384; l107 = l107 + 1) fVec43[l107] = 0.0;
	for (int l108 = 0; l108 < 2; l108 = l108 + 1) fRec87[l108] = 0.0;
	for (int l109 = 0; l109 < 2; l109 = l109 + 1) fVec44[l109] = 0.0;
	for (int l110 = 0; l110 < 2; l110 = l110 + 1) fRec28[l110] = 0.0;
	for (int l111 = 0; l111 < 2; l111 = l111 + 1) fRec26[l111] = 0.0;
	for (int l112 = 0; l112 < 16384; l112 = l112 + 1) fVec45[l112] = 0.0;
	for (int l113 = 0; l113 < 2; l113 = l113 + 1) fRec89[l113] = 0.0;
	for (int l114 = 0; l114 < 2; l114 = l114 + 1) fVec46[l114] = 0.0;
	for (int l115 = 0; l115 < 2; l115 = l115 + 1) fRec88[l115] = 0.0;
	for (int l116 = 0; l116 < 2; l116 = l116 + 1) fRec27[l116] = 0.0;
	for (int l117 = 0; l117 < 16384; l117 = l117 + 1) fVec47[l117] = 0.0;
	for (int l118 = 0; l118 < 16384; l118 = l118 + 1) fVec48[l118] = 0.0;
	for (int l119 = 0; l119 < 2; l119 = l119 + 1) fRec90[l119] = 0.0;
	for (int l120 = 0; l120 < 2; l120 = l120 + 1) fVec49[l120] = 0.0;
	for (int l121 = 0; l121 < 2; l121 = l121 + 1) fRec25[l121] = 0.0;
	for (int l122 = 0; l122 < 2; l122 = l122 + 1) fRec24[l122] = 0.0;
	for (int l123 = 0; l123 < 3; l123 = l123 + 1) fRec23[l123] = 0.0;
	for (int l124 = 0; l124 < 3; l124 = l124 + 1) fRec22[l124] = 0.0;
	for (int l125 = 0; l125 < 3; l125 = l125 + 1) fRec21[l125] = 0.0;
	for (int l126 = 0; l126 < 2; l126 = l126 + 1) fRec96[l126] = 0.0;
	for (int l127 = 0; l127 < 3; l127 = l127 + 1) fRec95[l127] = 0.0;
	for (int l128 = 0; l128 < 3; l128 = l128 + 1) fRec94[l128] = 0.0;
	for (int l129 = 0; l129 < 2; l129 = l129 + 1) fVec50[l129] = 0.0;
	for (int l130 = 0; l130 < 2; l130 = l130 + 1) fRec93[l130] = 0.0;
	for (int l131 = 0; l131 < 3; l131 = l131 + 1) fRec92[l131] = 0.0;
	for (int l132 = 0; l132 < 3; l132 = l132 + 1) fRec91[l132] = 0.0;
	for (int l133 = 0; l133 < 2; l133 = l133 + 1) fRec99[l133] = 0.0;
	for (int l134 = 0; l134 < 3; l134 = l134 + 1) fRec98[l134] = 0.0;
	for (int l135 = 0; l135 < 3; l135 = l135 + 1) fRec97[l135] = 0.0;
	for (int l136 = 0; l136 < 2; l136 = l136 + 1) fRec100[l136] = 0.0;
	for (int l137 = 0; l137 < 1024; l137 = l137 + 1) fVec51[l137] = 0.0;
	for (int l138 = 0; l138 < 2; l138 = l138 + 1) fRec15[l138] = 0.0;
	for (int l139 = 0; l139 < 16384; l139 = l139 + 1) fVec52[l139] = 0.0;
	for (int l140 = 0; l140 < 16384; l140 = l140 + 1) fVec53[l140] = 0.0;
	for (int l141 = 0; l141 < 2; l141 = l141 + 1) fRec107[l141] = 0.0;
	for (int l142 = 0; l142 < 2; l142 = l142 + 1) fVec54[l142] = 0.0;
	for (int l143 = 0; l143 < 2; l143 = l143 + 1) fRec106[l143] = 0.0;
	for (int l144 = 0; l144 < 2; l144 = l144 + 1) fRec105[l144] = 0.0;
	for (int l145 = 0; l145 < 3; l145 = l145 + 1) fRec104[l145] = 0.0;
	for (int l146 = 0; l146 < 3; l146 = l146 + 1) fRec103[l146] = 0.0;
	for (int l147 = 0; l147 < 3; l147 = l147 + 1) fRec102[l147] = 0.0;
	for (int l148 = 0; l148 < 2; l148 = l148 + 1) fRec113[l148] = 0.0;
	for (int l149 = 0; l149 < 3; l149 = l149 + 1) fRec112[l149] = 0.0;
	for (int l150 = 0; l150 < 3; l150 = l150 + 1) fRec111[l150] = 0.0;
	for (int l151 = 0; l151 < 2; l151 = l151 + 1) fVec55[l151] = 0.0;
	for (int l152 = 0; l152 < 2; l152 = l152 + 1) fRec110[l152] = 0.0;
	for (int l153 = 0; l153 < 3; l153 = l153 + 1) fRec109[l153] = 0.0;
	for (int l154 = 0; l154 < 3; l154 = l154 + 1) fRec108[l154] = 0.0;
	for (int l155 = 0; l155 < 2; l155 = l155 + 1) fRec116[l155] = 0.0;
	for (int l156 = 0; l156 < 3; l156 = l156 + 1) fRec115[l156] = 0.0;
	for (int l157 = 0; l157 < 3; l157 = l157 + 1) fRec114[l157] = 0.0;
	for (int l158 = 0; l158 < 1024; l158 = l158 + 1) fVec56[l158] = 0.0;
	for (int l159 = 0; l159 < 2; l159 = l159 + 1) fRec101[l159] = 0.0;
	for (int l160 = 0; l160 < 16384; l160 = l160 + 1) fVec57[l160] = 0.0;
	for (int l161 = 0; l161 < 2; l161 = l161 + 1) fVec58[l161] = 0.0;
	for (int l162 = 0; l162 < 2; l162 = l162 + 1) fRec14[l162] = 0.0;
	for (int l163 = 0; l163 < 2; l163 = l163 + 1) fRec12[l163] = 0.0;
	for (int l164 = 0; l164 < 16384; l164 = l164 + 1) fVec59[l164] = 0.0;
	for (int l165 = 0; l165 < 2; l165 = l165 + 1) fRec118[l165] = 0.0;
	for (int l166 = 0; l166 < 2; l166 = l166 + 1) fVec60[l166] = 0.0;
	for (int l167 = 0; l167 < 2; l167 = l167 + 1) fRec117[l167] = 0.0;
	for (int l168 = 0; l168 < 2; l168 = l168 + 1) fRec13[l168] = 0.0;
	for (int l169 = 0; l169 < 16384; l169 = l169 + 1) fVec61[l169] = 0.0;
	for (int l170 = 0; l170 < 2; l170 = l170 + 1) fVec62[l170] = 0.0;
	for (int l171 = 0; l171 < 2; l171 = l171 + 1) fRec11[l171] = 0.0;
	for (int l172 = 0; l172 < 2; l172 = l172 + 1) fRec9[l172] = 0.0;
	for (int l173 = 0; l173 < 16384; l173 = l173 + 1) fVec63[l173] = 0.0;
	for (int l174 = 0; l174 < 2; l174 = l174 + 1) fVec64[l174] = 0.0;
	for (int l175 = 0; l175 < 2; l175 = l175 + 1) fRec119[l175] = 0.0;
	for (int l176 = 0; l176 < 2; l176 = l176 + 1) fRec10[l176] = 0.0;
	for (int l177 = 0; l177 < 16384; l177 = l177 + 1) fVec65[l177] = 0.0;
	for (int l178 = 0; l178 < 2; l178 = l178 + 1) fVec66[l178] = 0.0;
	for (int l179 = 0; l179 < 2; l179 = l179 + 1) fRec8[l179] = 0.0;
	for (int l180 = 0; l180 < 2; l180 = l180 + 1) fRec6[l180] = 0.0;
	for (int l181 = 0; l181 < 16384; l181 = l181 + 1) fVec67[l181] = 0.0;
	for (int l182 = 0; l182 < 2; l182 = l182 + 1) fRec121[l182] = 0.0;
	for (int l183 = 0; l183 < 2; l183 = l183 + 1) fVec68[l183] = 0.0;
	for (int l184 = 0; l184 < 2; l184 = l184 + 1) fRec120[l184] = 0.0;
	for (int l185 = 0; l185 < 2; l185 = l185 + 1) fRec7[l185] = 0.0;
	for (int l186 = 0; l186 < 16384; l186 = l186 + 1) fVec69[l186] = 0.0;
	for (int l187 = 0; l187 < 2; l187 = l187 + 1) fRec122[l187] = 0.0;
	for (int l188 = 0; l188 < 2; l188 = l188 + 1) fVec70[l188] = 0.0;
	for (int l189 = 0; l189 < 2; l189 = l189 + 1) fRec5[l189] = 0.0;
	for (int l190 = 0; l190 < 2; l190 = l190 + 1) fRec3[l190] = 0.0;
	for (int l191 = 0; l191 < 16384; l191 = l191 + 1) fVec71[l191] = 0.0;
	for (int l192 = 0; l192 < 2; l192 = l192 + 1) fVec72[l192] = 0.0;
	for (int l193 = 0; l193 < 2; l193 = l193 + 1) fRec123[l193] = 0.0;
	for (int l194 = 0; l194 < 2; l194 = l194 + 1) fRec4[l194] = 0.0;
	for (int l195 = 0; l195 < 2; l195 = l195 + 1) fRec1[l195] = 0.0;
	for (int l196 = 0; l196 < 2; l196 = l196 + 1) fRec2[l196] = 0.0;
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
	fConst4 = 6.283185307179586 / fConst0;
	fConst5 = std::tan(12566.370614359172 / fConst0);
	fConst6 = mydsp_faustpower2_f(fConst5);
	fConst7 = 1.0 / fConst5;
	fConst8 = (fConst7 + 0.6180339887498947) / fConst5 + 1.0;
	fConst9 = 1.0 / (fConst6 * fConst8);
	fConst10 = (fConst7 + 1.6180339887498947) / fConst5 + 1.0;
	fConst11 = 1.0 / (fConst6 * fConst10);
	fConst12 = 1.0 / (fConst7 + 1.0);
	fConst13 = 1.0 - fConst7;
	fConst14 = 1.0 / fConst10;
	fConst15 = (fConst7 + -1.6180339887498947) / fConst5 + 1.0;
	fConst16 = 2.0 * (1.0 - 1.0 / fConst6);
	fConst17 = 1.0 / fConst8;
	fConst18 = (fConst7 + -0.6180339887498947) / fConst5 + 1.0;
	fConst19 = std::tan(1570.7963267948965 / fConst0);
	fConst20 = 1.0 / fConst19;
	fConst21 = 1.0 / ((fConst20 + 1.618033988749895) / fConst19 + 1.0);
	fConst22 = (fConst20 + -1.618033988749895) / fConst19 + 1.0;
	fConst23 = mydsp_faustpower2_f(fConst19);
	fConst24 = 1.0 / fConst23;
	fConst25 = 2.0 * (1.0 - fConst24);
	fConst26 = 1.0 / ((fConst20 + 0.6180339887498947) / fConst19 + 1.0);
	fConst27 = (fConst20 + 1.6180339887498947) / fConst19 + 1.0;
	fConst28 = 1.0 / fConst27;
	fConst29 = 1.0 / (fConst20 + 1.0);
	fConst30 = 1.0 - fConst20;
	fConst31 = (fConst20 + -1.6180339887498947) / fConst19 + 1.0;
	fConst32 = (fConst20 + -0.6180339887498947) / fConst19 + 1.0;
	fConst33 = 1.0 / (fConst23 * fConst27);
	fConst34 = 1.0 / (fConst19 * fConst8);
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
	if (!fVec1) fVec1 = new double[16384];
	if (!fVec3) fVec3 = new double[16384];
	if (!fVec5) fVec5 = new double[16384];
	if (!fVec7) fVec7 = new double[16384];
	if (!fVec9) fVec9 = new double[16384];
	if (!fVec11) fVec11 = new double[16384];
	if (!fVec13) fVec13 = new double[16384];
	if (!fVec15) fVec15 = new double[16384];
	if (!fVec17) fVec17 = new double[16384];
	if (!fVec19) fVec19 = new double[16384];
	if (!fVec22) fVec22 = new double[16384];
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
	if (!fVec48) fVec48 = new double[16384];
	if (!fVec52) fVec52 = new double[16384];
	if (!fVec53) fVec53 = new double[16384];
	if (!fVec57) fVec57 = new double[16384];
	if (!fVec59) fVec59 = new double[16384];
	if (!fVec61) fVec61 = new double[16384];
	if (!fVec63) fVec63 = new double[16384];
	if (!fVec65) fVec65 = new double[16384];
	if (!fVec67) fVec67 = new double[16384];
	if (!fVec69) fVec69 = new double[16384];
	if (!fVec71) fVec71 = new double[16384];
	mem_allocated = true;
}

void Dsp::mem_free()
{
	mem_allocated = false;
	if (fVec1) { delete[] fVec1; fVec1 = 0; }
	if (fVec3) { delete[] fVec3; fVec3 = 0; }
	if (fVec5) { delete[] fVec5; fVec5 = 0; }
	if (fVec7) { delete[] fVec7; fVec7 = 0; }
	if (fVec9) { delete[] fVec9; fVec9 = 0; }
	if (fVec11) { delete[] fVec11; fVec11 = 0; }
	if (fVec13) { delete[] fVec13; fVec13 = 0; }
	if (fVec15) { delete[] fVec15; fVec15 = 0; }
	if (fVec17) { delete[] fVec17; fVec17 = 0; }
	if (fVec19) { delete[] fVec19; fVec19 = 0; }
	if (fVec22) { delete[] fVec22; fVec22 = 0; }
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
	if (fVec48) { delete[] fVec48; fVec48 = 0; }
	if (fVec52) { delete[] fVec52; fVec52 = 0; }
	if (fVec53) { delete[] fVec53; fVec53 = 0; }
	if (fVec57) { delete[] fVec57; fVec57 = 0; }
	if (fVec59) { delete[] fVec59; fVec59 = 0; }
	if (fVec61) { delete[] fVec61; fVec61 = 0; }
	if (fVec63) { delete[] fVec63; fVec63 = 0; }
	if (fVec65) { delete[] fVec65; fVec65 = 0; }
	if (fVec67) { delete[] fVec67; fVec67 = 0; }
	if (fVec69) { delete[] fVec69; fVec69 = 0; }
	if (fVec71) { delete[] fVec71; fVec71 = 0; }
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
	double fSlow4 = static_cast<double>(fHslider4);
	int iSlow5 = itbl0mydspSIG0[static_cast<int>(1e+01 * fSlow4)];
	double fSlow6 = 9.999999999998899e-05 * static_cast<double>(iSlow5);
	int iSlow7 = itbl0mydspSIG0[static_cast<int>(1.1e+02 * fSlow4)];
	double fSlow8 = 9.999999999998899e-05 * static_cast<double>(iSlow7);
	int iSlow9 = itbl0mydspSIG0[static_cast<int>(4e+01 * fSlow4)];
	double fSlow10 = 9.999999999998899e-05 * static_cast<double>(iSlow9);
	int iSlow11 = itbl0mydspSIG0[static_cast<int>(1.4e+02 * fSlow4)];
	double fSlow12 = 9.999999999998899e-05 * static_cast<double>(iSlow11);
	int iSlow13 = itbl0mydspSIG0[static_cast<int>(7e+01 * fSlow4)];
	double fSlow14 = 9.999999999998899e-05 * static_cast<double>(iSlow13);
	int iSlow15 = itbl0mydspSIG0[static_cast<int>(1.7e+02 * fSlow4)];
	double fSlow16 = 9.999999999998899e-05 * static_cast<double>(iSlow15);
	int iSlow17 = itbl0mydspSIG0[static_cast<int>(1e+02 * fSlow4)];
	double fSlow18 = 9.999999999998899e-05 * static_cast<double>(iSlow17);
	int iSlow19 = itbl0mydspSIG0[static_cast<int>(2e+02 * fSlow4)];
	double fSlow20 = 9.999999999998899e-05 * static_cast<double>(iSlow19);
	int iSlow21 = itbl0mydspSIG0[static_cast<int>(1.3e+02 * fSlow4)];
	double fSlow22 = 9.999999999998899e-05 * static_cast<double>(iSlow21);
	int iSlow23 = itbl0mydspSIG0[static_cast<int>(2.3e+02 * fSlow4)];
	double fSlow24 = 9.999999999998899e-05 * static_cast<double>(iSlow23);
	int iSlow25 = itbl0mydspSIG0[static_cast<int>(54.0 * fSlow4)];
	double fSlow26 = 0.0050000000000000044 * static_cast<double>(iSlow25);
	int iSlow27 = itbl0mydspSIG0[static_cast<int>(204.0 * fSlow4)];
	double fSlow28 = 0.0050000000000000044 * static_cast<double>(iSlow27);
	int iSlow29 = itbl0mydspSIG0[static_cast<int>(125.0 * fSlow4)];
	double fSlow30 = 9.999999999998899e-05 * static_cast<double>(iSlow29);
	int iSlow31 = itbl0mydspSIG0[static_cast<int>(25.0 * fSlow4)];
	double fSlow32 = 9.999999999998899e-05 * static_cast<double>(iSlow31);
	int iSlow33 = itbl0mydspSIG0[static_cast<int>(155.0 * fSlow4)];
	double fSlow34 = 9.999999999998899e-05 * static_cast<double>(iSlow33);
	int iSlow35 = itbl0mydspSIG0[static_cast<int>(55.0 * fSlow4)];
	double fSlow36 = 9.999999999998899e-05 * static_cast<double>(iSlow35);
	int iSlow37 = itbl0mydspSIG0[static_cast<int>(185.0 * fSlow4)];
	double fSlow38 = 9.999999999998899e-05 * static_cast<double>(iSlow37);
	int iSlow39 = itbl0mydspSIG0[static_cast<int>(85.0 * fSlow4)];
	double fSlow40 = 9.999999999998899e-05 * static_cast<double>(iSlow39);
	int iSlow41 = itbl0mydspSIG0[static_cast<int>(215.0 * fSlow4)];
	double fSlow42 = 9.999999999998899e-05 * static_cast<double>(iSlow41);
	int iSlow43 = itbl0mydspSIG0[static_cast<int>(115.0 * fSlow4)];
	double fSlow44 = 9.999999999998899e-05 * static_cast<double>(iSlow43);
	int iSlow45 = itbl0mydspSIG0[static_cast<int>(245.0 * fSlow4)];
	double fSlow46 = 9.999999999998899e-05 * static_cast<double>(iSlow45);
	int iSlow47 = itbl0mydspSIG0[static_cast<int>(145.0 * fSlow4)];
	double fSlow48 = 9.999999999998899e-05 * static_cast<double>(iSlow47);
	int iSlow49 = itbl0mydspSIG0[static_cast<int>(134.0 * fSlow4)];
	double fSlow50 = 0.0050000000000000044 * static_cast<double>(iSlow49);
	double fSlow51 = 0.51 * (1.25 * fSlow4 + -0.25);
	double fSlow52 = fConst2 * static_cast<double>(fHslider5);
	int iSlow53 = itbl0mydspSIG0[static_cast<int>(34.0 * fSlow4)];
	double fSlow54 = 0.0050000000000000044 * static_cast<double>(iSlow53);
	int iSlow55 = itbl0mydspSIG0[static_cast<int>(2.4e+02 * fSlow4)];
	double fSlow56 = 9.999999999998899e-05 * static_cast<double>(iSlow55);
	int iSlow57 = itbl0mydspSIG0[static_cast<int>(1.9e+02 * fSlow4)];
	double fSlow58 = 9.999999999998899e-05 * static_cast<double>(iSlow57);
	int iSlow59 = itbl0mydspSIG0[static_cast<int>(175.0 * fSlow4)];
	double fSlow60 = 9.999999999998899e-05 * static_cast<double>(iSlow59);
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		double fTemp0 = static_cast<double>(input0[i0]);
		iVec0[0] = 1;
		fRec0[0] = fSlow0 + fConst3 * fRec0[1];
		fRec16[0] = fSlow1 + fConst3 * fRec16[1];
		double fTemp1 = 1.0 - fRec16[0];
		fRec17[0] = fSlow2 + fConst3 * fRec17[1];
		fRec20[0] = fSlow3 + fConst3 * fRec20[1];
		double fTemp2 = fConst4 * fRec20[0];
		double fTemp3 = std::sin(fTemp2);
		double fTemp4 = std::cos(fTemp2);
		fRec18[0] = fRec19[1] * fTemp3 + fRec18[1] * fTemp4;
		int iTemp5 = 1 - iVec0[1];
		fRec19[0] = static_cast<double>(iTemp5) + fRec19[1] * fTemp4 - fTemp3 * fRec18[1];
		double fTemp6 = 5e+01 * fRec17[0] * (fRec18[0] + 1.0);
		double fTemp7 = fTemp6 + 3.500005;
		double fTemp8 = std::floor(fTemp7);
		double fTemp9 = fTemp6 + (1.0 - fTemp8);
		double fTemp10 = fTemp6 + (2.0 - fTemp8);
		double fTemp11 = fTemp6 + (3.0 - fTemp8);
		double fTemp12 = fTemp6 + (4.0 - fTemp8);
		double fTemp13 = 5e+01 * fRec17[0] * (fRec19[0] + 1.0);
		double fTemp14 = fTemp13 + 3.500005;
		double fTemp15 = std::floor(fTemp14);
		double fTemp16 = fTemp13 + (1.0 - fTemp15);
		double fTemp17 = fTemp13 + (2.0 - fTemp15);
		double fTemp18 = fTemp13 + (3.0 - fTemp15);
		double fTemp19 = 0.7602445970756302 * fRec1[1] - 0.6496369390800624 * fRec54[1];
		double fTemp20 = 0.6496369390800624 * fRec55[1];
		double fTemp21 = 0.7602445970756302 * fRec2[1];
		fVec1[IOTA0 & 16383] = 0.7071067811865476 * fTemp19 + 0.7071067811865475 * (fTemp20 - fTemp21);
		fRec57[0] = 0.9999 * (fRec57[1] + static_cast<double>(iTemp5 * iSlow5)) + fSlow6;
		double fTemp22 = fRec57[0] + -1.49999;
		double fTemp23 = fVec1[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp22)))) & 16383];
		fVec2[0] = fTemp23;
		double fTemp24 = std::floor(fTemp22);
		fRec56[0] = fVec2[1] - (fTemp24 + (2.0 - fRec57[0])) * (fRec56[1] - fTemp23) / (fRec57[0] - fTemp24);
		fRec54[0] = fRec56[0];
		fVec3[IOTA0 & 16383] = 0.7071067811865475 * fTemp19 + 0.7071067811865476 * (fTemp21 - fTemp20);
		fRec59[0] = 0.9999 * (fRec59[1] + static_cast<double>(iTemp5 * iSlow7)) + fSlow8;
		double fTemp25 = fRec59[0] + -1.49999;
		double fTemp26 = fVec3[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp25)))) & 16383];
		fVec4[0] = fTemp26;
		double fTemp27 = std::floor(fTemp25);
		fRec58[0] = fVec4[1] - (fTemp27 + (2.0 - fRec59[0])) * (fRec58[1] - fTemp26) / (fRec59[0] - fTemp27);
		fRec55[0] = fRec58[0];
		double fTemp28 = 0.7602445970756302 * fRec54[1] + 0.6496369390800624 * fRec1[1];
		double fTemp29 = 0.7602445970756302 * fTemp28 - 0.6496369390800624 * fRec51[1];
		double fTemp30 = 0.6496369390800624 * fRec52[1];
		double fTemp31 = 0.7602445970756302 * fRec55[1] + 0.6496369390800624 * fRec2[1];
		double fTemp32 = 0.7602445970756302 * fTemp31;
		fVec5[IOTA0 & 16383] = 0.7071067811865476 * fTemp29 + 0.7071067811865475 * (fTemp30 - fTemp32);
		fRec60[0] = 0.9999 * (fRec60[1] + static_cast<double>(iTemp5 * iSlow9)) + fSlow10;
		double fTemp33 = fRec60[0] + -1.49999;
		double fTemp34 = fVec5[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp33)))) & 16383];
		fVec6[0] = fTemp34;
		double fTemp35 = std::floor(fTemp33);
		fRec53[0] = fVec6[1] - (fTemp35 + (2.0 - fRec60[0])) * (fRec53[1] - fTemp34) / (fRec60[0] - fTemp35);
		fRec51[0] = fRec53[0];
		fVec7[IOTA0 & 16383] = 0.7071067811865475 * fTemp29 + 0.7071067811865476 * (fTemp32 - fTemp30);
		fRec62[0] = 0.9999 * (fRec62[1] + static_cast<double>(iTemp5 * iSlow11)) + fSlow12;
		double fTemp36 = fRec62[0] + -1.49999;
		double fTemp37 = fVec7[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp36)))) & 16383];
		fVec8[0] = fTemp37;
		double fTemp38 = std::floor(fTemp36);
		fRec61[0] = fVec8[1] - (fTemp38 + (2.0 - fRec62[0])) * (fRec61[1] - fTemp37) / (fRec62[0] - fTemp38);
		fRec52[0] = fRec61[0];
		double fTemp39 = 0.7602445970756302 * fRec51[1] + 0.6496369390800624 * fTemp28;
		double fTemp40 = 0.7602445970756302 * fTemp39 - 0.6496369390800624 * fRec48[1];
		double fTemp41 = 0.6496369390800624 * fRec49[1];
		double fTemp42 = 0.7602445970756302 * fRec52[1] + 0.6496369390800624 * fTemp31;
		double fTemp43 = 0.7602445970756302 * fTemp42;
		fVec9[IOTA0 & 16383] = 0.7071067811865476 * fTemp40 + 0.7071067811865475 * (fTemp41 - fTemp43);
		fRec63[0] = 0.9999 * (fRec63[1] + static_cast<double>(iTemp5 * iSlow13)) + fSlow14;
		double fTemp44 = fRec63[0] + -1.49999;
		double fTemp45 = fVec9[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp44)))) & 16383];
		fVec10[0] = fTemp45;
		double fTemp46 = std::floor(fTemp44);
		fRec50[0] = fVec10[1] - (fTemp46 + (2.0 - fRec63[0])) * (fRec50[1] - fTemp45) / (fRec63[0] - fTemp46);
		fRec48[0] = fRec50[0];
		fVec11[IOTA0 & 16383] = 0.7071067811865475 * fTemp40 + 0.7071067811865476 * (fTemp43 - fTemp41);
		fRec65[0] = 0.9999 * (fRec65[1] + static_cast<double>(iTemp5 * iSlow15)) + fSlow16;
		double fTemp47 = fRec65[0] + -1.49999;
		double fTemp48 = fVec11[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp47)))) & 16383];
		fVec12[0] = fTemp48;
		double fTemp49 = std::floor(fTemp47);
		fRec64[0] = fVec12[1] - (fTemp49 + (2.0 - fRec65[0])) * (fRec64[1] - fTemp48) / (fRec65[0] - fTemp49);
		fRec49[0] = fRec64[0];
		double fTemp50 = 0.7602445970756302 * fRec48[1] + 0.6496369390800624 * fTemp39;
		double fTemp51 = 0.7602445970756302 * fTemp50 - 0.6496369390800624 * fRec45[1];
		double fTemp52 = 0.6496369390800624 * fRec46[1];
		double fTemp53 = 0.7602445970756302 * fRec49[1] + 0.6496369390800624 * fTemp42;
		double fTemp54 = 0.7602445970756302 * fTemp53;
		fVec13[IOTA0 & 16383] = 0.7071067811865476 * fTemp51 + 0.7071067811865475 * (fTemp52 - fTemp54);
		fRec66[0] = 0.9999 * (fRec66[1] + static_cast<double>(iTemp5 * iSlow17)) + fSlow18;
		double fTemp55 = fRec66[0] + -1.49999;
		double fTemp56 = fVec13[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp55)))) & 16383];
		fVec14[0] = fTemp56;
		double fTemp57 = std::floor(fTemp55);
		fRec47[0] = fVec14[1] - (fTemp57 + (2.0 - fRec66[0])) * (fRec47[1] - fTemp56) / (fRec66[0] - fTemp57);
		fRec45[0] = fRec47[0];
		fVec15[IOTA0 & 16383] = 0.7071067811865475 * fTemp51 + 0.7071067811865476 * (fTemp54 - fTemp52);
		fRec68[0] = 0.9999 * (fRec68[1] + static_cast<double>(iTemp5 * iSlow19)) + fSlow20;
		double fTemp58 = fRec68[0] + -1.49999;
		double fTemp59 = fVec15[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp58)))) & 16383];
		fVec16[0] = fTemp59;
		double fTemp60 = std::floor(fTemp58);
		fRec67[0] = fVec16[1] - (fTemp60 + (2.0 - fRec68[0])) * (fRec67[1] - fTemp59) / (fRec68[0] - fTemp60);
		fRec46[0] = fRec67[0];
		double fTemp61 = 0.7602445970756302 * fRec45[1] + 0.6496369390800624 * fTemp50;
		double fTemp62 = 0.7602445970756302 * fTemp61 - 0.6496369390800624 * fRec42[1];
		double fTemp63 = 0.6496369390800624 * fRec43[1];
		double fTemp64 = 0.7602445970756302 * fRec46[1] + 0.6496369390800624 * fTemp53;
		double fTemp65 = 0.7602445970756302 * fTemp64;
		fVec17[IOTA0 & 16383] = 0.7071067811865476 * fTemp62 + 0.7071067811865475 * (fTemp63 - fTemp65);
		fRec69[0] = 0.9999 * (fRec69[1] + static_cast<double>(iTemp5 * iSlow21)) + fSlow22;
		double fTemp66 = fRec69[0] + -1.49999;
		double fTemp67 = fVec17[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp66)))) & 16383];
		fVec18[0] = fTemp67;
		double fTemp68 = std::floor(fTemp66);
		fRec44[0] = fVec18[1] - (fTemp68 + (2.0 - fRec69[0])) * (fRec44[1] - fTemp67) / (fRec69[0] - fTemp68);
		fRec42[0] = fRec44[0];
		fVec19[IOTA0 & 16383] = 0.7071067811865475 * fTemp62 + 0.7071067811865476 * (fTemp65 - fTemp63);
		fRec71[0] = 0.9999 * (fRec71[1] + static_cast<double>(iTemp5 * iSlow23)) + fSlow24;
		double fTemp69 = fRec71[0] + -1.49999;
		double fTemp70 = fVec19[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp69)))) & 16383];
		fVec20[0] = fTemp70;
		double fTemp71 = std::floor(fTemp69);
		fRec70[0] = fVec20[1] - (fTemp71 + (2.0 - fRec71[0])) * (fRec70[1] - fTemp70) / (fRec71[0] - fTemp71);
		fRec43[0] = fRec70[0];
		double fTemp72 = 0.7602445970756302 * fRec42[1] + 0.6496369390800624 * fTemp61;
		fVec21[IOTA0 & 1023] = fTemp72;
		int iTemp73 = static_cast<int>(fTemp14);
		int iTemp74 = std::min<int>(512, std::max<int>(0, iTemp73));
		double fTemp75 = fTemp13 + (4.0 - fTemp15);
		double fTemp76 = fTemp13 + (5.0 - fTemp15);
		int iTemp77 = std::min<int>(512, std::max<int>(0, iTemp73 + 1));
		double fTemp78 = fTemp76 * fTemp75;
		int iTemp79 = std::min<int>(512, std::max<int>(0, iTemp73 + 2));
		double fTemp80 = fTemp78 * fTemp18;
		int iTemp81 = std::min<int>(512, std::max<int>(0, iTemp73 + 3));
		double fTemp82 = fTemp80 * fTemp17;
		int iTemp83 = std::min<int>(512, std::max<int>(0, iTemp73 + 4));
		fVec22[IOTA0 & 16383] = fTemp16 * (fTemp17 * (fTemp18 * (0.041666666666666664 * fVec21[(IOTA0 - iTemp74) & 1023] * fTemp75 - 0.16666666666666666 * fTemp76 * fVec21[(IOTA0 - iTemp77) & 1023]) + 0.25 * fTemp78 * fVec21[(IOTA0 - iTemp79) & 1023]) - 0.16666666666666666 * fTemp80 * fVec21[(IOTA0 - iTemp81) & 1023]) + 0.041666666666666664 * fTemp82 * fVec21[(IOTA0 - iTemp83) & 1023];
		fRec72[0] = 0.995 * (fRec72[1] + static_cast<double>(iTemp5 * iSlow25)) + fSlow26;
		double fTemp84 = fRec72[0] + -1.49999;
		double fTemp85 = fVec22[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp84)))) & 16383];
		fVec23[0] = fTemp85;
		double fTemp86 = std::floor(fTemp84);
		fRec41[0] = fVec23[1] - (fTemp86 + (2.0 - fRec72[0])) * (fRec41[1] - fTemp85) / (fRec72[0] - fTemp86);
		double fTemp87 = 0.7602445970756302 * fRec41[0] - 0.6496369390800624 * fRec38[1];
		double fTemp88 = 0.6496369390800624 * fRec39[1];
		double fTemp89 = 5e+01 * fRec17[0] * (1.0 - fRec19[0]);
		double fTemp90 = fTemp89 + 3.500005;
		double fTemp91 = std::floor(fTemp90);
		double fTemp92 = fTemp89 + (2.0 - fTemp91);
		double fTemp93 = fTemp89 + (3.0 - fTemp91);
		double fTemp94 = 0.7602445970756302 * fRec43[1] + 0.6496369390800624 * fTemp64;
		fVec24[IOTA0 & 1023] = fTemp94;
		int iTemp95 = static_cast<int>(fTemp90);
		double fTemp96 = fTemp89 + (4.0 - fTemp91);
		double fTemp97 = fTemp89 + (5.0 - fTemp91);
		double fTemp98 = fTemp97 * fTemp96;
		double fTemp99 = fTemp98 * fTemp93;
		fVec25[IOTA0 & 16383] = (fTemp89 + (1.0 - fTemp91)) * (fTemp92 * (fTemp93 * (0.041666666666666664 * fVec24[(IOTA0 - std::min<int>(512, std::max<int>(0, iTemp95))) & 1023] * fTemp96 - 0.16666666666666666 * fTemp97 * fVec24[(IOTA0 - std::min<int>(512, std::max<int>(0, iTemp95 + 1))) & 1023]) + 0.25 * fTemp98 * fVec24[(IOTA0 - std::min<int>(512, std::max<int>(0, iTemp95 + 2))) & 1023]) - 0.16666666666666666 * fTemp99 * fVec24[(IOTA0 - std::min<int>(512, std::max<int>(0, iTemp95 + 3))) & 1023]) + 0.041666666666666664 * fTemp99 * fTemp92 * fVec24[(IOTA0 - std::min<int>(512, std::max<int>(0, iTemp95 + 4))) & 1023];
		fRec74[0] = 0.995 * (fRec74[1] + static_cast<double>(iTemp5 * iSlow27)) + fSlow28;
		double fTemp100 = fRec74[0] + -1.49999;
		double fTemp101 = fVec25[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp100)))) & 16383];
		fVec26[0] = fTemp101;
		double fTemp102 = std::floor(fTemp100);
		fRec73[0] = fVec26[1] - (fTemp102 + (2.0 - fRec74[0])) * (fRec73[1] - fTemp101) / (fRec74[0] - fTemp102);
		double fTemp103 = 0.7602445970756302 * fRec73[0];
		fVec27[IOTA0 & 16383] = 0.7071067811865476 * fTemp87 + 0.7071067811865475 * (fTemp88 - fTemp103);
		fRec75[0] = 0.9999 * (fRec75[1] + static_cast<double>(iTemp5 * iSlow29)) + fSlow30;
		double fTemp104 = fRec75[0] + -1.49999;
		double fTemp105 = fVec27[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp104)))) & 16383];
		fVec28[0] = fTemp105;
		double fTemp106 = std::floor(fTemp104);
		fRec40[0] = fVec28[1] - (fTemp106 + (2.0 - fRec75[0])) * (fRec40[1] - fTemp105) / (fRec75[0] - fTemp106);
		fRec38[0] = fRec40[0];
		fVec29[IOTA0 & 16383] = 0.7071067811865475 * fTemp87 + 0.7071067811865476 * (fTemp103 - fTemp88);
		fRec77[0] = 0.9999 * (fRec77[1] + static_cast<double>(iTemp5 * iSlow31)) + fSlow32;
		double fTemp107 = fRec77[0] + -1.49999;
		double fTemp108 = fVec29[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp107)))) & 16383];
		fVec30[0] = fTemp108;
		double fTemp109 = std::floor(fTemp107);
		fRec76[0] = fVec30[1] - (fTemp109 + (2.0 - fRec77[0])) * (fRec76[1] - fTemp108) / (fRec77[0] - fTemp109);
		fRec39[0] = fRec76[0];
		double fTemp110 = 0.7602445970756302 * fRec38[1] + 0.6496369390800624 * fRec41[0];
		double fTemp111 = 0.7602445970756302 * fTemp110 - 0.6496369390800624 * fRec35[1];
		double fTemp112 = 0.6496369390800624 * fRec36[1];
		double fTemp113 = 0.7602445970756302 * fRec39[1] + 0.6496369390800624 * fRec73[0];
		double fTemp114 = 0.7602445970756302 * fTemp113;
		fVec31[IOTA0 & 16383] = 0.7071067811865476 * fTemp111 + 0.7071067811865475 * (fTemp112 - fTemp114);
		fRec78[0] = 0.9999 * (fRec78[1] + static_cast<double>(iTemp5 * iSlow33)) + fSlow34;
		double fTemp115 = fRec78[0] + -1.49999;
		double fTemp116 = fVec31[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp115)))) & 16383];
		fVec32[0] = fTemp116;
		double fTemp117 = std::floor(fTemp115);
		fRec37[0] = fVec32[1] - (fTemp117 + (2.0 - fRec78[0])) * (fRec37[1] - fTemp116) / (fRec78[0] - fTemp117);
		fRec35[0] = fRec37[0];
		fVec33[IOTA0 & 16383] = 0.7071067811865475 * fTemp111 + 0.7071067811865476 * (fTemp114 - fTemp112);
		fRec80[0] = 0.9999 * (fRec80[1] + static_cast<double>(iTemp5 * iSlow35)) + fSlow36;
		double fTemp118 = fRec80[0] + -1.49999;
		int iTemp119 = std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp118)));
		double fTemp120 = fVec33[(IOTA0 - iTemp119) & 16383];
		fVec34[0] = fTemp120;
		double fTemp121 = std::floor(fTemp118);
		double fTemp122 = fTemp121 + (2.0 - fRec80[0]);
		double fTemp123 = fRec80[0] - fTemp121;
		fRec79[0] = fVec34[1] - fTemp122 * (fRec79[1] - fTemp120) / fTemp123;
		fRec36[0] = fRec79[0];
		double fTemp124 = 0.7602445970756302 * fRec35[1] + 0.6496369390800624 * fTemp110;
		double fTemp125 = 0.7602445970756302 * fTemp124 - 0.6496369390800624 * fRec32[1];
		double fTemp126 = 0.6496369390800624 * fRec33[1];
		double fTemp127 = 0.7602445970756302 * fRec36[1] + 0.6496369390800624 * fTemp113;
		double fTemp128 = 0.7602445970756302 * fTemp127;
		fVec35[IOTA0 & 16383] = 0.7071067811865476 * fTemp125 + 0.7071067811865475 * (fTemp126 - fTemp128);
		fRec81[0] = 0.9999 * (fRec81[1] + static_cast<double>(iTemp5 * iSlow37)) + fSlow38;
		double fTemp129 = fRec81[0] + -1.49999;
		double fTemp130 = fVec35[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp129)))) & 16383];
		fVec36[0] = fTemp130;
		double fTemp131 = std::floor(fTemp129);
		fRec34[0] = fVec36[1] - (fTemp131 + (2.0 - fRec81[0])) * (fRec34[1] - fTemp130) / (fRec81[0] - fTemp131);
		fRec32[0] = fRec34[0];
		fVec37[IOTA0 & 16383] = 0.7071067811865475 * fTemp125 + 0.7071067811865476 * (fTemp128 - fTemp126);
		fRec83[0] = 0.9999 * (fRec83[1] + static_cast<double>(iTemp5 * iSlow39)) + fSlow40;
		double fTemp132 = fRec83[0] + -1.49999;
		int iTemp133 = std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp132)));
		double fTemp134 = fVec37[(IOTA0 - iTemp133) & 16383];
		fVec38[0] = fTemp134;
		double fTemp135 = std::floor(fTemp132);
		double fTemp136 = fTemp135 + (2.0 - fRec83[0]);
		double fTemp137 = fRec83[0] - fTemp135;
		fRec82[0] = fVec38[1] - fTemp136 * (fRec82[1] - fTemp134) / fTemp137;
		fRec33[0] = fRec82[0];
		double fTemp138 = 0.7602445970756302 * fRec32[1] + 0.6496369390800624 * fTemp124;
		double fTemp139 = 0.7602445970756302 * fTemp138 - 0.6496369390800624 * fRec29[1];
		double fTemp140 = 0.6496369390800624 * fRec30[1];
		double fTemp141 = 0.7602445970756302 * fRec33[1] + 0.6496369390800624 * fTemp127;
		double fTemp142 = 0.7602445970756302 * fTemp141;
		fVec39[IOTA0 & 16383] = 0.7071067811865476 * fTemp139 + 0.7071067811865475 * (fTemp140 - fTemp142);
		fRec84[0] = 0.9999 * (fRec84[1] + static_cast<double>(iTemp5 * iSlow41)) + fSlow42;
		double fTemp143 = fRec84[0] + -1.49999;
		int iTemp144 = std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp143)));
		double fTemp145 = fVec39[(IOTA0 - iTemp144) & 16383];
		fVec40[0] = fTemp145;
		double fTemp146 = std::floor(fTemp143);
		double fTemp147 = fTemp146 + (2.0 - fRec84[0]);
		double fTemp148 = fRec84[0] - fTemp146;
		fRec31[0] = fVec40[1] - fTemp147 * (fRec31[1] - fTemp145) / fTemp148;
		fRec29[0] = fRec31[0];
		fVec41[IOTA0 & 16383] = 0.7071067811865475 * fTemp139 + 0.7071067811865476 * (fTemp142 - fTemp140);
		fRec86[0] = 0.9999 * (fRec86[1] + static_cast<double>(iTemp5 * iSlow43)) + fSlow44;
		double fTemp149 = fRec86[0] + -1.49999;
		int iTemp150 = std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp149)));
		double fTemp151 = fVec41[(IOTA0 - iTemp150) & 16383];
		fVec42[0] = fTemp151;
		double fTemp152 = std::floor(fTemp149);
		double fTemp153 = fTemp152 + (2.0 - fRec86[0]);
		double fTemp154 = fRec86[0] - fTemp152;
		fRec85[0] = fVec42[1] - fTemp153 * (fRec85[1] - fTemp151) / fTemp154;
		fRec30[0] = fRec85[0];
		double fTemp155 = 0.7602445970756302 * fRec29[1] + 0.6496369390800624 * fTemp138;
		double fTemp156 = 0.7602445970756302 * fTemp155 - 0.6496369390800624 * fRec26[1];
		double fTemp157 = 0.6496369390800624 * fRec27[1];
		double fTemp158 = 0.7602445970756302 * fRec30[1] + 0.6496369390800624 * fTemp141;
		double fTemp159 = 0.7602445970756302 * fTemp158;
		fVec43[IOTA0 & 16383] = 0.7071067811865476 * fTemp156 + 0.7071067811865475 * (fTemp157 - fTemp159);
		fRec87[0] = 0.9999 * (fRec87[1] + static_cast<double>(iTemp5 * iSlow45)) + fSlow46;
		double fTemp160 = fRec87[0] + -1.49999;
		double fTemp161 = fVec43[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp160)))) & 16383];
		fVec44[0] = fTemp161;
		double fTemp162 = std::floor(fTemp160);
		fRec28[0] = fVec44[1] - (fTemp162 + (2.0 - fRec87[0])) * (fRec28[1] - fTemp161) / (fRec87[0] - fTemp162);
		fRec26[0] = fRec28[0];
		fVec45[IOTA0 & 16383] = 0.7071067811865475 * fTemp156 + 0.7071067811865476 * (fTemp159 - fTemp157);
		fRec89[0] = 0.9999 * (fRec89[1] + static_cast<double>(iTemp5 * iSlow47)) + fSlow48;
		double fTemp163 = fRec89[0] + -1.49999;
		int iTemp164 = std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp163)));
		double fTemp165 = fVec45[(IOTA0 - iTemp164) & 16383];
		fVec46[0] = fTemp165;
		double fTemp166 = std::floor(fTemp163);
		double fTemp167 = fTemp166 + (2.0 - fRec89[0]);
		double fTemp168 = fRec89[0] - fTemp166;
		fRec88[0] = fVec46[1] - fTemp167 * (fRec88[1] - fTemp165) / fTemp168;
		fRec27[0] = fRec88[0];
		double fTemp169 = 0.7602445970756302 * fRec26[1] + 0.6496369390800624 * fTemp155;
		fVec47[IOTA0 & 16383] = fTemp169;
		int iTemp170 = static_cast<int>(fTemp7);
		int iTemp171 = std::max<int>(0, iTemp170);
		double fTemp172 = fTemp6 + (5.0 - fTemp8);
		int iTemp173 = std::max<int>(0, iTemp170 + 1);
		double fTemp174 = fTemp172 * fTemp12;
		int iTemp175 = std::max<int>(0, iTemp170 + 2);
		double fTemp176 = fTemp174 * fTemp11;
		int iTemp177 = std::max<int>(0, iTemp170 + 3);
		double fTemp178 = fTemp176 * fTemp10;
		int iTemp179 = std::max<int>(0, iTemp170 + 4);
		fVec48[IOTA0 & 16383] = fTemp9 * (fTemp10 * (fTemp11 * (0.041666666666666664 * fVec47[(IOTA0 - std::min<int>(8192, iTemp171)) & 16383] * fTemp12 - 0.16666666666666666 * fTemp172 * fVec47[(IOTA0 - std::min<int>(8192, iTemp173)) & 16383]) + 0.25 * fTemp174 * fVec47[(IOTA0 - std::min<int>(8192, iTemp175)) & 16383]) - 0.16666666666666666 * fTemp176 * fVec47[(IOTA0 - std::min<int>(8192, iTemp177)) & 16383]) + 0.041666666666666664 * fTemp178 * fVec47[(IOTA0 - std::min<int>(8192, iTemp179)) & 16383];
		fRec90[0] = 0.995 * (fRec90[1] + static_cast<double>(iTemp5 * iSlow49)) + fSlow50;
		double fTemp180 = fRec90[0] + -1.49999;
		double fTemp181 = fVec48[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp180)))) & 16383];
		fVec49[0] = fTemp181;
		double fTemp182 = std::floor(fTemp180);
		fRec25[0] = fVec49[1] - (fTemp182 + (2.0 - fRec90[0])) * (fRec25[1] - fTemp181) / (fRec90[0] - fTemp182);
		fRec24[0] = -(fConst12 * (fConst13 * fRec24[1] - fConst7 * (fRec25[0] - fRec25[1])));
		fRec23[0] = fRec24[0] - fConst14 * (fConst15 * fRec23[2] + fConst16 * fRec23[1]);
		fRec22[0] = fConst11 * (fRec23[2] + (fRec23[0] - 2.0 * fRec23[1])) - fConst17 * (fConst18 * fRec22[2] + fConst16 * fRec22[1]);
		double fTemp183 = fConst25 * fRec21[1];
		fRec21[0] = fConst9 * (fRec22[2] + (fRec22[0] - 2.0 * fRec22[1])) - fConst21 * (fConst22 * fRec21[2] + fTemp183);
		fRec96[0] = -(fConst12 * (fConst13 * fRec96[1] - (fRec25[0] + fRec25[1])));
		fRec95[0] = fRec96[0] - fConst14 * (fConst15 * fRec95[2] + fConst16 * fRec95[1]);
		fRec94[0] = fConst14 * (fRec95[2] + fRec95[0] + 2.0 * fRec95[1]) - fConst17 * (fConst18 * fRec94[2] + fConst16 * fRec94[1]);
		double fTemp184 = fRec94[2] + fRec94[0] + 2.0 * fRec94[1];
		fVec50[0] = fTemp184;
		fRec93[0] = -(fConst29 * (fConst30 * fRec93[1] - fConst17 * (fTemp184 + fVec50[1])));
		fRec92[0] = fRec93[0] - fConst28 * (fConst31 * fRec92[2] + fConst25 * fRec92[1]);
		fRec91[0] = fConst28 * (fRec92[2] + fRec92[0] + 2.0 * fRec92[1]) - fConst26 * (fConst32 * fRec91[2] + fConst25 * fRec91[1]);
		fRec99[0] = -(fConst29 * (fConst30 * fRec99[1] - fConst34 * (fTemp184 - fVec50[1])));
		fRec98[0] = fRec99[0] - fConst28 * (fConst31 * fRec98[2] + fConst25 * fRec98[1]);
		fRec97[0] = fConst33 * (fRec98[2] + (fRec98[0] - 2.0 * fRec98[1])) - fConst26 * (fConst32 * fRec97[2] + fConst25 * fRec97[1]);
		fRec100[0] = fSlow52 + fConst3 * fRec100[1];
		double fTemp185 = std::pow(1e+01, -(fSlow51 / std::max<double>(0.2, fRec100[0])));
		double fTemp186 = fTemp0 + (fRec21[2] + fConst21 * (fTemp183 + fConst22 * fRec21[0]) + fConst26 * (fRec91[2] + fRec91[0] + 2.0 * fRec91[1] + fConst24 * (fRec97[2] + (fRec97[0] - 2.0 * fRec97[1])))) * fTemp185;
		fVec51[IOTA0 & 1023] = fTemp186;
		fRec15[0] = fTemp1 * (fTemp9 * (fTemp10 * (fTemp11 * (0.041666666666666664 * fTemp12 * fVec51[(IOTA0 - std::min<int>(512, iTemp171)) & 1023] - 0.16666666666666666 * fTemp172 * fVec51[(IOTA0 - std::min<int>(512, iTemp173)) & 1023]) + 0.25 * fTemp174 * fVec51[(IOTA0 - std::min<int>(512, iTemp175)) & 1023]) - 0.16666666666666666 * fTemp176 * fVec51[(IOTA0 - std::min<int>(512, iTemp177)) & 1023]) + 0.041666666666666664 * fTemp178 * fVec51[(IOTA0 - std::min<int>(512, iTemp179)) & 1023]) + fRec16[0] * fRec15[1];
		double fTemp187 = 0.6967067093471654 * fRec15[0] - 0.7173560908995228 * fRec12[1];
		double fTemp188 = 0.7173560908995228 * fRec13[1];
		double fTemp189 = 5e+01 * fRec17[0] * (1.0 - fRec18[0]);
		double fTemp190 = fTemp189 + 3.500005;
		double fTemp191 = std::floor(fTemp190);
		double fTemp192 = fTemp189 + (2.0 - fTemp191);
		double fTemp193 = fTemp189 + (3.0 - fTemp191);
		double fTemp194 = 0.7602445970756302 * fRec27[1] + 0.6496369390800624 * fTemp158;
		fVec52[IOTA0 & 16383] = fTemp194;
		int iTemp195 = static_cast<int>(fTemp190);
		double fTemp196 = fTemp189 + (4.0 - fTemp191);
		double fTemp197 = fTemp189 + (5.0 - fTemp191);
		double fTemp198 = fTemp197 * fTemp196;
		double fTemp199 = fTemp198 * fTemp193;
		fVec53[IOTA0 & 16383] = (fTemp189 + (1.0 - fTemp191)) * (fTemp192 * (fTemp193 * (0.041666666666666664 * fVec52[(IOTA0 - std::min<int>(8192, std::max<int>(0, iTemp195))) & 16383] * fTemp196 - 0.16666666666666666 * fTemp197 * fVec52[(IOTA0 - std::min<int>(8192, std::max<int>(0, iTemp195 + 1))) & 16383]) + 0.25 * fTemp198 * fVec52[(IOTA0 - std::min<int>(8192, std::max<int>(0, iTemp195 + 2))) & 16383]) - 0.16666666666666666 * fTemp199 * fVec52[(IOTA0 - std::min<int>(8192, std::max<int>(0, iTemp195 + 3))) & 16383]) + 0.041666666666666664 * fTemp199 * fTemp192 * fVec52[(IOTA0 - std::min<int>(8192, std::max<int>(0, iTemp195 + 4))) & 16383];
		fRec107[0] = 0.995 * (fRec107[1] + static_cast<double>(iTemp5 * iSlow53)) + fSlow54;
		double fTemp200 = fRec107[0] + -1.49999;
		double fTemp201 = fVec53[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp200)))) & 16383];
		fVec54[0] = fTemp201;
		double fTemp202 = std::floor(fTemp200);
		fRec106[0] = fVec54[1] - (fTemp202 + (2.0 - fRec107[0])) * (fRec106[1] - fTemp201) / (fRec107[0] - fTemp202);
		fRec105[0] = -(fConst12 * (fConst13 * fRec105[1] - fConst7 * (fRec106[0] - fRec106[1])));
		fRec104[0] = fRec105[0] - fConst14 * (fConst15 * fRec104[2] + fConst16 * fRec104[1]);
		fRec103[0] = fConst11 * (fRec104[2] + (fRec104[0] - 2.0 * fRec104[1])) - fConst17 * (fConst18 * fRec103[2] + fConst16 * fRec103[1]);
		double fTemp203 = fConst25 * fRec102[1];
		fRec102[0] = fConst9 * (fRec103[2] + (fRec103[0] - 2.0 * fRec103[1])) - fConst21 * (fConst22 * fRec102[2] + fTemp203);
		fRec113[0] = -(fConst12 * (fConst13 * fRec113[1] - (fRec106[0] + fRec106[1])));
		fRec112[0] = fRec113[0] - fConst14 * (fConst15 * fRec112[2] + fConst16 * fRec112[1]);
		fRec111[0] = fConst14 * (fRec112[2] + fRec112[0] + 2.0 * fRec112[1]) - fConst17 * (fConst18 * fRec111[2] + fConst16 * fRec111[1]);
		double fTemp204 = fRec111[2] + fRec111[0] + 2.0 * fRec111[1];
		fVec55[0] = fTemp204;
		fRec110[0] = -(fConst29 * (fConst30 * fRec110[1] - fConst17 * (fTemp204 + fVec55[1])));
		fRec109[0] = fRec110[0] - fConst28 * (fConst31 * fRec109[2] + fConst25 * fRec109[1]);
		fRec108[0] = fConst28 * (fRec109[2] + fRec109[0] + 2.0 * fRec109[1]) - fConst26 * (fConst32 * fRec108[2] + fConst25 * fRec108[1]);
		fRec116[0] = -(fConst29 * (fConst30 * fRec116[1] - fConst34 * (fTemp204 - fVec55[1])));
		fRec115[0] = fRec116[0] - fConst28 * (fConst31 * fRec115[2] + fConst25 * fRec115[1]);
		fRec114[0] = fConst33 * (fRec115[2] + (fRec115[0] - 2.0 * fRec115[1])) - fConst26 * (fConst32 * fRec114[2] + fConst25 * fRec114[1]);
		double fTemp205 = fTemp0 + fTemp185 * (fRec102[2] + fConst21 * (fTemp203 + fConst22 * fRec102[0]) + fConst26 * (fRec108[2] + fRec108[0] + 2.0 * fRec108[1] + fConst24 * (fRec114[2] + (fRec114[0] - 2.0 * fRec114[1]))));
		fVec56[IOTA0 & 1023] = fTemp205;
		fRec101[0] = fTemp1 * (fTemp16 * (fTemp17 * (fTemp18 * (0.041666666666666664 * fTemp75 * fVec56[(IOTA0 - iTemp74) & 1023] - 0.16666666666666666 * fTemp76 * fVec56[(IOTA0 - iTemp77) & 1023]) + 0.25 * fTemp78 * fVec56[(IOTA0 - iTemp79) & 1023]) - 0.16666666666666666 * fTemp80 * fVec56[(IOTA0 - iTemp81) & 1023]) + 0.041666666666666664 * fTemp82 * fVec56[(IOTA0 - iTemp83) & 1023]) + fRec16[0] * fRec101[1];
		double fTemp206 = 0.6967067093471654 * fRec101[0];
		fVec57[IOTA0 & 16383] = 0.7071067811865476 * fTemp187 + 0.7071067811865475 * (fTemp188 - fTemp206);
		double fTemp207 = fVec57[(IOTA0 - iTemp119) & 16383];
		fVec58[0] = fTemp207;
		fRec14[0] = fVec58[1] - fTemp122 * (fRec14[1] - fTemp207) / fTemp123;
		fRec12[0] = fRec14[0];
		fVec59[IOTA0 & 16383] = 0.7071067811865475 * fTemp187 + 0.7071067811865476 * (fTemp206 - fTemp188);
		fRec118[0] = 0.9999 * (fRec118[1] + static_cast<double>(iTemp5 * iSlow55)) + fSlow56;
		double fTemp208 = fRec118[0] + -1.49999;
		double fTemp209 = fVec59[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp208)))) & 16383];
		fVec60[0] = fTemp209;
		double fTemp210 = std::floor(fTemp208);
		fRec117[0] = fVec60[1] - (fTemp210 + (2.0 - fRec118[0])) * (fRec117[1] - fTemp209) / (fRec118[0] - fTemp210);
		fRec13[0] = fRec117[0];
		double fTemp211 = 0.6967067093471654 * fRec12[1] + 0.7173560908995228 * fRec15[0];
		double fTemp212 = 0.6967067093471654 * fTemp211 - 0.7173560908995228 * fRec9[1];
		double fTemp213 = 0.7173560908995228 * fRec10[1];
		double fTemp214 = 0.6967067093471654 * fRec13[1] + 0.7173560908995228 * fRec101[0];
		double fTemp215 = 0.6967067093471654 * fTemp214;
		fVec61[IOTA0 & 16383] = 0.7071067811865476 * fTemp212 + 0.7071067811865475 * (fTemp213 - fTemp215);
		double fTemp216 = fVec61[(IOTA0 - iTemp144) & 16383];
		fVec62[0] = fTemp216;
		fRec11[0] = fVec62[1] - fTemp147 * (fRec11[1] - fTemp216) / fTemp148;
		fRec9[0] = fRec11[0];
		fVec63[IOTA0 & 16383] = 0.7071067811865475 * fTemp212 + 0.7071067811865476 * (fTemp215 - fTemp213);
		double fTemp217 = fVec63[(IOTA0 - iTemp133) & 16383];
		fVec64[0] = fTemp217;
		fRec119[0] = fVec64[1] - fTemp136 * (fRec119[1] - fTemp217) / fTemp137;
		fRec10[0] = fRec119[0];
		double fTemp218 = 0.6967067093471654 * fRec9[1] + 0.7173560908995228 * fTemp211;
		double fTemp219 = 0.6967067093471654 * fTemp218 - 0.7173560908995228 * fRec6[1];
		double fTemp220 = 0.7173560908995228 * fRec7[1];
		double fTemp221 = 0.6967067093471654 * fRec10[1] + 0.7173560908995228 * fTemp214;
		double fTemp222 = 0.6967067093471654 * fTemp221;
		fVec65[IOTA0 & 16383] = 0.7071067811865476 * fTemp219 + 0.7071067811865475 * (fTemp220 - fTemp222);
		double fTemp223 = fVec65[(IOTA0 - iTemp150) & 16383];
		fVec66[0] = fTemp223;
		fRec8[0] = fVec66[1] - fTemp153 * (fRec8[1] - fTemp223) / fTemp154;
		fRec6[0] = fRec8[0];
		fVec67[IOTA0 & 16383] = 0.7071067811865475 * fTemp219 + 0.7071067811865476 * (fTemp222 - fTemp220);
		fRec121[0] = 0.9999 * (fRec121[1] + static_cast<double>(iTemp5 * iSlow57)) + fSlow58;
		double fTemp224 = fRec121[0] + -1.49999;
		double fTemp225 = fVec67[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp224)))) & 16383];
		fVec68[0] = fTemp225;
		double fTemp226 = std::floor(fTemp224);
		fRec120[0] = fVec68[1] - (fTemp226 + (2.0 - fRec121[0])) * (fRec120[1] - fTemp225) / (fRec121[0] - fTemp226);
		fRec7[0] = fRec120[0];
		double fTemp227 = 0.6967067093471654 * fRec6[1] + 0.7173560908995228 * fTemp218;
		double fTemp228 = 0.6967067093471654 * fTemp227 - 0.7173560908995228 * fRec3[1];
		double fTemp229 = 0.7173560908995228 * fRec4[1];
		double fTemp230 = 0.6967067093471654 * fRec7[1] + 0.7173560908995228 * fTemp221;
		double fTemp231 = 0.6967067093471654 * fTemp230;
		fVec69[IOTA0 & 16383] = 0.7071067811865476 * fTemp228 + 0.7071067811865475 * (fTemp229 - fTemp231);
		fRec122[0] = 0.9999 * (fRec122[1] + static_cast<double>(iTemp5 * iSlow59)) + fSlow60;
		double fTemp232 = fRec122[0] + -1.49999;
		double fTemp233 = fVec69[(IOTA0 - std::min<int>(8192, std::max<int>(0, static_cast<int>(fTemp232)))) & 16383];
		fVec70[0] = fTemp233;
		double fTemp234 = std::floor(fTemp232);
		fRec5[0] = fVec70[1] - (fTemp234 + (2.0 - fRec122[0])) * (fRec5[1] - fTemp233) / (fRec122[0] - fTemp234);
		fRec3[0] = fRec5[0];
		fVec71[IOTA0 & 16383] = 0.7071067811865475 * fTemp228 + 0.7071067811865476 * (fTemp231 - fTemp229);
		double fTemp235 = fVec71[(IOTA0 - iTemp164) & 16383];
		fVec72[0] = fTemp235;
		fRec123[0] = fVec72[1] - fTemp167 * (fRec123[1] - fTemp235) / fTemp168;
		fRec4[0] = fRec123[0];
		fRec1[0] = 0.6967067093471654 * fRec3[1] + 0.7173560908995228 * fTemp227;
		fRec2[0] = 0.6967067093471654 * fRec4[1] + 0.7173560908995228 * fTemp230;
		output0[i0] = static_cast<FAUSTFLOAT>(fTemp0 * (1.0 - fRec0[0]) + 0.5 * fRec0[0] * (fRec1[0] + fRec2[0]));
		iVec0[1] = iVec0[0];
		fRec0[1] = fRec0[0];
		fRec16[1] = fRec16[0];
		fRec17[1] = fRec17[0];
		fRec20[1] = fRec20[0];
		fRec18[1] = fRec18[0];
		fRec19[1] = fRec19[0];
		IOTA0 = IOTA0 + 1;
		fRec57[1] = fRec57[0];
		fVec2[1] = fVec2[0];
		fRec56[1] = fRec56[0];
		fRec54[1] = fRec54[0];
		fRec59[1] = fRec59[0];
		fVec4[1] = fVec4[0];
		fRec58[1] = fRec58[0];
		fRec55[1] = fRec55[0];
		fRec60[1] = fRec60[0];
		fVec6[1] = fVec6[0];
		fRec53[1] = fRec53[0];
		fRec51[1] = fRec51[0];
		fRec62[1] = fRec62[0];
		fVec8[1] = fVec8[0];
		fRec61[1] = fRec61[0];
		fRec52[1] = fRec52[0];
		fRec63[1] = fRec63[0];
		fVec10[1] = fVec10[0];
		fRec50[1] = fRec50[0];
		fRec48[1] = fRec48[0];
		fRec65[1] = fRec65[0];
		fVec12[1] = fVec12[0];
		fRec64[1] = fRec64[0];
		fRec49[1] = fRec49[0];
		fRec66[1] = fRec66[0];
		fVec14[1] = fVec14[0];
		fRec47[1] = fRec47[0];
		fRec45[1] = fRec45[0];
		fRec68[1] = fRec68[0];
		fVec16[1] = fVec16[0];
		fRec67[1] = fRec67[0];
		fRec46[1] = fRec46[0];
		fRec69[1] = fRec69[0];
		fVec18[1] = fVec18[0];
		fRec44[1] = fRec44[0];
		fRec42[1] = fRec42[0];
		fRec71[1] = fRec71[0];
		fVec20[1] = fVec20[0];
		fRec70[1] = fRec70[0];
		fRec43[1] = fRec43[0];
		fRec72[1] = fRec72[0];
		fVec23[1] = fVec23[0];
		fRec41[1] = fRec41[0];
		fRec74[1] = fRec74[0];
		fVec26[1] = fVec26[0];
		fRec73[1] = fRec73[0];
		fRec75[1] = fRec75[0];
		fVec28[1] = fVec28[0];
		fRec40[1] = fRec40[0];
		fRec38[1] = fRec38[0];
		fRec77[1] = fRec77[0];
		fVec30[1] = fVec30[0];
		fRec76[1] = fRec76[0];
		fRec39[1] = fRec39[0];
		fRec78[1] = fRec78[0];
		fVec32[1] = fVec32[0];
		fRec37[1] = fRec37[0];
		fRec35[1] = fRec35[0];
		fRec80[1] = fRec80[0];
		fVec34[1] = fVec34[0];
		fRec79[1] = fRec79[0];
		fRec36[1] = fRec36[0];
		fRec81[1] = fRec81[0];
		fVec36[1] = fVec36[0];
		fRec34[1] = fRec34[0];
		fRec32[1] = fRec32[0];
		fRec83[1] = fRec83[0];
		fVec38[1] = fVec38[0];
		fRec82[1] = fRec82[0];
		fRec33[1] = fRec33[0];
		fRec84[1] = fRec84[0];
		fVec40[1] = fVec40[0];
		fRec31[1] = fRec31[0];
		fRec29[1] = fRec29[0];
		fRec86[1] = fRec86[0];
		fVec42[1] = fVec42[0];
		fRec85[1] = fRec85[0];
		fRec30[1] = fRec30[0];
		fRec87[1] = fRec87[0];
		fVec44[1] = fVec44[0];
		fRec28[1] = fRec28[0];
		fRec26[1] = fRec26[0];
		fRec89[1] = fRec89[0];
		fVec46[1] = fVec46[0];
		fRec88[1] = fRec88[0];
		fRec27[1] = fRec27[0];
		fRec90[1] = fRec90[0];
		fVec49[1] = fVec49[0];
		fRec25[1] = fRec25[0];
		fRec24[1] = fRec24[0];
		fRec23[2] = fRec23[1];
		fRec23[1] = fRec23[0];
		fRec22[2] = fRec22[1];
		fRec22[1] = fRec22[0];
		fRec21[2] = fRec21[1];
		fRec21[1] = fRec21[0];
		fRec96[1] = fRec96[0];
		fRec95[2] = fRec95[1];
		fRec95[1] = fRec95[0];
		fRec94[2] = fRec94[1];
		fRec94[1] = fRec94[0];
		fVec50[1] = fVec50[0];
		fRec93[1] = fRec93[0];
		fRec92[2] = fRec92[1];
		fRec92[1] = fRec92[0];
		fRec91[2] = fRec91[1];
		fRec91[1] = fRec91[0];
		fRec99[1] = fRec99[0];
		fRec98[2] = fRec98[1];
		fRec98[1] = fRec98[0];
		fRec97[2] = fRec97[1];
		fRec97[1] = fRec97[0];
		fRec100[1] = fRec100[0];
		fRec15[1] = fRec15[0];
		fRec107[1] = fRec107[0];
		fVec54[1] = fVec54[0];
		fRec106[1] = fRec106[0];
		fRec105[1] = fRec105[0];
		fRec104[2] = fRec104[1];
		fRec104[1] = fRec104[0];
		fRec103[2] = fRec103[1];
		fRec103[1] = fRec103[0];
		fRec102[2] = fRec102[1];
		fRec102[1] = fRec102[0];
		fRec113[1] = fRec113[0];
		fRec112[2] = fRec112[1];
		fRec112[1] = fRec112[0];
		fRec111[2] = fRec111[1];
		fRec111[1] = fRec111[0];
		fVec55[1] = fVec55[0];
		fRec110[1] = fRec110[0];
		fRec109[2] = fRec109[1];
		fRec109[1] = fRec109[0];
		fRec108[2] = fRec108[1];
		fRec108[1] = fRec108[0];
		fRec116[1] = fRec116[0];
		fRec115[2] = fRec115[1];
		fRec115[1] = fRec115[0];
		fRec114[2] = fRec114[1];
		fRec114[1] = fRec114[0];
		fRec101[1] = fRec101[0];
		fVec58[1] = fVec58[0];
		fRec14[1] = fRec14[0];
		fRec12[1] = fRec12[0];
		fRec118[1] = fRec118[0];
		fVec60[1] = fVec60[0];
		fRec117[1] = fRec117[0];
		fRec13[1] = fRec13[0];
		fVec62[1] = fVec62[0];
		fRec11[1] = fRec11[0];
		fRec9[1] = fRec9[0];
		fVec64[1] = fVec64[0];
		fRec119[1] = fRec119[0];
		fRec10[1] = fRec10[0];
		fVec66[1] = fVec66[0];
		fRec8[1] = fRec8[0];
		fRec6[1] = fRec6[0];
		fRec121[1] = fRec121[0];
		fVec68[1] = fVec68[0];
		fRec120[1] = fRec120[0];
		fRec7[1] = fRec7[0];
		fRec122[1] = fRec122[0];
		fVec70[1] = fVec70[0];
		fRec5[1] = fRec5[0];
		fRec3[1] = fRec3[0];
		fVec72[1] = fVec72[0];
		fRec123[1] = fRec123[0];
		fRec4[1] = fRec4[0];
		fRec1[1] = fRec1[0];
		fRec2[1] = fRec2[0];
	}
}

void __rt_func Dsp::compute_static(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0, PluginDef *p)
{
	static_cast<Dsp*>(p)->compute(count, input0, output0);
}

int Dsp::register_par(const ParamReg& reg)
{
	reg.registerFloatVar("ambient_reverb.damping",N_("Damping"),"S","",&fHslider1, 0.3, 0.0, 0.95, 0.01, 0);
	reg.registerFloatVar("ambient_reverb.decay",N_("Decay"),"S","",&fHslider5, 4.0, 0.2, 2e+01, 0.1, 0);
	reg.registerFloatVar("ambient_reverb.depth",N_("Mod Depth"),"S","",&fHslider2, 0.3, 0.0, 1.0, 0.01, 0);
	reg.registerFloatVar("ambient_reverb.mix",N_("Mix"),"S","",&fHslider0, 3e+01, 0.0, 1e+02, 1.0, 0);
	reg.registerFloatVar("ambient_reverb.rate",N_("Mod Rate"),"S","",&fHslider3, 0.4, 0.1, 3.0, 0.05, 0);
	reg.registerFloatVar("ambient_reverb.size",N_("Size"),"S","",&fHslider4, 1.5, 0.5, 3.0, 0.05, 0);
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

} // end namespace ambient_reverb

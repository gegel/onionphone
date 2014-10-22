/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*  Define CELPDIAG to enable diagnostics for abnormal situations
    during encoding and decoding.  Diagnostics are written to
    standard error.  */

/* #define CELPDIAG */

#define CELP_ERROR(code) { if (celp_error == CELP_OK) { celp_error = code; } }

#define CELP_PI 3.14159265358979323846

static void autohf(float si[], float w[], int n, int p, float omega, float a[],
		   float rc[]);
static void bwexp(float alpha, float pc[], float pcexp[], int n);
static void cbsearch(int l, float v[]);
static float cgain(const float ex[], int l, int first, int len, float *match);
static int clip(float s[], int l);
static void confg(float s[], int l, float d1[], float d2[],
		  float d3[], float d4[], int isw1, int isw2, int isw3,
		  int isw4);
static void cor(float *rar, int idim, int n, float *c0, float *c);
static void csub(float s[], float v[], int l, int lp);
static void dcodcbg(int cbgbits, int bitsum1, int bitsum2,
		    int *bitpointer, int nn, short stream[], float cbg[]);
static void dcodcbi(int cbbits, int bitsum1, int bitsum2,
		    int *bitpointer, int nn, short stream[], int cbi[]);
static void dcodpg(int pgbits, int bitsum1, int bitsum2,
		   int *bitpointer, int nn, short stream[], float pgs[]);
static void dcodtau(int taubits, int taudelta, int bitsum1,
		    int bitsum2, int *bitpointer, int nn, short stream[],
		    const float pddecode[], int pdtabi[], float taus[]);
static float sinc(float arg);
static int qd(float d);
static void delay(float x[], int start, int n, float d, int m, float y[]);
static float sinc(float arg);
static void durbin(float c0, float *c, float *a, int n);
static float gainencode(float input, int *index);
static void gaindecode(int gindex, int bits, float *gain);
#ifdef PROTECT
static void encodeham(int codelength1, int codelength2,
		      int hmatrix[], int *paritybit, int codeword[]);
static void decodeham(int codelength1, int hmatrix[], int syndrometable[],
		      int paritybit, int codeword[], int *twoerror,
		      int *synflag);
#endif
static void ham(float win[], int n);
static void impulse(int l);
static void intanaly(float lspnew[], int nn, float lsp[][MAXNO]);
static void intsynth(float lspnew[], int nn, float lsp[][MAXNO],
		     int twoerror, float syndavg);
static void ldelay(float x[], int start, int n, float d, int m, float y[]);
static void lsp34(float freq[], int no, const int bits[], int findex[]);
static void lspdecode34(int findex[], int no, float freq[]);
static void lsptopc(float f[], float pc[]);
static void mexcite1(int l);
static void mexcite2(int l);
static void mexcite3(float *cgain);
#ifdef MOVEFR_SUB
static void movefr(int n, float *a, float *b);
#else
#define movefr(n, src, dest) memcpy((dest), (src), (n) * sizeof(float))
#endif
#ifdef PROTECT
static void matrixgen(int codelength1, int codelength2,
		      int hmatrix[], int syndrometable[]);
#endif
static void pack(int value, int bits, short array[], int *pointer);
static void packtau(int value, int bits, const int pdencode[], short array[],
		    int *pointer);
static void pctolsp2(float a[], int m, float freq[], int *lspflag);
static void pctorc(const float lpc[], float rc[], int n);
static float pgain(const float ex[], int l, int first, int m, int len,
		   float *match);
static void pitchvq(float rar[], int idim, float buf[], int idimb, float b[],
		    char type[]);
static void polefilt(const float a[], int n, float z[], float xy[], int len);
static void postfilt(float s[], int l, float alpha, float beta,
		     float *powerin, float *powerout, float dp1[],
		     float dp2[], float dp3[]);
static void prefilt(float s[], int l, float dpp[]);
static void psearch(int l);
static float pitchencode(float input, int *index);
static void pitchdecode(int pindex, float *pitch);
#ifdef ROUNDFUNC
static int round(float afloat);
#else
#define round(f) (((f) < 0) ? \
    (((int) (f)) - ((((f) - ((int) (f))) <= -0.5) ? 1 : 0)) \
    	    	    	    : \
    (((int) (f)) + ((((f) - ((int) (f))) >=  0.5) ? 1 : 0)))

#endif
static void setr(int n, float v, float a[]);
#ifdef PROTECT
static void smoothcbgain(float *cbgain, int twoerror, float syndavg,
			 float gains[], int subframe);
static void smoothpgain(float *pgain, int twoerror, float syndavg,
			float pgains[], int subframe);
static void smoothtau(float *tau, int twoerror, float syndavg,
		      float tau3, int subframe);
#endif
static void unpack(const short array[], int bits, int *value, int *pointer);
#ifdef PROTECT
static void variance(float arr[], int no, float *var, float *avg);
#endif
static void vdecode(float decodedgain, int l, float vdecoded[]);
static void zerofilt(const float b[], int n, float z[], float xy[], int len);

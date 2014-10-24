/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */


extern void analys(float speech[], int voice[2], int *pitch, float *rms,
		   float rc[]);
extern void bsynz(float coef[], int ip, int iv, float sout[], float rms,
		  float ratio, float g2pass);
extern void channel(int which, int *ipitv, int *irms, int irc[ORDER],
		    int ibits[54]);
extern void dcbias(int len, float *speech, float *sigout);
extern void decode(int ipitv, int *irms, int irc[MAXORD],
		   int voice[2], int *pitch, float *rms, float rc[ORDER]);
extern void deemp0(float x[], int n);
extern void difmag(float speech[], int tau[], int ltau, int maxlag,
		   float amdf[], int *minptr, int *maxptr);
extern void dyptrk(float amdf[], int minptr, int voice, int *pitch, int *midx);
extern void encode(int voice[2], int *pitch, float *rms, float rc[ORDER],
		   int *ipitch, int *irms, int irc[ORDER]);
extern void energy(int len, float speech[], float *rms);
extern void ham84(int input, int *output, int *errcnt);
extern void hp100(float speech[]);
extern void initialize1(void);
extern void initialize2(void);
extern void invert(float phi[MAXORD][MAXORD], float psi[],
		   float rc[MAXORD][AF]);
extern void irc2pc(float rc[MAXORD][11], float pc[], float gprime,
		   float *g2pass, int where);
extern void ivfilt(float ivbuf[], float lpbuf[], float ivrc[]);
extern void lpfilt31(float inbuf[], float lpbuf[]);
extern int median(int d1, int d2, int d3);
extern void mload(int awinf, float speech[], float phi[ORDER][ORDER],
		  float psi[]);
extern void onset(float pebuf[], int osbuf[], int *osptr);
extern void pitsyn(int voice[], int *pitch, float *rms, float rc[],
		   int ivuv[], int ipiti[], float rmsi[],
		   float rci[ORDER][11], int *nout, float *ratio);
extern void placea(int ipitch, int voibuf[2][AF + 1], int obound,
		   int vwin[2][AF], int awin[2][AF], int ewin[2][AF]);
extern void placev(int osbuf[], int osptr, int *obound, int vwin[2][AF]);
extern void preemp(float *inbuf, float *pebuf, int nsamp, float coef, float *z);
extern int Rrandom(void);
extern int lp_round(double afloat);
extern void rcchk(float rc1f[MAXORD][AF]);
extern void synths(int voice[], int *pitch, float *rms,
		   float rc[], float speech[], int *k);
extern void tbdm(float speech[], int tau[], float amdf[], int *minptr,
		 int *maxptr, int *mintau);
extern void voicin(int vwin[2][AF], float *inbuf, float *lpbuf, int half,
		   float minamd, float maxamd, int mintau, float ivrc[2],
		   int *obound, int voibuf[2][AF + 1]);
#ifdef NN_VOICE
extern void voicing(float speech[], float lpspeech[], int start, int end,
		    int pitch, int *v1, int *v2);
#endif
extern void vparms(int *vwin, float *inbuf, float *lpbuf, int half,
		   float *dither, int mintau, int *zc, int *lbe, int *fbe,
		   float *qs, float *rc1, float *ar_b, float *ar_f);

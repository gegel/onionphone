/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                         Version 2.1 of October 1999
*/
/*
 File : LD8CP.H
*/

/*--------------------------------------------------------------*
 * LD8CP.H                                                       *
 * ~~~~~~                                                       *
 *--------------------------------------------------------------*/

#include <stdint.h>

/*---------------------------------------------------------------------------*
 * constants for bitstream packing                                           *
 *---------------------------------------------------------------------------*/
#define PRM_SIZE_E_fwd  18	/* Size of vector of analysis parameters.    */
#define PRM_SIZE_E_bwd  16	/* Size of vector of analysis parameters.    */
#define PRM_SIZE_E      18	/* Size of vector of analysis parameters.    */
#define PRM_SIZE_D      10	/* Size of vector of analysis parameters.    */
#define PRM_SIZE_SID    4	/* Size of vector of analysis parameters.    */

#define SERIAL_SIZE_E (116+4)	/* Bits/frame + bfi+ number of speech bits
				   + bit for mode + protection */

#define RATE_6400       64	/* Low  rate  (6400 bit/s)       */
#define RATE_8000       80	/* Full rate  (8000 bit/s)       */
#define RATE_11800      118	/* High rate (11800 bit/s)       */
#define RATE_SID        15	/* SID                           */
#define RATE_0          0	/* 0 bit/s rate                  */

#define G729D           0	/* Low  rate  (6400 bit/s)       */
#define G729            1	/* Full rate  (8000 bit/s)       */
#define G729E           2	/* High rate (11800 bit/s)       */

/*--------------------------------------------------------------*
 * Function prototypes and constants use in G.729E              *
 *                                                              *
 *--------------------------------------------------------------*/

/* backward LPC analysis parameters */
#define M_BWD       30		/* Order of Backward LP filter.              */
#define M_BWDP1     (M_BWD+1)	/* Order of Backward LP filter + 1           */
#define NRP         35
#define MEM_SYN_BWD     (M_BWD + NRP)
#define N1              (M_BWD + L_FRAME)
#define L_ANA_BWD       (L_FRAME + MEM_SYN_BWD)
#define L_ANA_BWD_M1    (L_ANA_BWD - 1)
#define W_FACT  (float)0.31640625	/* 10368 */
#define GAMMA_BWD (float)0.98	/* 32113 */

/* short term pst parameters :                                              */
#define GAMMA1_PST_E  (float)0.7	/* denominator weighting factor */
#define GAMMA2_PST_E  (float)0.65	/* numerator  weighting factor */
#define LONG_H_ST_E   32	/* impulse response length                   */
#define GAMMA_HARM_E (float)0.25
#define GAMMA_HARM (float)0.5

/* ACELP codebooks parameters */
#define NB_TRACK  5
#define Q15_1_5   (float)0.2

/* Bw / Fw constants */
#define THRES_ENERGY (float)40.
#define TH1 (float)1.
#define TH2 (float)2.
#define TH3 (float)3.
#define TH4 (float)4.
#define TH5 (float)4.7
#define GAP_FACT (float)0.000114375
#define INV_LOG2 (float) (1./log10(2.))

/*--------------------------------------------------------------------------*
 *       6.4kbps                                                            *
 *--------------------------------------------------------------------------*/

#ifndef max
#define max(a, b)     ((a) > (b) ? (a) : (b))
#endif

#define BPS_8K        0		/* Indicates 8kbps mode                      */
#define BPS_6K        1		/* Indicates 6.4kbps mode                    */

#define SIZE_WORD_6K (short)64	/* number of speech bits                    */

#define PRM_SIZE_6K   10	/* Size of vector of analysis parameters.    */
#define SERIAL_SIZE_6K (64+2)	/* Bits/frame + bfi+ number of speech bits   */

#define NB_PULSES_6K  2		/* number of pulses */
#define NC1_B_6K    4		/* number of second stage bits higher        */
#define NC1_6K   (1<<NC1_B_6K)	/* number of entries in second stage (higher) */

#define NCODE1_B_6K  3		/* number of Codebook-bit                */
#define NCODE2_B_6K  3		/* number of Codebook-bit                */
#define NCODE1_6K (1<<NCODE1_B_6K)	/* Codebook 1 size                       */
#define NCODE2_6K (1<<NCODE2_B_6K)	/* Codebook 2 size                       */
#define NCAN1_6K  6		/* Pre-selecting order for #1            */
#define NCAN2_6K  6		/* Pre-selecting order for #2            */
#define INV_COEF_6K  ((float)-0.027599)

#define GAIN_PIT_MAX_6K (float)1.4	/* maximum adaptive codebook gain        */

/*--------------------------------------------------------------------------*
 *       VAD                                                                *
 *--------------------------------------------------------------------------*/
#define EPSI            (float)1.0e-38	/* very small positive floating point number      */
/*--------------------------------------------------------------------------*
 * Main coder and decoder functions                                         *
 *--------------------------------------------------------------------------*/
void init_coder_ld8c(int dtx_enable);
void coder_ld8c(int ana[],	/* output: analysis parameters */
		int frame,	/* input : frame counter */
		int dtx_enable,	/* input : VAD enable flag */
		int rate);

void init_decod_ld8c(void);
void decod_ld8c(int parm[],	/* (i)   : vector of synthesis parameters
				   parm[0] = bad frame indicator (bfi)    */
		int voicing,	/* (i)   : voicing decision from previous frame */
		float synth_buf[],	/* (i/o) : synthesis speech                     */
		float Az_dec[],	/* (o)   : decoded LP filter in 2 subframes     */
		int *t0_first,	/* (o)   : decoded pitch lag in first subframe  */
		int *bwd_dominant,	/* (o)   : bwd dominant indicator               */
		int *m_pst,	/* (o)   : LPC order for postfilter             */
		int *Vad	/* output: decoded frame type                         */
    );
/*--------------------------------------------------------------------------*
 * bitstream packing VQ functions.                                          *
 *--------------------------------------------------------------------------*/
void prm2bits_ld8c(int prm[], int16_t bits[]);
void bits2prm_ld8c(int16_t bits[], int prm[]);

/*--------------------------------------------------------------------------*
 * protypes of functions  similar to G729                                   *
 * differences :                                                            *
 * list of arguments modified                                               *
 * local static variables and arrays are now passed as parameters           *
 * LPC order formerly constant is now passed as variable parameter          *
 * some temporary variables are now passed to the calling routine           *
 *--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*
 * LPC analysis and filtering                                               *
 *--------------------------------------------------------------------------*/
float levinsone(int m,		/* (i)  : LPC order                         */
		float * r,	/* (i)  : r[m+1] autocorrelation coefficients */
		float * A,	/* (o)  : A[m]    LPC coefficients  (m = 10)         */
		float * rc,	/* (o)  : rc[M]   Reflection coefficients.           */
		float * old_A,	/* (i/o) : last stable filter LPC coefficients  */
		float * old_rc	/* (i/o) : last stable filter Reflection coefficients.         */
    );

void residue(int m,		/* (i)    : LPC order                         */
	     float a[],		/* (i)  : prediction coefficients                     */
	     float x[],		/* (i)     : speech (values x[-m..-1] are needed         */
	     float y[],		/* (o)     : residual signal                             */
	     int lg		/* (i)     : size of filtering                           */
    );
void syn_filte(int m,		/* (i)    : LPC order                         */
	       float a[],	/* (i)  : a[m+1] prediction coefficients   (m=10)  */
	       float x[],	/* (i)     : input signal                             */
	       float y[],	/* (o)     : output signal                            */
	       int lg,		/* (i)     : size of filtering                        */
	       float mem[],	/* (i/o)   : memory associated with this filtering.   */
	       int update	/* (i)     : 0=no update, 1=update of memory.         */
    );

/*--------------------------------------------------------------------------*
 * LSP VQ functions.                                                        *
 *--------------------------------------------------------------------------*/
void lsp_az(float * lsp, float * a);
void qua_lspe(float lsp[],	/* (i)  : Unquantized LSP                            */
	      float lsp_q[],	/* (o)  : Quantized LSP                              */
	      int ana[],	/* (o)     : indexes                                    */
	      float freq_prev[MA_NP][M],	/* (i)  : previous LSP MA vector        */
	      float freq_cur[]	/* (o)  : current LSP MA vector        */
    );
void lsp_encw_resete(float freq_prev[MA_NP][M]	/* (i)  : previous LSP MA vector        */
    );
void lsp_stability(float buf[]);
void lsp_prev_compose(float lsp_ele[], float lsp[], float fg[][M],
		      float freq_prev[][M], float fg_sum[]);
void lsp_qua_cse(float flsp_in[M],	/* (i)  : Original LSP parameters    */
		 float lspq_out[M],	/* (o)  : Quantized LSP parameters   */
		 int *code,	/* (o)     : codes of the selected LSP  */
		 float freq_prev[MA_NP][M],	/* (i)  : previous LSP MA vector        */
		 float freq_cur[]	/* (o)  : current LSP MA vector        */
    );
void lsp_get_quante(float lspcb1[][M],	/* (i)  : first stage LSP codebook      */
		    float lspcb2[][M],	/* (i)  : Second stage LSP codebook     */
		    int code0,	/* (i)     : selected code of first stage  */
		    int code1,	/* (i)     : selected code of second stage */
		    int code2,	/* (i)     : selected code of second stage */
		    float fg[][M],	/* (i)  : MA prediction coef.           */
		    float freq_prev[][M],	/* (i)  : previous LSP vector           */
		    float lspq[],	/* (o)  : quantized LSP parameters      */
		    float fg_sum[],	/* (i)  : present MA prediction coef.   */
		    float freq_cur[]	/* (i)  : present MA prediction coef.   */
    );
void relspwede(float lsp[],	/* (i)  : unquantized LSP parameters */
	       float wegt[],	/* (i) norm: weighting coefficients     */
	       float lspq[],	/* (o)  : quantized LSP parameters   */
	       float lspcb1[][M],	/* (i)  : first stage LSP codebook   */
	       float lspcb2[][M],	/* (i)  : Second stage LSP codebook  */
	       float fg[MODE][MA_NP][M],	/* (i)  : MA prediction coefficients */
	       float freq_prev[MA_NP][M],	/* (i)  : previous LSP vector        */
	       float fg_sum[MODE][M],	/* (i)  : present MA prediction coef. */
	       float fg_sum_inv[MODE][M],	/* (i)  : inverse coef.              */
	       int code_ana[],	/* (o)     : codes of the selected LSP  */
	       float freq_cur[]	/* (o)  : current LSP MA vector        */
    );
void get_wegt(float flsp[], float wegt[]);
void d_lspe(int prm[],		/* (i)     : indexes of the selected LSP */
	    float lsp_q[],	/* (o)  : Quantized LSP parameters    */
	    int erase,		/* (i)     : frame erase information     */
	    float freq_prev[MA_NP][M],	/* (i/o)  : previous LSP MA vector        */
	    float prev_lsp[],	/* (i/o)  : previous LSP vector        */
	    int *prev_ma	/* (i/o) previous MA prediction coef. */
    );
void lsp_decw_resete(float freq_prev[MA_NP][M],	/* (o)  : previous LSP MA vector        */
		     float prev_lsp[],	/* (o)  : previous LSP vector        */
		     int *prev_ma	/* previous MA prediction coef. */
    );
/*--------------------------------------------------------------------------*
 *       LTP prototypes                                                     *
 *--------------------------------------------------------------------------*/
int pitch_fr3cp(float exc[], float xn[], float h[], int l_subfr,
		int t0_min, int t0_max, int i_subfr, int *pit_frac, int rate);
int enc_lag3cp(int T0, int T0_frac, int *T0_min, int *T0_max, int pit_min,
	       int pit_max, int pit_flag, int rate);
void dec_lag3cp(int index, int pit_min, int pit_max, int i_subfr,
		int *T0, int *T0_frac, int rate);

/*--------------------------------------------------------------------------*
 * Postfilter functions                                                     *
 *--------------------------------------------------------------------------*/

void poste(int t0,		/* input : pitch delay given by coder */
	   float * signal_ptr,	/* input : input signal (pointer to current subframe */
	   float * coeff,	/* input : LPC coefficients for current subframe */
	   float * sig_out,	/* output: postfiltered output */
	   int *vo,		/* output: voicing decision 0 = uv,  > 0 delay */
	   float gamma1,	/* input: short term postfilt. den. weighting factor */
	   float gamma2,	/* input: short term postfilt. num. weighting factor */
	   float gamma_harm,	/* input: long term postfilter weighting factor */
	   int long_h_st,	/* input: impulse response length */
	   int m_pst,		/* input:  LPC order */
	   int Vad		/* input : VAD information (frame type)  */
    );

/*--------------------------------------------------------------------------*
 * protypes of functions  containing G729 source code + specific G729E code *
 *--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*
 * gain VQ functions.                                                       *
 *--------------------------------------------------------------------------*/
void dec_gaine(int index,	/* (i)    :Index of quantization.         */
	       float code[],	/* (i)  :Innovative vector.             */
	       int L_subfr,	/* (i)    :Subframe length.               */
	       int bfi,		/* (i)    :Bad frame indicator            */
	       float * gain_pit,	/* (o)  :Pitch gain.                    */
	       float * gain_cod,	/* (o) :Code gain.                     */
	       int rate,	/* input   : rate selector/frame  =0 8kbps,= 1 11.8 kbps */
	       float gain_pit_mem,
	       float gain_cod_mem,
	       float * c_muting, int count_bfi, int stationnary);

/*--------------------------------------------------------------------------*
 * functions  specific to G729E                                             *
 *--------------------------------------------------------------------------*/
/* backward LPC analysis and switch forward/backward */
void autocorr_hyb_window(float * x,	/* Input speech signal  */
			 float * r_bwd,	/* (out)    Autocorrelations    */
			 float * rexp	/* (in/out) */
    );
void glag_window_bwd(float * r_bwd);
void int_bwd(float * a_bwd, float * prev_filter, float * C_int);

void set_lpc_mode(float * signal_ptr, float * a_fwd, float * a_bwd,
		  int *mode, float * lsp_new, float * lsp_old,
		  int *bwd_dominant, int prev_mode, float * prev_filter,
		  float * C_int, int16_t * glob_stat, int16_t * stat_bwd,
		  int16_t * val_stat_bwd);
void update_bwd(int *mode,	/* O  Backward / forward Indication */
		int *bwd_dominant,	/* O   Bwd dominant mode indication */
		float * C_int,	/*I/O filter interpolation parameter */
		int16_t * glob_stat	/* I/O Mre of global stationnarity */
    );
float ener_dB(float * synth, int L);
void tst_bwd_dominant(int *high_stat, int mode);

void perc_vare(float * gamma1,	/* Bandwidth expansion parameter */
	       float * gamma2,	/* Bandwidth expansion parameter */
	       int high_stat	/* high_stat indication (see file bwfw.c) */
    );

/*--------------------------------------------------------------------------*
 * G729E fixed (ACELP) codebook excitation.                                               *
 *--------------------------------------------------------------------------*/

void ACELP_12i40_44bits(float x[],	/* (i)  : target vector                                 */
			float cn[],	/* (i)  : residual after long term prediction           */
			float H[],	/* (i) : impulse response of weighted synthesis filter */
			float code[],	/* (o) : algebraic (fixed) codebook excitation         */
			float y[],	/* (o) : filtered fixed codebook excitation            */
			int indx[]	/* (o)    : index 5 words: 13,10,7,7,7 = 44 bits          */
    );
void ACELP_10i40_35bits(float x[],	/* (i)  : target vector                                 */
			float cn[],	/* (i)  : residual after long term prediction           */
			float H[],	/* (i) : impulse response of weighted synthesis filter */
			float code[],	/* (o) : algebraic (fixed) codebook excitation         */
			float y[],	/* (o) : filtered fixed codebook excitation            */
			int indx[]	/* (o)    : index 5 words: 7,7,7,7,7 = 35 bits            */
    );
void dec_ACELP_12i40_44bits(int *index,	/* (i)     : 5 words index (positions & sign)      */
			    float cod[]	/* (o)  : algebraic (fixed) codebook excitation */
    );
void dec_ACELP_10i40_35bits(int *index,	/* (i)     : 5 words index (positions & sign)      */
			    float cod[]	/* (o)  : algebraic (fixed) codebook excitation */
    );
/* pitch tracking routine */
void track_pit(int *T0, int *T0_frac, int *prev_pitch,
	       int *stat_pitch, int *pitch_sta, int *frac_sta);
/*--------------------------------------------------------------------------*
 * G729D fixed (ACELP) codebook excitation.                                               *
 *--------------------------------------------------------------------------*/
int ACELP_codebook64(		/* (o)     :Index of pulses positions    */
			    float x[],	/* (i)     :Target vector                */
			    float h[],	/* (i)     :Impulse response of filters  */
			    int t0,	/* (i)     :Pitch lag                    */
			    float pitch_sharp,	/* (i)     :Last quantized pitch gain    */
			    float code[],	/* (o)     :Innovative codebook          */
			    float y[],	/* (o)     :Filtered innovative codebook */
			    int *sign	/* (o)     :Signs of 4 pulses            */
    );
void decod_ACELP64(int sign,	/* input : signs of 2 pulses     */
		   int index,	/* input : positions of 2 pulses */
		   float cod[]	/* output: innovative codevector */
    );
/*--------------------------------------------------------------------------*
 * G729D gain                                                               *
 *--------------------------------------------------------------------------*/
int qua_gain_6k(		/* output: quantizer index                   */
		       float code[],	/* input : fixed codebook vector             */
		       float * g_coeff,	/* input : correlation factors               */
		       int l_subfr,	/* input : fcb vector length                 */
		       float * gain_pit,	/* output: quantized acb gain                */
		       float * gain_code,	/* output: quantized fcb gain                */
		       int tameflag	/* input : flag set to 1 if taming is needed */
    );
void dec_gain_6k(int index,	/* input : quantizer index              */
		 float code[],	/* input : fixed code book vector       */
		 int l_subfr,	/* input : subframe size                */
		 int bfi,	/* input : bad frame indicator good = 0 */
		 float * gain_pit,	/* output: quantized acb gain           */
		 float * gain_code	/* output: quantized fcb gain           */
    );
/*--------------------------------------------------------------------------*
 * G729D gain  phase dispersion                                             *
 *--------------------------------------------------------------------------*/
void Update_PhDisp(float ltpGain,	/* (i)  : pitch gain                  */
		   float cbGain	/* (i)  : codebook gain               */
    );
void PhDisp(float x[],		/* input : excitation signal                */
	    float x_phdisp[],	/* output : excitation signal after phase dispersion */
	    float cbGain, float ltpGainQ, float inno[]
    );

/*--------------------------------------------------------------------------*
 * Prototypes for auxiliary functions                                       *
 *--------------------------------------------------------------------------*/
int16_t random_g729c(int16_t * seed);
void dvsub(float * in1, float * in2, float * out, int16_t npts);
float dvdot(float * in1, float * in2, int16_t npts);
void dvwadd(float * in1, float scalar1, float * in2, float scalar2,
	    float * out, int16_t npts);
void dvsmul(float * in, float scalar, float * out, int16_t npts);

void musdetect(int rate, float Energy, float * rc, int *lags, float * pgains,
	       int stat_flg, int frm_count, int prev_vad, int *Vad,
	       float Energy_db);

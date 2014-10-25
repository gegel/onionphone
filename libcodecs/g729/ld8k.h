/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C - Reference C code for floating point
                         implementation of G.729
                         Version 1.01 of 15.September.98
*/

/*
----------------------------------------------------------------------
                    COPYRIGHT NOTICE
----------------------------------------------------------------------
   ITU-T G.729 Annex C ANSI C source code
   Copyright (C) 1998, AT&T, France Telecom, NTT, University of
   Sherbrooke.  All rights reserved.

----------------------------------------------------------------------
*/

/*
 File : LD8K.H
 Used for the floating point version of G.729 main body
 (not for G.729A)
*/

/*---------------------------------------------------------------------------
 * ld8k.h - include file for all ITU-T 8 kb/s CELP coder routines
 *---------------------------------------------------------------------------
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef PI
#undef PI
#endif
#ifdef PI2
#undef PI2
#endif
#define PI              (float)3.14159265358979323846
#define PI2             (float)6.283185307
#define FLT_MAX_G729         (float)1.e38	/* largest floating point number             */
#define FLT_MIN_G729         -FLT_MAX_G729	/* largest floating point number             */

#define L_TOTAL         240	/* Total size of speech buffer               */
#define L_FRAME         80	/* LPC update frame size                     */
#define L_SUBFR         40	/* Sub-frame size                            */

/*---------------------------------------------------------------------------*
 * constants for bitstream packing                                           *
 *---------------------------------------------------------------------------*/
#define BIT_1     (int16_t)0x0081	/* definition of one-bit in bit-stream      */
#define BIT_0     (int16_t)0x007f	/* definition of zero-bit in bit-stream      */
#define SYNC_WORD (int16_t)0x6b21	/* definition of frame erasure flag          */
#define SIZE_WORD       80	/* size of bitstream frame */
#define PRM_SIZE        11	/* number of parameters per 10 ms frame      */
#define SERIAL_SIZE     82	/* bits per frame                            */

/*---------------------------------------------------------------------------*
 * constants for lpc analysis and lsp quantizer                              *
 *---------------------------------------------------------------------------*/
#define L_WINDOW        240	/* LPC analysis window size                  */
#define L_NEXT          40	/* Samples of next frame needed for LPC ana. */

#define M               10	/* LPC order                                 */
#define MP1            (M+1)	/* LPC order+1                               */
#define GRID_POINTS     60	/* resolution of lsp search                  */

#define MA_NP           4	/* MA prediction order for LSP               */
#define MODE            2	/* number of modes for MA prediction         */
#define NC0_B           7	/* number of bits in first stage             */
#define NC0          (1<<NC0_B)	/* number of entries in first stage          */
#define NC1_B           5	/* number of bits in second stage            */
#define NC1          (1<<NC1_B)	/* number of entries in second stage         */
#define NC              (M/2)	/* LPC order / 2                            */

#define L_LIMIT         (float)0.005	/*  */
#define M_LIMIT         (float)3.135	/*  */
#define GAP1            (float)0.0012	/*  */
#define GAP2            (float)0.0006	/*  */
#define GAP3            (float)0.0392	/*  */
#define PI04            PI*(float)0.04	/* pi*0.04 */
#define PI92            PI*(float)0.92	/* pi*0.92 */
#define CONST12         (float)1.2

/*-------------------------------------------------------------------------
 *  pwf constants
 *-------------------------------------------------------------------------
 */

#define THRESH_L1   (float)-1.74
#define THRESH_L2   (float)-1.52
#define THRESH_H1   (float)0.65
#define THRESH_H2   (float)0.43
#define GAMMA1_0    (float)0.98
#define GAMMA2_0_H  (float)0.7
#define GAMMA2_0_L  (float)0.4
#define GAMMA1_1    (float)0.94
#define GAMMA2_1    (float)0.6
#define ALPHA       (float)-6.0
#define BETA        (float)1.0

/*----------------------------------------------------------------------------
 *  Constants for long-term predictor
 *----------------------------------------------------------------------------
 */
#define PIT_MIN         20	/* Minimum pitch lag in samples              */
#define PIT_MAX         143	/* Maximum pitch lag in samples              */
#define L_INTERPOL      (10+1)	/* Length of filter for interpolation.       */
#define L_INTER10       10	/* Length for pitch interpolation            */
#define L_INTER4        4	/* upsampling ration for pitch search        */
#define UP_SAMP         3	/* resolution of fractional delays           */
#define THRESHPIT    (float)0.85	/* Threshold to favor smaller pitch lags     */
#define GAIN_PIT_MAX (float)1.2	/* maximum adaptive codebook gain            */
#define FIR_SIZE_ANA (UP_SAMP*L_INTER4+1)
#define FIR_SIZE_SYN (UP_SAMP*L_INTER10+1)

/*---------------------------------------------------------------------------*
 * constants for fixed codebook                                              *
 *---------------------------------------------------------------------------*/
#define DIM_RR  616		/* size of correlation matrix                            */
#define NB_POS  8		/* Number of positions for each pulse                    */
#define STEP    5		/* Step betweem position of the same pulse.              */
#define MSIZE   64		/* Size of vectors for cross-correlation between 2 pulses */

#define SHARPMAX        (float)0.7945	/* Maximum value of pitch sharpening */
#define SHARPMIN        (float)0.2	/* minimum value of pitch sharpening */

 /*--------------------------------------------------------------------------*
  * Example values for threshold and approximated worst case complexity:     *
  *                                                                          *
  *     threshold=0.40   maxtime= 75   extra=30   Mips =  6.0                *
  *--------------------------------------------------------------------------*/
#define THRESHFCB       (float)0.40	/*  */
#define MAX_TIME        75	/*  */

/*--------------------------------------------------------------------------*
 * Constants for taming procedure.                           *
 *--------------------------------------------------------------------------*/
#define GPCLIP      (float)0.95	/* Maximum pitch gain if taming is needed */
#define GPCLIP2     (float)0.94	/* Maximum pitch gain if taming is needed */
#define GP0999      (float)0.9999	/* Maximum pitch gain if taming is needed    */
#define THRESH_ERR  (float)60000.	/* Error threshold taming    */
#define INV_L_SUBFR (float) ((float)1./(float)L_SUBFR)	/* =0.025 */
/*-------------------------------------------------------------------------
 *  gain quantizer  constants
 *-------------------------------------------------------------------------
 */
#define MEAN_ENER        (float)36.0	/* average innovation energy */
#define NCODE1_B  3		/* number of Codebook-bit                */
#define NCODE2_B  4		/* number of Codebook-bit                */
#define NCODE1    (1<<NCODE1_B)	/* Codebook 1 size                       */
#define NCODE2    (1<<NCODE2_B)	/* Codebook 2 size                       */
#define NCAN1            4	/* Pre-selecting order for #1 */
#define NCAN2            8	/* Pre-selecting order for #2 */
#define INV_COEF   (float)-0.032623

/*---------------------------------------------------------------------------
 * Constants for postfilter
 *---------------------------------------------------------------------------
 */
	 /* int16_t term pst parameters :  */
#define GAMMA1_PST      (float)0.7	/* denominator weighting factor           */
#define GAMMA2_PST      (float)0.55	/* numerator  weighting factor            */
#define LONG_H_ST       20	/* impulse response length                   */
#define GAMMA3_PLUS     (float)0.2	/* tilt weighting factor when k1>0        */
#define GAMMA3_MINUS    (float)0.9	/* tilt weighting factor when k1<0        */

/* long term pst parameters :   */
#define L_SUBFRP1 (L_SUBFR + 1)	/* Sub-frame size + 1                        */
#define F_UP_PST        8	/* resolution for fractionnal delay          */
#define LH2_S           4	/* length of int16_t interp. subfilters        */
#define LH2_L           16	/* length of long interp. subfilters         */
#define THRESCRIT       (float)0.5	/* threshold LT pst switch off            */
#define GAMMA_G         (float)0.5	/* LT weighting factor                    */
#define AGC_FAC         (float)0.9875	/* gain adjustment factor                 */

#define AGC_FAC1         ((float)1. - AGC_FAC)	/* gain adjustment factor                 */
#define LH_UP_S         (LH2_S/2)
#define LH_UP_L         (LH2_L/2)
#define LH2_L_P1    (LH2_L + 1)
#define MIN_GPLT    ((float)1. / ((float)1. + GAMMA_G))	/* LT gain minimum          */

/* Array sizes */
#define MEM_RES2 (PIT_MAX + 1 + LH_UP_L)
#define SIZ_RES2 (MEM_RES2 + L_SUBFR)
#define SIZ_Y_UP  ((F_UP_PST-1) * L_SUBFRP1)
#define SIZ_TAB_HUP_L ((F_UP_PST-1) * LH2_L)
#define SIZ_TAB_HUP_S ((F_UP_PST-1) * LH2_S)

/*--------------------------------------------------------------------------*
 * Main coder and decoder functions                                         *
 *--------------------------------------------------------------------------*/
void init_coder_ld8k(void);
void coder_ld8k(int *);

void init_decod_ld8k(void);
void decod_ld8k(int parm[], int voi, float synth[], float Az_dec[], int *t0);

/*--------------------------------------------------------------------------*
 * Pre and post-process functions                                           *
 *--------------------------------------------------------------------------*/
void init_pre_process(void);
void pre_process(float signal[], int lg);

void init_post_process(void);
void post_process(float signal[], int lg);

/*--------------------------------------------------------------------------*
 * LPC analysis and filtering                                               *
 *--------------------------------------------------------------------------*/
void autocorr(float * x, int m, float * r);
void glag_window(int m, float r[]);
float levinson(float * r, float * a, float * r_c);
void az_lsp(float a[], float lsp[], float old_lsp[]);
void qua_lsp(float lsp[], float lsp_q[], int ana[]);

void lsf_lsp(float * lsf, float * lsp, int m);
void lsp_lsf(float * lsp, float * lsf, int m);
void int_lpc(float lsp_old[], float lsp_new[], float lsf_int[],
	     float lsf_new[], float A_t[]);
void int_qlpc(float lsp_old[], float lsp_new[], float Az[]);

/*--------------------------------------------------------------------------*
 * Prototypes of LSP VQ functions                                           *
 *--------------------------------------------------------------------------*/
void lsp_expand_1(float buf[], float c);
void lsp_expand_2(float buf[], float c);
void lsp_expand_1_2(float buf[], float c);
void lsp_decw_reset(void);
void lsp_encw_reset(void);
void lsp_prev_update(float lsp_ele[M], float freq_prev[MA_NP][M]);
void lsp_prev_extract(float lsp[M], float lsp_ele[M], float fg[MA_NP][M],
		      float freq_prev[MA_NP][M], float fg_sum_inv[M]);
void lsp_get_quant(float lspcb1[][M], float lspcb2[][M], int code0,
		   int code1, int code2, float fg[][M], float freq_prev1[][M],
		   float lspq[], float fg_sum[]);
void d_lsp(int index[], float lsp_new[], int bfi);

/*--------------------------------------------------------------------------*
 *       PWF prototypes                                                     *
 *--------------------------------------------------------------------------*/
void perc_var(float * gamma1, float * gamma2, float * lsfint, float * lsfnew,
	      float * r_c);
void weight_az(float * a, float gamma, int m, float * ap);

/*-------------------------------------------------------------------------
 * Prototypes of general signal processing routines.
 *-------------------------------------------------------------------------
 */
void convolve(float x[], float h[], float y[], int L);
void residu(float * a, float * x, float * y, int l);
void syn_filt(float a[], float x[], float y[],
	      int l, float mem[], int update_m);

/*--------------------------------------------------------------------------*
 *       LTP prototypes                                                     *
 *--------------------------------------------------------------------------*/
int pitch_fr3(float exc[], float xn[], float h[], int l_subfr,
	      int t0_min, int t0_max, int i_subfr, int *pit_frac);
int pitch_ol(float signal[], int pit_min, int pit_max, int L_frame);
int enc_lag3(int T0, int T0_frac, int *T0_min, int *T0_max, int pit_min,
	     int pit_max, int pit_flag);
void dec_lag3(int index, int pit_min, int pit_max, int i_subfr,
	      int *T0, int *T0_frac);
void pred_lt_3(float exc[], int t0, int frac, int L);
int parity_pitch(int pitch_i);
int check_parity_pitch(int pitch_i, int parity);
float g_pitch(float xn[], float y1[], float g_coeff[], int L_subfr);

/*--------------------------------------------------------------------------*
 * fixed codebook excitation.                                               *
 *--------------------------------------------------------------------------*/
void cor_h_x(float h[], float X[], float D[]);
int ACELP_codebook(float x[], float h[], int T0, float pitch_sharp,
		   int i_subfr, float code[], float y[], int *sign);
void decod_ACELP(int signs, int positions, float cod[]);

/*--------------------------------------------------------------------------*
 * gain VQ functions.                                                       *
 *--------------------------------------------------------------------------*/
int qua_gain(float code[], float * coeff, int lcode, float * gain_pit,
	     float * gain_code, int taming);
void dec_gain(int indice, float code[], int lcode, int bfi,
	      float * gain_pit, float * gain_code);
void gain_predict(float past_qua_en[], float code[], int l_subfr,
		  float * gcode0);
void gain_update(float past_qua_en[], float g_code);
void gain_update_erasure(float * past_qua_en);
void corr_xy2(float xn[], float y1[], float y2[], float g_coeff[]);

/*--------------------------------------------------------------------------*
 * bitstream packing VQ functions.                                          *
 *--------------------------------------------------------------------------*/
void prm2bits_ld8k(int prm[], int16_t bits[]);
void bits2prm_ld8k(int16_t bits[], int prm[]);

/*--------------------------------------------------------------------------*
 * postfilter  functions.                                                   *
 *--------------------------------------------------------------------------*/
void init_post_filter(void);
void post(int t0, float * syn, float * a_t, float * pst, int *sf_voic);

/*------------------------------------------------------------*
 * prototypes for taming procedure.                           *
 *------------------------------------------------------------*/
void init_exc_err(void);
void update_exc_err(float gain_pit, int t0);
int test_err(int t0, int t0_frac);

/*--------------------------------------------------------------------------*
 * Prototypes for auxiliary functions                                       *
 *--------------------------------------------------------------------------*/
void fwrite16(float * data, int length, FILE * fp);
int16_t random_g729(void);
void set_zero(float x[], int l);
void copy(float x[], float y[], int L);

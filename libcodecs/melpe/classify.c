/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* ================================================================== */
/*                                                                    */
/*    Microsoft Speech coder     ANSI-C Source Code                   */
/*    SC1200 1200 bps speech coder                                    */
/*    Fixed Point Implementation      Version 7.0                     */
/*    Copyright (C) 2000, Microsoft Corp.                             */
/*    All rights reserved.                                            */
/*                                                                    */
/* ================================================================== */

/*------------------------------------------------------------------*/
/*																	*/
/* File:		classify.c											*/
/*																	*/
/* Description: classification routines								*/
/*																	*/
/*------------------------------------------------------------------*/

#include "sc1200.h"
#include "cprv.h"
#include "global.h"
#include "mat_lib.h"
#include "macro.h"
#include "mathhalf.h"
#include "constant.h"
#include "math_lib.h"
#include "dsp_sub.h"
#include "coeff.h"
#include "melp_sub.h"

#define PITCH_RANGE				5
#define SQRT_PIT_SUBFRAME_Q11	19429	/* sqrt(PIT_SUBFRAME) * (1 << 11) */
#define SILENCE_DB_Q11			6144	/* Originally 30.  subEnergy is */
				     /* changed to 1/10 of its floating point */
					  /* counterpart.  30/10 * (1 << 11). */
#define TWO_Q11					4096	/* 2 * (1 << 11) */
#define THREE_Q11				6144	/* 3 * (1 << 11) */
#define FIVE_Q11				10240	/* 5 * (1 << 11) */
#define M_TEN_Q11				-20480
#define X04_Q15					13107	/* 0.4 * (1 << 15) */
#define X045_Q15				14746	/* 0.45 * (1 << 15) */
#define X05_Q12					2048	/* 0.5 * (1 << 12) */
#define X055_Q15				18022	/* 0.55 * (1 << 15) */
#define X06_Q15					19661	/* 0.6 * (1 << 15) */
#define X065_Q15				21299	/* 0.65 * (1 << 15) */
#define X14_Q11					2867	/* 1.4 * (1 << 15) */
#define X15_Q11					3072	/* 1.5 * (1 << 11) */
#define X16_Q11					3277	/* 1.6 * (1 << 11) */
#define LOG2_Q11				617	/* log10(2.0) * (1 << 11) */
#define ONE_OV_SQRT2_Q15		23170	/* 1/sqrt(2) * (1 << 15) */

const int16_t enlpf_coef[EN_FILTER_ORDER] = {	/* Q14 */
	/* the coefs of the filter (NOT h) */
	6764, 4336, -274, -2536, -1491,
	24, -228, -1370, -1502, -480,
	383, 390, 57, -18, 104,
	132, 51
};

const int16_t enhpf_coef[EN_FILTER_ORDER] = {	/* Q14 */
	/* the coefs of the filter (NOT h) */
	7783, -5211, 439, 1707, -483,
	-978, 564, 630, -861, 214,
	205, -86, -82, 43, 26,
	-18, 2
};

/* ========== Prototypes ========== */

static int16_t zeroCrosCount(int16_t speech[]);
static int16_t bandEn(int16_t autocorr[], int16_t band);
static void frac_cor(int16_t inbuf[], int16_t pitch, int16_t * cor);

/****************************************************************************
**
** Function:		classify()
**
** Description: 	classification routine
**
** Arguments:
**
**	int16_t inbuf[]		speech buffer (Q0)
**	classParam *classStat	classification structure
**  int16_t autocorr[]	autocorrelation coefficients (normalized, Q value
**                          irrelevant)
**
** Return value:	None
**
*****************************************************************************/
void classify(int16_t inbuf[], classParam * classStat, int16_t autocorr[])
{
	register int16_t i, j;
	static BOOLEAN firstTime = TRUE;
	static int16_t bpfdel[BPF_ORD + BPF_ORD / 3];
	static int16_t back_sigbuf[PIT_COR_LEN - PIT_SUBFRAME];
	const int16_t *ptr_bpf_num, *ptr_bpf_den;
	int16_t classy, sum1_shift = 0;
	int32_t L_sum1, L_sum2;	/* Q0 */
	int16_t temp1, temp2, max;
	int32_t L_temp;
	int16_t sigbuf_a[BPF_ORD / 3 + PIT_COR_LEN];
	int16_t sigbuf_b[BPF_ORD / 3 + PIT_COR_LEN];
	int16_t *sigbuf_in, *sigbuf_out, *temp_sigbuf;
	int16_t sigbuf_len;
	int16_t inspeech[PIT_SUBFRAME];	/* Normalized buffer for inbuf[]. */
	int16_t lowhighBandDiff;	/* Q12 */
	int16_t zeroCrosRateDiff;	/* Q15 */
	int16_t subEnergyDiff;	/* Q11 */
	int16_t lowBandCorx;	/* low band pcor moved from classParam, Q15 */

	if (firstTime) {
		voicedEn = FIVE_Q11;	/* Both voicedEn and silenceEn are 1/10 of */
		silenceEn = THREE_Q11;	/* their values in floating point version. */
		voicedCnt = 0;
		v_zap(bpfdel, BPF_ORD + BPF_ORD / 3);
	}

	/* The following inline implementation of iir_2nd_s() is for the          */
	/* complexity reduction.  The memory bpfdel[] plays both the roles for    */
	/* bpf_delin[] and bpf_delout[] in the past as the output memory for      */
	/* stage i (bpf_delout[2*i]@2) is the input memory for stage (i+1)        */
	/* (bpf_delin[2*(i+1)]@2).                                                */

	if (firstTime) {

		/* The first time we run the 3-stage filters, we have to compute the  */
		/* whole sequence of length PIT_COR_LEN (220).  After that, every     */
		/* time we only need to compute PIT_SUBFRAME (90) new ones and        */
		/* combine it with the result shifted from the previous run           */
		/* (back_sigbuf[]).                                                   */

		sigbuf_in = sigbuf_a;
		sigbuf_out = sigbuf_b;
		sigbuf_len = PIT_COR_LEN;

		v_equ(&(sigbuf_in[BPF_ORD / 3]),
		      &(inbuf[(PIT_SUBFRAME - PIT_COR_LEN) / 2]), sigbuf_len);
	} else {
		sigbuf_in = sigbuf_a + PIT_COR_LEN - PIT_SUBFRAME;
		sigbuf_out = sigbuf_b + PIT_COR_LEN - PIT_SUBFRAME;
		sigbuf_len = PIT_SUBFRAME;

		/* Whether back_sigbuf is assigned to sigbuf_in or sigbuf_out depends */
		/* on the oddity of BPF_ORD/2.                                        */

		v_equ(&(sigbuf_in[BPF_ORD / 3]),
		      &(inbuf[(PIT_COR_LEN - PIT_SUBFRAME) / 2]), sigbuf_len);
		v_equ(&(sigbuf_out[BPF_ORD / 3 - PIT_COR_LEN + PIT_SUBFRAME]),
		      back_sigbuf, PIT_COR_LEN - PIT_SUBFRAME);
	}

	ptr_bpf_num = bpf_num;	/* It will point to bpf_num[3*i] */
	ptr_bpf_den = bpf_den + 1;	/* and bpf_den[3*i+1]. */
	for (i = 0; i < BPF_ORD / 2; i++) {
		/*      iir_2nd_s(&(sigbuf[BPF_ORD/3]), &bpf_den[i*3], &bpf_num[i*3],
		   &(sigbuf[BPF_ORD/3]), &bpf_delin[i*2], &bpf_delout[i*2],
		   sigbuf_len); */
		v_equ(sigbuf_in, &(bpfdel[2 * i]), BPF_ORD / 3);
		v_equ(sigbuf_out, &(bpfdel[2 * i + 2]), BPF_ORD / 3);
		for (j = BPF_ORD / 3; j < melpe_add(sigbuf_len, BPF_ORD / 3); j++) {
			L_temp = melpe_L_mult(sigbuf_in[j], ptr_bpf_num[0]);
			L_temp =
			    melpe_L_mac(L_temp, sigbuf_in[j - 1], ptr_bpf_num[1]);
			L_temp =
			    melpe_L_mac(L_temp, sigbuf_in[j - 2], ptr_bpf_num[2]);

			L_temp =
			    melpe_L_mac(L_temp, sigbuf_out[j - 1], ptr_bpf_den[0]);
			L_temp =
			    melpe_L_mac(L_temp, sigbuf_out[j - 2], ptr_bpf_den[1]);

			L_temp = melpe_L_shl(L_temp, 2);

			sigbuf_out[j] = melpe_r_ound(L_temp);
		}
		v_equ(&(bpfdel[2 * i]), &(sigbuf_in[sigbuf_len]), BPF_ORD / 3);

		temp_sigbuf = sigbuf_in;
		sigbuf_in = sigbuf_out;
		sigbuf_out = temp_sigbuf;

		ptr_bpf_num += 3;
		ptr_bpf_den += 3;
	}

	if (firstTime) {
		firstTime = FALSE;
		sigbuf_out = sigbuf_in;	/* The loop is over and we switch it back. */
	} else
		sigbuf_out = sigbuf_in - (PIT_COR_LEN - PIT_SUBFRAME);

	v_equ(&(bpfdel[BPF_ORD]), &(sigbuf_out[PIT_COR_LEN]), BPF_ORD / 3);

	v_equ(back_sigbuf, &(sigbuf_out[BPF_ORD / 3 + PIT_SUBFRAME]),
	      PIT_COR_LEN - PIT_SUBFRAME);

	/* Compute subframe energy.  Note that L_sum1 can have a large dynamic    */
	/* range as sum1 can be as small as 0 and as large as                     */
	/* PIT_SUBFRAME * 32767^2 (in theory, though in reality almost            */
	/* impossible).  Because L_v_magsq() implements by adding all the         */
	/* int32_t partial products before adjusting the Q value, we need to     */
	/* avoid overflow this temporary sum, which means making sure the input   */
	/* for L_v_magsq() is less than sqrt(LW_MAX/PIT_SUBFRAME) ~ 4884.  If we  */
	/* assume a single-frequency pure sinusoidal inbuf[], the limit is raised */
	/* to sqrt(2*LW_MAX/PIT_SUBFRAME) ~ 6908.  The input of classify() is     */
	/* basically raw speech (probably processed by NPP).                      */

	/*      sum1 = v_inner(inbuf, inbuf, PIT_SUBFRAME); */
	/* ------ Compute peakiness measure ------ */

	max = 0;
	L_sum2 = 0;
	for (i = 0; i < PIT_SUBFRAME; i++) {
		temp1 = melpe_abs_s(inbuf[i]);
		if (max < temp1)
			max = temp1;
		L_sum2 = melpe_L_add(L_sum2, temp1);	/* L_sum2 is safe from overflow. */
	}

	if (max == 0) {		/* All zero signal */
		L_sum1 = 0;
	} else if (max <= 4884) {	/* Partial sum of L_v_magsq() not overflowing */
		L_sum1 = L_v_magsq(inbuf, PIT_SUBFRAME, 0, 0);
		sum1_shift = 0;
	} else {
		v_equ_shr(inspeech, inbuf, 3, PIT_SUBFRAME);	/* (inbuf[] >> 3) < 4884 */
		L_sum1 = L_v_magsq(inspeech, PIT_SUBFRAME, 0, 0);
		sum1_shift = 6;
	}

	/* Adjust L_sum1 so it does not overflow a int16_t. */
	while (L_sum1 > SW_MAX) {
		L_sum1 = melpe_L_shr(L_sum1, 2);
		sum1_shift = melpe_add(sum1_shift, 2);
	}

	/* Note that classStat->subEnergy is only 1/10 of its floating point      */
	/* counterpart.                                                           */

	if (L_sum1 == 0)
		classStat->subEnergy = M_TEN_Q11;
	else {
		/*      classStat->subEnergy = log10(L_sum1); */
		temp1 = log10_fxp(melpe_extract_l(L_sum1), 0);	/* Q12 */
		temp1 = melpe_shr(temp1, 1);	/* Q11 */
		L_temp = melpe_L_mult(LOG2_Q11, sum1_shift);	/* Q12 */
		temp2 = melpe_extract_l(melpe_L_shr(L_temp, 1));	/* Q11 */
		classStat->subEnergy = melpe_add(temp1, temp2);	/* Q11 */
	}

	/* ------ Compute zero crossing rate ------ */
	classStat->zeroCrosRate = zeroCrosCount(inbuf);

	/* If L_sum2 == 0, L_sum1 == 0 too and we can set classStat->peakiness to */
	/* 1.  We later compare it against 1.4, 1.5, 1.6 and 2.0, so 0 and 1      */
	/* makes no differences.                                                  */

	/* Now L_sum1*pow(2.0, sum1_shift) and L_sum2 are the square of the       */
	/* 2-norm and the 1-norm of inbuf[], respectively.  It can be proved that */
	/* the 2-norm is always no larger than the 1-norm and we can call         */
	/* divide_s() to find their quotient.  That is,                           */
	/*     || x ||_2  <=  || x ||_1  <=  sqrt(n)*|| x ||_2.                   */
	/* See "Matrix Computation" by Gene H. Golub and Charles F. van Loan.     */
	/* The peakiness is therefore between 1 and sqrt(PIT_SUBFRAME).           */

	if (L_sum2 == 0)
		classStat->peakiness = ONE_Q11;
	else {
		/*      classStat->peakiness =
		   sqrt(L_sum1/PIT_SUBFRAME)/(L_sum2/PIT_SUBFRAME); */
		sum1_shift = melpe_add(sum1_shift, 15);	/* L_sum1 Q0 -> Q15 */
		if (sum1_shift & 0x0001) {	/* sum1_shift is odd */
			temp1 = melpe_extract_l(melpe_L_shr(L_sum1, 1));	/* Q15 */
			sum1_shift = melpe_add(sum1_shift, 1);
		} else		/* sum1_shift is even */
			temp1 = melpe_extract_l(L_sum1);	/* Q15 */
		sum1_shift = melpe_shr(sum1_shift, 1);
		temp1 = sqrt_Q15(temp1);	/* Q15 */

		/* (temp1 * 2^-15 * 2^sum1_shift) is || x ||_2 above, so              */
		/* L_shr(L_sum2, sum1_shift) is (|| x ||_1 / || x ||_2) * temp1 *     */
		/* 2^-15, guaranteed to fit in a int16_t.  However, the above       */
		/* result is upper-bounded by sqrt(PIT_SUBFRAME) and a Q0 could mean  */
		/* significant loss of precision.  We use Q8 for it and therefore we  */
		/* adjust sum1_shift beforehand.                                      */

		sum1_shift = melpe_sub(sum1_shift, 8);
		temp2 = melpe_extract_l(melpe_L_shr(L_sum2, sum1_shift));	/* Q8 */
		temp1 = melpe_shr(temp1, 7);	/* Q8 */
		temp1 = melpe_divide_s(temp1, temp2);	/* Q15 */

		/* The computation for classStat->peakiness is not accurate when the  */
		/* signal in the current frame is small.                              */

		classStat->peakiness = melpe_mult(SQRT_PIT_SUBFRAME_Q11, temp1);	/* Q11 */
	}

	/* ------ Compute band energy ------ */
	/* bandEn(autocorr, 0) is lowBandEn and bandEn(autocorr, 1) is highBandEn */
	lowhighBandDiff = melpe_sub(bandEn(autocorr, 0), bandEn(autocorr, 1));	/* Q12 */

	frac_cor(&(sigbuf_out[BPF_ORD / 3]), classStat->pitch, &lowBandCorx);

	if (silenceEn > melpe_sub(voicedEn, X15_Q11))
		silenceEn = melpe_sub(voicedEn, X15_Q11);
	/* the noise level is way too high */
	zeroCrosRateDiff = melpe_sub(classStat->zeroCrosRate, classStat[-1].zeroCrosRate);	/* Q15 */
	subEnergyDiff = melpe_sub(classStat->subEnergy, classStat[-1].subEnergy);
	/* Q11 */

	/* ========== Classifier ========== */
	if (classStat->subEnergy < SILENCE_DB_Q11)
		classy = SILENCE;
	else if (classStat->subEnergy <
		 interp_scalar(voicedEn, silenceEn, X065_Q15)) {
		/* quite possible it is silence or UNVOICED */
		/* unless the noise level is very high          */
		if ((classStat->zeroCrosRate > X06_Q15) &&
		    ((classStat->corx < X04_Q15) || (lowBandCorx < X05_Q15)))
			classy = UNVOICED;
		else if ((lowBandCorx > X07_Q15) || ((lowBandCorx > X04_Q15) &&
						     (classStat->corx >
						      X07_Q15)))
			classy = VOICED;
		else if ((zeroCrosRateDiff > X03_Q15)
			 || (subEnergyDiff > TWO_Q11)
			 || (classStat->peakiness > X16_Q11))
			classy = TRANSITION;
		else if ((classStat->zeroCrosRate > X055_Q15) ||
			 ((lowhighBandDiff < X05_Q12) &&
			  (classStat->zeroCrosRate > X04_Q15)))
			classy = UNVOICED;
		else
			classy = SILENCE;
	} else if ((zeroCrosRateDiff > X02_Q15) || (subEnergyDiff > TWO_Q11) ||
		   (classStat->peakiness > X16_Q11)) {
		if ((lowBandCorx > X07_Q15) || (classStat->corx > X08_Q15))
			classy = VOICED;
		else
			classy = TRANSITION;
	} else if (classStat->zeroCrosRate < X02_Q15) {
		/* unless very low energy, it should be voiced */
		if ((lowBandCorx > X05_Q15) ||
		    ((lowBandCorx > X03_Q15) && (classStat->corx > X06_Q15)))
			classy = VOICED;
		else if (classStat->subEnergy >
			 interp_scalar(voicedEn, silenceEn, X03_Q15)) {
			if (classStat->peakiness > X15_Q11)
				classy = TRANSITION;
			else
				classy = VOICED;
		} else
			classy = SILENCE;
	} else if (classStat->zeroCrosRate < X05_Q15) {	/* not sure */
		if ((lowBandCorx > X055_Q15) ||
		    ((lowBandCorx > X03_Q15) && (classStat->corx > X065_Q15)))
			classy = VOICED;
		else if ((classStat->subEnergy <
			  interp_scalar(voicedEn, silenceEn, X06_Q15)) &&
			 (lowhighBandDiff > ONE_Q12))
			classy = SILENCE;
		else if (classStat->peakiness > X14_Q11)
			classy = TRANSITION;
		else
			classy = UNVOICED;
	} else if (classStat->zeroCrosRate < X07_Q15) {
		/* quite possible unvoiced */
		if (((lowBandCorx > X06_Q15) && (classStat->corx > X03_Q15)) ||
		    ((lowBandCorx > X04_Q15) && (classStat->corx > X07_Q15)))
			classy = VOICED;
		else if (classStat->peakiness > X15_Q11)
			classy = TRANSITION;
		else
			classy = UNVOICED;
	} else {		/* very likely unvoiced */
		if (((lowBandCorx > X065_Q15) && (classStat->corx > X03_Q15)) ||
		    ((lowBandCorx > X045_Q15) && (classStat->corx > X07_Q15)))
			classy = VOICED;
		else if (classStat->peakiness > TWO_Q11)
			classy = TRANSITION;
		else
			classy = UNVOICED;
	}

	classStat->classy = classy;
}

/****************************************************************************
**
** Function:		zeroCrosCount
**
** Description: 	Zero Crossing Rate Computation
**
** Arguments:
**
**	int16_t speech[]		Input speech buffer (Q0)
**
** Return value:
**	int16_t				Zero Crossing Rate (Q15)
**
*****************************************************************************/
static int16_t zeroCrosCount(int16_t speech[])
{
	register int16_t i;
	int16_t dcfree_speech[PIT_SUBFRAME];
	int16_t prev_sign, current_sign;
	int16_t count;

	/* ======== Short term DC remove ======== */
	remove_dc(speech, dcfree_speech, PIT_SUBFRAME);

	/* ======== Count the number of zero crossings ======== */
	count = 0;
	if (dcfree_speech[0] >= 0)
		prev_sign = 1;
	else
		prev_sign = -1;
	for (i = 1; i < PIT_SUBFRAME; i++) {
		if (dcfree_speech[i] >= 0)
			current_sign = 1;
		else
			current_sign = -1;
		if ((prev_sign + current_sign) == 0)
			count++;
		prev_sign = current_sign;
	}

	return (melpe_divide_s(count, PIT_SUBFRAME));	/* Q15 */
}

/****************************************************************************
**
** Function:		bandEn
**
** Description: 	computate the low band energy from autocorrelation func.
**
** Arguments:
**
**	int16_t autocorr[]    ---- input autocorrelation function (Q15)
**
** Return value:
**	int16_t				---- the low band energy (Q12)
**
*****************************************************************************/

static int16_t bandEn(int16_t autocorr[], int16_t band)
{
	register int16_t i;
	const int16_t *coef;	/* Q16 */
	int16_t temp;
	int32_t energy;

	if (band == 0)		/* low band */
		coef = enlpf_coef;
	else
		coef = enhpf_coef;

	/* Accumulated sum for "energy".  coef[0] is the one with the largest     */
	/* absolute value in coef[], but it is still less than 0.5.  autocorr[]   */
	/* are all less than 1.  EN_FILTER_ORDER is 17, so energy is at most 17   */
	/* (even this is a very loose upper bound).  We might use a Q10 int16_t */
	/* to store energy after the accumulation is done.                        */

	energy = 0;
	for (i = 1; i < EN_FILTER_ORDER; i++) {
		/*      energy += 2.0*coef[i]*autocorr[i]; */
		temp = melpe_mult(coef[i], autocorr[i]);	/* Q14 */
		energy = melpe_L_add(energy, melpe_L_deposit_l(temp));
	}
	energy = melpe_L_shl(energy, 1);	/* multiplication by 2 */
	temp = melpe_mult(coef[0], autocorr[0]);	/* Q14 */
	energy = melpe_L_add(energy, melpe_L_deposit_l(temp));

	/* energy is Q14.  Note that energy could be negative. */

	if (energy < ONE_Q14)
		return (0);
	else {
		temp = melpe_extract_l(melpe_L_shr(energy, 4));	/* Q10 */
		temp = log10_fxp(temp, 10);	/* Q12 */

		/* Note that we return log10(energy) instead of 10.0*log10(energy).   */
		return (temp);
	}
}

/****************************************************************************
**
** Function:		frac_cor
**
** Description: 	computate the correlation coeff ar_ound specific pitch
**
** Arguments:
**
**	int16_t inbuf[]	---- input data buffer (Q0)
**	int16_t pitch		---- the input pitch center (Q0)
**	int16_t *cor		---- output cor coeff (Q15)
**
** Return value:	None
**
*****************************************************************************/
static void frac_cor(int16_t inbuf[], int16_t pitch, int16_t * cor)
{
	register int16_t i, j;
	int16_t lowPitch, highPitch;	/* Q0 */
	int16_t lowStart, highStart;
	int16_t gp, maxgp, root, win, temp;
	int16_t r0_shift, rk_shift, shift;
	int32_t L_r0, L_rk, L_temp;	/* Q7 */
	int64_t ACC_r0, ACC_rk, ACC_A;	/* Emulating 40Bit-Accumulator */

	/* ------ Calculate the autocorrelation function ------- */
	/* This is the new version of the autocorrelation function */
	/* (Andre Ebner, 11/30/99) */
	lowPitch = melpe_sub(pitch, PITCH_RANGE);
	highPitch = melpe_add(pitch, PITCH_RANGE);
	if (lowPitch < MINPITCH)
		lowPitch = MINPITCH;
	if (highPitch > MAXPITCH)
		highPitch = MAXPITCH;

	ACC_r0 = 0;
	for (i = 0; i < (PIT_COR_LEN - highPitch); i++) {
		ACC_r0 = melpe_L40_mac(ACC_r0, inbuf[i], inbuf[i]);
	}
	if (ACC_r0 == 0)
		ACC_r0 = 1;
	r0_shift = melpe_norm32(ACC_r0);
	ACC_r0 = melpe_L40_shl(ACC_r0, r0_shift);
	L_r0 = (int32_t) ACC_r0;

	ACC_rk = 0;
	for (i = highPitch; i < PIT_COR_LEN; i++) {
		ACC_rk = melpe_L40_mac(ACC_rk, inbuf[i], inbuf[i]);	/* Q31 */
	}
	if (ACC_rk == 0)
		ACC_rk = 1;
	rk_shift = melpe_norm32(ACC_rk);
	ACC_rk = melpe_L40_shl(ACC_rk, rk_shift);
	L_rk = (int32_t) ACC_rk;

	ACC_A = 0;
	for (i = 0; i < PIT_COR_LEN - highPitch; i++) {
		ACC_A = melpe_L40_mac(ACC_A, inbuf[i], inbuf[i + highPitch]);	/* Q31 */
	}
	shift = melpe_add(r0_shift, rk_shift);
	if (shift & 1) {
		L_r0 = melpe_L_shr(L_r0, 1);
		r0_shift = melpe_sub(r0_shift, 1);
		shift = melpe_add(r0_shift, rk_shift);
	}
	shift = melpe_shr(shift, 1);
	ACC_A = melpe_L40_shl(ACC_A, shift);
	temp = melpe_mult(melpe_extract_h(L_r0), melpe_extract_h(L_rk));
	root = sqrt_Q15(temp);
	L_temp = (int32_t) ACC_A;
	temp = melpe_extract_h(L_temp);
	if (temp < 0)
		temp = 0;	/* Negative Autocorrelation doesn't make sense here */
	maxgp = melpe_divide_s(temp, root);
	lowStart = 0;
	highStart = highPitch;
	win = melpe_sub(PIT_COR_LEN, highPitch);
	for (i = melpe_sub(highPitch, 1); i >= lowPitch; i--) {
		if (i % 2 == 0) {
			ACC_r0 = L_r0;
			ACC_r0 = melpe_L40_shr(ACC_r0, r0_shift);
			ACC_r0 =
			    melpe_L40_msu(ACC_r0, inbuf[lowStart], inbuf[lowStart]);
			ACC_r0 =
			    melpe_L40_mac(ACC_r0, inbuf[lowStart + win],
				    inbuf[lowStart + win]);
			if (ACC_r0 == 0)
				ACC_r0 = 1;
			r0_shift = melpe_norm32(ACC_r0);
			ACC_r0 = melpe_L40_shl(ACC_r0, r0_shift);
			L_r0 = (int32_t) ACC_r0;
			lowStart++;
		} else {
			highStart--;
			ACC_rk = L_rk;
			ACC_rk = melpe_L40_shr(ACC_rk, rk_shift);
			ACC_rk =
			    melpe_L40_mac(ACC_rk, inbuf[highStart], inbuf[highStart]);
			ACC_rk =
			    melpe_L40_msu(ACC_rk, inbuf[highStart + win],
				    inbuf[highStart + win]);
			if (ACC_rk == 0)
				ACC_rk = 1;
			rk_shift = melpe_norm32(ACC_rk);
			ACC_rk = melpe_L40_shl(ACC_rk, rk_shift);
			L_rk = (int32_t) ACC_rk;
		}
		ACC_A = 0;
		for (j = lowStart; j < lowStart + win; j++) {
			ACC_A = melpe_L40_mac(ACC_A, inbuf[j], inbuf[j + i]);
		}
		shift = melpe_add(r0_shift, rk_shift);
		if (shift & 1) {
			L_r0 = melpe_L_shr(L_r0, 1);
			r0_shift = melpe_sub(r0_shift, 1);
			shift = melpe_add(r0_shift, rk_shift);
		}
		shift = melpe_shr(shift, 1);
		ACC_A = melpe_L40_shl(ACC_A, shift);
		temp = melpe_mult(melpe_extract_h(L_r0), melpe_extract_h(L_rk));
		root = sqrt_Q15(temp);
		L_temp = (int32_t) ACC_A;
		temp = melpe_extract_h(L_temp);
		gp = melpe_divide_s(temp, root);
		if (gp > maxgp)
			maxgp = gp;
	}
	*cor = maxgp;
}

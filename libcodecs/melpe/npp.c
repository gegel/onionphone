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

/*---------------------------------------------------------------------
 * enh_fun.c - Speech Enhancement Functions
 *
 * Author: Rainer Martin, AT&T Labs-Research
 *
 * Last Update: $Id: enh_fun.c,v 1.2 1999/02/19 17:55:12 martinr Exp martinr $
 *
 *---------------------------------------------------------------------
 */

#include "npp.h"
#include "fs_lib.h"
#include "mat_lib.h"
#include "constant.h"
#include "dsp_sub.h"
#include "fft_lib.h"
#include "global.h"

#define X003_Q15		983	/* 0.03 * (1 << 15) */
#define X005_Q15		1638	/* 0.05 * (1 << 15) */
#define X006_Q15		1966	/* 0.06 * (1 << 15) */
#define X018_Q15		5898	/* 0.18 * (1 << 15) */
#define X065_Q15		21299	/* 0.65 * (1 << 15) */
#define X132_Q11		2703	/* 1.32 * (1 << 11) */
#define X15_Q13			12288	/* 1.5 * (1 << 13) */
#define X22_Q11			4506	/* 2.2 * (1 << 11) */
#define X44_Q11			9011	/* 4.4 * (1 << 11) */
#define X88_Q11			18022	/* 8.8 * (1 << 11) */

static const int16_t sqrt_tukey_256_180[ENH_WINLEN] = {	/* Q15 */
	677, 1354, 2030, 2705, 3380, 4053, 4724, 5393,
	6060, 6724, 7385, 8044, 8698, 9349, 9996, 10639,
	11277, 11911, 12539, 13162, 13780, 14391, 14996, 15595,
	16188, 16773, 17351, 17922, 18485, 19040, 19587, 20126,
	20656, 21177, 21690, 22193, 22686, 23170, 23644, 24108,
	24561, 25004, 25437, 25858, 26268, 26668, 27056, 27432,
	27796, 28149, 28490, 28818, 29134, 29438, 29729, 30008,
	30273, 30526, 30766, 30992, 31205, 31405, 31592, 31765,
	31924, 32070, 32202, 32321, 32425, 32516, 32593, 32656,
	32705, 32740, 32761, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32761, 32740, 32705, 32656,
	32593, 32516, 32425, 32321, 32202, 32070, 31924, 31765,
	31592, 31405, 31205, 30992, 30766, 30526, 30273, 30008,
	29729, 29438, 29134, 28818, 28490, 28149, 27796, 27432,
	27056, 26668, 26268, 25858, 25437, 25004, 24561, 24108,
	23644, 23170, 22686, 22193, 21690, 21177, 20656, 20126,
	19587, 19040, 18485, 17922, 17351, 16773, 16188, 15595,
	14996, 14391, 13780, 13162, 12539, 11911, 11277, 10639,
	9996, 9349, 8698, 8044, 7385, 6724, 6060, 5393,
	4724, 4053, 3380, 2705, 2030, 1354, 677, 0
};

/* ====== Entities from Enhanced_Data ====== */
static int16_t enh_i = 0;	/* formerly D->I */
static int16_t lambdaD[ENH_VEC_LENF];	/* overestimated noise */
					       /* psd(noise_bias * noisespect) */
static int16_t lambdaD_shift[ENH_VEC_LENF];
static int16_t SN_LT;		/* long term SNR */
static int16_t SN_LT_shift;
static int16_t n_pwr;
static int16_t n_pwr_shift;
static int16_t YY[ENH_VEC_LENF];	/* signal periodogram of current frame */
static int16_t YY_shift[ENH_VEC_LENF];
static int16_t sm_shift[ENH_VEC_LENF];
static int16_t noise_shift[ENH_VEC_LENF];
static int16_t noise2_shift[ENH_VEC_LENF];
static int16_t av_shift[ENH_VEC_LENF];
static int16_t av2_shift[ENH_VEC_LENF];
static int16_t act_min[ENH_VEC_LENF];	/* current minimum of long window */
static int16_t act_min_shift[ENH_VEC_LENF];
static int16_t act_min_sub[ENH_VEC_LENF];
					     /* current minimum of sub-window */
static int16_t act_min_sub_shift[ENH_VEC_LENF];
static int16_t vk[ENH_VEC_LENF];
static int16_t vk_shift[ENH_VEC_LENF];
static int16_t ksi[ENH_VEC_LENF];	/* a priori SNR */
static int16_t ksi_shift[ENH_VEC_LENF];

static int16_t var_rel[ENH_VEC_LENF];
				       /* est. relative var. of smoothedspect */
static int16_t var_rel_av;	/* average relative var. of smoothedspect */

/* only for minimum statistics */

static int16_t smoothedspect[ENH_VEC_LENF];	/* smoothed signal spectrum */
static int16_t var_sp_av[ENH_VEC_LENF];
					   /* estimated mean of smoothedspect */
static int16_t var_sp_2[ENH_VEC_LENF];
				     /* esitmated 2nd moment of smoothedspect */
static int16_t noisespect[ENH_VEC_LENF];	/* estimated noise psd */
static int16_t noisespect2[ENH_VEC_LENF];
static int16_t alphacorr;	/* correction factor for alpha_var, Q15 */
static int16_t alpha_var[ENH_VEC_LENF];	/* variable smoothing */
							 /* parameter for psd */
static int16_t circb[NUM_MINWIN][ENH_VEC_LENF];	/* ring buffer */
static int16_t circb_shift[NUM_MINWIN][ENH_VEC_LENF];

static int16_t initial_noise[ENH_WINLEN];	/* look ahead noise */
							      /* samples (Q0) */
static int16_t speech_in_npp[ENH_WINLEN];	/* input of one frame */
static int16_t ybuf[2 * ENH_WINLEN + 2];
				 /* buffer for FFT, this can be eliminated if */
						    /* we can write a better real-FFT program for DSP */
static int32_t temp_yy[ENH_WINLEN + 2];

/* ====== Prototypes ====== */

static void smoothing_win(int16_t initial_noise[]);
static void compute_qk(int16_t qk[], int16_t gamaK[],
		       int16_t gamaK_shift[], int16_t GammaQ_TH);
static void gain_log_mmse(int16_t qk[], int16_t Gain[],
			  int16_t gamaK[], int16_t gamaK_shift[],
			  int16_t m);
static int16_t ksi_min_adapt(BOOLEAN n_flag, int16_t ksi_min,
			       int16_t sn_lt, int16_t sn_lt_shift);
static void smoothed_periodogram(int16_t YY_av, int16_t yy_shift);
static void bias_compensation(int16_t biased_spect[],
			      int16_t bias_shift[],
			      int16_t biased_spect_sub[],
			      int16_t bias_sub_shift[]);
static int16_t noise_slope();
static int16_t comp_data_shift(int16_t num1, int16_t shift1,
				 int16_t num2, int16_t shift2);
static void min_search(int16_t biased_spect[], int16_t bias_shift[],
		       int16_t biased_spect_sub[],
		       int16_t bias_sub_shift[]);
void enh_init();
static void minstat_init();
static void process_frame(int16_t inspeech[], int16_t outspeech[]);
static void gain_mod(int16_t qk[], int16_t GainD[], int16_t m);

/****************************************************************************
**
** Function:		npp
**
** Description: 	The noise pre-processor
**
** Arguments:
**
**	int16_t	sp_in[]  ---- input speech data buffer (Q0)
**	int16_t	sp_out[] ---- output speech data buffer (Q0)
**
** Return value:	None
**
*****************************************************************************/
void npp(int16_t sp_in[], int16_t sp_out[])
{
	static int16_t speech_overlap[ENH_OVERLAP_LEN];
	static BOOLEAN first_time = TRUE;
	int16_t speech_out_npp[ENH_WINLEN];	/* output of one frame */

	if (first_time) {

		if (rate == RATE1200)
			v_equ(initial_noise, sp_in, ENH_WINLEN);
		else {
			v_zap(initial_noise, ENH_OVERLAP_LEN);
			v_equ(&initial_noise[ENH_OVERLAP_LEN], sp_in,
			      ENH_WIN_SHIFT);
		}
		enh_init();	/* Initialize enhancement routine */
		v_zap(speech_in_npp, ENH_WINLEN);

		first_time = FALSE;
	}

	/* Shift input buffer from the previous frame */
	v_equ(speech_in_npp, &(speech_in_npp[ENH_WIN_SHIFT]), ENH_OVERLAP_LEN);
	v_equ(&(speech_in_npp[ENH_OVERLAP_LEN]), sp_in, ENH_WIN_SHIFT);

	/* ======== Process one frame ======== */
	process_frame(speech_in_npp, speech_out_npp);

	/* Overlap-add the output buffer. */
	v_add(speech_out_npp, speech_overlap, ENH_OVERLAP_LEN);
	v_equ(speech_overlap, &(speech_out_npp[ENH_WIN_SHIFT]),
	      ENH_OVERLAP_LEN);

	/* ======== Output enhanced speech ======== */
	v_equ(sp_out, speech_out_npp, ENH_WIN_SHIFT);
}

/*****************************************************************************/
/*	  Subroutine gain_mod: compute gain modification factor based on		 */
/*		 signal absence probabilities qk									 */
/*****************************************************************************/
static void gain_mod(int16_t qk[], int16_t GainD[], int16_t m)
{
	register int16_t i;
	int16_t temp, temp1, temp2, temp3, temp4, shift;
	int16_t temp_shift, temp2_shift;
	int32_t L_sum, L_tmp;

	for (i = 0; i < m; i++) {
		/*      temp = 1.0 - qk[i]; */
		temp = melpe_sub(ONE_Q15, qk[i]);	/* Q15 */
		if (temp == 0)
			temp = 1;
		shift = melpe_norm_s(temp);
		temp = melpe_shl(temp, shift);
		temp_shift = melpe_negate(shift);

		/*      temp2 = temp*temp; */
		L_sum = melpe_L_mult(temp, temp);
		shift = melpe_norm_l(L_sum);
		temp2 = melpe_extract_h(melpe_L_shl(L_sum, shift));
		temp2_shift = melpe_sub(melpe_shl(temp_shift, 1), shift);

		/*      GM[i] = temp2 / (temp2 + (temp + ksi[i])*qk[i]*exp(-vk[i])) */

		/*      exp(-vk) = 2^(2*vk*(-0.5*log2(e))), and -23637 is -0.5*log2(e).   */
		L_sum = melpe_L_mult(vk[i], -23637);	/* vk*(-0.5*log2(e)), Q15 */
		shift = melpe_add(vk_shift[i], 1);
		L_sum = melpe_L_shr(L_sum, melpe_sub(15, shift));	/* 2^x x Q16 */

		/*      temp + ksi[i] */
		shift = melpe_sub(ksi_shift[i], temp_shift);
		if (shift > 0) {
			temp4 = melpe_add(ksi_shift[i], 1);
			temp3 = melpe_add(melpe_shr(temp, melpe_add(shift, 1)), melpe_shr(ksi[i], 1));
		} else {
			temp4 = melpe_add(temp_shift, 1);
			temp3 = melpe_add(melpe_shr(temp, 1), melpe_shl(ksi[i], melpe_sub(shift, 1)));
		}
		/*      (temp + ksi[i])*qk[i] */
		L_tmp = melpe_L_mult(temp3, qk[i]);

		/*      temp + ksi[i])*qk[i]*exp(-vk[i] */
		L_sum = melpe_L_add(L_sum, melpe_L_deposit_h(temp4));	/* add exp */
		shift = melpe_extract_h(L_sum);	/* this is exp part */

		temp4 = (int16_t) (melpe_extract_l(melpe_L_shr(L_sum, 1)) & 0x7fff);
		temp1 = melpe_shr(melpe_mult(temp4, 9864), 3);	/* change to base 10 Q12 */
		temp1 = pow10_fxp(temp1, 14);	/* out Q14 */
		L_tmp = L_mpy_ls(L_tmp, temp1);	/* Q30 */
		temp3 = melpe_norm_l(L_tmp);
		temp1 = melpe_extract_h(melpe_L_shl(L_tmp, temp3));
		shift = melpe_add(shift, melpe_sub(1, temp3));	/* make L_tmp Q31 */

		/*      temp2 + (temp + ksi[i])*qk[i]*exp(-vk[i]) */
		temp1 = melpe_shr(temp1, 1);
		temp2 = melpe_shr(temp2, 1);
		temp = melpe_sub(shift, temp2_shift);
		if (temp > 0) {
			temp3 = melpe_add(temp1, melpe_shr(temp2, temp));
			temp4 = melpe_shr(temp2, temp);
		} else {
			temp3 = melpe_add(melpe_shl(temp1, temp), temp2);
			temp4 = temp2;
		}

		temp = melpe_divide_s(temp4, temp3);	/* temp is previously known as GM[]. */

		/* limit lowest GM value */
		if (temp < GM_MIN)
			temp = GM_MIN;	/* Q15 */
		GainD[i] = melpe_mult(GainD[i], temp);	/* modified gain */
	}
}

/*****************************************************************************/
/*	  Subroutine compute_qk: compute the probability of speech absence       */
/*		This uses an harddecision approach due to Malah (ICASSP 1999).		 */
/*****************************************************************************/
static void compute_qk(int16_t qk[], int16_t gamaK[],
		       int16_t gamaK_shift[], int16_t GammaQ_TH)
{
	register int16_t i;
	static BOOLEAN first_time = TRUE;
	static int16_t qla[ENH_VEC_LENF];

	/* Initializing qla[] */
	if (first_time) {
		fill(qla, X05_Q15, ENH_VEC_LENF);
		first_time = FALSE;
	}

	/*      qla[] = alphaq * qla[]; */
	v_scale(qla, ENH_ALPHAQ, ENH_VEC_LENF);
	for (i = 0; i < ENH_VEC_LENF; i++) {

		/*      if (gamaK[i] < GammaQ_TH) */
		if (comp_data_shift(gamaK[i], gamaK_shift[i], GammaQ_TH, 0) < 0)
			/*      qla[] += betaq; */
			qla[i] = melpe_add(qla[i], ENH_BETAQ);
	}
	v_equ(qk, qla, ENH_VEC_LENF);
}

/*****************************************************************************/
/*	  Subroutine gain_log_mmse: compute the gain factor and the auxiliary	 */
/*		variable vk for the Ephraim&Malah 1985 log spectral estimator.		 */
/*	  Approximation of the exponential integral due to Malah, 1996			 */
/*****************************************************************************/
static void gain_log_mmse(int16_t qk[], int16_t Gain[],
			  int16_t gamaK[], int16_t gamaK_shift[],
			  int16_t m)
{
	register int16_t i;
	int16_t ksi_vq, temp1, temp2, shift;
	int32_t L_sum;

	for (i = 0; i < m; i++) {

		/*      1.0 - qk[] */
		temp1 = melpe_sub(ONE_Q15, qk[i]);
		shift = melpe_norm_s(temp1);
		temp1 = melpe_shl(temp1, shift);
		temp2 = melpe_sub(ksi_shift[i], (int16_t) (-shift));
		/*      ksi[] + 1.0 - qk[] */
		if (temp2 > 0) {
			temp1 = melpe_shr(temp1, melpe_add(temp2, 1));
			temp1 = melpe_add(temp1, melpe_shr(ksi[i], 1));
			temp2 = melpe_shr(ksi[i], 1);
		} else {
			temp1 = melpe_add(melpe_shr(temp1, 1), melpe_shl(ksi[i], melpe_sub(temp2, 1)));
			temp2 = melpe_shl(ksi[i], melpe_sub(temp2, 1));
		}

		/*      ksi_vq = ksi[i] / (ksi[i] + 1.0 - qk[i]); */
		ksi_vq = melpe_divide_s(temp2, temp1);	/* Q15 */

		/*      vk[i] = ksi_vq * gamaK[i]; */
		L_sum = melpe_L_mult(ksi_vq, gamaK[i]);
		shift = melpe_norm_l(L_sum);
		vk[i] = melpe_extract_h(melpe_L_shl(L_sum, shift));
		vk_shift[i] = melpe_sub(gamaK_shift[i], shift);

		/* The original floating version compares vk[] against 2^{-52} ==     */
		/* 2.220446049250313e-16.  Tian uses 32767*2^{-52} instead.           */
		if (comp_data_shift(vk[i], vk_shift[i], 32767, -52) < 0) {
			vk[i] = 32767;	/* MATLAB eps */
			vk_shift[i] = -52;
		}

		if (comp_data_shift(vk[i], vk_shift[i], 26214, -3) < 0) {
			/* eiv = -2.31 * log10(vk[i]) - 0.6; */
			temp1 = log10_fxp(vk[i], 15);	/* Q12 */
			L_sum = melpe_L_shl(melpe_L_deposit_l(temp1), 14);	/* Q26 */
			L_sum =
			    melpe_L_add(L_sum, melpe_L_shl(melpe_L_mult(vk_shift[i], 9864), 10));
			L_sum = L_mpy_ls(L_sum, -18923);
			/* -18923 = -2.31 (Q13). out Q24 */
			L_sum = melpe_L_sub(L_sum, 10066330L);	/* 10066330L = 0.6 (Q24), Q24 */
		} else if (comp_data_shift(vk[i], vk_shift[i], 25600, 8) > 0) {
			/*      eiv = pow(10, -0.52 * 200 - 0.26); */
			L_sum = 1;	/* Q24 */

			/*      vk[] = 200; */
			vk[i] = 25600;
			vk_shift[i] = 8;
		} else if (comp_data_shift(vk[i], vk_shift[i], 32767, 0) > 0) {
			/*      eiv = pow(10, -0.52 * vk[i] - 0.26); */
			L_sum = melpe_L_mult(vk[i], -17039);	/* -17039 == -0.52 Q15 */
			L_sum =
			    melpe_L_sub(L_sum, melpe_L_shr(melpe_L_deposit_h(8520), vk_shift[i]));
			/* 8520 == 0.26 Q15 */
			L_sum = melpe_L_shr(L_sum, melpe_sub(14, vk_shift[i]));
			L_sum = L_mpy_ls(L_sum, 27213);	/* Q15 to base 2 */
			shift = melpe_extract_h(melpe_L_shl(L_sum, 1));	/* integer part */
			temp1 = (int16_t) (melpe_extract_l(L_sum) & 0x7fff);
			temp1 = melpe_shr(melpe_mult(temp1, 9864), 3);	/* Q12 to base 10 */
			temp1 = pow10_fxp(temp1, 14);	/* output Q14 */
			L_sum = melpe_L_shl(melpe_L_deposit_l(temp1), 10);	/* Q24 now */
			L_sum = melpe_L_shl(L_sum, shift);	/* shift must < 0 */
		} else {
			/* eiv = -1.544 * log10(vk[i]) + 0.166; */
			temp1 = vk[i];
			if (vk_shift[i] != 0)
				temp1 = melpe_shl(temp1, vk_shift[i]);
			temp1 = log10_fxp(temp1, 15);	/* Q12 */
			L_sum = melpe_L_shl(melpe_L_deposit_l(temp1), 13);	/* Q25 */
			L_sum = L_mpy_ls(L_sum, -25297);	/* -25297 = -1.544, Q14. Q24 */
			L_sum = melpe_L_add(L_sum, 2785018L);	/* 2785018L == 0.166, Q24 */
		}

		/* Now "eiv" is kept in L_sum. */
		/*      Gain[i] = ksi_vq * exp (0.5 * eiv); */

		L_sum = L_mpy_ls(L_sum, 23637);	/* 0.72135 to base 2 */
		shift = melpe_shr(melpe_extract_h(L_sum), 8);	/* get shift */
		if (comp_data_shift(ksi_vq, shift, 32767, 0) > 0) {
			Gain[i] = 32767;
			continue;
		} else {
			temp1 = melpe_extract_l(melpe_L_shr(L_sum, 9));
			temp1 = (int16_t) (temp1 & 0x7fff);
			temp1 = melpe_shr(melpe_mult(temp1, 9864), 3);	/* change to base 10 Q12 */
			temp1 = pow10_fxp(temp1, 14);

			L_sum = melpe_L_shl(melpe_L_deposit_h(ksi_vq), shift);
			L_sum = L_mpy_ls(L_sum, temp1);
			if (melpe_L_sub(L_sum, 1073676288L) > 0)
				Gain[i] = 32767;
			else
				Gain[i] = melpe_extract_h(melpe_L_shl(L_sum, 1));
		}
	}
}

/*****************************************************************************/
/*	   Subroutine ksi_min_adapt: computes the adaptive ksi_min				 */
/*****************************************************************************/
static int16_t ksi_min_adapt(BOOLEAN n_flag, int16_t ksi_min,
			       int16_t sn_lt, int16_t sn_lt_shift)
{
	int16_t ksi_min_new, temp, shift;
	int32_t L_sum;

	if (n_flag)		/* RM: adaptive modification of ksi limit (10/98) */
		ksi_min_new = ksi_min;
	else {
		if (sn_lt_shift > 0) {
			L_sum =
			    melpe_L_add(melpe_L_deposit_l(sn_lt),
				  melpe_L_shr(X05_Q15, sn_lt_shift));
			shift = sn_lt_shift;
		} else {
			L_sum =
			    melpe_L_add(melpe_L_shl(melpe_L_deposit_l(sn_lt), sn_lt_shift),
				  X05_Q15);
			shift = 0;
		}

		/* L_sum is (sn_lt * 2^sn_lt_shift + 0.5) */
		if (L_sum > ONE_Q15) {
			L_sum = melpe_L_shr(L_sum, 1);
			shift = melpe_add(shift, 1);
		}
		temp = melpe_extract_l(L_sum);

		/* We want to compute 2^{0.65*[log2(0.5+sn_lt)]-7.2134752} */
		/* equiv. 2^{0.65*log10(temp)/log10(2) + 0.65*shift - 7.2134752} */
		temp = log10_fxp(temp, 15);	/* log(0.5 + sn_lt), Q12 */
		L_sum = melpe_L_shr(melpe_L_mult(temp, 8844), 9);	/* Q12*Q12->Q25 --> Q16 */

		L_sum = melpe_L_add(L_sum, melpe_L_mult(shift, 21299));	/* 21299 = 0.65 Q15 */
		L_sum = melpe_L_sub(L_sum, 472742L);	/* 472742 = 7.2134752 Q16 */
		shift = melpe_extract_h(L_sum);	/* the pure shift */
		temp = (int16_t) (melpe_extract_l(melpe_L_shr(L_sum, 1)) & 0x7fff);
		temp = melpe_mult(temp, 9864);	/* change to base 10 */
		temp = melpe_shr(temp, 3);	/* change to Q12 */
		temp = pow10_fxp(temp, 14);
		L_sum = melpe_L_shl(melpe_L_mult(ksi_min, temp), 1);	/* Now Q31 */
		temp = melpe_extract_h(L_sum);

		/*      if (ksi_min_new > 0.25) */
		if (comp_data_shift(temp, shift, 8192, 0) > 0)
			ksi_min_new = 8192;
		else
			ksi_min_new = melpe_shl(temp, shift);
	}

	return (ksi_min_new);
}

/*****************************************************************************/
/* Subroutine smoothing_win: applies the Parzen window.  The window applies  */
/* an inverse trapezoid window and wtr_front[] supplies the coefficients for */
/* the two edges.                                                            */
/*****************************************************************************/
static void smoothing_win(int16_t initial_noise[])
{
	register int16_t i;
	static const int16_t wtr_front[WTR_FRONT_LEN] = {	/* Q15 */
		32767, 32582, 32048, 31202, 30080, 28718, 27152, 25418,
		23552, 21590, 19568, 17522, 15488, 13502, 11600, 9818,
		8192, 6750, 5488, 4394, 3456, 2662, 2000, 1458,
		1024, 686, 432, 250, 128, 54, 16, 2
	};

	for (i = 1; i < WTR_FRONT_LEN; i++)
		initial_noise[i] = melpe_mult(initial_noise[i], wtr_front[i]);
	for (i = ENH_WINLEN - WTR_FRONT_LEN + 1; i < ENH_WINLEN; i++)
		initial_noise[i] =
		    melpe_mult(initial_noise[i], wtr_front[ENH_WINLEN - i]);

	/* Clearing the central part of initial_noise[]. */
	v_zap(&(initial_noise[WTR_FRONT_LEN]),
	      ENH_WINLEN - 2 * WTR_FRONT_LEN + 1);
}

/***************************************************************************/
/*	Subroutine smoothed_periodogram: compute short time psd with optimal   */
/*						 smoothing										   */
/***************************************************************************/
static void smoothed_periodogram(int16_t YY_av, int16_t yy_shift)
{
	register int16_t i;
	int16_t smoothed_av, alphacorr_new, alpha_N_min_1, alpha_num;
	int16_t smav_shift, shift, temp, temp1, tmpns, tmpalpha;
	int16_t noise__shift, temp_shift, tmpns_shift, max_shift;
	int32_t L_sum, L_tmp;

	/* ---- compute smoothed_av ---- */
	max_shift = SW_MIN;
	for (i = 0; i < ENH_VEC_LENF; i++) {
		if (sm_shift[i] > max_shift)
			max_shift = sm_shift[i];
	}

	L_sum = melpe_L_shl(melpe_L_deposit_l(smoothedspect[0]),
		      melpe_sub(7, melpe_sub(max_shift, sm_shift[0])));
	L_sum = melpe_L_add(L_sum, melpe_L_shl(melpe_L_deposit_l(smoothedspect[ENH_VEC_LENF - 1]),
				   melpe_sub(7,
				       melpe_sub(max_shift,
					   sm_shift[ENH_VEC_LENF - 1]))));
	for (i = 1; i < ENH_VEC_LENF - 1; i++)
		L_sum = melpe_L_add(L_sum, melpe_L_shl(melpe_L_deposit_l(smoothedspect[i]),
					   melpe_sub(8,
					       melpe_sub(max_shift, sm_shift[i]))));

	/* Now L_sum contains smoothed_av.  Here we do not multiply L_sum with    */
	/* win_len_inv because we do not do this on YY_av either.                 */

	if (L_sum == 0)
		L_sum = 1;
	temp = melpe_norm_l(L_sum);
	temp = melpe_sub(temp, 1);	/* make sure smoothed_av < YY_av */
	smoothed_av = melpe_extract_h(melpe_L_shl(L_sum, temp));
	smav_shift = melpe_sub(melpe_add(max_shift, 1), temp);

	/* ---- alphacorr_new = smoothed_av / YY_av - 1.0 ---- */
	alphacorr_new = melpe_divide_s(smoothed_av, YY_av);
	shift = melpe_sub(smav_shift, yy_shift);
	if (shift <= 0) {
		if (shift > -15)
			alphacorr_new = melpe_sub(melpe_shl(alphacorr_new, shift), ONE_Q15);
		else
			alphacorr_new = melpe_negate(ONE_Q15);
		shift = 0;
	} else {
		if (shift < 15)
			alphacorr_new = melpe_sub(alphacorr_new, melpe_shr(ONE_Q15, shift));
	}

	/* -- alphacorr_new = 1.0 / (1.0 + alphacorr_new * alphacorr_new) -- */
	alphacorr_new = melpe_mult(alphacorr_new, alphacorr_new);
	alphacorr_new = melpe_shr(alphacorr_new, 1);	/* avoid overflow when +1 */
	shift = melpe_shl(shift, 1);
	if (shift < 15)
		alphacorr_new = melpe_add(alphacorr_new, melpe_shr(ONE_Q14, shift));
	if (alphacorr_new == 0)
		alphacorr_new = ONE_Q15;
	else {
		if (shift < 15) {
			temp = melpe_shr(ONE_Q14, shift);
			alphacorr_new = melpe_divide_s(temp, alphacorr_new);
		} else		/* too small */
			alphacorr_new = 0;
	}

	/* -- alphacorr_new > 0.7 ? alphacorr_new : 0.7 -- */
	if (alphacorr_new < X07_Q15)
		alphacorr_new = X07_Q15;

	/* -- alphacorr = 0.7*alphacorr + 0.3*alphacorr_new -- */
	alphacorr = melpe_extract_h(melpe_L_add(melpe_L_mult(X07_Q15, alphacorr),
				    melpe_L_mult(X03_Q15, alphacorr_new)));

	/* -- compute alpha_N_min_1 -- */
	/* -- alpha_N_min_1 = pow(SN_LT, PDECAY_NUM) */
	if (comp_data_shift(SN_LT, SN_LT_shift, 16384, 15) > 0)
		L_sum = 536870912L;	/* Q15 */
	else if (comp_data_shift(SN_LT, SN_LT_shift, 32, 15) < 0)
		L_sum = 1048576L;
	else
		L_sum = melpe_L_shl(melpe_L_deposit_l(SN_LT), SN_LT_shift);
	temp = L_log10_fxp(L_sum, 15);	/* output Q11 */
	temp = melpe_shl(temp, 1);	/* convert to Q12 */
	temp = melpe_mult(temp, (int16_t) PDECAY_NUM);
	alpha_N_min_1 = pow10_fxp(temp, 15);

	/* -- alpha_N_min_1 = (0.3 < alpha_N_min_1 ? 0.3 : alpha_N_min_1) -- */
	/* -- alpha_N_min_1 = (alpha_N_min_1 < 0.05 ? 0.05 : alpha_N_min_1) -- */
	if (alpha_N_min_1 > X03_Q15)
		alpha_N_min_1 = X03_Q15;
	else if (alpha_N_min_1 < X005_Q15)
		alpha_N_min_1 = X005_Q15;

	/* -- alpha_num = ALPHA_N_MAX * alphacorr -- */
	alpha_num = melpe_mult(ALPHA_N_MAX, alphacorr);

	/* -- compute smoothed spectrum -- */
	for (i = 0; i < ENH_VEC_LENF; i++) {
		tmpns = noisespect[i];

		/*      temp = smoothedspect[i] - tmpns; */
		shift = melpe_sub(sm_shift[i], noise_shift[i]);
		if (shift > 0) {
			L_tmp = melpe_L_sub(melpe_L_deposit_h(smoothedspect[i]),
				      melpe_L_shr(melpe_L_deposit_h(noisespect[i]), shift));
			shift = sm_shift[i];
		} else {
			L_tmp =
			    melpe_L_sub(melpe_L_shr
				  (melpe_L_deposit_h(smoothedspect[i]), melpe_abs_s(shift)),
				  melpe_L_deposit_h(tmpns));
			shift = noise_shift[i];
		}

		temp1 = melpe_norm_l(L_tmp);
		temp = melpe_extract_h(melpe_L_shl(L_tmp, temp1));
		shift = melpe_sub(shift, temp1);

		/*      tmpns = tmpns * tmpns; */
		L_sum = melpe_L_mult(tmpns, tmpns);	/* noise square, shift*2 */
		noise__shift = melpe_norm_l(L_sum);
		tmpns = melpe_extract_h(melpe_L_shl(L_sum, noise__shift));
		tmpns_shift = melpe_sub(melpe_shl(noise_shift[i], 1), noise__shift);
		noisespect2[i] = tmpns;
		noise2_shift[i] = tmpns_shift;

		if (temp == SW_MIN)
			L_sum = 0x7fffffff;
		else
			L_sum = melpe_L_mult(temp, temp);
		noise__shift = melpe_norm_l(L_sum);
		temp = melpe_extract_h(melpe_L_shl(L_sum, noise__shift));
		if (temp == 0)
			temp_shift = -20;
		else
			temp_shift = melpe_sub(melpe_shl(shift, 1), noise__shift);

		/* -- tmpalpha = alpha_num * tmpns / (tmpns + temp * temp) -- */
		temp1 = melpe_sub(temp_shift, tmpns_shift);
		if (temp1 > 0) {
			temp = melpe_shr(temp, 1);
			tmpns = melpe_shr(tmpns, melpe_add(temp1, 1));
		} else {
			tmpns = melpe_shr(tmpns, 1);
			temp = melpe_shl(temp, melpe_sub(temp1, 1));
		}
		tmpalpha = melpe_divide_s(tmpns, melpe_add(temp, tmpns));
		tmpalpha = melpe_mult(tmpalpha, alpha_num);

		/* tmpalpha = (tmpalpha < alpha_N_min_1 ? alpha_N_min_1 : tmpalpha) */
		if (tmpalpha < alpha_N_min_1)
			tmpalpha = alpha_N_min_1;

		temp = melpe_sub(ONE_Q15, tmpalpha);	/* 1 - tmpalpha */
		alpha_var[i] = tmpalpha;	/* save alpha */

		/*      smoothedspect[i] = tmpalpha * smoothedspect[i] + */
		/*                     (1 - tmpalpha) * YY[i]; */
		smav_shift = melpe_sub(YY_shift[i], sm_shift[i]);
		if (smav_shift > 0) {
			L_sum =
			    melpe_L_shr(melpe_L_mult(tmpalpha, smoothedspect[i]),
				  smav_shift);
			L_sum = melpe_L_add(L_sum, melpe_L_mult(temp, YY[i]));
			sm_shift[i] = YY_shift[i];
		} else {
			L_sum = melpe_L_mult(tmpalpha, smoothedspect[i]);
			L_sum =
			    melpe_L_add(L_sum,
				  melpe_L_shl(melpe_L_mult(temp, YY[i]), smav_shift));
		}
		if (L_sum < 1)
			L_sum = 1;
		shift = melpe_norm_l(L_sum);
		smoothedspect[i] = melpe_extract_h(melpe_L_shl(L_sum, shift));
		sm_shift[i] = melpe_sub(sm_shift[i], shift);
	}
}

/*****************************************************************************/
/* Subroutine normalized_variance: compute variance of smoothed periodogram, */
/* normalize it, and use it to compute a biased smoothed periodogram		 */
/*****************************************************************************/
static void bias_compensation(int16_t biased_spect[],
			      int16_t bias_shift[],
			      int16_t biased_spect_sub[],
			      int16_t bias_sub_shift[])
{
	register int16_t i;
	int16_t tmp, tmp1, tmp2, tmp5;
	int16_t beta_var, var_rel_av_sqrt;
	int16_t av__shift, av2__shift, shift1, shift3, shift4;
	int32_t L_max1, L_max2, L_sum, L_var_sum, L_tmp3, L_tmp4;

	/* ---- compute variance of smoothed psd ---- */
	L_var_sum = 0;
	for (i = 0; i < ENH_VEC_LENF; i++) {

		/*      tmp1 = alpha_var[i]*alpha_var[i]; */
		/*      beta_var = (tmp1 > 0.8 ? 0.8 : tmp1); */
		beta_var = melpe_mult(alpha_var[i], alpha_var[i]);
		if (beta_var > X08_Q15)
			beta_var = X08_Q15;

		/*      tmp2 = (1 - beta_var) * smoothedspect[i]; */
		L_sum = melpe_L_mult(melpe_sub(ONE_Q15, beta_var), smoothedspect[i]);

		/*      var_sp_av[i] = beta_var * var_sp_av[i] + tmp2; */
		av__shift = melpe_sub(sm_shift[i], av_shift[i]);
		if (av__shift > 0) {
			L_max1 =
			    melpe_L_add(melpe_L_shr
				  (melpe_L_mult(beta_var, var_sp_av[i]), av__shift),
				  L_sum);
			av_shift[i] = sm_shift[i];
		} else
			L_max1 = melpe_L_add(melpe_L_mult(beta_var, var_sp_av[i]),
				       melpe_L_shl(L_sum, av__shift));
		if (L_max1 < 1)
			L_max1 = 1;
		shift1 = melpe_norm_l(L_max1);
		var_sp_av[i] = melpe_extract_h(melpe_L_shl(L_max1, shift1));
		av_shift[i] = melpe_sub(av_shift[i], shift1);

		/*      tmp4 = tmp2 * smoothedspect[i]; */
		/*      var_sp_2[i] = beta_var * var_sp_2[i] + tmp4; */
		av2__shift = melpe_sub(melpe_shl(sm_shift[i], 1), av2_shift[i]);
		if (av2__shift > 0) {
			L_max2 =
			    melpe_L_add(melpe_L_shr
				  (melpe_L_mult(beta_var, var_sp_2[i]), av2__shift),
				  L_mpy_ls(L_sum, smoothedspect[i]));
			av2_shift[i] = melpe_shl(sm_shift[i], 1);
		} else
			L_max2 = melpe_L_add(melpe_L_mult(beta_var, var_sp_2[i]),
				       melpe_L_shl(L_mpy_ls(L_sum, smoothedspect[i]),
					     av2__shift));

		if (L_max2 < 1)
			L_max2 = 1;
		shift1 = melpe_norm_l(L_max2);
		var_sp_2[i] = melpe_extract_h(melpe_L_shl(L_max2, shift1));
		av2_shift[i] = melpe_sub(av2_shift[i], shift1);

		/*      tmp3 = var_sp_av[i] * var_sp_av[i]; */
		L_sum = melpe_L_mult(var_sp_av[i], var_sp_av[i]);

		/*      var_sp = var_sp_2[i] - tmp3; */
		shift3 = melpe_sub(av2_shift[i], melpe_shl(av_shift[i], 1));
		if (shift3 > 0) {
			L_sum =
			    melpe_L_sub(melpe_L_deposit_h(var_sp_2[i]),
				  melpe_L_shr(L_sum, shift3));
			shift4 = av2_shift[i];
		} else {
			L_sum =
			    melpe_L_sub(melpe_L_shl(melpe_L_deposit_h(var_sp_2[i]), shift3),
				  L_sum);
			shift4 = melpe_shl(av_shift[i], 1);
		}
		shift1 = melpe_sub(melpe_norm_l(L_sum), 1);
		tmp1 = melpe_extract_h(melpe_L_shl(L_sum, shift1));
		tmp = melpe_sub(melpe_sub(shift4, shift1), noise2_shift[i]);

		/*      tmp1 = var_sp / noisespect2[i]; */
		var_rel[i] = melpe_divide_s(tmp1, noisespect2[i]);

		/*      var_rel[i] = (tmp1 > 0.5 ? 0.5 : tmp1); */
		if (comp_data_shift(var_rel[i], tmp, X05_Q15, 0) > 0)
			var_rel[i] = X05_Q15;
		else
			var_rel[i] = melpe_shl(var_rel[i], tmp);
		if (var_rel[i] < 0)
			var_rel[i] = 0;

		/*      var_sum += var_rel[i]; */
		L_var_sum = melpe_L_add(L_var_sum, melpe_L_deposit_l(var_rel[i]));
	}

	/*      var_rel_av = (2 * var_sum - var_rel[0] - var_rel[ENH_VEC_LENF - 1]);  */
	L_var_sum = melpe_L_shl(L_var_sum, 1);
	L_var_sum = melpe_L_sub(L_var_sum, melpe_L_deposit_l(var_rel[0]));
	L_var_sum = melpe_L_sub(L_var_sum, melpe_L_deposit_l(var_rel[ENH_VEC_LENF - 1]));
	var_rel_av = melpe_extract_l(melpe_L_shr(L_var_sum, 8));	/* Q15 */

	/*      var_rel_av = (var_rel_av < 0 ? 0 : var_rel_av); */
	if (var_rel_av < 0)
		var_rel_av = 0;

	var_rel_av_sqrt = melpe_mult(X15_Q13, sqrt_Q15(var_rel_av));	/* Q13 */
	var_rel_av_sqrt = melpe_add(ONE_Q13, var_rel_av_sqrt);	/* Q13 */

	/*      tmp1 = var_rel_av_sqrt * Fvar; */
	/*      tmp2 = var_rel_av_sqrt * Fvar_sub; */
	tmp1 = melpe_extract_h(melpe_L_shl(melpe_L_mult(var_rel_av_sqrt, FVAR), 1));	/* Q10 */
	tmp2 = melpe_extract_h(melpe_L_shl(melpe_L_mult(var_rel_av_sqrt, FVAR_SUB), 1));	/* Q13 */

	for (i = 0; i < ENH_VEC_LENF; i++) {
		L_tmp3 = melpe_L_mult(var_rel_av_sqrt, smoothedspect[i]);	/* Q29 */
		tmp5 = var_rel[i];	/* Q15 */
		L_tmp4 = melpe_L_mult(tmp5, smoothedspect[i]);	/* Q31 */

		/* quadratic approximation */
		tmp = melpe_add(MINV, melpe_shr(tmp5, 1));	/* Q14 */
		tmp = melpe_add(MINV2, melpe_shr(melpe_mult(tmp5, tmp), 1));	/* Q13 */
		L_sum = L_mpy_ls(L_tmp4, tmp1);	/* Q26 */
		L_sum = L_mpy_ls(L_sum, tmp);	/* Q24 */
		L_sum = melpe_L_add(melpe_L_shr(L_tmp3, 6), melpe_L_shr(L_sum, 1));	/* Q23 */
		if (L_sum < 1)
			L_sum = 1;
		shift1 = melpe_norm_l(L_sum);
		biased_spect[i] = melpe_extract_h(melpe_L_shl(L_sum, shift1));
		bias_shift[i] = melpe_add(sm_shift[i], melpe_sub(8, shift1));

		tmp = melpe_add(MINV_SUB, melpe_shr(tmp5, 2));	/* Q13 */
		tmp = melpe_add(MINV_SUB2, melpe_shr(melpe_mult(tmp5, tmp), 1));	/* Q12 */
		L_sum = L_mpy_ls(L_tmp4, tmp2);	/* Q29 */
		L_sum = L_mpy_ls(L_sum, tmp);	/* Q26 */
		L_sum = melpe_L_add(melpe_L_shr(L_tmp3, 4), melpe_L_shr(L_sum, 1));	/* Q25 */
		if (L_sum < 1)
			L_sum = 1;
		shift1 = melpe_norm_l(L_sum);
		biased_spect_sub[i] = melpe_extract_h(melpe_L_shl(L_sum, shift1));
		bias_sub_shift[i] = melpe_add(sm_shift[i], melpe_sub(6, shift1));
	}
}

/***************************************************************************/
/*	Subroutine noise_slope: compute maximum of the permitted increase of   */
/*		   the noise estimate as a function of the mean signal variance    */
/***************************************************************************/
static int16_t noise_slope()
{
	int16_t noise_slope_max;

	if (var_rel_av > X018_Q15)
		noise_slope_max = X132_Q11;
	else if ((var_rel_av < X003_Q15) || (enh_i < 50))
		noise_slope_max = X88_Q11;
	else if (var_rel_av < X005_Q15)
		noise_slope_max = X44_Q11;
	else if (var_rel_av < X006_Q15)
		noise_slope_max = X22_Q11;
	else
		noise_slope_max = X132_Q11;

	return (noise_slope_max);
}

/***************************************************************************/
/* Subroutine comp_data_shift: compare two block floating-point numbers.   */
/* It actually compares x1 = (num1 * 2^shift1) and x2 = (num2 * 2^shift2). */
/* The sign of the returned value is the same as that of (x1 - x2).        */
/***************************************************************************/
static int16_t comp_data_shift(int16_t num1, int16_t shift1,
				 int16_t num2, int16_t shift2)
{
	int16_t shift;

	if ((num1 > 0) && (num2 < 0))
		return (1);
	else if ((num1 < 0) && (num2 > 0))
		return (-1);
	else {
		shift = melpe_sub(shift1, shift2);
		if (shift > 0)
			num2 = melpe_shr(num2, shift);
		else
			num1 = melpe_shl(num1, shift);

		return (melpe_sub(num1, num2));
	}
}

/***************************************************************************/
/*	Subroutine min_search: find minimum of psd's in circular buffer 	   */
/***************************************************************************/
static void min_search(int16_t biased_spect[], int16_t bias_shift[],
		       int16_t biased_spect_sub[], int16_t bias_sub_shift[])
{
	register int16_t i, k;
	static BOOLEAN localflag[ENH_VEC_LENF];	/* local minimum indicator */
	static int16_t minspec_counter = 0;	/* count sub-windows */
	static int16_t circb_index = 0;	/* ring buffer counter */
	static int16_t circb_min[ENH_VEC_LENF];
	/* minimum of circular buffer */
	static int16_t circb_min_shift[ENH_VEC_LENF];
	int16_t noise_slope_max, tmp, tmp_shift, temp1, temp2;
	int32_t L_sum;

	/* localflag[] is initialized to FALSE since it is static. */

	if (minspec_counter == 0) {
		noise_slope_max = noise_slope();

		for (i = 0; i < ENH_VEC_LENF; i++) {
			if (comp_data_shift(biased_spect[i], bias_shift[i],
					    act_min[i], act_min_shift[i]) < 0) {
				act_min[i] = biased_spect[i];	/* update minimum */
				act_min_shift[i] = bias_shift[i];
				act_min_sub[i] = biased_spect_sub[i];
				act_min_sub_shift[i] = bias_sub_shift[i];
				localflag[i] = FALSE;
			}
		}

		/* write new minimum into ring buffer */
		v_equ(circb[circb_index], act_min, ENH_VEC_LENF);
		v_equ(circb_shift[circb_index], act_min_shift, ENH_VEC_LENF);

		for (i = 0; i < ENH_VEC_LENF; i++) {
			/* Find minimum of ring buffer.  Using temp1 and temp2 as cache   */
			/* for circb_min[i] and circb_min_shift[i].                       */

			temp1 = circb[0][i];
			temp2 = circb_shift[0][i];
			for (k = 1; k < NUM_MINWIN; k++) {
				if (comp_data_shift
				    (circb[k][i], circb_shift[k][i], temp1,
				     temp2) < 0) {
					temp1 = circb[k][i];
					temp2 = circb_shift[k][i];
				}
			}
			circb_min[i] = temp1;
			circb_min_shift[i] = temp2;
		}
		for (i = 0; i < ENH_VEC_LENF; i++) {
			/*      rapid update in case of local minima which do not deviate
			   more than noise_slope_max from the current minima */
			tmp = melpe_mult(noise_slope_max, circb_min[i]);
			tmp_shift = melpe_add(circb_min_shift[i], 4);	/* adjust for Q11 */
			if (localflag[i] &&
			    comp_data_shift(act_min_sub[i],
					    act_min_sub_shift[i], circb_min[i],
					    circb_min_shift[i]) > 0
			    && comp_data_shift(act_min_sub[i],
					       act_min_sub_shift[i], tmp,
					       tmp_shift) < 0) {
				circb_min[i] = act_min_sub[i];
				circb_min_shift[i] = act_min_sub_shift[i];

				/* propagate new rapid update minimum into ring buffer */
				for (k = 0; k < NUM_MINWIN; k++) {
					circb[k][i] = circb_min[i];
					circb_shift[k][i] = circb_min_shift[i];
				}
			}
		}

		/* reset local minimum indicator */
		fill(localflag, FALSE, ENH_VEC_LENF);

		/* increment ring buffer pointer */
		circb_index = melpe_add(circb_index, 1);
		if (circb_index == NUM_MINWIN)
			circb_index = 0;
	} else if (minspec_counter == 1) {

		v_equ(act_min, biased_spect, ENH_VEC_LENF);
		v_equ(act_min_shift, bias_shift, ENH_VEC_LENF);
		v_equ(act_min_sub, biased_spect_sub, ENH_VEC_LENF);
		v_equ(act_min_sub_shift, bias_sub_shift, ENH_VEC_LENF);

	} else {		/* minspec_counter > 1 */

		/* At this point localflag[] is all FALSE.  As we loop through        */
		/* minspec_counter, if any localflag[] is turned TRUE, it will be     */
		/* preserved until we go through the (minspec_counter == 0) branch.   */

		for (i = 0; i < ENH_VEC_LENF; i++) {
			if (comp_data_shift(biased_spect[i], bias_shift[i],
					    act_min[i], act_min_shift[i]) < 0) {
				/* update minimum */
				act_min[i] = biased_spect[i];
				act_min_shift[i] = bias_shift[i];
				act_min_sub[i] = biased_spect_sub[i];
				act_min_sub_shift[i] = bias_sub_shift[i];
				localflag[i] = TRUE;
			}
		}
		for (i = 0; i < ENH_VEC_LENF; i++) {
			if (comp_data_shift
			    (act_min_sub[i], act_min_sub_shift[i], circb_min[i],
			     circb_min_shift[i]) < 0) {
				circb_min[i] = act_min_sub[i];
				circb_min_shift[i] = act_min_sub_shift[i];
			}
		}

		v_equ(noisespect, circb_min, ENH_VEC_LENF);
		v_equ(noise_shift, circb_min_shift, ENH_VEC_LENF);

		for (i = 0; i < ENH_VEC_LENF; i++) {
			L_sum = melpe_L_mult(ENH_NOISE_BIAS, noisespect[i]);
			if (L_sum < 0x40000000L) {
				L_sum = melpe_L_shl(L_sum, 1);
				lambdaD_shift[i] = noise_shift[i];
			} else
				lambdaD_shift[i] = melpe_add(noise_shift[i], 1);
			lambdaD[i] = melpe_extract_h(L_sum);
		}
	}
	minspec_counter = melpe_add(minspec_counter, 1);
	if (minspec_counter == LEN_MINWIN)
		minspec_counter = 0;
}

/***************************************************************************/
/*	Subroutine enh_init: initialization of variables for the enhancement   */
/***************************************************************************/
void enh_init()
{
	register int16_t i;
	int16_t max, shift, temp, norm_shift, guard;
	int16_t a_shift;
	int32_t L_sum = 0, L_data;

	/* initialize noise spectrum */

	/* Because initial_noise[] is read once and then never used, we can use   */
	/* it for tempV2[] too.                                                   */

	window(initial_noise, sqrt_tukey_256_180, initial_noise, ENH_WINLEN);

	/* ---- Find normalization factor of the input signal ---- */
	/* Starting max with 1 to avoid norm_s(0). */
	max = 1;
	for (i = 0; i < ENH_WINLEN; i++) {
		temp = melpe_abs_s(initial_noise[i]);
		if (temp > max)
			max = temp;
	}
	shift = melpe_norm_s(max);

	/* Now remember the amount of shift, normalize initial_noise[], then      */
	/* process initial_noise[] as Q15. */
	a_shift = melpe_sub(15, shift);

	/* transfer to frequency domain */
	v_zap(ybuf, 2 * ENH_WINLEN + 2);
	for (i = 0; i < ENH_WINLEN; i++)
		ybuf[2 * i] = melpe_shl(initial_noise[i], shift);

	guard = fft_npp(ybuf, 1);
	/* guard = fft(ybuf, ENH_WINLEN, MONE_Q15); */

	/* get rough estimate of lambdaD  */
	temp_yy[0] = melpe_L_shr(melpe_L_mult(ybuf[0], ybuf[0]), 1);
	temp_yy[1] = 0;
	temp_yy[ENH_WINLEN] =
	    melpe_L_shr(melpe_L_mult(ybuf[ENH_WINLEN], ybuf[ENH_WINLEN]), 1);
	temp_yy[ENH_WINLEN + 1] = 0;

	for (i = 2; i < ENH_WINLEN - 1; i += 2) {
		temp_yy[i + 1] = 0;
		temp_yy[i] = melpe_L_shr(melpe_L_add(melpe_L_mult(ybuf[i], ybuf[i]),
					 melpe_L_mult(ybuf[i + 1], ybuf[i + 1])), 1);
	}

	L_sum = temp_yy[0];
	for (i = 1; i < ENH_WINLEN + 1; i++) {
		if (L_sum < temp_yy[i])
			L_sum = temp_yy[i];
	}
	shift = melpe_norm_l(L_sum);
	for (i = 0; i < ENH_WINLEN + 1; i++)
		ybuf[i] = melpe_extract_h(melpe_L_shl(temp_yy[i], shift));
	shift = melpe_sub(melpe_shl(melpe_add(a_shift, guard), 1), melpe_add(shift, 7));

	/* convert to correlation domain */
	for (i = 0; i < ENH_WINLEN / 2 - 1; i++) {
		ybuf[ENH_WINLEN + 2 * i + 2] = ybuf[ENH_WINLEN - 2 * i - 2];
		ybuf[ENH_WINLEN + 2 * i + 3] =
		    melpe_negate(ybuf[ENH_WINLEN - 2 * i - 2 + 1]);
	}
	guard = fft_npp(ybuf, -1);	/* inverse fft */
	/* guard = fft(ybuf, ENH_WINLEN, ONE_Q15); */
	shift = melpe_add(shift, guard);
	shift = melpe_sub(shift, 8);
	for (i = 0; i < ENH_WINLEN; i++)
		initial_noise[i] = ybuf[2 * i];

	max = 0;
	for (i = 0; i < ENH_WINLEN; i++) {
		temp = melpe_abs_s(initial_noise[i]);
		if (temp > max)
			max = temp;
	}
	temp = melpe_norm_s(max);
	shift = melpe_sub(shift, temp);
	for (i = 0; i < ENH_WINLEN; i++)
		initial_noise[i] = melpe_shl(initial_noise[i], temp);

	smoothing_win(initial_noise);

	/* convert back to frequency domain */
	v_zap(ybuf, 2 * ENH_WINLEN + 2);
	for (i = 0; i < ENH_WINLEN; i++)
		ybuf[2 * i] = initial_noise[i];
	guard = fft_npp(ybuf, 1);
	/* guard = fft(ybuf, ENH_WINLEN, MONE_Q15); */
	for (i = 0; i <= (ENH_WINLEN * 2); i += 2) {
		if (ybuf[i] < 0)
			ybuf[i] = 0;
	}
	norm_shift = melpe_add(shift, guard);

	/* rough estimate of lambdaD */
	L_sum = melpe_L_shl(melpe_L_mult(NOISE_B_BY_NSMTH_B, ybuf[0]), 7);
	L_sum = melpe_L_add(L_sum, 2);
	shift = melpe_norm_l(L_sum);
	lambdaD[0] = melpe_extract_h(melpe_L_shl(L_sum, shift));
	lambdaD_shift[0] = melpe_add(norm_shift, melpe_sub(1, shift));
	L_sum = melpe_L_shr(L_sum, 8);

	L_data = melpe_L_shl(melpe_L_mult(NOISE_B_BY_NSMTH_B, ybuf[ENH_WINLEN]), 7);
	L_data = melpe_L_add(L_data, 2);
	shift = melpe_norm_l(L_data);
	lambdaD[ENH_WINLEN / 2] = melpe_extract_h(melpe_L_shl(L_data, shift));
	lambdaD_shift[ENH_WINLEN / 2] = melpe_add(norm_shift, melpe_sub(1, shift));
	L_sum = melpe_L_add(L_sum, melpe_L_shr(L_data, 8));

	for (i = 1; i < ENH_WINLEN / 2; i++) {
		L_data = melpe_L_shl(melpe_L_mult(NOISE_B_BY_NSMTH_B, ybuf[2 * i]), 7);
		L_data = melpe_L_add(L_data, 2);
		shift = melpe_norm_l(L_data);
		lambdaD[i] = melpe_extract_h(melpe_L_shl(L_data, shift));
		lambdaD_shift[i] = melpe_add(norm_shift, melpe_sub(1, shift));
		L_sum = melpe_L_add(L_sum, melpe_L_shr(L_data, 7));
	}

	shift = melpe_norm_l(L_sum);
	n_pwr = melpe_extract_h(melpe_L_shl(L_sum, shift));
	n_pwr_shift = melpe_sub(melpe_add(norm_shift, 1), shift);

	/* compute initial long term SNR; speech signal power depends on
	   the window; the Hanning window is used as a reference here with
	   a squared norm of 96 Q4 */

	temp = 14648;		/* 0.894/2 Q15 */
	shift = 22;		/* exp = 22, sum = 1875000 */
	SN_LT = melpe_divide_s(temp, n_pwr);
	SN_LT_shift = melpe_sub(shift, n_pwr_shift);

	minstat_init();
}

/***************************************************************************/
/*	Subroutine minstat_init: initialization of variables for minimum	   */
/*							 statistics noise estimation				   */
/***************************************************************************/
static void minstat_init()
{
	/* Initialize Minimum Statistics Noise Estimator */
	register int16_t i;
	int16_t shift;
	int32_t L_sum;

	v_equ(smoothedspect, lambdaD, ENH_VEC_LENF);
	v_scale(smoothedspect, ENH_INV_NOISE_BIAS, ENH_VEC_LENF);

	for (i = 0; i < NUM_MINWIN; i++) {
		v_equ(circb[i], smoothedspect, ENH_VEC_LENF);
		v_equ(circb_shift[i], lambdaD_shift, ENH_VEC_LENF);
	}

	v_equ(sm_shift, lambdaD_shift, ENH_VEC_LENF);
	v_equ(act_min, smoothedspect, ENH_VEC_LENF);
	v_equ(act_min_shift, lambdaD_shift, ENH_VEC_LENF);
	v_equ(act_min_sub, smoothedspect, ENH_VEC_LENF);
	v_equ(act_min_sub_shift, lambdaD_shift, ENH_VEC_LENF);
	v_equ(noisespect, smoothedspect, ENH_VEC_LENF);
	v_equ(noise_shift, lambdaD_shift, ENH_VEC_LENF);

	for (i = 0; i < ENH_VEC_LENF; i++) {
		var_sp_av[i] = melpe_mult(smoothedspect[i], 20066);	/* Q14, sqrt(3/2) */
		av_shift[i] = melpe_add(lambdaD_shift[i], 1);
		L_sum = melpe_L_mult(smoothedspect[i], smoothedspect[i]);
		shift = melpe_norm_l(L_sum);
		var_sp_2[i] = melpe_extract_h(melpe_L_shl(L_sum, shift));
		av2_shift[i] = melpe_sub(melpe_shl(lambdaD_shift[i], 1), melpe_sub(shift, 1));
	}

	alphacorr = X09_Q15;
}

/***************************************************************************/
/* Subroutine:		process_frame
**
** Description:     Enhance a given frame of speech
**
** Arguments:
**
**  int16_t   inspeech[]  ---- input speech data buffer (Q0)
**  int16_t   outspeech[] ---- output speech data buffer (Q0)
**
** Return value:    None
**
 ***************************************************************************/
static void process_frame(int16_t inspeech[], int16_t outspeech[])
{
	register int16_t i;
	static BOOLEAN first_time = TRUE;
	static int16_t agal[ENH_VEC_LENF];	/* GainD*Ymag */
	static int16_t agal_shift[ENH_VEC_LENF];
	static int16_t Ksi_min_var = GM_MIN;
	static int16_t qk[ENH_VEC_LENF];	/* probability of noise only, Q15 */
	static int16_t Gain[ENH_VEC_LENF];	/* MMSE LOG STA estimator gain */
	static int16_t YY_LT, YY_LT_shift;
	static int16_t SN_LT0, SN_LT0_shift;
	BOOLEAN n_flag;
	int16_t temp, temp_shift;
	int16_t YY_av, gamma_av, gamma_max, gamma_av_shift, gamma_max_shift;
	int16_t shift, YY_av_shift, max, temp1, temp2, temp3, temp4;
	int16_t max_YY_shift, Y_shift, guard;
	int16_t a_shift, ygal;
	int16_t Ymag[ENH_VEC_LENF];	/* magnitude of current frame */
	int16_t Ymag_shift[ENH_VEC_LENF];
	int16_t GainD[ENH_VEC_LENF];	/* Gain[].*GM[] */
	int16_t analy[ENH_WINLEN];
	int16_t gamaK[ENH_VEC_LENF];	/* a posteriori SNR */
	int16_t gamaK_shift[ENH_VEC_LENF];
	int16_t biased_smoothedspect[ENH_VEC_LENF];	/* weighted smoothed */
	/* spect. for long window */
	int16_t biased_smoothedspect_sub[ENH_VEC_LENF];	/* for subwindow */
	int16_t bias_shift[ENH_VEC_LENF], bias_sub_shift[ENH_VEC_LENF];
	int32_t L_sum, L_max;

	if (first_time) {
		v_zap(agal, ENH_VEC_LENF);
		v_zap(agal_shift, ENH_VEC_LENF);
		fill(ksi, GM_MIN, ENH_VEC_LENF);
		v_zap(ksi_shift, ENH_VEC_LENF);
		fill(qk, ENH_QK_MAX, ENH_VEC_LENF);
		fill(Gain, GM_MIN, ENH_VEC_LENF);
		fill(GainD, GM_MIN, ENH_VEC_LENF);
		YY_LT = 0;
		YY_LT_shift = 0;
		SN_LT0 = SN_LT;
		SN_LT0_shift = SN_LT_shift;

		first_time = FALSE;
	}

	if (enh_i < 50)		/* This ensures enh_i does not overflow. */
		enh_i++;

	/* analysis window */
	window(inspeech, sqrt_tukey_256_180, analy, ENH_WINLEN);	/* analy[] Q0 */

	/* ---- Find normalization factor of the input signal ---- */
	max = 1;		/* max starts with 1 to avoid problems */
	/* with shift = norm_s(max). */
	for (i = 0; i < ENH_WINLEN; i++) {
		temp1 = melpe_abs_s(analy[i]);
		if (temp1 > max)
			max = temp1;
	}
	shift = melpe_norm_s(max);
	/* ---- Now think input is Q15 ---- */
	a_shift = melpe_sub(15, shift);

	/* ------------- Fixed point fft -------------- */
	/* into frequency domain - D->Y, D->YD->Y, YY_av,
	   real and imaginary parts are interleaved */
	for (i = 0; i < ENH_WINLEN; i++)
		analy[i] = melpe_shl(analy[i], shift);

	v_zap(ybuf, 2 * ENH_WINLEN + 2);
	for (i = 0; i < 2 * ENH_WINLEN; i += 2)
		ybuf[i] = analy[i / 2];
	guard = fft_npp(ybuf, 1);
	/* guard = fft(ybuf, ENH_WINLEN, MONE_Q15); */

	/* Previously here we copy ybuf[] to Y[] and process Y[].  In fact this   */
	/* is not necessary because Y[] is not changed before we use ybuf[] and   */
	/* update ybuf[] the next time.                                           */

	/* ---- D->YY shift is compared to original data ---- */
	Y_shift = melpe_add(a_shift, guard);
	YY_av_shift = melpe_shl(Y_shift, 1);

	temp_yy[0] = melpe_L_mult(ybuf[0], ybuf[0]);
	temp_yy[ENH_VEC_LENF - 1] = melpe_L_mult(ybuf[ENH_WINLEN], ybuf[ENH_WINLEN]);

	for (i = 1; i < ENH_VEC_LENF - 1; i++)
		temp_yy[i] = melpe_L_add(melpe_L_mult(ybuf[2 * i], ybuf[2 * i]),
				   melpe_L_mult(ybuf[2 * i + 1], ybuf[2 * i + 1]));

	max_YY_shift = SW_MIN;
	for (i = 0; i < ENH_VEC_LENF; i++) {
		if (temp_yy[i] < 1)
			temp_yy[i] = 1;
		shift = melpe_norm_l(temp_yy[i]);
		YY[i] = melpe_extract_h(melpe_L_shl(temp_yy[i], shift));
		YY_shift[i] = melpe_sub(YY_av_shift, shift);
		if (max_YY_shift < YY_shift[i])
			max_YY_shift = YY_shift[i];
	}

	for (i = 0; i < ENH_VEC_LENF; i++) {
		temp = YY[i];
		temp_shift = YY_shift[i];
		if (temp_shift & 0x0001) {
			temp = melpe_shr(temp, 1);
			temp_shift = melpe_add(temp_shift, 1);
		}
		Ymag[i] = sqrt_Q15(temp);
		Ymag_shift[i] = melpe_shr(temp_shift, 1);
		YY_shift[i] = melpe_sub(YY_shift[i], 8);
	}

	/* ---- Compute YY_av ---- */
	L_sum =
	    melpe_L_shl(melpe_L_deposit_l(YY[0]), melpe_sub(7, melpe_sub(max_YY_shift, YY_shift[0])));
	L_sum =
	    melpe_L_add(L_sum,
		  melpe_L_shl(melpe_L_deposit_l(YY[ENH_VEC_LENF - 1]),
			melpe_sub(7, melpe_sub(max_YY_shift, YY_shift[ENH_VEC_LENF - 1]))));
	for (i = 1; i < ENH_VEC_LENF - 1; i++)
		L_sum = melpe_L_add(L_sum, melpe_L_shl(melpe_L_deposit_l(YY[i]),
					   melpe_sub(8,
					       melpe_sub(max_YY_shift,
						   YY_shift[i]))));
	if (L_sum == 0)
		L_sum = 1;
	temp1 = melpe_norm_l(L_sum);
	YY_av = melpe_extract_h(melpe_L_shl(L_sum, temp1));
	YY_av_shift = melpe_sub(melpe_add(max_YY_shift, 1), temp1);

	/* compute smoothed short time periodogram */
	smoothed_periodogram(YY_av, YY_av_shift);

	/* compute inverse bias and multiply short time periodogram with inverse  */
	/* bias */
	bias_compensation(biased_smoothedspect, bias_shift,
			  biased_smoothedspect_sub, bias_sub_shift);

	/* determine unbiased noise psd estimate by minimum search */
	min_search(biased_smoothedspect, bias_shift, biased_smoothedspect_sub,
		   bias_sub_shift);

	/* compute 'gammas' */
	for (i = 0; i < ENH_VEC_LENF; i++) {
		gamaK[i] = melpe_divide_s(melpe_shr(YY[i], 1), lambdaD[i]);
		gamaK_shift[i] = melpe_sub(melpe_add(YY_shift[i], 1), lambdaD_shift[i]);
	}

	/* compute average of gammas */
	L_sum = melpe_L_shl(melpe_L_deposit_l(gamaK[0]), 7);
	shift = melpe_sub(gamaK_shift[0], 1);
	for (i = 1; i < ENH_VEC_LENF - 1; i++) {
		temp1 = melpe_sub(shift, gamaK_shift[i]);
		if (temp1 > 0)
			L_sum =
			    melpe_L_add(L_sum,
				  melpe_L_shr(melpe_L_deposit_l(gamaK[i]), melpe_sub(temp1, 7)));
		else {
			L_sum = melpe_L_add(melpe_L_shl(L_sum, temp1),
				      melpe_L_shl(melpe_L_deposit_l(gamaK[i]), 7));
			shift = gamaK_shift[i];
		}
	}
	temp1 = melpe_sub(shift, melpe_sub(gamaK_shift[ENH_VEC_LENF - 1], 1));
	if (temp1 > 0)
		L_sum = melpe_L_add(L_sum, melpe_L_shr(melpe_L_deposit_l(gamaK[ENH_VEC_LENF - 1]),
					   melpe_sub(temp1, 7)));
	else {
		L_sum = melpe_L_add(melpe_L_shl(L_sum, temp1),
			      melpe_L_shl(melpe_L_deposit_l(gamaK[ENH_VEC_LENF - 1]), 7));
		shift = melpe_sub(gamaK_shift[ENH_VEC_LENF - 1], 1);
	}
	if (L_sum == 0)
		L_sum = 1;
	temp1 = melpe_norm_l(L_sum);
	gamma_av = melpe_extract_h(melpe_L_shl(L_sum, temp1));
	gamma_av_shift = melpe_add(melpe_sub(shift, temp1), 10 - 8);

	gamma_max = gamaK[0];
	gamma_max_shift = gamaK_shift[0];
	for (i = 1; i < ENH_VEC_LENF; i++) {
		if (comp_data_shift(gamma_max, gamma_max_shift, gamaK[i],
				    gamaK_shift[i]) < 0) {
			gamma_max = gamaK[i];
			gamma_max_shift = gamaK_shift[i];
		}
	}

	/* determine signal presence */
	n_flag = FALSE;		/* default flag - signal present */

	if ((comp_data_shift(gamma_max, gamma_max_shift, GAMMAX_THR, 6) < 0) &&
	    (comp_data_shift(gamma_av, gamma_av_shift, GAMMAV_THR, 1) < 0)) {
		n_flag = TRUE;	/* noise-only */

		temp1 = melpe_mult(n_pwr, GAMMAV_THR);
		temp2 = melpe_add(n_pwr_shift, 1 + 1);
		if (comp_data_shift(YY_av, YY_av_shift, temp1, temp2) > 0)
			n_flag = FALSE;	/* overiding if frame SNR > 3dB (9/98) */
	}

	if (enh_i == 1) {	/* Initial estimation of apriori SNR and Gain */
		for (i = 0; i < ENH_VEC_LENF; i++) {
			temp_yy[i] = melpe_L_mult(Ymag[i], GM_MIN);
			shift = melpe_norm_l(temp_yy[i]);
			agal[i] = melpe_extract_h(melpe_L_shl(temp_yy[i], shift));
			agal_shift[i] = melpe_sub(Ymag_shift[i], shift);
		}
	} else {		/* enh_i > 1 */

		/* estimation of apriori SNR */
		for (i = 0; i < ENH_VEC_LENF; i++) {
			L_sum = melpe_L_mult(agal[i], agal[i]);
			L_sum = L_mpy_ls(L_sum, ENH_ALPHAK);
			if (L_sum < 1)
				L_sum = 1;
			shift = melpe_norm_l(L_sum);
			temp1 = melpe_extract_h(melpe_L_shl(L_sum, shift));
			temp2 = melpe_sub(melpe_shl(agal_shift[i], 1), melpe_add(shift, 8));
			temp3 = lambdaD[i];
			temp4 = lambdaD_shift[i];
			if (melpe_sub(temp3, temp1) < 0) {
				temp1 = melpe_shr(temp1, 1);
				temp2 = (int16_t) (temp2 + 1);
			}
			ksi[i] = melpe_divide_s(temp1, temp3);
			ksi_shift[i] = melpe_sub(temp2, temp4);
			if (comp_data_shift
			    (gamaK[i], gamaK_shift[i], ENH_INV_NOISE_BIAS,
			     0) > 0) {
				L_sum =
				    melpe_L_shr(melpe_L_deposit_h(ENH_INV_NOISE_BIAS),
					  gamaK_shift[i]);
				L_sum = melpe_L_sub(melpe_L_deposit_h(gamaK[i]), L_sum);
				shift = melpe_norm_l(L_sum);
				temp1 = melpe_extract_h(melpe_L_shl(L_sum, shift));
				temp1 = melpe_mult(temp1, ENH_BETAK);	/* note ENH_BETAK is Q18 */
				temp2 = melpe_sub(gamaK_shift[i], melpe_add(shift, 3));
				shift = melpe_sub(ksi_shift[i], temp2);
				if (shift > 0) {
					ksi[i] =
					    melpe_add(melpe_shr(ksi[i], 1),
						melpe_shr(temp1,
						    (int16_t) (shift + 1)));
					ksi_shift[i] = melpe_add(ksi_shift[i], 1);
				} else {
					ksi[i] =
					    melpe_add(melpe_shl
						(ksi[i],
						 (int16_t) (shift - 1)),
						melpe_shr(temp1, 1));
					ksi_shift[i] = melpe_add(temp2, 1);
				}
			}
		}

		/*      Ksi_min_var = 0.9*Ksi_min_var +
		   0.1*ksi_min_adapt(n_flag, ksi_min, SN_LT);          */
		temp1 = melpe_mult(X09_Q15, Ksi_min_var);
		temp2 = melpe_mult(X01_Q15, ksi_min_adapt(n_flag, GM_MIN, SN_LT,
						    SN_LT_shift));
		Ksi_min_var = melpe_add(temp1, temp2);

		shift = melpe_norm_s(Ksi_min_var);
		temp1 = melpe_shl(Ksi_min_var, shift);
		/* ---- limit the bottom of the ksi ---- */
		for (i = 0; i < ENH_VEC_LENF; i++) {
			if (comp_data_shift(ksi[i], ksi_shift[i],
					    temp1, melpe_negate(shift)) < 0) {
				ksi[i] = temp1;
				ksi_shift[i] = melpe_negate(shift);
			}
		}

		/* estimation of k-th component 'signal absence' prob and gain */
		/* default value for qk's % (9/98)      */
		fill(qk, ENH_QK_MAX, ENH_VEC_LENF);

		if (!n_flag) {	/* SIGNAL PRESENT */
			/* computation of the long term SNR */
			if (comp_data_shift
			    (gamma_av, gamma_av_shift, GAMMAV_THR, 1) > 0) {

				/*      YY_LT = YY_LT * alpha_LT + beta_LT * YY_av; */
				L_sum = melpe_L_mult(YY_LT, ENH_ALPHA_LT);
				shift = melpe_norm_l(L_sum);
				temp1 = melpe_extract_h(melpe_L_shl(L_sum, shift));
				temp2 = melpe_sub(YY_LT_shift, shift);
				L_sum = melpe_L_mult(YY_av, ENH_BETA_LT);
				shift = melpe_norm_l(L_sum);
				temp3 = melpe_extract_h(melpe_L_shl(L_sum, shift));
				temp4 = melpe_sub(YY_av_shift, shift);
				temp1 = melpe_shr(temp1, 1);
				temp3 = melpe_shr(temp3, 1);
				shift = melpe_sub(temp2, temp4);
				if (shift > 0) {
					YY_LT = melpe_add(temp1, melpe_shr(temp3, shift));
					YY_LT_shift = temp2;
				} else {
					YY_LT = melpe_add(melpe_shl(temp1, shift), temp3);
					YY_LT_shift = temp4;
				}
				YY_LT_shift = melpe_add(YY_LT_shift, 1);

				/*      SN_LT = (YY_LT/n_pwr) - 1;  Long-term S/N */
				temp1 = melpe_sub(YY_LT, n_pwr);
				if (temp1 > 0) {
					YY_LT = melpe_shr(YY_LT, 1);
					YY_LT_shift = melpe_add(YY_LT_shift, 1);
				}
				SN_LT = melpe_divide_s(YY_LT, n_pwr);
				SN_LT_shift = melpe_sub(YY_LT_shift, n_pwr_shift);

				/*      if (SN_LT < 0); we didn't subtract 1 from SN_LT yet.      */
				if (comp_data_shift
				    (SN_LT, SN_LT_shift, ONE_Q15, 0) < 0) {
					SN_LT = SN_LT0;
					SN_LT_shift = SN_LT0_shift;
				} else {
					temp1 = ONE_Q15;
					L_sum = melpe_L_sub(melpe_L_deposit_h(SN_LT),
						      melpe_L_shr(melpe_L_deposit_h(temp1),
							    SN_LT_shift));
					shift = melpe_norm_l(L_sum);
					SN_LT = melpe_extract_h(melpe_L_shl(L_sum, shift));
					SN_LT_shift = melpe_sub(SN_LT_shift, shift);
				}

				/*      SN_LT0 = SN_LT; */
				SN_LT0 = SN_LT;
				SN_LT0_shift = SN_LT_shift;
			}

			/* Estimating qk's using hard-decision approach (7/98) */
			compute_qk(qk, gamaK, gamaK_shift, ENH_GAMMAQ_THR);

			/*      vec_limit_top(qk, qk, qk_max, vec_lenf); */
			/*      vec_limit_bottom(qk, qk, qk_min, vec_lenf); */
			for (i = 0; i < ENH_VEC_LENF; i++) {
				if (qk[i] > ENH_QK_MAX)
					qk[i] = ENH_QK_MAX;
				else if (qk[i] < ENH_QK_MIN)
					qk[i] = ENH_QK_MIN;
			}

		}

		gain_log_mmse(qk, Gain, gamaK, gamaK_shift, ENH_VEC_LENF);

		/* Previously the gain modification is done outside of gain_mod() and */
		/* now it is moved inside. */

		v_equ(GainD, Gain, ENH_VEC_LENF);
		gain_mod(qk, GainD, ENH_VEC_LENF);

		/*      vec_mult(Agal, GainD, Ymag, vec_lenf); */
		for (i = 0; i < ENH_VEC_LENF; i++) {
			L_sum = melpe_L_mult(GainD[i], Ymag[i]);
			shift = melpe_norm_l(L_sum);
			agal[i] = melpe_extract_h(melpe_L_shl(L_sum, shift));
			agal_shift[i] = melpe_sub(Ymag_shift[i], shift);

		}

	}

	/* enhanced signal in frequency domain */
	/*       (implement ygal = GainD .* Y)   */
	for (i = 0; i < ENH_WINLEN + 2; i++)
		temp_yy[i] = melpe_L_mult(ybuf[i], GainD[i / 2]);
	L_max = 0;
	for (i = 0; i < ENH_WINLEN + 2; i++)
		if (L_max < melpe_L_abs(temp_yy[i]))
			L_max = melpe_L_abs(temp_yy[i]);

	shift = melpe_norm_l(L_max);
	for (i = 0; i < ENH_WINLEN + 2; i++)
		ybuf[i] = melpe_extract_h(melpe_L_shl(temp_yy[i], shift));
	shift = melpe_sub(Y_shift, shift);

	/* transformation to time domain */
	for (i = 0; i < ENH_WINLEN / 2 - 1; i++) {
		ybuf[ENH_WINLEN + 2 * i + 2] = ybuf[ENH_WINLEN - 2 * i - 2];
		ybuf[ENH_WINLEN + 2 * i + 3] =
		    melpe_negate(ybuf[ENH_WINLEN - 2 * i - 2 + 1]);
	}

	guard = fft_npp(ybuf, -1);
	/* guard = fft(ybuf, ENH_WINLEN, ONE_Q15); */
	shift = melpe_add(shift, guard);
	shift = melpe_sub(shift, 8);

#if USE_SYN_WINDOW
	for (i = 0; i < ENH_WINLEN; i++) {
		ygal = melpe_shl(ybuf[2 * i], melpe_sub(shift, 15));
		outspeech[i] = melpe_mult(ygal, sqrt_tukey_256_180[i]);
	}
#endif

	/* Misc. updates */
	max_YY_shift = SW_MIN;
	for (i = 0; i < ENH_VEC_LENF; i++) {
		if (max_YY_shift < lambdaD_shift[i])
			max_YY_shift = lambdaD_shift[i];
	}

	L_sum = melpe_L_shl(melpe_L_deposit_l(lambdaD[0]),
		      melpe_sub(7, melpe_sub(max_YY_shift, lambdaD_shift[0])));
	L_sum = melpe_L_add(L_sum, melpe_L_shl(melpe_L_deposit_l(lambdaD[ENH_VEC_LENF - 1]),
				   melpe_sub(7, melpe_sub(max_YY_shift,
					      lambdaD_shift[ENH_VEC_LENF -
							    1]))));

	for (i = 1; i < ENH_VEC_LENF - 1; i++)
		L_sum = melpe_L_add(L_sum, melpe_L_shl(melpe_L_deposit_l(lambdaD[i]),
					   melpe_sub(8,
					       melpe_sub(max_YY_shift,
						   lambdaD_shift[i]))));
	if (L_sum == 0)
		L_sum = 1;
	shift = melpe_norm_l(L_sum);
	n_pwr = melpe_extract_h(melpe_L_shl(L_sum, shift));
	n_pwr_shift = melpe_add(melpe_sub(max_YY_shift, shift), 1);
}

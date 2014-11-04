/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*

2.4 kbps MELP Proposed Federal Standard speech coder

Fixed-point C code, version 1.0

Copyright (c) 1998, Texas Instruments, Inc.

Texas Instruments has intellectual property rights on the MELP
algorithm.	The Texas Instruments contact for licensing issues for
commercial and non-government use is William Gordon, Director,
Government Contracts, Texas Instruments Incorporated, Semiconductor
Group (phone 972 480 7442).

The fixed-point version of the voice codec Mixed Excitation Linear
Prediction (MELP) is based on specifications on the C-language software
simulation contained in GSM 06.06 which is protected by copyright and
is the property of the European Telecommunications Standards Institute
(ETSI). This standard is available from the ETSI publication office
tel. +33 (0)4 92 94 42 58. ETSI has granted a license to United States
Department of Defense to use the C-language software simulation contained
in GSM 06.06 for the purposes of the development of a fixed-point
version of the voice codec Mixed Excitation Linear Prediction (MELP).
Requests for authorization to make other use of the GSM 06.06 or
otherwise distribute or modify them need to be addressed to the ETSI
Secretariat fax: +33 493 65 47 16.

*/

/* ===================================== */
/* pit_lib.c: pitch analysis subroutines */
/* ===================================== */

#include "sc1200.h"
#include "mathhalf.h"
#include "mathdp31.h"
#include "mat_lib.h"
#include "math_lib.h"
#include "dsp_sub.h"
#include "pit_lib.h"
#include "constant.h"
#include "global.h"
#include "coeff.h"

#define PDECAY_Q15			31129	/* 0.95 * (1 << 15) */
#define PDECAY_PITCH_Q7 	320	/* (0.05*DEFAULT_PITCH) * (1 << 7) */
#define NUM_MULT			8
#define SHORT_PITCH			3840	/* 30 * (1 << 7) */
#define MAXFRAC				16384	/* 2.0 * (1 << 13) */
#define MINFRAC				-8192	/* -1.0 * (1 << 13) */
#define X05_Q13				4096	/* 0.5 * (1 << 13) */

/* Added 1 to variables which appear in comparison statements to make it      */
/* bit-exact as tested version                                                */
#define UVMAX				(9011 + 1)	/* 0.55 * (1 << 14) */
#define PCORR_THR			(9830 + 1)	/* 0.6 * (1 << 14) */
#define PDOUBLE1			96	/* 0.75 * (1 << 7) */
#define PDOUBLE2			64	/* 0.5 * (1 << 7) */
#define PDOUBLE3			115	/* 0.9 * (1 << 7) */
#define PDOUBLE4			89	/* 0.7 * (1 << 7) */
#define LONG_PITCH			12800	/* 100.0 * (1 << 7)) */
#define LPF_ORD_SOS			2

/* Prototypes */

static int16_t double_chk(int16_t sig_in[], int16_t * pcorr,
			    int16_t pitch, int16_t pdouble,
			    int16_t pmin, int16_t pmax,
			    int16_t pmin_q7, int16_t pmax_q7,
			    int16_t lmin);

static void double_ver(int16_t sig_in[], int16_t * pcorr,
		       int16_t pitch, int16_t pmin, int16_t pmax,
		       int16_t pmin_q7, int16_t pmax_q7, int16_t lmin);

/*	double_chk.c: check for pitch doubling and also verify pitch multiple for */
/*                short pitches.                                              */
/*                                                                            */
/* Q values                                                                   */
/*     sig_in - Q0, pcorr - Q14, pitch - Q7, pdouble - Q7                     */

static int16_t double_chk(int16_t sig_in[], int16_t * pcorr,
			    int16_t pitch, int16_t pdouble,
			    int16_t pmin, int16_t pmax,
			    int16_t pmin_q7, int16_t pmax_q7,
			    int16_t lmin)
{
	int16_t mult, corr, thresh, temp_pit;
	int16_t temp1, temp2;
	int32_t L_temp;

	pitch = frac_pch(sig_in, pcorr, pitch, 0, pmin, pmax, pmin_q7, pmax_q7,
			 lmin);

	/* compute threshold Q14*Q7>>8 */
	/* extra right shift to compensate left shift of L_mult */
	L_temp = melpe_L_mult(*pcorr, pdouble);
	L_temp = melpe_L_shr(L_temp, 8);
	thresh = melpe_extract_l(L_temp);	/* Q14 */

	/* Check pitch submultiples from shortest to longest */
	for (mult = NUM_MULT; mult >= 2; mult--) {

		/* temp_pit = pitch / mult */
		temp1 = 0;
		temp2 = melpe_shl(mult, 11);	/* Q11 */
		temp_pit = pitch;
		while (temp_pit > temp2) {
			temp_pit = melpe_shr(temp_pit, 1);
			temp1 = melpe_add(temp1, 1);
		}
		/* Q7*Q15/Q11 -> Q11 */
		temp2 = melpe_divide_s(temp_pit, temp2);
		temp1 = melpe_sub(4, temp1);
		/* temp_pit=pitch/mult in Q7 */
		temp_pit = melpe_shr(temp2, temp1);

		if (temp_pit >= pmin_q7) {
			temp_pit =
			    frac_pch(sig_in, &corr, temp_pit, 0, pmin, pmax,
				     pmin_q7, pmax_q7, lmin);
			double_ver(sig_in, &corr, temp_pit, pmin, pmax, pmin_q7,
				   pmax_q7, lmin);

			/* stop if submultiple greater than threshold */
			if (corr > thresh) {
				/* refine estimate one more time since previous window */
				/* may be off center slightly and temp_pit has moved */
				pitch =
				    frac_pch(sig_in, pcorr, temp_pit, 0, pmin,
					     pmax, pmin_q7, pmax_q7, lmin);
				break;
			}
		}
	}

	/* Verify pitch multiples for short pitches */
	double_ver(sig_in, pcorr, pitch, pmin, pmax, pmin_q7, pmax_q7, lmin);

	/* Return full floating point pitch value and correlation */
	return (pitch);
}

/* double_ver.c: verify pitch multiple for short pitches.                     */
/*                                                                            */
/* Q values                                                                   */
/*      pitch - Q7, pcorr - Q14                                               */

static void double_ver(int16_t sig_in[], int16_t * pcorr,
		       int16_t pitch, int16_t pmin, int16_t pmax,
		       int16_t pmin_q7, int16_t pmax_q7, int16_t lmin)
{
	int16_t multiple;
	int16_t corr, temp_pit;

	/* Verify pitch multiples for short pitches */
	multiple = 1;
	while (melpe_extract_l(melpe_L_shr(melpe_L_mult(pitch, multiple), 1)) < SHORT_PITCH) {
		multiple = melpe_add(multiple, 1);
	}

	if (multiple > 1) {
		temp_pit = melpe_extract_l(melpe_L_shr(melpe_L_mult(pitch, multiple), 1));
		frac_pch(sig_in, &corr, temp_pit, 0, pmin, pmax,
			 pmin_q7, pmax_q7, lmin);

		/* use smaller of two correlation values */
		if (corr < *pcorr) {
			*pcorr = corr;
		}
	}
}

/* f_pitch_scale.c: Scale pitch signal buffer for best precision              */

int16_t f_pitch_scale(int16_t sig_out[], int16_t sig_in[],
			int16_t length)
{
	register int16_t i;
	int16_t scale;
	int16_t *temp_buf;
	int32_t corr;
	int32_t L_temp, L_sum, L_margin;

	/* Compute signal buffer scale factor */
	scale = 0;

	/*      corr = L_v_magsq(sig_in, length, 0, 1); */
	L_sum = 0;
	L_margin = LW_MAX;
	temp_buf = sig_in;
	for (i = 0; i < length; i++) {
		L_temp = melpe_L_mult(*temp_buf, *temp_buf);
		if (L_temp <= L_margin) {
			L_sum = melpe_L_add(L_sum, L_temp);
			L_margin = melpe_L_sub(L_margin, L_temp);
		} else {
			L_margin = LW_MIN;
			break;
		}
		temp_buf++;
	}
	corr = L_sum;

	if (L_margin == LW_MIN) {

		/* allocate scratch buffer */
		temp_buf = v_get(length);

		/* saturation: right shift input signal and try again */
		scale = 5;
		v_equ_shr(temp_buf, sig_in, scale, length);
		corr = L_v_magsq(temp_buf, length, 0, 1);

		/* could add delta to compensate possible truncation error */

		/* free scratch buffer */
		v_free(temp_buf);
	}

	scale = melpe_sub(scale, melpe_shr(melpe_norm_l(corr), 1));

	/* Scale signal buffer */
	v_equ_shr(sig_out, sig_in, scale, length);

	/* return scale factor */
	return (scale);
}

/* find_pitch.c: Determine pitch value.                                       */
/*                                                                            */
/* Q values:                                                                  */
/*      sig_in - Q0, ipitch - Q0, *pcorr - Q14                                */
/*                                                                            */
/* WARNING: this function assumes the input buffer has been normalized by     */
/*          f_pitch_scale().                                                  */

int16_t find_pitch(int16_t sig_in[], int16_t * pcorr, int16_t lower,
		     int16_t upper, int16_t length)
{
	register int16_t i;
	int16_t cbegin, ipitch, even_flag;
	int16_t s_corr, shift1a, shift1b, shift2, shift;
	int32_t c0_0, cT_T, corr;
	int32_t denom, max_denom, num, max_num;

	/* Find beginning of correlation window centered on signal */
	ipitch = lower;
	max_num = 0;
	max_denom = 1;
	even_flag = 1;
	/* cbegin = -((length+upper)/2) */
	cbegin = melpe_negate(melpe_shr(melpe_add(length, upper), 1));

	c0_0 = L_v_magsq(&sig_in[cbegin], length, 0, 1);
	cT_T = L_v_magsq(&sig_in[cbegin + upper], length, 0, 1);

	for (i = upper; i >= lower; i--) {

		/* calculate normalized crosscorrelation */
		corr =
		    L_v_inner(&sig_in[cbegin], &sig_in[cbegin + i], length, 0,
			      0, 1);

		/* calculate normalization for numerator and denominator */
		shift1a = melpe_norm_s(melpe_extract_h(c0_0));
		shift1b = melpe_norm_s(melpe_extract_h(cT_T));
		shift = melpe_add(shift1a, shift1b);
		shift2 = melpe_shr(shift, 1);	/* shift2 = half of total shift */
		if (melpe_shl(shift2, 1) != shift)
			shift1a = melpe_sub(shift1a, 1);

		/* check if current maximum value */
		if (corr > 0) {
			s_corr = melpe_extract_h(melpe_L_shl(corr, shift2));
			num = melpe_extract_h(melpe_L_mult(s_corr, s_corr));
		} else
			num = 0;
		denom = melpe_extract_h(melpe_L_mult(melpe_extract_h(melpe_L_shl(c0_0, shift1a)),
					 melpe_extract_h(melpe_L_shl(cT_T, shift1b))));
		if (denom < 1)
			denom = 1;

		if (melpe_L_mult(melpe_extract_l(num), melpe_extract_l(max_denom)) >
		    melpe_L_mult(melpe_extract_l(max_num), melpe_extract_l(denom))) {
			max_denom = denom;
			max_num = num;
			ipitch = i;
		}

		/* update for next iteration */
		if (even_flag) {
			even_flag = 0;
			c0_0 = melpe_L_msu(c0_0, sig_in[cbegin], sig_in[cbegin]);
			c0_0 = melpe_L_mac(c0_0, sig_in[cbegin + length],
				     sig_in[cbegin + length]);
			cbegin = melpe_add(cbegin, 1);
		} else {
			even_flag = 1;
			cT_T = melpe_L_msu(cT_T, sig_in[cbegin + i - 1 + length],
				     sig_in[cbegin + i - 1 + length]);
			cT_T =
			    melpe_L_mac(cT_T, sig_in[cbegin + i - 1],
				  sig_in[cbegin + i - 1]);
		}

	}

	/* Return pitch value and correlation */
	*pcorr =
	    melpe_shr(sqrt_fxp
		(melpe_divide_s(melpe_extract_l(max_num), melpe_extract_l(max_denom)), 15), 1);

	return (ipitch);
}

/* Name: frac_pch.c                                                           */
/* Description: Determine fractional pitch.                                   */
/* Inputs:                                                                    */
/*    sig_in - input signal                                                   */
/*    fpitch - initial floating point pitch estimate                          */
/*    range - range for local integer pitch search (0=none)                   */
/*    pmin - minimum allowed pitch value                                      */
/*    pmax - maximum allowed pitch value                                      */
/*    lmin - minimum correlation length                                       */
/* Outputs:                                                                   */
/*    pcorr - correlation at fractional pitch value                           */
/* Returns: fpitch - fractional pitch value                                   */
/*                                                                            */
/* Copyright (c) 1995 by Texas Instruments, Inc.  All rights reserved.        */
/*                                                                            */
/* Q values                                                                   */
/*     ipitch - Q0, fpitch - Q7, sig_in - Q0, *pcorr - Q14                    */
/*                                                                            */
/* WARNING: this function assumes the input buffer has been normalized by     */
/* f_pitch_scale().                                                           */

int16_t frac_pch(int16_t sig_in[], int16_t * pcorr, int16_t fpitch,
		   int16_t range, int16_t pmin, int16_t pmax,
		   int16_t pmin_q7, int16_t pmax_q7, int16_t lmin)
{
	int16_t length, cbegin, lower, upper, ipitch;
	int16_t c0_0, c0_T, c0_T1, cT_T, cT_T1, cT1_T1, c0_Tm1;
	int16_t shift1a, shift1b, shift2, shift;
	int16_t frac, frac1, corr;
	int16_t temp;
	int32_t denom, denom1, denom2, denom3, numer;
	int32_t mag_sq;
	int32_t L_temp1;

	/* Perform local integer pitch search for better fpitch estimate */
	if (range > 0) {
		ipitch = melpe_shift_r(fpitch, -7);
		lower = melpe_sub(ipitch, range);
		upper = melpe_add(ipitch, range);
		if (upper > pmax) {
			upper = pmax;
		}
		if (lower < pmin) {
			lower = pmin;
		}
		if (lower < melpe_add(melpe_shr(ipitch, 1), melpe_shr(ipitch, 2))) {
			lower = melpe_add(melpe_shr(ipitch, 1), melpe_shr(ipitch, 2));
		}
		length = ipitch;
		if (length < lmin) {
			length = lmin;
		}
		fpitch =
		    melpe_shl(find_pitch(sig_in, &corr, lower, upper, length), 7);
	}

	/* Estimate needed crosscorrelations */
	ipitch = melpe_shift_r(fpitch, -7);
	if (ipitch >= pmax) {
		ipitch = melpe_sub(pmax, 1);
	}
	length = ipitch;
	if (length < lmin) {
		length = lmin;
	}
	cbegin = melpe_negate(melpe_shr(melpe_add(length, ipitch), 1));

	/* Calculate normalization for numerator and denominator */
	mag_sq = L_v_magsq(&sig_in[cbegin], length, 0, 1);
	shift1a = melpe_norm_s(melpe_extract_h(mag_sq));
	shift1b = melpe_norm_s(melpe_extract_h(L_v_magsq(&sig_in[cbegin + ipitch - 1],
					     (int16_t) (length + 2), 0, 1)));
	shift = melpe_add(shift1a, shift1b);
	shift2 = melpe_shr(shift, 1);	/* shift2 = half of total shift */
	if (melpe_shl(shift2, 1) != shift)
		shift1a = melpe_sub(shift1a, 1);

	/* Calculate correlations with appropriate normalization */
	c0_0 = melpe_extract_h(melpe_L_shl(mag_sq, shift1a));

	c0_T =
	    melpe_extract_h(melpe_L_shl
		      (L_v_inner
		       (&sig_in[cbegin], &sig_in[cbegin + ipitch], length, 0, 0,
			1), shift2));
	c0_T1 =
	    melpe_extract_h(melpe_L_shl
		      (L_v_inner
		       (&sig_in[cbegin], &sig_in[cbegin + ipitch + 1], length,
			0, 0, 1), shift2));
	c0_Tm1 =
	    melpe_extract_h(melpe_L_shl
		      (L_v_inner
		       (&sig_in[cbegin], &sig_in[cbegin + ipitch - 1], length,
			0, 0, 1), shift2));

	if (c0_Tm1 > c0_T1) {
		/* fractional component should be less than 1, so decrement pitch */
		c0_T1 = c0_T;
		c0_T = c0_Tm1;
		ipitch = melpe_sub(ipitch, 1);
	}
	cT_T1 = melpe_extract_h(melpe_L_shl(L_v_inner(&sig_in[cbegin + ipitch],
					  &sig_in[cbegin + ipitch + 1], length,
					  0, 0, 1), shift1b));
	cT_T = melpe_extract_h(melpe_L_shl(L_v_inner(&sig_in[cbegin + ipitch],
					 &sig_in[cbegin + ipitch], length,
					 0, 0, 1), shift1b));
	cT1_T1 = melpe_extract_h(melpe_L_shl(L_v_inner(&sig_in[cbegin + ipitch + 1],
					   &sig_in[cbegin + ipitch + 1], length,
					   0, 0, 1), shift1b));

	/* Find fractional component of pitch within integer range */
	/* frac = Q13 */
	denom = melpe_L_add(melpe_L_mult(c0_T1, melpe_sub(melpe_shr(cT_T, 1), melpe_shr(cT_T1, 1))),
		      melpe_L_mult(c0_T, melpe_sub(melpe_shr(cT1_T1, 1), melpe_shr(cT_T1, 1))));
	numer = melpe_L_sub(melpe_L_shr(melpe_L_mult(c0_T1, cT_T), 1),
		      melpe_L_shr(melpe_L_mult(c0_T, cT_T1), 1));

	L_temp1 = melpe_L_abs(denom);
	if (L_temp1 > 0) {
		if (melpe_L_abs(melpe_L_shr(numer, 2)) > L_temp1) {
			if (((numer > 0) && (denom < 0))
			    || ((numer < 0) && (denom > 0)))
				frac = MINFRAC;
			else
				frac = MAXFRAC;
		} else
			frac = L_divider2(numer, denom, 2, 0);
	} else {
		frac = X05_Q13;
	}
	if (frac > MAXFRAC) {
		frac = MAXFRAC;
	}
	if (frac < MINFRAC) {
		frac = MINFRAC;
	}

	/* Make sure pitch is still within range */
	fpitch = melpe_add(melpe_shl(ipitch, 7), melpe_shr(frac, 6));
	if (fpitch > pmax_q7) {
		fpitch = pmax_q7;
		frac = melpe_shl(melpe_sub(fpitch, melpe_shl(ipitch, 7)), 6);
	}
	if (fpitch < pmin_q7) {
		fpitch = pmin_q7;
		frac = melpe_shl(melpe_sub(fpitch, melpe_shl(ipitch, 7)), 6);
	}

	/* Calculate interpolated correlation strength */
	frac1 = melpe_sub(ONE_Q13, frac);

	/* Calculate denominator */
	denom1 = melpe_L_shr(L_mpy_ls(melpe_L_mult(cT_T, frac1), frac1), 1);	/* Q(X+11) */
	denom2 = L_mpy_ls(melpe_L_mult(cT_T1, frac1), frac);
	denom3 = melpe_L_shr(L_mpy_ls(melpe_L_mult(cT1_T1, frac), frac), 1);	/* Q(X+11) */
	denom = L_mpy_ls(melpe_L_add(melpe_L_add(denom1, denom2), denom3), c0_0);	/* Q(2X-4) */
	temp = L_sqrt_fxp(denom, 0);	/* temp in Q(X-2) */

	/* Calculate numerator */
	L_temp1 = melpe_L_mult(c0_T, frac1);	/* Q(X+14) */
	L_temp1 = melpe_L_mac(L_temp1, c0_T1, frac);
	if (L_temp1 <= 0) {
		corr = 0;
	} else {
		corr = melpe_extract_h(L_temp1);
	}

	/* Q value of *pcorr =           Q(L_temp1)                             X+14
	   - extract_h                            -   16
	   + scale in divide_s()          +   15
	   - Q(temp)                              -(X-2)
	   =   15   */
	if (corr < temp) {
		*pcorr = melpe_shr(melpe_divide_s(corr, temp), 1);
	} else if (temp <= 0) {
		*pcorr = 0;
	} else {
		*pcorr = ONE_Q14;
	}

	/* Return fractional pitch value */
	return (fpitch);
}

/* Name: p_avg_update.c                                                       */
/* Description: Update pitch average value.                                   */
/* Inputs:                                                                    */
/*    pitch - current pitch value                                             */
/*    pcorr - correlation strength at current pitch value                     */
/*    pthresh - pitch correlation threshold                                   */
/* Returns: pitch_avg - updated average pitch value                           */
/*                                                                            */
/* Copyright (c) 1995 by Texas Instruments, Inc.  All rights reserved.        */

int16_t p_avg_update(int16_t pitch, int16_t pcorr, int16_t pthresh)
{
	register int16_t i;
	static int16_t good_pitch[NF];
	static BOOLEAN firstTime = TRUE;
	int16_t pitch_avg, temp;

	if (firstTime) {
		fill(good_pitch, DEFAULT_PITCH_Q7, NF);
		firstTime = FALSE;
	}

	/* Strong correlation: update good pitch array */
	if (pcorr > pthresh) {
		/* We used to insert pitch to good_pitch[0] and shift everything up.  */
		/* Now we insert at good_pitch[NF - 1] and shift everything down.     */

		v_equ(good_pitch, &(good_pitch[1]), NF - 1);
		good_pitch[NF - 1] = pitch;
	} else {		/* Otherwise decay good pitch array to default value */
		for (i = 0; i < NF; i++) {
			/*      good_pitch[i] =
			   (PDECAY*good_pitch[i]) +((1.0-PDECAY)*DEFAULT_PITCH); */
			temp = melpe_mult(PDECAY_Q15, good_pitch[i]);
			good_pitch[i] = melpe_add(temp, PDECAY_PITCH_Q7);
		}
	}

	/* Pitch_avg = median of pitch values */
	pitch_avg = median3(good_pitch);

	return (pitch_avg);
}

/* Name: pitch_ana.c                                                          */
/* Description: Pitch analysis - outputs candidates                           */
/* Inputs:                                                                    */
/*    resid[] - input residual signal                                         */
/*    pitch_est - initial (floating point) pitch estimate                     */
/* Outputs:                                                                   */
/*    pitch_cand - pitch candidates for frame                                 */
/*    pcorr - pitch correlation strengths for candidates                      */
/* Returns: void                                                              */
/* See_Also:                                                                  */
/* Includes:                                                                  */
/*    mat.h                                                                   */
/* Organization:                                                              */
/*    Speech Research, Corporate R&D                                          */
/*    Texas Instruments                                                       */
/* Author: Alan McCree                                                        */
/*                                                                            */
/* Copyright (c) 1995 by Texas Instruments, Inc.  All rights reserved.        */
/*                                                                            */
/* Q values                                                                   */
/*      speech - Q0, resid - Q0, pitch_est - Q7, pitch_avg - Q7, pcorr2 - Q14 */

int16_t pitch_ana(int16_t speech[], int16_t resid[],
		    int16_t pitch_est, int16_t pitch_avg,
		    int16_t * pcorr2)
{
	register int16_t i, section;
	static int16_t lpres_delin[LPF_ORD];
	static int16_t lpres_delout[LPF_ORD];
	static int16_t sigbuf[LPF_ORD + PITCH_FR];
	static BOOLEAN firstTime = TRUE;
	int16_t pcorr, pitch;
	int16_t temp, temp2;
	int16_t temp_delin[LPF_ORD], temp_delout[LPF_ORD];

	if (firstTime) {
		v_zap(lpres_delin, LPF_ORD);
		v_zap(lpres_delout, LPF_ORD);
		firstTime = FALSE;
	}

	/* Lowpass filter residual signal */
	v_equ(&sigbuf[LPF_ORD_SOS], &resid[-PITCHMAX], PITCH_FR);

	for (section = 0; section < LPF_ORD / 2; section++) {
		iir_2nd_s(&sigbuf[LPF_ORD_SOS], &lpf_den[section * 3],
			  &lpf_num[section * 3], &sigbuf[LPF_ORD_SOS],
			  &lpres_delin[section * 2], &lpres_delout[section * 2],
			  FRAME);
		/* save delay buffers for the next overlapping frame */
		for (i = (int16_t) (section * 2);
		     i < (int16_t) (section * 2 + 2); i++) {
			temp_delin[i] = lpres_delin[i];
			temp_delout[i] = lpres_delout[i];
		}
		iir_2nd_s(&sigbuf[LPF_ORD_SOS + FRAME], &lpf_den[section * 3],
			  &lpf_num[section * 3], &sigbuf[LPF_ORD_SOS + FRAME],
			  &lpres_delin[section * 2], &lpres_delout[section * 2],
			  PITCH_FR - FRAME);
		/* restore delay buffers for the next overlapping frame */
		for (i = (int16_t) (section * 2);
		     i < (int16_t) (section * 2 + 2); i++) {
			lpres_delin[i] = temp_delin[i];
			lpres_delout[i] = temp_delout[i];
		}
	}

	/* Scale lowpass residual for pitch correlations */
	f_pitch_scale(&sigbuf[LPF_ORD_SOS], &sigbuf[LPF_ORD_SOS], PITCH_FR);

	/* Perform local search ar_ound pitch estimate */
	temp =
	    frac_pch(&sigbuf[LPF_ORD_SOS + (PITCH_FR / 2)], &pcorr, pitch_est,
		     5, PITCHMIN, PITCHMAX, PITCHMIN_Q7, PITCHMAX_Q7,
		     MINLENGTH);

	if (pcorr < PCORR_THR) {

		/* If correlation is too low, try speech signal instead */
		v_equ(&sigbuf[LPF_ORD], &speech[-PITCHMAX], PITCH_FR);

		/* Scale speech for pitch correlations */
		f_pitch_scale(&sigbuf[LPF_ORD_SOS], &sigbuf[LPF_ORD_SOS],
			      PITCH_FR);

		temp =
		    frac_pch(&sigbuf[LPF_ORD + (PITCH_FR / 2)], &pcorr,
			     pitch_est, 0, PITCHMIN, PITCHMAX, PITCHMIN_Q7,
			     PITCHMAX_Q7, MINLENGTH);

		if (pcorr < UVMAX)	/* If correlation still too low, use average pitch */
			pitch = pitch_avg;
		else {
			/* Else check for pitch doubling (speech thresholds) */
			if (temp > LONG_PITCH)
				/* longer pitches are more */
				/* likely to be doubles */
				temp2 = PDOUBLE4;
			else
				temp2 = PDOUBLE3;
			pitch =
			    double_chk(&sigbuf[LPF_ORD + (PITCH_FR / 2)],
				       &pcorr, temp, temp2, PITCHMIN, PITCHMAX,
				       PITCHMIN_Q7, PITCHMAX_Q7, MINLENGTH);
		}
	} else {

		/* Else check for pitch doubling (residual thresholds) */
		if (temp > LONG_PITCH)
			/* longer pitches are more */
			/* likely to be doubles */
			temp2 = PDOUBLE2;
		else
			temp2 = PDOUBLE1;
		pitch = double_chk(&sigbuf[LPF_ORD + (PITCH_FR / 2)], &pcorr,
				   temp, temp2, PITCHMIN, PITCHMAX, PITCHMIN_Q7,
				   PITCHMAX_Q7, MINLENGTH);
	}

	if (pcorr < UVMAX)	/* If correlation still too low, use average pitch */
		pitch = pitch_avg;

	/* Return pitch and set correlation strength */
	*pcorr2 = pcorr;
	return (pitch);
}

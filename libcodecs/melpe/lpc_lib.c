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

/* =========================== */
/* lpc_lib.c: LPC subroutines. */
/* =========================== */

/*	compiler include files	*/

#include <stdio.h>

#include "sc1200.h"
#include "macro.h"
#include "mathhalf.h"
#include "mathdp31.h"
#include "math_lib.h"
#include "mat_lib.h"
#include "lpc_lib.h"
#include "constant.h"
#include "global.h"
#include "dsp_sub.h"

#define ALMOST_ONE_Q14		16382	/* ((1 << 14)-2) */
#define ONE_Q25				33554431L	/* (1 << 25) */
#define ONE_Q26				67108864L	/* (1 << 26) */
#define LOW_LIMIT			54	/* lower limit of return value for lpc_aejw() */
				 /* to prevent overflow of weighting function */
#define MAX_LOOPS			10
#define DFTLENGTH			512
#define DFTLENGTH_D2		(DFTLENGTH/2)
#define DFTLENGTH_D4		(DFTLENGTH/4)

/* Prototype */

static void lsp_to_freq(Shortword lsp[], Shortword freq[], Shortword order);
static Shortword lpc_refl2pred(Shortword refc[], Shortword lpc[],
			       Shortword order);

/* LPC_ACOR                                                                   */
/*		Compute autocorrelations based on windowed speech frame               */
/*                                                                            */
/*	Synopsis: lpc_acor(input, window, r, hf_correction, order, npts)          */
/*		Input:                                                                */
/*			input- input vector (npts samples, s[0..npts-1])                  */
/*			win_cof- window vector (npts samples, s[0..npts-1])               */
/*			hf_correction- high frequency correction value                    */
/*			order- order of lpc filter                                        */
/*			npts- number of elements in window                                */
/*		Output:                                                               */
/*			autocorr- output autocorrelation vector (order + 1 samples,       */
/*                    autocorr[0..order])                                     */
/*                                                                            */
/*	Q values: input - Q0, win_cof - Q15, hf_correction - Q15                  */

void lpc_acor(Shortword input[], const Shortword win_cof[],
	      Shortword autocorr[], Shortword hf_correction, Shortword order,
	      Shortword npts)
{
	/* Lag window coefficients */
	static const Shortword lagw_cof[EN_FILTER_ORDER - 1] = {
		32756, 32721, 32663, 32582, 32478, 32351, 32201, 32030, 31837,
		    31622,
		31387, 31131, 30855, 30560, 30246, 29914
	};
	register Shortword i, j;
	Longword L_temp;
	Shortword *inputw;
	Shortword norm_var, scale_fact, temp;

	/* window optimized for speed and readability.  does windowing and        */
	/* autocorrelation sequentially and in the usual manner                   */

	inputw = v_get(npts);
	for (i = 0; i < npts; i++) {
		inputw[i] = mult(win_cof[i], shr(input[i], 4));
	}

	/* Find scaling factor */
	L_temp = L_v_magsq(inputw, npts, 0, 1);
	if (L_temp) {
		norm_var = norm_l(L_temp);
		norm_var = sub(4, shr(norm_var, 1));
		if (norm_var < 0)
			norm_var = 0;
	} else
		norm_var = 0;

	for (i = 0; i < npts; i++) {
		inputw[i] = shr(mult(win_cof[i], input[i]), norm_var);
	}

	/* Compute r[0] */
	L_temp = L_v_magsq(inputw, npts, 0, 1);
	if (L_temp > 0) {
		/* normalize with 1 bit of headroom */
		norm_var = norm_l(L_temp);
		norm_var = sub(norm_var, 1);
		L_temp = L_shl(L_temp, norm_var);

		/* High frequency correction */
		L_temp = L_add(L_temp, L_mpy_ls(L_temp, hf_correction));

		/* normalize result */
		temp = norm_s(extract_h(L_temp));
		L_temp = L_shl(L_temp, temp);
		norm_var = add(norm_var, temp);
		autocorr[0] = r_ound(L_temp);

		/* Multiply by 1/autocorr[0] for full normalization */
		scale_fact = divide_s(ALMOST_ONE_Q14, autocorr[0]);
		L_temp = L_shl(L_mpy_ls(L_temp, scale_fact), 1);
		autocorr[0] = r_ound(L_temp);

	} else {
		norm_var = 0;
		autocorr[0] = ONE_Q15;	/* 1 in Q15 */
		scale_fact = 0;
	}

	/* Compute remaining autocorrelation terms */
	for (j = 1; j <= order; j++) {
		L_temp = 0;
		for (i = j; i < npts; i++)
			L_temp = L_mac(L_temp, inputw[i], inputw[i - j]);
		L_temp = L_shl(L_temp, norm_var);

		/* Scaling */
		L_temp = L_shl(L_mpy_ls(L_temp, scale_fact), 1);

		/* Lag windowing */
		L_temp = L_mpy_ls(L_temp, lagw_cof[j - 1]);

		autocorr[j] = r_ound(L_temp);
	}
	v_free(inputw);
}

/* Name: lpc_aejw- Compute square of A(z) evaluated at exp(jw)                */
/* Description:                                                               */
/*		Compute the magnitude squared of the z-transform of                   */
/*                                                                            */
/*		A(z) = 1 + a(1)z^-1 + ... + a(p)z^-p                                  */
/*                                                                            */
/*		evaluated at z = exp(jw)                                              */
/*	 Inputs:                                                                  */
/*		lpc (a)- LPC filter (a[0] is undefined, a[1..p])                      */
/*		omega (w)- radian frequency                                           */
/*		order (p)- predictor order                                            */
/*	 Returns:                                                                 */
/* 		|A(exp(jw))|^2                                                        */
/*	 See_Also: cos(3), sin(3)                                                 */
/*	 Includes:                                                                */
/*		lpc.h                                                                 */
/*	 Systems and Info. Science Lab                                            */
/*	 Copyright (c) 1995 by Texas Instruments, Inc.	All rights reserved.      */
/*                                                                            */
/* Q values:                                                                  */
/*      lpc - Q12, omega - Q15, return - Q19                                  */

Longword lpc_aejw(Shortword lpc[], Shortword omega, Shortword order)
{
	register Shortword i;
	Shortword c_re, c_im;
	Shortword cs, sn, temp;
	Shortword temp1, temp2;
	Longword L_temp;

	if (order == 0)
		return ((Longword) ONE_Q19);

	/* use horners method                                                     */
	/* A(exp(jw)) = 1+ e(-jw)[a(1) + e(-jw)[a(2) + e(-jw)[a(3) +..            */
	/*                              ...[a(p-1) + e(-jw)a(p)]]]]                               */

	cs = cos_fxp(omega);	/* Q15 */
	sn = negate(sin_fxp(omega));	/* Q15 */

	temp1 = lpc[order - 1];
	c_re = shr(mult(cs, temp1), 3);	/* -> Q9 */
	c_im = shr(mult(sn, temp1), 3);	/* -> Q9 */

	for (i = sub(order, 2); i >= 0; i--) {
		/* add a[i] */
		temp = shr(lpc[i], 3);
		c_re = add(c_re, temp);

		/* multiply by exp(-jw) */
		temp = c_im;
		temp1 = mult(cs, temp);	/* temp1 in Q9 */
		temp2 = mult(sn, c_re);	/* temp2 in Q9 */
		c_im = add(temp1, temp2);
		temp1 = mult(cs, c_re);	/* temp1 in Q9 */
		temp2 = mult(sn, temp);	/* temp2 in Q9 */
		c_re = sub(temp1, temp2);
	}

	/* add one */
	c_re = add(c_re, ONE_Q9);

	/* L_temp in Q19 */
	L_temp = L_add(L_mult(c_re, c_re), L_mult(c_im, c_im));
	if (L_temp < LOW_LIMIT)
		L_temp = (Longword) LOW_LIMIT;

	return (L_temp);
}

/* Name: lpc_bwex- Move the zeros of A(z) toward the origin.                  */
/*	Aliases: lpc_bw_expand                                                    */
/*	Description:                                                              */
/*		Expand the zeros of the LPC filter by gamma, which                    */
/*		moves each zero radially into the origin.                             */
/*                                                                            */
/*		for j = 1 to p                                                        */
/*			aw[j] = a[j]*gamma^j                                              */
/*		(Can also be used to perform an exponential windowing procedure).     */
/*	Inputs:                                                                   */
/*		lpc (a)- lpc vector (order p, a[1..p])                                */
/*		gamma- the bandwidth expansion factor                                 */
/*		order (p)- order of lpc filter                                        */
/*	Outputs:                                                                  */
/*		aw- the bandwidth expanded LPC filter                                 */
/*	Returns: NULL                                                             */
/*	See_Also: lpc_lagw(3l)                                                    */
/*	Includes:                                                                 */
/*		lpc.h                                                                 */
/*                                                                            */
/*	Systems and Info. Science Lab                                             */
/*	Copyright (c) 1995 by Texas Instruments, Inc.  All rights reserved.       */
/*                                                                            */
/*	Q values: lpc[], aw[] - Q12, gamma - Q15, gk - Q15                        */

Shortword lpc_bwex(Shortword lpc[], Shortword aw[], Shortword gamma,
		   Shortword order)
{
	register Shortword i;
	Shortword gk;		/* gk is Q15 */

	gk = gamma;

	for (i = 0; i < order; i++) {
		aw[i] = mult(lpc[i], gk);
		gk = mult(gk, gamma);
	}
	return (0);
}

/* Name: lpc_clmp- Sort and ensure minimum separation in LSPs.                */
/*	Aliases: lpc_clamp                                                        */
/*	Description:                                                              */
/*		Ensure that all LSPs are ordered and separated                        */
/*		by at least delta.	The algorithm isn't guarenteed                    */
/*		to work, so it prints an error message when it fails                  */
/*		to sort the LSPs properly.                                            */
/*	Inputs:                                                                   */
/*		lsp (w)- lsp vector (order p, w[1..p])                                */
/*		delta- the clamping factor                                            */
/*		order (p)- order of lpc filter                                        */
/*	Outputs:                                                                  */
/*		lsp (w)- the sorted and clamped lsps                                  */
/*	Returns: NULL                                                             */
/*	See_Also:                                                                 */
/*	Includes:                                                                 */
/*		lpc.h                                                                 */
/*	Bugs:                                                                     */
/*		Currently only supports 10 loops, which is too                        */
/*		complex and perhaps unneccesary.                                      */
/*                                                                            */
/*	Systems and Info. Science Lab                                             */
/*	Copyright (c) 1995 by Texas Instruments, Inc.  All rights reserved.       */
/*                                                                            */
/*	Q values: lsp - Q15, delta - Q15                                          */

Shortword lpc_clmp(Shortword lsp[], Shortword delta, Shortword order)
{
	register Shortword i, j;
	BOOLEAN unsorted;
	Shortword temp, d, step1, step2;

	/* sort the LSPs for 10 loops */
	for (j = 0, unsorted = TRUE; unsorted && (j < MAX_LOOPS); j++) {
		for (i = 0, unsorted = FALSE; i < order - 1; i++)
			if (lsp[i] > lsp[i + 1]) {
				temp = lsp[i + 1];
				lsp[i + 1] = lsp[i];
				lsp[i] = temp;
				unsorted = TRUE;
			}
	}

	/* ensure minimum separation */
	if (!unsorted) {
		for (j = 0; j < MAX_LOOPS; j++) {
			for (i = 0; i < order - 1; i++) {
				d = sub(lsp[i + 1], lsp[i]);
				if (d < delta) {
					step1 = step2 = shr(sub(delta, d), 1);

/* --> */ if (i == 0
						      && (lsp[i] < delta)) {
						step1 = shr(lsp[i], 1);
					} else {
/* --> */ if (i > 0) {
							temp =
							    sub(lsp[i],
								lsp[i - 1]);
							if (temp < delta) {
								step1 = 0;
							} else {
								if (temp <
								    shl(delta,
									1))
									step1 =
									    shr
									    (sub
									     (temp,
									      delta),
									     1);
							}
						}
					}

/* --> */ if (i == (order - 2)
						      &&
						      (lsp
											       [i
																	+
																	1]
											       >
											       sub
											       (ONE_Q15,
																	delta)))
					{
						step2 =
						    shr(sub
							(ONE_Q15, lsp[i + 1]),
							1);
					} else {
/* --> */ if (i < (order - 2)) {
							temp =
							    sub(lsp[i + 2],
								lsp[i + 1]);
							if (temp < delta) {
								step2 = 0;
							} else {
								if (temp <
								    shl(delta,
									1))
									step2 =
									    shr
									    (sub
									     (temp,
									      delta),
									     1);
							}
						}
					}
					lsp[i] = sub(lsp[i], step1);
					lsp[i + 1] = add(lsp[i + 1], step2);
				}
			}
		}
	}

	/* Debug: check if the minimum separation rule was met */
	/* temp = 0.99*delta */
	/*
	   temp = mult(32440, delta);
	   for (i = 0; i < order - 1; i++)
	   if ((lsp[i + 1] - lsp[i]) < temp)
	   fprintf(stderr, "%s: LSPs not separated enough (line %d)\n",
	   __FILE__, __LINE__);

	   if (unsorted)
	   fprintf(stderr, "%s: Fxp LSPs still unsorted (line %d)\n",
	   __FILE__, __LINE__);
	 */
	return (0);
}

/* Name: lpc_schr- Schur recursion (autocorrelations to refl coef)            */
/*	Aliases: lpc_schur                                                        */
/*	Description:                                                              */
/*		Compute reflection coefficients from autocorrelations                 */
/*		based on schur recursion.  Will also compute predictor                */
/*		parameters by calling lpc_refl2pred(3l) if necessary.                 */
/*	Inputs:                                                                   */
/*		autocorr- autocorrelation vector (autocorr[0..p]).                    */
/*		order- order of lpc filter.                                           */
/*	Outputs:                                                                  */
/*		lpc-   predictor parameters    (can be NULL)                          */
/*	Returns:                                                                  */
/*		alphap- the minimum residual energy                                   */
/*	Includes:                                                                 */
/*		lpc.h                                                                 */
/*	See_Also:                                                                 */
/*		lpc_refl2pred(3l) in lpc.h or lpc(3l)                                 */
/*                                                                            */
/*	Q values:                                                                 */
/*	autocorr - Q0, lpc - Q12,                                                 */

/* Previously the output reflection coefficients refc[] is now changed to     */
/* local dynamic arrays because the calling environment does not need it nor  */
/* use it.                                                                    */

Shortword lpc_schr(Shortword autocorr[], Shortword lpc[], Shortword order)
{
	register Shortword i, j;
	Shortword shift, alphap;
	Shortword *refc;	/* Q15 */
	Longword L_temp, *y1, *y2;
	Shortword temp1, temp2;

	y1 = L_v_get(order);
	y2 = L_v_get((Shortword) (order + 1));
	refc = v_get(order);

	temp2 = abs_s(autocorr[1]);
	temp1 = abs_s(autocorr[0]);

	refc[0] = divide_s(temp2, temp1);

	/* if (((autocorr[1] < 0) && (autocorr[0] < 0)) ||
	   ((autocorr[1] > 0) && (autocorr[0] > 0))) */
	if ((autocorr[1] ^ autocorr[0]) >= 0) {
		refc[0] = negate(refc[0]);
	}
	mult(autocorr[0], sub(ONE_Q15, mult(refc[0], refc[0])));

	y2[0] = L_deposit_h(autocorr[1]);
	y2[1] = L_add(L_deposit_h(autocorr[0]), L_mult(refc[0], autocorr[1]));

	for (i = 1; i < order; i++) {
		y1[0] = L_deposit_h(autocorr[i + 1]);
		L_temp = L_deposit_h(autocorr[i + 1]);

		for (j = 0; j < i; j++) {
			y1[j + 1] = L_add(y2[j], L_mpy_ls(L_temp, refc[j]));
			L_temp = L_add(L_temp, L_mpy_ls(y2[j], refc[j]));
		}

		/*      refc[i] = -temp/y2[i]; */
		/* Under normal conditions the condition for the following IF         */
		/* statement should never be true.                                    */

		if (L_temp > y2[i]) {
			v_zap(&(refc[i]), (Shortword) (order - i));
			break;
		}

		shift = norm_l(y2[i]);
		temp1 = abs_s(extract_h(L_shl(L_temp, shift)));
		temp2 = abs_s(extract_h(L_shl(y2[i], shift)));

		refc[i] = divide_s(temp1, temp2);

		if ((L_temp ^ y2[i]) >= 0) {
			refc[i] = negate(refc[i]);
		}

		y2[i + 1] = L_add(y2[i], L_mpy_ls(L_temp, refc[i]));
		L_v_equ(y2, y1, (Shortword) (i + 1));
	}

	lpc_refl2pred(refc, lpc, order);

	alphap = autocorr[0];
	for (i = 0; i < order; i++) {
		alphap = mult(alphap, sub(ONE_Q15, mult(refc[i], refc[i])));
	}

	v_free(refc);
	v_free(y2);
	v_free(y1);

	return (alphap);	/* alhap in Q15 */
}

/* LPC_REFL2PRED                                                              */
/*	  get predictor coefficients from the reflection coeffs                   */
/* Synopsis: lpc_refl2pred(refc, lpc, order)                                  */
/*                                                                            */
/*	  Input:                                                                  */
/*		 refc- the reflection coeffs                                          */
/*		 order- the predictor order                                           */
/*	  Output:                                                                 */
/*		 lpc- the predictor coefficients                                      */
/* Reference:  Markel and Gray, Linear Prediction of Speech                   */
/*                                                                            */
/* Q values:                                                                  */
/* refc - Q15, lpc - Q12                                                      */

static Shortword lpc_refl2pred(Shortword refc[], Shortword lpc[],
			       Shortword order)
{
	register Shortword i, j;
	Shortword *a1;

	a1 = v_get((Shortword) (order - 1));

	for (i = 0; i < order; i++) {
		/* refl to a recursion */
		lpc[i] = shift_r(refc[i], -3);	/* lpc in Q12 */
		v_equ(a1, lpc, i);
		for (j = 0; j < i; j++) {
			lpc[j] = add(a1[j], mult(refc[i], a1[i - j - 1]));
		}
	}

	v_free(a1);
	return (0);
}

/* LPC_PRED2LSP                                                               */
/*	  get LSP coeffs from the predictor coeffs                                */
/*	  Input:                                                                  */
/*		 lpc- the predictor coefficients                                      */
/*		 order- the predictor order                                           */
/*	  Output:                                                                 */
/*		 lsf- the lsp coefficients                                            */
/*                                                                            */
/*	  This function uses a DFT to evaluate the P and Q polynomials,           */
/*	  and is hard-coded to work only for 10th order LPC.                      */
/*                                                                            */
/* Q values:                                                                  */
/* lpc - Q12, lsf - Q15                                                       */

Shortword lpc_pred2lsp(Shortword lpc[], Shortword lsf[], Shortword order)
{
	register Shortword i;
	Shortword p_cof[LPC_ORD / 2 + 1], q_cof[LPC_ORD / 2 + 1],
	    p_freq[LPC_ORD / 2 + 1], q_freq[LPC_ORD / 2 + 1];
	Longword L_p_cof[LPC_ORD / 2 + 1], L_q_cof[LPC_ORD / 2 + 1];
	Longword L_ai, L_api, L_temp;
	Shortword p2;

	p2 = shr(order, 1);

	/* Generate P' and Q' polynomials.  We only compute for indices from 0 to */
	/* p2 = order/2 because lsp_to_freq() only uses p_cof[] and q_cof[] for   */
	/* for these indices, and hence L_p_cof[] and L_q_cof[] are needed only   */
	/* from 0 to order/2.                                                     */

	L_p_cof[0] = (Longword) ONE_Q26;
	L_q_cof[0] = (Longword) ONE_Q26;
	for (i = 1; i <= p2; i++) {
		/*      temp = sub(lpc[i - 1], lpc[order - i]); *//* temp in Q12 */
		L_ai = L_shr(L_deposit_h(lpc[i - 1]), 2);
		L_api = L_shr(L_deposit_h(lpc[order - i]), 2);
		L_temp = L_sub(L_ai, L_api);	/* L_temp in Q26 */
		L_p_cof[i] = L_add(L_temp, L_p_cof[i - 1]);	/* L_p_cof in Q26 */
		/*      temp = add(lpc[i - 1], lpc[order - i]); */
		L_temp = L_add(L_ai, L_api);
		L_q_cof[i] = L_sub(L_temp, L_q_cof[i - 1]);	/* L_q_cof in Q26 */
	}

	/* Convert p_cof and q_cof to short.  We only compute for indices from 0  */
	/* to p2 = order/2 because lsp_to_freq() only uses p_cof[] and q_cof[]    */
	/* for these indices.                                                     */

	for (i = 0; i <= p2; i++) {
		p_cof[i] = r_ound(L_p_cof[i]);	/* p_cof in Q10 */
		q_cof[i] = r_ound(L_q_cof[i]);	/* q_cof in Q10 */
	}

	/* Find root frequencies of LSP polynomials */
	lsp_to_freq(p_cof, p_freq, order);
	lsp_to_freq(q_cof, q_freq, order);

	/* Combine frequencies into single array */

	for (i = 0; i < p2; i++) {
		lsf[2 * i] = q_freq[i];
		lsf[2 * i + 1] = p_freq[i];
	}

	return (0);
}

/* Subroutine LSP_TO_FREQ: Calculate line spectrum pair	root frequencies from */
/* LSP polynomial.  Only lsp[0], ...... lsp[order/2] are used and the other   */
/* lsp[]'s are ignored.  Similarly, only freq[0], ...... freq[order/2] are    */
/* computed.                                                                  */
/*                                                                            */
/* Q values:                                                                  */
/* lsp - Q10, freq - Q15                                                      */

static void lsp_to_freq(Shortword lsp[], Shortword freq[], Shortword order)
{
	register Shortword i, j;
	static BOOLEAN firstTime = TRUE;
	static Shortword lsp_cos[DFTLENGTH];	/* cosine table */
	static Shortword default_w, default_w0;
	Shortword p2, count;
	BOOLEAN prev_less;
	Longword mag[3];
	Shortword s_mag[3];
	Shortword p_cos;
	Shortword temp1, temp2;
	Longword L_temp1, L_temp2;

	if (firstTime) {
		/* for (i = 0; i < DFTLENGTH; i++)
		   lsp_cos[i] = cos(i*(TWOPI / DFTLENGTH)); */

		/* cos_fxp() takes Q15 input.  (TWO/DFTLENGTH) above is DFTLENGTH_D4  */
		/* in Q15.  The first for loop fills lsp_cos[] in the first quadrant, */
		/* and the next loop fills lsp_cos[] for the other three quadrants.   */

		temp1 = 0;
		for (i = 0; i <= DFTLENGTH_D4; i++) {
			lsp_cos[i] = cos_fxp(temp1);
			lsp_cos[i + DFTLENGTH_D2] = negate(lsp_cos[i]);
			temp1 = add(temp1, DFTLENGTH_D4);
		}
		temp1 = DFTLENGTH_D4;
		temp2 = DFTLENGTH_D4;
		for (i = 0; i < DFTLENGTH_D4; i++) {
			lsp_cos[temp1] = negate(lsp_cos[temp2]);
			lsp_cos[temp1 + DFTLENGTH_D2] = lsp_cos[temp2];
			temp1 = add(temp1, 1);
			temp2 = sub(temp2, 1);
		}

		/* compute default values for freq[] */
		/* (1./p2) in Q15 */
		default_w = divide_s(ONE_Q11, shl(order, 10));
		/* freq[0] = (0.5/p2) in Q15 */
		default_w0 = shr(default_w, 1);

		firstTime = FALSE;
	}

	prev_less = TRUE;
	L_fill(mag, 0x7fffffff, 2);
	fill(s_mag, 0x7fff, 2);

	/*      p2 = p/2; */
	p2 = shr(order, 1);
	count = 0;

	/*      Search all frequencies for minima of Pc(w) */
	for (i = 0; i <= DFTLENGTH_D2; i++) {
		p_cos = i;
		/* mag2 = 0.5 * lsp[p2]; */
		L_temp2 = L_mult(lsp[p2], X05_Q14);	/* mag[2] in Q25 */
		for (j = sub(p2, 1); j >= 0; j--) {
			L_temp1 = L_shr(L_mult(lsp[j], lsp_cos[p_cos]), 1);
			L_temp2 = L_add(L_temp2, L_temp1);
			p_cos = add(p_cos, i);
			if (p_cos > DFTLENGTH - 1)
				p_cos -= DFTLENGTH;
		}
		s_mag[2] = extract_h(L_temp2);
		mag[2] = L_abs(L_temp2);

		if (mag[2] < mag[1]) {
			prev_less = TRUE;
		} else {
			if (prev_less) {
				if ((s_mag[0] ^ s_mag[2]) < 0) {
					/* Minimum frequency found */
					/*      freq[count] = i - 1 + (0.5 *
					   (mag[0] - mag[2]) / (mag[0] + mag[2] - 2*mag[1]));
					   freq[count] *= (2. / DFTLENGTH) ; */
					L_temp1 =
					    L_shr(L_sub(mag[0], mag[2]), 1);
					L_temp2 =
					    L_sub(mag[0], L_shl(mag[1], 1));
					L_temp2 = L_add(L_temp2, mag[2]);
					temp1 =
					    L_divider2(L_temp1, L_temp2, 0, 0);
					/* temp1 in Q15 */
					temp1 = shr(temp1, 9);	/* Q6 */
					temp2 = sub(i, 1);
					temp1 = add(shl(temp2, 6), temp1);
					freq[count] =
					    divide_s(temp1, shl(DFTLENGTH, 5));
					count = add(count, 1);
				}
			}
			prev_less = FALSE;
		}
		L_v_equ(mag, &(mag[1]), 2);
		v_equ(s_mag, &(s_mag[1]), 2);
	}

	/* Verify that all roots were found.  Under normal conditions the condi-  */
	/* tion for the following IF statement should never be true.              */

	if (count != p2) {
		/* use default values */
		freq[0] = default_w0;
		for (i = 1; i < p2; i++)
			freq[i] = add(freq[i - 1], default_w);
	}
}

/* LPC_PRED2REFL                                                              */
/*	  get refl coeffs from the predictor coeffs                               */
/*	  Input:                                                                  */
/*		 lpc- the predictor coefficients                                      */
/*		 order- the predictor order                                           */
/*	  Output:                                                                 */
/*		 refc- the reflection coefficients                                    */
/*	  Returns:                                                                */
/*		 energy - energy of residual signal                                   */
/* Reference:  Markel and Gray, Linear Prediction of Speech                   */
/*                                                                            */
/* Q values:                                                                  */
/* lpc[] - Q12, *refc - Q15,                                                  */

Shortword lpc_pred2refl(Shortword lpc[], Shortword * refc, Shortword order)
{
	register Shortword i, j;
	Longword acc;
	Shortword *b, *b1, e;
	Shortword energy = ONE_Q15;
	Shortword shift, shift1, sign;
	Shortword temp;

	b = v_get(order);
	b1 = v_get((Shortword) (order - 1));

	/* equate temporary variables (b = lpc) */
	v_equ(b, lpc, order);

	/* compute reflection coefficients */
	for (i = sub(order, 1); i >= 0; i--) {

		if (b[i] >= 4096)
			b[i] = 4095;
		if (b[i] <= -4096)
			b[i] = -4095;

		acc = L_mult(b[i], b[i]);
		acc = L_sub(ONE_Q25, acc);
		acc = L_shl(acc, 6);	/* Q31 */
		energy = mult(energy, extract_h(acc));

		shift = norm_l(acc);
		e = extract_h(L_shl(acc, shift));

		v_equ(b1, b, i);

		for (j = 0; j < i; j++) {
			/*      b[j] = (b1[j] - local_refc*b1[i - j])/e; */
			acc = L_mult(b[i], b1[i - j - 1]);	/* Q25 */
			acc = L_sub(L_shl(L_deposit_l(b1[j]), 13), acc);	/* Q25 */

			/* check signs of temp and e before division */
			sign = extract_h(acc);
			acc = L_abs(acc);
			shift1 = norm_l(acc);
			temp = extract_h(L_shl(acc, shift1));
			if (temp > e) {
				temp = shr(temp, 1);
				shift1 = sub(shift1, 1);
			}
			b[j] = divide_s(temp, e);
			shift1 = sub(shift1, 3);
			shift1 = sub(shift1, shift);
			b[j] = shr(b[j], shift1);	/* b[j] in Q12 */

			if (sign < 0)
				b[j] = negate(b[j]);
		}
	}

	*refc = shl(b[0], 3);
	v_free(b1);
	v_free(b);
	return (energy);
}

/* LPC_LSP2PRED                                                               */
/*	  get predictor coefficients from the LSPs                                */
/* Synopsis: lpc_lsp2pred(w,a,p)                                              */
/*	  Input:                                                                  */
/*		 lsf- the LSPs                                                        */
/*		 order- the predictor order                                           */
/*	  Output:                                                                 */
/*		 lpc- the predictor coefficients                                      */
/* Reference:  Kabal and Ramachandran                                         */
/*                                                                            */
/* Q values:                                                                  */
/* lsf - Q15, lpc - Q12, c - Q14                                              */

Shortword lpc_lsp2pred(Shortword lsf[], Shortword lpc[], Shortword order)
{
	register Shortword i, j, k;
	Shortword p2;
	Shortword c0, c1;
	Longword *f0, *f1;
	Longword L_temp;

	/* ensure minimum separation and sort */
	lpc_clmp(lsf, 0, order);

	/* p2 = p/2 */
	p2 = shr(order, 1);

	f0 = L_v_get((Shortword) (p2 + 1));
	f1 = L_v_get((Shortword) (p2 + 1));

	/* f is Q25 */
	f0[0] = f1[0] = (Longword) ONE_Q25;

	/* -2.0*cos((double) lsf[0]*M_PI) */
	f0[1] = L_shr(L_deposit_h(negate(cos_fxp(lsf[0]))), 5);
	f1[1] = L_shr(L_deposit_h(negate(cos_fxp(lsf[1]))), 5);

	k = 2;

	for (i = 2; i <= p2; i++) {
		/* c is Q14 */
		/* multiply by 2 is considered as Q15->Q14 */
		c0 = negate(cos_fxp(lsf[k]));
		k++;
		c1 = negate(cos_fxp(lsf[k]));
		k++;

		f0[i] = f0[i - 2];
		f1[i] = f1[i - 2];

		for (j = i; j >= 2; j--) {
			/* f0[j] += c0*f0[j - 1] + f0[j - 2] */
			L_temp = L_mpy_ls(f0[j - 1], c0);
			L_temp = L_add(L_shl(L_temp, 1), f0[j - 2]);
			f0[j] = L_add(f0[j], L_temp);

			/* f1[j] += c1*f1[j - 1] + f1[j - 2] */
			L_temp = L_mpy_ls(f1[j - 1], c1);
			L_temp = L_add(L_shl(L_temp, 1), f1[j - 2]);
			f1[j] = L_add(f1[j], L_temp);
		}

		f0[1] = L_add(f0[1], L_shl(L_mpy_ls(f0[0], c0), 1));
		f1[1] = L_add(f1[1], L_shl(L_mpy_ls(f1[0], c1), 1));
	}

	for (i = sub(p2, 1); i >= 0; i--) {
		/* short f (f is a Q14) */
		f0[i + 1] = L_add(f0[i + 1], f0[i]);
		f1[i + 1] = L_sub(f1[i + 1], f1[i]);

		/* lpc[] is Q12 */
		/* lpc[i] = 0.50*(f0[i] + f1[i]) */
		/* lpc[p + 1 - i] = 0.50*(f0[i] - f1[i]) */
		/* Q25 -> Q27 -> Q12 */
		lpc[i] = extract_h(L_shl(L_add(f0[i + 1], f1[i + 1]), 2));
		lpc[order - 1 - i] =
		    extract_h(L_shl(L_sub(f0[i + 1], f1[i + 1]), 2));
	}

	v_free(f0);
	v_free(f1);
	return (0);
}

/* Name: lpc_syn- LPC synthesis filter.                                       */
/*	Aliases: lpc_synthesis                                                    */
/*	Description:                                                              */
/*		LPC all-pole synthesis filter                                         */
/*                                                                            */
/* 		for j = 0 to n-1                                                      */
/* 			y[j] = x[j] - sum(k=1 to p) y[j-k] a[k]                           */
/*                                                                            */
/*	Inputs:                                                                   */
/*		x- input vector (n samples, x[0..n-1])                                */
/*		a- lpc vector (order p, a[1..p])                                      */
/*		order- order of lpc filter                                            */
/*		length- number of elements in vector which is to be filtered          */
/*		y[-p..-1]- filter memory (past outputs)                               */
/*	Outputs:                                                                  */
/*		y- output vector (n samples, y[0..n-1]) (Q0)                          */
/*	Returns: NULL                                                             */
/*	Includes:                                                                 */
/*		lpc.h                                                                 */
/*                                                                            */
/*	Systems and Info. Science Lab                                             */
/*	Copyright (c) 1995 by Texas Instruments, Inc.  All rights reserved.       */

Shortword lpc_syn(Shortword x[], Shortword y[], Shortword a[], Shortword order,
		  Shortword length)
{
	register Shortword i, j;
	Longword accum;

/* Tung-chiang believes a[] is Q12, x[] and y[] are Q0. */

	for (j = 0; j < length; j++) {
		accum = L_shr(L_deposit_h(x[j]), 3);
		for (i = order; i > 0; i--)
			accum = L_msu(accum, y[j - i], a[i - 1]);
		/* r_ound off output */
		accum = L_shl(accum, 3);
		y[j] = r_ound(accum);
	}
	return (0);
}

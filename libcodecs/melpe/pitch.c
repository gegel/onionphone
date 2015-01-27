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
/* File:		pitch.c												*/
/*																	*/
/* Description: new pitch estimation routines						*/
/*																	*/
/*------------------------------------------------------------------*/

#include "sc1200.h"
#include "cprv.h"
#include "macro.h"
#include "global.h"
#include "mat_lib.h"
#include "pitch.h"
#include "mathhalf.h"
#include "math_lib.h"
#include "constant.h"
#include "dsp_sub.h"
#include "melp_sub.h"

#define PIT_WIN			(PIT_COR_LEN - MAXPITCH)
#define PIT_WEIGHT		200	/* 100 * (1 << 1) */
#define PIT_WEIGHT_Q5	3200	/* 100 Q5 */

#define ONE_Q5			32	/* (1 << 5) */
#define ONE_Q10			1024	/* (1 << 10) */
#define THREE_Q10		3072	/* 3 * (1 << 10) */

/* ========== Prototypes ========== */
static void lpfilt(int16_t inbuf[], int16_t lpbuf[], int16_t len);
static void ivfilt(int16_t ivbuf[], int16_t lpbuf[], int16_t len);
static void corPeak(int16_t inbuf[], pitTrackParam * pitTrack,
		    classParam * classStat);

/****************************************************************************
**
** Function:		pitchAuto()
**
** Description: 	Calculating time domain pitch estimation
**					structure.
**
** Arguments:
**
**	int16_t inbuf[]		---- speech buffer (Q0)
**	pitTrackParam *pitTrack ---- pitch pitTrackParam structure
**	classParam *classStat	---- classification parameters
**
** Return value:	None
**
*****************************************************************************/
void pitchAuto(int16_t inbuf[], pitTrackParam * pitTrack,
	       classParam * classStat)
{
	static BOOLEAN firstTime = TRUE;
	static int16_t lpbuf[PIT_COR_LEN];	/* low pass filter buffer, Q0 */
	static int16_t ivbuf[PIT_COR_LEN];	/* inverse filter buffer, Q12 */

	if (firstTime) {	/* initialize the buffers */
		v_zap(lpbuf, PIT_COR_LEN);
		v_zap(ivbuf, PIT_COR_LEN);
		firstTime = FALSE;
	}

	/* The input inbuf[] is not modified in pitchAuto() and the functions     */
	/* pitchAuto() directly or indirectly calls.  On the other hand, an       */
	/* inspection of these functions shows that the output ivbuf[] for        */
	/* ivfilt() is in direct proportion to the input lpbuf[] and inside       */
	/* corPeak() we remove the DC component of ivbuf[] and then compute the   */
	/* normalized crosscorrelations.  This means we can scale lpbuf[] and     */
	/* ivbuf[] any way we want.                                               */

	/* ------ 800Hz low pass filter ------ */
	lpfilt(inbuf, lpbuf, PIT_SUBFRAME);

	/* ------ Two order inverse filter ------- */
	ivfilt(ivbuf, lpbuf, PIT_SUBFRAME);

	/* ------ Calculate the autocorrelation function ------- */
	corPeak(ivbuf, pitTrack, classStat);
}

/*============================================================*
 *				  LPFILT: low pass filter					  *
 * inbuf[]	---- input	speech data (Q0)					  *
 * lpbuf	---- ouput	low pass filtered data				  *
 * len		---- update buffer length						  *
 *============================================================*/
static void lpfilt(int16_t inbuf[], int16_t lpbuf[], int16_t len)
{
	register int16_t i, j;
	static const int16_t lpar[4] = {	/* Q15 */
		20113, -20113, 9437, -1720
	};
	int32_t L_sum;

	/* ====== Shift the lpbuf ====== */
	v_equ(lpbuf, &(lpbuf[len]), (int16_t) (PIT_COR_LEN - len));

	/* ====== low pass filter ====== */

	/* lpbuf[] is a shifted output of the following filter with inbuf[] being */
	/* the input:                                                             */
	/*    H(z) = 0.3069/(1 - 2.4552 z^{-1} + 2.4552 z^{-2} - 1.152 z^{-3} +   */
	/*                   0.2099 z^{-4}).                                      */
	/* First we drop the factor 0.3069 in the numerator because we can afford */
	/* scale the input arbitrarily.  Then we divide everything by 4 to obtain */
	/*    H(z) = 1.0/(0.25 - 0.6138 z^{-1} + 0.6138 z^{-2} - 0.288 z^{-3} +   */
	/*                0.052475 z^{-4}).                                       */

	for (i = 0; i < len; i++) {
		L_sum = melpe_L_shr(melpe_L_deposit_h(inbuf[i]), 3);
		for (j = 0; j < 4; j++) {
			/*      sum += lpbuf[PIT_COR_LEN - len + i - j - 1] * lpar[j]; */
			L_sum = melpe_L_mac(L_sum, lpbuf[PIT_COR_LEN - len + i - j - 1], lpar[j]);	/* Q0 */
		}
		lpbuf[PIT_COR_LEN - len + i] = melpe_r_ound(L_sum);	/* Q0 */
	}
}

/*============================================================*
 *				  IVFILT: inverse filter					  *
 * lpbuf ---- input speech data that through low-pass filter  *
 * ivbuf ---- residaul values of lpbuf through inverse filer  *
 * len	 ---- update buffer length							  *
 *============================================================*/
static void ivfilt(int16_t ivbuf[], int16_t lpbuf[], int16_t len)
{
	register int16_t i, j;
	int64_t L40_sum;
	int32_t L_temp;
	int16_t shift, rc1, temp1, temp2, temp3;
	int16_t r_coeff[3];	/* Q15 */
	int16_t pc1, pc2;	/* Q12 */

	/* ====== Shift the ivbuf ====== */
	v_equ(ivbuf, &(ivbuf[len]), (int16_t) (PIT_COR_LEN - len));

	/* compute pc1 and pc2 in Q12 as                                          */
	/*       r(0)*r(1)-r(1)*r(2)           r(0)*r(2)-r(1)**2                  */
	/*  pc1 = ------------------- , pc2 = ----------------- .                 */
	/*        r(0)**2-r(1)**2               r(0)**2-r(1)**2                   */

	L40_sum = 0;
	for (i = 0; i < PIT_COR_LEN; i++)
		L40_sum = melpe_L40_mac(L40_sum, lpbuf[i], lpbuf[i]);
	shift = melpe_norm32(L40_sum);
	L_temp = (int32_t) melpe_L40_shl(L40_sum, shift);
	r_coeff[0] = melpe_r_ound(L_temp);	/* normalized r0 */
	for (i = 1; i < 3; i++) {
		L40_sum = 0;
		for (j = i; j < PIT_COR_LEN; j++)
			L40_sum = melpe_L40_mac(L40_sum, lpbuf[j], lpbuf[j - i]);
		L_temp = (int32_t) melpe_L40_shl(L40_sum, shift);
		r_coeff[i] = melpe_r_ound(L_temp);
	}

	/* Now compute pc1 and pc2 */
	if (r_coeff[0] == 0) {
		pc1 = 0;
		pc2 = 0;
	} else {
		/* rc1 = r[1] / r[0];                                                           */
		/* rc2 =(r[2] - rc1 * r[1]) / (r[0] - rc1 * r[1]);      */
		/* pc1 = rc1 - rc1 * rc2;                                                       */
		/* pc2 = rc2;                                                                           */
		rc1 = melpe_divide_s(r_coeff[1], r_coeff[0]);	/* Q15 */
		temp1 = melpe_mult(rc1, r_coeff[1]);	/* Q15 */
		temp2 = melpe_sub(r_coeff[0], temp1);
		temp3 = melpe_sub(r_coeff[2], temp1);
		temp1 = melpe_abs_s(temp3);
		if (temp1 > temp2) {
			pc2 = (int16_t) - 4096;	/* Q12 */
		} else {
			pc2 = melpe_divide_s(temp1, temp2);
			if (temp3 < 0)
				pc2 = melpe_negate(pc2);	/* Q15 */
			pc2 = melpe_shr(pc2, 3);	/* Q12 */
		}

		melpe_sub(0x1000, pc2);	/* 1.0 - pc2 */
		pc1 = melpe_mult(rc1, pc2);	/* Q12 */
	}

	/* --- inverse filter ---- */
	for (i = 0; i < len; i++) {
		/*      ivbuf[i] = lpbuf[i] - pc1 * lpbuf[i - 1] - pc2 * lpbuf[i - 2]; */
		L_temp = melpe_L_shl(melpe_L_deposit_l(lpbuf[PIT_COR_LEN - len + i]), 13);
		L_temp =
		    melpe_L_sub(L_temp,
			  melpe_L_mult(pc1, lpbuf[PIT_COR_LEN - len + i - 1]));
		L_temp =
		    melpe_L_sub(L_temp,
			  melpe_L_mult(pc2, lpbuf[PIT_COR_LEN - len + i - 2]));
		ivbuf[PIT_COR_LEN - len + i] = melpe_r_ound(melpe_L_shl(L_temp, 3));
	}
}

/*==============================================================*
 *			corPeak: fill pitTrack structure 					*
 *	inbuf		---- input data buffer (Q12)					*
 *	pitTrack	---- pitch pitTrackParam structure				*
 *	classStat	---- classification paramters					*
 *==============================================================*/
static void corPeak(int16_t inbuf[], pitTrackParam * pitTrack,
		    classParam * classStat)
{
	register int16_t i, j;
	int16_t temp, temp1, temp2, shift;
	int16_t proBuf[PIT_COR_LEN];	/* Q15 */
	int32_t L_temp;	/* Q0 */
	int16_t index[MAXPITCH + 1];
	int16_t lowStart, highStart;
	int64_t ACC_r0, ACC_rk, ACC_A;	/* Emulating 40Bit-Accumulator */
	int32_t L_r0, L_rk;
	int16_t r0_shift, rk_shift, root;
	int16_t gp[MAXPITCH + 1], peak[MAXPITCH + 1], corx[NODE];	/* Q15 */

	/* ------ Remove DC component. ------ */
	remove_dc(inbuf, proBuf, PIT_COR_LEN);

	/* ------ Caculate the autocorrelation function ------- */
	ACC_r0 = 0;
	for (i = 0; i < PIT_COR_LEN - MAXPITCH; i++) {
		ACC_r0 = melpe_L40_mac(ACC_r0, proBuf[i], proBuf[i]);	/* Q31 */
	}
	if (ACC_r0 == 0)
		ACC_r0 = 1;
	r0_shift = melpe_norm32(ACC_r0);
	ACC_r0 = melpe_L40_shl(ACC_r0, r0_shift);
	L_r0 = (int32_t) ACC_r0;

	ACC_rk = 0;
	for (i = MAXPITCH; i < PIT_COR_LEN; i++) {
		ACC_rk = melpe_L40_mac(ACC_rk, proBuf[i], proBuf[i]);	/* Q31 */
	}
	if (ACC_rk == 0)
		ACC_rk = 1;
	rk_shift = melpe_norm32(ACC_rk);
	ACC_rk = melpe_L40_shl(ACC_rk, rk_shift);
	L_rk = (int32_t) ACC_rk;

	ACC_A = 0;
	for (i = 0; i < PIT_COR_LEN - MAXPITCH; i++) {
		ACC_A = melpe_L40_mac(ACC_A, proBuf[i], proBuf[i + MAXPITCH]);	/* Q31 */
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
	gp[MAXPITCH] = melpe_divide_s(temp, root);

	/* ==== Here comes the Main loop ==== */

	lowStart = 0;
	highStart = MAXPITCH;
	for (i = MAXPITCH - 1; i >= MINPITCH; i--) {
		if (i % 2 == 0) {
			ACC_r0 = L_r0;
			ACC_r0 = melpe_L40_shr(ACC_r0, r0_shift);
			ACC_r0 =
			    melpe_L40_msu(ACC_r0, proBuf[lowStart], proBuf[lowStart]);
			ACC_r0 =
			    melpe_L40_mac(ACC_r0, proBuf[lowStart + PIT_WIN],
				    proBuf[lowStart + PIT_WIN]);
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
			    melpe_L40_mac(ACC_rk, proBuf[highStart],
				    proBuf[highStart]);
			ACC_rk =
			    melpe_L40_msu(ACC_rk, proBuf[highStart + PIT_WIN],
				    proBuf[highStart + PIT_WIN]);
			if (ACC_rk == 0)
				ACC_rk = 1;
			rk_shift = melpe_norm32(ACC_rk);
			ACC_rk = melpe_L40_shl(ACC_rk, rk_shift);
			L_rk = (int32_t) ACC_rk;
		}
		ACC_A = 0;
		for (j = lowStart; j < lowStart + PIT_WIN; j++) {
			ACC_A = melpe_L40_mac(ACC_A, proBuf[j], proBuf[j + i]);
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
			temp = 0;	/* ignore negative autocorrelation */
		gp[i] = melpe_divide_s(temp, root);
	}			/* Main loop ends */

	/* ------ Find the local peak of gp function ------- */
	if (gp[MINPITCH + 1] < gp[MINPITCH])
		peak[MINPITCH] = gp[MINPITCH];
	else
		peak[MINPITCH] = 0;

	if (gp[MAXPITCH] > gp[MAXPITCH - 1])
		peak[MAXPITCH] = gp[MAXPITCH];
	else
		peak[MAXPITCH] = 0;

	for (i = MINPITCH + 1; i < MAXPITCH; i++) {
		if ((gp[i] > gp[i - 1]) && (gp[i] > gp[i + 1]))
			peak[i] = gp[i];
		else
			peak[i] = 0;
	}

	/* --- Fill in the TRACK struct using the first NODE peaks --- */
	v_zap(index, MAXPITCH + 1);

	for (i = 0; i < NODE; i++) {
		temp = MAXPITCH;
		for (j = MAXPITCH - 1; j >= MINPITCH; j--) {
			if (peak[j] > peak[temp])
				temp = j;
		}
		index[temp] = (int16_t) (i + 1);
		corx[i] = peak[temp];
		peak[temp] = 0;
		if (i == 0)
			classStat->pitch = temp;
	}

	classStat->corx = corx[0];

	j = 0;
	for (i = MINPITCH; i <= MAXPITCH; i++) {
		if (index[i] != 0) {
			pitTrack->pit[j] = i;
			pitTrack->weight[j] = corx[index[i] - 1];
			j++;
		}
	}

	for (; j < NODE; j++) {
		pitTrack->pit[j] = 100;

		/* The floating point version set pitTrack->weight[j] to -100.0 */
		/* here we use 0                                                */
		pitTrack->weight[j] = 0;
	}

	/* ---- Modify time domain correlation by muliple checking ---- */
	for (i = 0; i < NODE - 1; i++) {
		for (j = (int16_t) (i + 1); j < NODE; j++) {
			temp1 = pitTrack->pit[j];
			temp2 = pitTrack->pit[i];
			temp = temp2;
			while (temp < temp1) {
				temp = melpe_add(temp, temp2);
			}
			temp2 = melpe_shr(temp2, 1);
			temp2 = melpe_sub(temp, temp2);
			if (temp2 >= temp1)
				temp = melpe_sub(temp, pitTrack->pit[i]);

			/* Now temp is the multiples of pitTrack->pit[i] which is the     */
			/* closest to pitTrack->pit[j].                                   */

			/*      rk_f = abs(pitTrack->pit[j] -
			   temp*pitTrack->pit[i])/pitTrack->pit[j]; */
			temp = melpe_sub(pitTrack->pit[j], temp);
			temp = melpe_abs_s(temp);
			temp2 = melpe_divide_s(temp, pitTrack->pit[j]);	/* Q15 */

			if (temp2 < X008_Q15) {
				/*      pitTrack->weight[j] -= pitTrack->weight[i]/5.0; */
				temp1 = pitTrack->weight[i];
				temp1 = melpe_mult(temp1, X02_Q15);
				temp2 = pitTrack->weight[j];
				temp2 = melpe_sub(temp2, temp1);
				if (temp2 < 0)
					temp2 = 0;
				pitTrack->weight[j] = temp2;
			}
		}
	}
}

/******************************************************************
**
** Function:	multiCheck
**
** Description: multiple or sub-multiple pitch check
**
** Input:
**		int16_t f1	------	the pitch candidate 1 (Q7)
**		int16_t f2	------	the pitch candidate 2 (Q7)
**
** Return: int16_t	----- ratio of f1, f2 with multiple (Q15)
**
********************************************************************/
int16_t multiCheck(int16_t f1, int16_t f2)
{
	int16_t temp, f2_multiple;	/* Q7 */

	if (f1 <= f2) {		/* Swap f1 and f2 so that f1 is larger than f2. */
		temp = f1;
		f1 = f2;
		f2 = temp;
	}

	f2_multiple = f2;
	while (f2_multiple <= f1)
		f2_multiple = melpe_add(f2_multiple, f2);

	/* Now f2_multiple is larger than f1.  Check whether f2_multiple or       */
	/* (f2_multiple - f2) is closer to f1 and use the better one.             */

	temp = melpe_shr(f2, 1);
	temp = melpe_sub(f2_multiple, temp);	/* Q7 */
	if (temp > f1)
		f2_multiple = melpe_sub(f2_multiple, f2);

	return (ratio(f1, f2_multiple));
}

/******************************************************************
**
** Function:	trackPitch
**
** Description: Find coresponding pitch index for pitch tracking
**
** Input:
**		int16_t pitch			------	the pitch candidate (Q7)
**		pitTrackParam *pitTrack ------	The pitch tracker structure
**
** Return: int16_t ----- index of the time domain candidates
**
********************************************************************/
int16_t trackPitch(int16_t pitch, pitTrackParam * pitTrack)
{
	register int16_t i;
	int16_t index;
	int16_t best, temp;	/* Q7 */
	int16_t co = MONE_Q15;	/* Minus infinity. */

	/* The following loop finds the index for pitTrack->pit[] so that it      */
	/* maximizes pitTrack->weight[] among those indices with                  */
	/* ratio(pitTrack->pit[*], pitch) < 0.2 as a qualification.  If index     */
	/* remains -1, it means none of pitTrack->pit[] qualifies.                */

	index = -1;
	for (i = 0; i < NODE; i++) {
		temp = melpe_shl(pitTrack->pit[i], 7);	/* Q7 */
		if (ratio(temp, pitch) < X02_Q15) {
			if (pitTrack->weight[i] > co) {
				co = pitTrack->weight[i];
				index = i;
			}
		}
	}

	if (index < 0) {	/* All of pitTrack->pit[] makes ratio(*, pitch) > 0.2; */
		/* none of pitTrack->pit[] is close enough to "pitch". */

		/* Then we find the "index" which is the closest to "pitch".          */

		index = 0;
		temp = melpe_shl(pitTrack->pit[index], 7);
		temp = melpe_sub(temp, pitch);
		best = melpe_abs_s(temp);	/* Q7 */
		for (i = 1; i < NODE; i++) {
			temp = melpe_shl(pitTrack->pit[i], 7);
			temp = melpe_sub(temp, pitch);
			temp = melpe_abs_s(temp);	/* Q7 */
			if (temp < best) {
				index = i;
				best = temp;
			}
		}
	}

	return (index);
}

/******************************************************************
**
** Function:	pitLookahead	
**
** Description: Find pitch at onset segment
**
** Input:
**		pitTrackParam *pitTrack ------ The pitch tracker structure
**		int16_t num			------ The number of available structures
**
** Return: int16_t ----- the smoothed pitch (Q7)
**
********************************************************************/
int16_t pitLookahead(pitTrackParam * pitTrack, int16_t num)
{
	register int16_t i, j;
	int16_t index;
	int32_t L_sum, L_cost;

	/* compute the cost function of the last strucutre */
	for (i = 0; i < NODE; i++) {
		L_sum = melpe_L_sub(LW_MAX, melpe_L_deposit_h(pitTrack[num].weight[i]));
		L_cost = melpe_L_mult(melpe_extract_h(L_sum), PIT_WEIGHT_Q5);
		pitTrack[num].cost[i] = melpe_extract_h(L_cost);	/* Q5 */
	}

	/* -------- forward tracker -------- */
	for (i = melpe_sub(num, 1); i >= 0; i--) {
		for (j = 0; j < NODE; j++) {
			index =
			    trackPitch(melpe_shl(pitTrack[i].pit[j], 7),
				       &pitTrack[i + 1]);

			/* pitTrack[i].cost[j] = PIT_WEIGHT*(1.0-pitTrack[i].weight[j]); */
			L_sum =
			    melpe_L_sub(LW_MAX, melpe_L_deposit_h(pitTrack[i].weight[j]));
			L_cost = melpe_L_mult(melpe_extract_h(L_sum), PIT_WEIGHT_Q5);	/* Q5 */

			/*      pitTrack[i].cost[j] +=
			   abs(pitTrack[i].pit[j] - pitTrack[i + 1].pit[index]); */
			L_sum = melpe_L_sub(melpe_L_deposit_h(pitTrack[i].pit[j]),
				      melpe_L_deposit_h(pitTrack[i + 1].pit[index]));
			L_cost = melpe_L_add(L_cost, melpe_L_shl(melpe_L_abs(L_sum), 5));	/* Q5 */

			/* pitTrack[i].cost[j] += pitTrack[i+1].cost[index]; */
			L_cost =
			    melpe_L_add(L_cost,
				  melpe_L_deposit_h(pitTrack[i + 1].cost[index]));
			pitTrack[i].cost[j] = melpe_extract_h(L_cost);	/* Q5 */
		}
	}

	/* find the best index to minimize the cost */
	L_cost = melpe_L_deposit_h(pitTrack[0].cost[0]);
	index = 0;
	for (i = 1; i < NODE; i++) {
		if (melpe_L_deposit_h(pitTrack[0].cost[i]) < L_cost) {
			L_cost = melpe_L_deposit_h(pitTrack[0].cost[i]);
			index = i;
		}
	}

	return (melpe_shl(pitTrack[0].pit[index], 7));
}

/* ========================================================================== */
/* This function replaces the previous macro declaration for RATIO(x, y):     */
/*                                                                            */
/*    RATIO(x, y) = (fabs((float)(x)-(float)(y))/(float)((x)>(y)?(x):(y)))    */
/*                                                                            */
/* The int16_t Q7 arguments x and y are implied to be positive.  The        */
/* returned value is Q15.                                                     */
/* ========================================================================== */

int16_t ratio(int16_t x, int16_t y)
{
	int16_t diff, larger;

	diff = melpe_sub(x, y);
	diff = melpe_abs_s(diff);
	larger = (int16_t) ((x > y) ? x : y);
	return (melpe_divide_s(diff, larger));
}

/* ========================================================================== */
/* This function should be functionally almost the same as ratio() except     */
/* that the second argument is int32_t.  It is used in sc_ana() of           */
/* "melp_ana.c" when we call ratio() in                                       */
/*                                                                            */
/*    ratio(par[1].pitch, (prev_pitch*3)) < X008_Q15                          */
/*                                                                            */
/* where a Q7 prev_pitch multiplied by 3 could overflow a int16_t.  Both    */
/* inputs are still Q7 and the returned value is still Q15.                   */
/* ========================================================================== */

int16_t L_ratio(int16_t x, int32_t y)
{
	int32_t L_x, diff, larger;

	L_x = melpe_L_deposit_l(x);
	diff = melpe_L_sub(y, L_x);
	if (diff < 0)
		diff = melpe_L_negate(diff);
	larger = (L_x > y) ? L_x : y;
	return (L_divider2(diff, larger, 0, 0));
}

/* ========================================================================== */
/* This function updates silenceEn and voicedEn used in sc_ana() of           */
/* "melp_ana.c".   prev_en, curr_en and the returned result are of the same   */
/* Q value (Q11), while ifact is Q15.                                         */
/*                                                                            */
/* Note that because of the dynamic range problem, approximations are used in */
/* the fixed point implementation.  The implementation assumes "ifact" is     */
/* always equal to 0.9.                                                       */
/* ========================================================================== */
int16_t updateEn(int16_t prev_en, int16_t ifact, int16_t curr_en)
{
	int16_t temp1, temp2;
	int16_t result;	/* Q11 */

	/* Now we want to compute                                                 */
	/* log10(ifact*pow(10.0, prev_en) + (1.0 - ifact)*pow(10.0, curr_en)).    */
	/* We rearrange the expression as                                         */
	/*        prev_en + log10(ifact + (1 - ifact) *                           */
	/*                                pow(10.0, curr_en - prev_en))           */
	/* Depending on the range of (curr_en - prev_en), we might choose to      */
	/* ignore the pow() term, or ignore the "ifact" term inside log10() (so   */
	/* the expression becomes curr_en + log10(1 - ifact)).  This manipulation */
	/* is necessary because pow(10.0, prev_en) and pow(10.0, curr_en) can     */
	/* easily overflow the dynamic range for int16_t.                       */

	temp1 = melpe_shr(curr_en, 1);	/* Q10 */
	temp2 = melpe_shr(prev_en, 1);
	temp1 = melpe_sub(temp1, temp2);	/* Q10 */
	if (temp1 < melpe_negate(ONE_Q10)) {	/* Case (a) */
		/* Compute log10(ifact) + prev_en. */
		temp1 = log10_fxp(ifact, 15);	/* Q12 */
		temp1 = melpe_shr(temp1, 1);	/* Q11 */
		result = melpe_add(temp1, prev_en);
	} else if (temp1 > THREE_Q10) {	/* Case (b) */
		temp1 = melpe_sub(ONE_Q15, ifact);	/* Q15 */
		temp1 = log10_fxp(temp1, 15);	/* Q12 */
		temp1 = melpe_shr(temp1, 1);	/* Q11 */
		result = melpe_add(temp1, curr_en);	/* Q11 */
	} else {		/* Case (c) */
		/* temp1 = (curr_en - prev_en) is between -1 and 3. */
		temp1 = melpe_shl(temp1, 2);	/* Q12 */
		temp1 = pow10_fxp(temp1, 5);	/* Q5 */
		temp1 = interp_scalar(temp1, ONE_Q5, ifact);	/* Q5 */
		temp1 = log10_fxp(temp1, 5);	/* Q12 */
		temp1 = melpe_shr(temp1, 1);	/* Q11 */
		result = melpe_add(prev_en, temp1);
	}
	return (result);
}

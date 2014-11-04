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
/* File:		harm.c												*/
/*																	*/
/* Description: harmonic synthesis routines			 				*/
/*																	*/
/*------------------------------------------------------------------*/

#include "sc1200.h"
#include "constant.h"
#include "mathhalf.h"
#include "mat_lib.h"
#include "math_lib.h"
#include "dsp_sub.h"
#include <ophtools.h>

#define FIXED_PHASE		1
#define SYN_FFT_SIZE	256
#define TWO_Q12			8192	/* 2 * (1 << 12) */
#define X075_Q15		24576	/* 0.75 * (1 << 15) */
#define X500_Q3			4000	/* 500 * (1 << 3) */
#define X1000_Q3		8000	/* 1000 * (1 << 3) */
#define X2000_Q3		16000	/* 2000 * (1 << 3) */
#define X3000_Q3		24000	/* 3000 * (1 << 3) */
#define X085_Q14		13926	/* 0.85 * (1 << 15) */
#define X092_Q14		15073	/* 0.92 * (1 << 15) */
#define X095_Q14		15565	/* 0.95 * (1 << 15) */
#define X098_Q14		16056	/* 0.98 * (1 << 15) */
#define X102_Q14		16712	/* 1.02 * (1 << 15) */
#define X105_Q14		17203	/* 1.05 * (1 << 15) */

static void realIDFT(int16_t mag[], int16_t phase[],
		     int16_t signal[], int16_t length);

/***************************************************************************
**
** Function:		realIDFT()
**
** Description:		IDFT to generate real output
**
** Arguments:
**
**	int16_t mag[]		input magnitudes (Q13)
**	int16_t phase[]	input phase (Q0)
**	int16_t signal[]	output signal (Q15)
**	int16_t length	The IDFT length
**
** Return value:			None
**
*****************************************************************************/
static void realIDFT(int16_t mag[], int16_t phase[], int16_t signal[],
		     int16_t length)
{
	register int16_t i, j, k;
	int16_t w, w2, length2;
	int16_t temp;
	int32_t L_temp;
	int16_t idftc[PITCHMAX];

	/*      length2 = (length/2) + 1; */
	length2 = melpe_add(melpe_shr(length, 1), 1);
	/*      w = TWOPI / length; */
	w = melpe_divide_s(TWO_Q3, length);	/* w = 2/length in Q18 */

	/* The following for loop builds up the lookup table for cosines with     */
	/* radians from 0 to 1.                                                   */
	for (i = 0; i < length; i++) {
		L_temp = melpe_L_mult(w, i);	/* L_temp in Q19 */

		/* make sure argument for cos function is less than 1 */
		if (L_temp > (int32_t) ONE_Q19) {
			/*      cos(pi+x) = cos(pi-x) */
			L_temp = melpe_L_sub((int32_t) TWO_Q19, L_temp);
		} else if (L_temp == (int32_t) ONE_Q19)
			L_temp = melpe_L_sub(L_temp, 1);

		L_temp = melpe_L_shr(L_temp, 4);	/* L_temp in Q15 */
		temp = melpe_extract_l(L_temp);
		idftc[i] = cos_fxp(temp);	/* idftc in Q15 */
	}

	w = melpe_shr(w, 1);		/* w = 2/length in Q17 */
	w2 = melpe_shr(w, 1);		/* w2 = 1/length in Q17 */
	mag[0] = melpe_mult(mag[0], w2);	/* mag[] in Q15 */
	temp = melpe_sub(length2, 1);
	for (i = 1; i < temp; i++) {
		/*      mag[i] *= (2.0/length); */
		mag[i] = melpe_mult(mag[i], w);	/* mag[] is now Q15 */
	}

	temp = melpe_shl(i, 1);
	if (temp == length)	/* length is even, mag[i] *= (1.0/length); */
		mag[i] = melpe_mult(mag[i], w2);
	else			/* length is odd, mag[i] *= (2.0/length); */
		mag[i] = melpe_mult(mag[i], w);

	for (i = 0; i < length; i++) {
		L_temp = melpe_L_deposit_h(mag[0]);	/* L_temp in Q15 */
		k = i;
		for (j = 1; j < length2; j++) {
			k = melpe_add(k, phase[j]);
			while (k < 0)
				k = melpe_add(k, length);
			while (k >= length)
				k = melpe_sub(k, length);
			L_temp = melpe_L_mac(L_temp, mag[j], idftc[k]);
			k = melpe_sub(k, phase[j]);
			k = melpe_add(k, i);
		}

		/* It might take some proofs, but mag[] is already weighted by w      */
		/* (which is inversely proportional to length) and L_temp here never  */
		/* overflows a int16_t.                                             */
		signal[i] = melpe_r_ound(L_temp);
	}
}

/***************************************************************************
**
** Function:		set_fc()
**
** Description: 	Set cut-off frequency based on voicing information
**
** Arguments:
**
**	int16_t bpvc[]	band voicing information (Q14)
**	int16_t *fc		output cut-off frequency (Q3)
**
** Return value:			None
**
*****************************************************************************/

void set_fc(int16_t bpvc[], int16_t * fc)
{
	register int16_t i;
	int16_t index;
	const int16_t syn_bp_map[16] = {	/* Q0 */
		500, 500, 500, 500, 500, 500, 500, 4000,
		1000, 1000, 1000, 4000, 2000, 3000, 3000, 4000
	};

	/* ====== Generate voicing information ====== */
	if (bpvc[0] < X05_Q14) {	/* ---- Pure unvoiced ---- */
		*fc = 0;
		return;
	}

	/* ---- Voiced: pack bandpass voicing ---- */
	index = 0;
	bpvc[0] = ONE_Q14;
	for (i = 1; i < NUM_BANDS; i++) {
		index <<= 1;	/* left shift */
		if (bpvc[i] > X05_Q14) {
			bpvc[i] = ONE_Q14;
			index |= 1;
		} else {
			bpvc[i] = 0;
			index |= 0;
		}
	}
	*fc = (int16_t) (syn_bp_map[index] << 3);
}

/***************************************************************************
**
** Function:		harm_syn_pitch()
**
** Description:		harmonic synthesis for one pitch
**
** Arguments:
**
**	int16_t amp[]			input harmonic mags (Q13)
**	int16_t signal[]		output synthesized signal buffer (Q15)
**	int16_t fc			The cut-off frequency (Q3)
**	int16_t length		The pitch length
**
** Return value:			None
**
*****************************************************************************/
void harm_syn_pitch(int16_t amp[], int16_t signal[], int16_t fc,
		    int16_t length)
{
	register int16_t i;
	int16_t rndphase[SYN_FFT_SIZE / 2 + 1];	/* Q0 */
	int16_t factor, fn;	/* Q15 */
	int16_t temp1, temp2;
	int16_t totalCnt, voicedCnt, mixedCnt, index;
	int16_t mag[SYN_FFT_SIZE / 2 + 1];
	int16_t phase[SYN_FFT_SIZE / 2 + 1];	/* Q0 */
	int16_t fc1, fc2;

	memzero(phase, (SYN_FFT_SIZE / 2 + 1) * sizeof(int16_t));

	/* ====== Generate random phase for unvoiced segment ====== */
	/* Note that phase[] and rndphase[] computed in harm_syn_pitch() are now  */
	/* the actual phases divided by (2*PI)/length.                            */

	for (i = 0; i < length / 2 + 1; i++)
		rndphase[i] = melpe_mult(length, rand_minstdgen());

	/* ====== Harmonic Synthesis ====== */
	/* The fc1 and fc2 computed in the if block below are Q2 */

	if (fc <= X500_Q3) {
		fc1 = melpe_mult(X085_Q14, fc);
		fc2 = melpe_mult(X105_Q14, fc);
		factor = ONE_Q15;
	} else if (fc <= X1000_Q3) {
		fc1 = melpe_mult(X095_Q14, fc);
		fc2 = melpe_mult(X105_Q14, fc);
		factor = X09_Q15;
	} else if (fc <= X2000_Q3) {
		fc1 = melpe_mult(X098_Q14, fc);
		fc2 = melpe_mult(X102_Q14, fc);
		factor = X08_Q15;
	} else if (fc <= X3000_Q3) {
		fc1 = melpe_mult(X095_Q14, fc);
		fc2 = melpe_mult(X105_Q14, fc);
		factor = X075_Q15;
	} else {
		fc1 = melpe_mult(X092_Q14, fc);
		fc2 = melpe_shift_r(fc, -1);	/* We map fc (Q3) to fc2 (Q2) with r_ounding. */
		factor = X07_Q15;
	}

	/* fc1 and fc2 are now Q2. */

	temp1 = melpe_divide_s(fc1, melpe_shl(FSAMP, 2));
	temp2 = melpe_divide_s(fc2, melpe_shl(FSAMP, 2));	/* Now temp1 and temp2 are Q15. */
	voicedCnt = melpe_mult(temp1, length);
	mixedCnt = melpe_mult(temp2, length);
	totalCnt = (int16_t) ((length / 2) + 1);

	/* ====== set values to mag and phase ====== */
	v_equ(mag, amp, melpe_add(voicedCnt, 1));	/* Q13 */

	/* Now we compute phase[] in multiples of w = (2*PI)/length.  Therefore   */
	/* phase[] -> (i*FIXED_PHASE)*length/(2*PI).                              */
	temp1 = 0;		/* temp1 = i * temp2 in the following for loop. */
	temp2 = melpe_extract_l(melpe_L_mult(FIXED_PHASE, length));
	temp2 = melpe_shr(temp2, 1);	/* temp2 = FIXED_PHASE * length */
	while (temp2 >= 2 * length)
		temp2 = melpe_sub(temp2, (int16_t) (2 * length));
	for (i = 0; i < mixedCnt + 1; i++) {
		phase[i] = melpe_shr(temp1, 1);
		temp1 = melpe_add(temp1, temp2);
		if (temp1 >= 2 * length)
			temp1 = melpe_sub(temp1, (int16_t) (2 * length));
	}
	index = 0;
	for (i = melpe_add(voicedCnt, 1); i < melpe_add(mixedCnt, 1); i++, index++) {
		temp1 = melpe_sub(i, voicedCnt);
		temp2 = melpe_sub(mixedCnt, voicedCnt);
		fn = melpe_divide_s(temp1, temp2);	/* Q15 */
		temp1 = melpe_mult(factor, fn);
		temp2 = melpe_sub(ONE_Q15, fn);
		temp1 = melpe_add(temp1, temp2);
		mag[i] = melpe_mult(amp[i], temp1);	/* Q13 */
		temp1 = melpe_mult(fn, rndphase[index]);	/* Q0 */
		temp2 = melpe_sub(phase[i], temp1);
		if (temp2 < 0)
			temp2 = melpe_add(temp2, length);
		phase[i] = temp2;
	}
	for (i = melpe_add(mixedCnt, 1); i < totalCnt; i++, index++) {
		mag[i] = melpe_mult(amp[i], factor);	/* Q13 */
		temp2 = melpe_negate(rndphase[index]);	/* Q0 */
		if (temp2 < 0)
			temp2 = melpe_add(temp2, length);
		phase[i] = temp2;	/* This moves phase[i] from */
		/* negative to positive. */
	}

	/* ====== getting one pitch cycle ====== */
	realIDFT(mag, phase, signal, length);
}

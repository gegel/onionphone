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
#define TWO_Q12			8192                                 /* 2 * (1 << 12) */
#define X075_Q15		24576                             /* 0.75 * (1 << 15) */
#define X500_Q3			4000                                /* 500 * (1 << 3) */
#define X1000_Q3		8000                               /* 1000 * (1 << 3) */
#define X2000_Q3		16000                              /* 2000 * (1 << 3) */
#define X3000_Q3		24000                              /* 3000 * (1 << 3) */
#define X085_Q14		13926                             /* 0.85 * (1 << 15) */
#define X092_Q14		15073                             /* 0.92 * (1 << 15) */
#define X095_Q14		15565                             /* 0.95 * (1 << 15) */
#define X098_Q14		16056                             /* 0.98 * (1 << 15) */
#define X102_Q14		16712                             /* 1.02 * (1 << 15) */
#define X105_Q14		17203                             /* 1.05 * (1 << 15) */

static void		realIDFT(Shortword mag[], Shortword phase[],
						 Shortword signal[], Shortword length);


/***************************************************************************
**
** Function:		realIDFT()
**
** Description:		IDFT to generate real output
**
** Arguments:
**
**	Shortword mag[]		input magnitudes (Q13)
**	Shortword phase[]	input phase (Q0)
**	Shortword signal[]	output signal (Q15)
**	Shortword length	The IDFT length
**
** Return value:			None
**
*****************************************************************************/
static void realIDFT(Shortword mag[], Shortword phase[], Shortword signal[],
					 Shortword length)
{
	register Shortword	i, j, k;
	Shortword	w, w2, length2;
	Shortword	temp;
	Longword	L_temp;
	Shortword	idftc[PITCHMAX];

	/*	length2 = (length/2) + 1; */
	length2 = add(shr(length, 1), 1);
	/*	w = TWOPI / length; */
	w = divide_s(TWO_Q3, length);                      /* w = 2/length in Q18 */

	/* The following for loop builds up the lookup table for cosines with     */
	/* radians from 0 to 1.                                                   */
	for (i = 0; i < length; i++){
		L_temp = L_mult(w, i);                               /* L_temp in Q19 */

		/* make sure argument for cos function is less than 1 */
		if (L_temp > (Longword) ONE_Q19){
			/*	cos(pi+x) = cos(pi-x) */
			L_temp = L_sub((Longword) TWO_Q19, L_temp);
		} else if (L_temp == (Longword) ONE_Q19)
			L_temp = L_sub(L_temp, 1);

		L_temp = L_shr(L_temp, 4);                           /* L_temp in Q15 */
		temp = extract_l(L_temp);
		idftc[i] = cos_fxp(temp);                             /* idftc in Q15 */
	}

	w = shr(w, 1);                                     /* w = 2/length in Q17 */
	w2 = shr(w, 1);                                   /* w2 = 1/length in Q17 */
	mag[0] = mult(mag[0], w2);                                /* mag[] in Q15 */
	temp = sub(length2, 1);
	for (i = 1; i < temp; i++){
		/*	mag[i] *= (2.0/length); */
		mag[i] = mult(mag[i], w);                         /* mag[] is now Q15 */
	}

	temp = shl(i, 1);
	if (temp == length)            /* length is even, mag[i] *= (1.0/length); */
		mag[i] = mult(mag[i], w2);
	else                            /* length is odd, mag[i] *= (2.0/length); */
		mag[i] = mult(mag[i], w);

	for (i = 0; i < length; i++){
		L_temp = L_deposit_h(mag[0]);                        /* L_temp in Q15 */
		k = i;
		for (j = 1; j < length2; j++){
			k = add(k, phase[j]);
			while (k < 0)
				k = add(k, length);
			while (k >= length)
				k = sub(k, length);
			L_temp = L_mac(L_temp, mag[j], idftc[k]);
			k = sub(k, phase[j]);
			k = add(k, i);
		}

		/* It might take some proofs, but mag[] is already weighted by w      */
		/* (which is inversely proportional to length) and L_temp here never  */
		/* overflows a Shortword.                                             */
		signal[i] = r_ound(L_temp);
		k = k;
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
**	Shortword bpvc[]	band voicing information (Q14)
**	Shortword *fc		output cut-off frequency (Q3)
**
** Return value:			None
**
*****************************************************************************/

void set_fc(Shortword bpvc[], Shortword *fc)
{
	register Shortword	i;
	Shortword	index;
	const Shortword		syn_bp_map[16] = {                              /* Q0 */
		 500,  500,  500,  500,  500,  500,  500, 4000,
		1000, 1000, 1000, 4000, 2000, 3000, 3000, 4000
	};

	/* ====== Generate voicing information ====== */
	if (bpvc[0] < X05_Q14){                        /* ---- Pure unvoiced ---- */
		*fc = 0;
		return;
	}

	/* ---- Voiced: pack bandpass voicing ---- */
	index = 0;
	bpvc[0] = ONE_Q14;
	for (i = 1; i < NUM_BANDS; i++){
		index <<= 1;                                            /* left shift */
		if (bpvc[i] > X05_Q14){
			bpvc[i] = ONE_Q14;
			index |= 1;
		} else {
			bpvc[i] = 0;
			index |= 0;
		}
	}
	*fc = (Shortword) (syn_bp_map[index] << 3);
}


/***************************************************************************
**
** Function:		harm_syn_pitch()
**
** Description:		harmonic synthesis for one pitch
**
** Arguments:
**
**	Shortword amp[]			input harmonic mags (Q13)
**	Shortword signal[]		output synthesized signal buffer (Q15)
**	Shortword fc			The cut-off frequency (Q3)
**	Shortword length		The pitch length
**
** Return value:			None
**
*****************************************************************************/
void harm_syn_pitch(Shortword amp[], Shortword signal[], Shortword fc, 
					Shortword length)
{
	register Shortword	i;
	Shortword	rndphase[SYN_FFT_SIZE/2 + 1];                           /* Q0 */
	Shortword	factor, fn;                                            /* Q15 */
	Shortword	temp1, temp2;
	Shortword	totalCnt, voicedCnt, mixedCnt, index;
	Shortword	mag[SYN_FFT_SIZE/2 + 1];
	Shortword	phase[SYN_FFT_SIZE/2 + 1];                              /* Q0 */
	Shortword	fc1, fc2;

	memzero(phase, (SYN_FFT_SIZE / 2 + 1) * sizeof(Shortword));

	/* ====== Generate random phase for unvoiced segment ====== */
	/* Note that phase[] and rndphase[] computed in harm_syn_pitch() are now  */
	/* the actual phases divided by (2*PI)/length.                            */

	for (i = 0; i < length/2 + 1; i++)
		rndphase[i] = mult(length, rand_minstdgen());

	/* ====== Harmonic Synthesis ====== */
	/* The fc1 and fc2 computed in the if block below are Q2 */

	if (fc <= X500_Q3){
		fc1 = mult(X085_Q14, fc);
		fc2 = mult(X105_Q14, fc);
		factor = ONE_Q15;
	} else if (fc <= X1000_Q3){
		fc1 = mult(X095_Q14, fc);
		fc2 = mult(X105_Q14, fc);
		factor = X09_Q15;
	} else if (fc <= X2000_Q3){
		fc1 = mult(X098_Q14, fc);
		fc2 = mult(X102_Q14, fc);
		factor = X08_Q15;
	} else if (fc <= X3000_Q3){
		fc1 = mult(X095_Q14, fc);
		fc2 = mult(X105_Q14, fc);
		factor = X075_Q15;
	} else {
		fc1 = mult(X092_Q14, fc);
		fc2 = shift_r(fc, -1);   /* We map fc (Q3) to fc2 (Q2) with r_ounding. */
		factor = X07_Q15;
	}

	/* fc1 and fc2 are now Q2. */

	temp1 = divide_s(fc1, shl(FSAMP, 2));
	temp2 = divide_s(fc2, shl(FSAMP, 2));     /* Now temp1 and temp2 are Q15. */
	voicedCnt = mult(temp1, length);
	mixedCnt = mult(temp2, length);
	totalCnt = (Shortword) ((length/2) + 1);

	/* ====== set values to mag and phase ====== */
	v_equ(mag, amp, add(voicedCnt, 1));                                /* Q13 */

	/* Now we compute phase[] in multiples of w = (2*PI)/length.  Therefore   */
	/* phase[] -> (i*FIXED_PHASE)*length/(2*PI).                              */
	temp1 = 0;                /* temp1 = i * temp2 in the following for loop. */
	temp2 = extract_l(L_mult(FIXED_PHASE, length));
	temp2 = shr(temp2, 1);                    /* temp2 = FIXED_PHASE * length */
	while (temp2 >= 2*length)
		temp2 = sub(temp2, (Shortword) (2*length));
	for (i = 0; i < mixedCnt + 1; i++){
		phase[i] = shr(temp1, 1);
		temp1 = add(temp1, temp2);
		if (temp1 >= 2*length)
			temp1 = sub(temp1, (Shortword) (2*length));
	}
	index = 0;
	for (i = add(voicedCnt, 1); i < add(mixedCnt, 1); i++, index ++){
		temp1 = sub(i, voicedCnt);
		temp2 = sub(mixedCnt, voicedCnt);
		fn = divide_s(temp1, temp2);                                   /* Q15 */
		temp1 = mult(factor, fn);
		temp2 = sub(ONE_Q15, fn);
		temp1 = add(temp1, temp2);
		mag[i] = mult(amp[i], temp1);                                  /* Q13 */
		temp1 = mult(fn, rndphase[index]);                              /* Q0 */
		temp2 = sub(phase[i], temp1);
		if (temp2 < 0)
			temp2 = add(temp2, length);
		phase[i] = temp2;
	}
	for (i = add(mixedCnt, 1); i < totalCnt; i++, index ++){
		mag[i] = mult(amp[i], factor);                                 /* Q13 */
		temp2 = negate(rndphase[index]);                                /* Q0 */
		if (temp2 < 0)
			temp2 = add(temp2, length);
		phase[i] = temp2;                         /* This moves phase[i] from */
                                                     /* negative to positive. */
	}

	/* ====== getting one pitch cycle ====== */
	realIDFT(mag, phase, signal, length);
}


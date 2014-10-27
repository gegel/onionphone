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
 File : PWF.C
 Used for the floating point version of G.729 main body
 (not for G.729A)
*/

#include <math.h>
#include "ld8k.h"

static int smooth = 1;
static float lar_old[2] = { (float) 0.0, (float) 0.0 };

/*----------------------------------------------------------------------------
* perc_var -adaptive bandwidth expansion for perceptual weighting filter
*----------------------------------------------------------------------------
*/
void perc_var(float * gamma1,	/* output: gamma1 value */
	      float * gamma2,	/* output: gamma2 value */
	      float * lsfint,	/* input : Interpolated lsf vector : 1st subframe */
	      float * lsfnew,	/* input : lsf vector : 2nd subframe */
	      float * r_c	/* input : Reflection coefficients */
    )
{
	float lar[4];
	float *lar_new;
	float *lsf;
	float critlar0, critlar1;
	float d_min, temp;
	int i, k;

	lar_new = &lar[2];

	/* reflection coefficients --> lar */
	for (i = 0; i < 2; i++)
		lar_new[i] = (float)
		    log10((double)(((float) 1.0 + r_c[i]) / ((float) 1.0 - r_c[i])));

	/* Interpolation of lar for the 1st subframe */
	for (i = 0; i < 2; i++) {
		lar[i] = (float) 0.5 *(lar_new[i] + lar_old[i]);
		lar_old[i] = lar_new[i];
	}

	for (k = 0; k < 2; k++) {	/* LOOP : gamma2 for 1st to 2nd subframes */

		/* ----------------------------------------------------- */
		/*   First criterion based on the first two lars         */
		/*                                                       */
		/* smooth == 1  ==>  gamma2 is set to 0.6                    */
		/*                   gamma1 is set to 0.94               */
		/*                                                       */
		/* smooth == 0  ==>  gamma2 can vary from 0.4 to 0.7     */
		/*                   (gamma2 = -6.0 dmin + 1.0)          */
		/*                   gamma1 is set to 0.98               */
		/* ----------------------------------------------------- */
		critlar0 = lar[2 * k];
		critlar1 = lar[2 * k + 1];

		if (smooth != 0) {
			if ((critlar0 < THRESH_L1) && (critlar1 > THRESH_H1))
				smooth = 0;
		} else {
			if ((critlar0 > THRESH_L2) || (critlar1 < THRESH_H2))
				smooth = 1;
		}

		if (smooth == 0) {
			/* ------------------------------------------------------ */
			/* Second criterion based on the minimum distance between */
			/* two successives lsfs                                   */
			/* ------------------------------------------------------ */
			gamma1[k] = GAMMA1_0;
			if (k == 0)
				lsf = lsfint;
			else
				lsf = lsfnew;
			d_min = lsf[1] - lsf[0];
			for (i = 1; i < M - 1; i++) {
				temp = lsf[i + 1] - lsf[i];
				if (temp < d_min)
					d_min = temp;
			}

			gamma2[k] = ALPHA * d_min + BETA;

			if (gamma2[k] > GAMMA2_0_H)
				gamma2[k] = GAMMA2_0_H;
			if (gamma2[k] < GAMMA2_0_L)
				gamma2[k] = GAMMA2_0_L;
		} else {
			gamma1[k] = GAMMA1_1;
			gamma2[k] = GAMMA2_1;;
		}
	}
	return;
}

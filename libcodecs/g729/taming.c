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
 File : TAMING.C
*/

/**************************************************************************
 * Taming functions.                                                      *
 **************************************************************************/
#include "ld8k.h"

static float exc_err[4];

void init_exc_err(void)
{
	int i;
	for (i = 0; i < 4; i++)
		exc_err[i] = (float) 1.;
	return;
}

/**************************************************************************
* routine test_err - computes the accumulated potential error in the     *
* adaptive codebook contribution                                         *
**************************************************************************/
int test_err(			/* (o) flag set to 1 if taming is necessary  */
		    int t0,	/* (i) integer part of pitch delay           */
		    int t0_frac	/* (i) fractional part of pitch delay        */
    )
{

	int i, t1, zone1, zone2, flag;
	float maxloc;

	t1 = (t0_frac > 0) ? (t0 + 1) : t0;

	i = t1 - L_SUBFR - L_INTER10;
	if (i < 0)
		i = 0;
	zone1 = (int)((float) i * INV_L_SUBFR);

	i = t1 + L_INTER10 - 2;
	zone2 = (int)((float) i * INV_L_SUBFR);

	maxloc = (float) - 1.;
	flag = 0;
	for (i = zone2; i >= zone1; i--) {
		if (exc_err[i] > maxloc)
			maxloc = exc_err[i];
	}
	if (maxloc > THRESH_ERR) {
		flag = 1;
	}
	return (flag);
}

/**************************************************************************
*routine update_exc_err - maintains the memory used to compute the error *
* function due to an adaptive codebook mismatch between encoder and      *
* decoder                                                                *
**************************************************************************/

void update_exc_err(float gain_pit,	/* (i) pitch gain */
		    int t0	/* (i) integer part of pitch delay */
    )
{
	int i, zone1, zone2, n;
	float worst, temp;

	worst = (float) - 1.;

	n = t0 - L_SUBFR;
	if (n < 0) {
		temp = (float) 1. + gain_pit * exc_err[0];
		if (temp > worst)
			worst = temp;
		temp = (float) 1. + gain_pit * temp;
		if (temp > worst)
			worst = temp;
	}

	else {
		zone1 = (int)((float) n * INV_L_SUBFR);

		i = t0 - 1;
		zone2 = (int)((float) i * INV_L_SUBFR);

		for (i = zone1; i <= zone2; i++) {
			temp = (float) 1. + gain_pit * exc_err[i];
			if (temp > worst)
				worst = temp;
		}
	}

	for (i = 3; i >= 1; i--)
		exc_err[i] = exc_err[i - 1];
	exc_err[0] = worst;

	return;
}

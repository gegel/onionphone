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
 File : COR_FUNC.C
*/

/* Functions corr_xy2() and cor_h_x()   */
#include "ld8k.h"

/*----------------------------------------------------------------------------
* corr_xy2 - compute the correlation products needed for gain computation
*----------------------------------------------------------------------------
*/
void corr_xy2(float xn[],	/* input : target vector x[0:l_subfr] */
	      float y1[],	/* input : filtered adaptive codebook vector */
	      float y2[],	/* input : filtered 1st codebook innovation */
	      float g_coeff[]	/* output: <y2,y2> , -2<xn,y2> , and 2<y1,y2> */
    )
{
	float y2y2, xny2, y1y2;
	int i;

	y2y2 = (float) 0.01;
	for (i = 0; i < L_SUBFR; i++)
		y2y2 += y2[i] * y2[i];
	g_coeff[2] = y2y2;

	xny2 = (float) 0.01;
	for (i = 0; i < L_SUBFR; i++)
		xny2 += xn[i] * y2[i];
	g_coeff[3] = (float) - 2.0 * xny2;

	y1y2 = (float) 0.01;
	for (i = 0; i < L_SUBFR; i++)
		y1y2 += y1[i] * y2[i];
	g_coeff[4] = (float) 2.0 *y1y2;

	return;

}

/*--------------------------------------------------------------------------*
*  Function  cor_h_x()                                                     *
*  ~~~~~~~~~~~~~~~~~~~~                                                    *
* Compute  correlations of input response h[] with the target vector X[].  *
*--------------------------------------------------------------------------*/

void cor_h_x(float h[],		/* (i) :Impulse response of filters      */
	     float x[],		/* (i) :Target vector                    */
	     float d[]		/* (o) :Correlations between h[] and x[] */
    )
{
	int i, j;
	float s;

	for (i = 0; i < L_SUBFR; i++) {
		s = (float) 0.0;
		for (j = i; j < L_SUBFR; j++)
			s += x[j] * h[j - i];
		d[i] = s;
	}

	return;
}

/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                         Version 2.1 of October 1999
*/

/*
 File : LPFUNCCP.C
*/

#include <math.h>
#include "ld8k.h"
#include "tab_ld8k.h"
#include "ld8cp.h"

/* local function */
static void get_lsp_pol(float lsf[], float f[]);

/*-----------------------------------------------------------------------------
* lsp_az - convert LSPs to predictor coefficients a[]
*-----------------------------------------------------------------------------
*/
void lsp_az(float * lsp,	/* input : lsp[0:M-1] */
	    float * a		/* output: predictor coeffs a[0:M], a[0] = 1. */
    )
{

	float f1[NC + 1], f2[NC + 1];
	int i, j;

	get_lsp_pol(&lsp[0], f1);
	get_lsp_pol(&lsp[1], f2);

	for (i = NC; i > 0; i--) {
		f1[i] += f1[i - 1];
		f2[i] -= f2[i - 1];
	}
	a[0] = (float) 1.0;
	for (i = 1, j = M; i <= NC; i++, j--) {
		a[i] = (float) 0.5 *(f1[i] + f2[i]);
		a[j] = (float) 0.5 *(f1[i] - f2[i]);
	}

	return;
}

/*----------------------------------------------------------------------------
* get_lsp_pol - find the polynomial F1(z) or F2(z) from the LSFs
*----------------------------------------------------------------------------
*/
static void get_lsp_pol(float lsp[],	/* input : line spectral freq. (cosine domain)  */
			float f[]	/* output: the coefficients of F1 or F2 */
    )
{
	float b;
	int i, j;

	f[0] = (float) 1.0;
	b = (float) - 2.0 * lsp[0];
	f[1] = b;
	for (i = 2; i <= NC; i++) {
		b = (float) - 2.0 * lsp[2 * i - 2];
		f[i] = b * f[i - 1] + (float) 2.0 *f[i - 2];
		for (j = i - 1; j > 1; j--)
			f[j] += b * f[j - 1] + f[j - 2];
		f[1] += b;
	}
	return;
}

/*----------------------------------------------------------------------------
* lsp_lsf - convert from lsp[0..M-1 to lsf[0..M-1]
*----------------------------------------------------------------------------
*/
void lsp_lsf(float lsp[],	/* input :  lsp coefficients */
	     float lsf[],	/* output:  lsf (normalized frequencies */
	     int m) {
	int i;

	for (i = 0; i < m; i++)
		lsf[i] = (float) acos((double)lsp[i]);
	return;
}

/*---------------------------------------------------------------------------
* weigh_az:  Weighting of LPC coefficients  ap[i]  =  a[i] * (gamma ** i)
*---------------------------------------------------------------------------
*/
void weight_az(float * a,	/* input : lpc coefficients a[0:m] */
	       float gamma,	/* input : weighting factor */
	       int m,		/* input : filter order */
	       float * ap	/* output: weighted coefficients ap[0:m] */
    )
{
	float fac;
	int i;

	ap[0] = a[0];
	fac = gamma;
	for (i = 1; i < m; i++) {
		ap[i] = fac * a[i];
		fac *= gamma;
	}
	ap[m] = fac * a[m];
	return;
}

/*-----------------------------------------------------------------------------
* int_qlpc -  interpolated M LSP parameters and convert to M+1 LPC coeffs
*-----------------------------------------------------------------------------
*/
void int_qlpc(float lsp_old[],	/* input : LSPs for past frame (0:M-1) */
	      float lsp_new[],	/* input : LSPs for present frame (0:M-1) */
	      float az[]	/* output: filter parameters in 2 subfr (dim 2(m+1)) */
    )
{
	int i;
	float lsp[M];

	for (i = 0; i < M; i++)
		lsp[i] = lsp_old[i] * (float) 0.5 + lsp_new[i] * (float) 0.5;

	lsp_az(lsp, az);
	lsp_az(lsp_new, &az[M + 1]);

	return;
}

/*-----------------------------------------------------------------------------
* int_lpc -  interpolated M LSP parameters and convert to M+1 LPC coeffs
*-----------------------------------------------------------------------------
*/
void int_lpc(float lsp_old[],	/* input : LSPs for past frame (0:M-1) */
	     float lsp_new[],	/* input : LSPs for present frame (0:M-1) */
	     float lsf_int[],	/* output: interpolated lsf coefficients */
	     float lsf_new[],	/* input : LSFs for present frame (0:M-1) */
	     float az[]		/* output: filter parameters in 2 subfr (dim 2(m+1)) */
    )
{
	int i;
	float lsp[M];

	for (i = 0; i < M; i++)
		lsp[i] = lsp_old[i] * (float) 0.5 + lsp_new[i] * (float) 0.5;

	lsp_az(lsp, az);

	lsp_lsf(lsp, lsf_int, M);
	lsp_lsf(lsp_new, lsf_new, M);

	return;
}

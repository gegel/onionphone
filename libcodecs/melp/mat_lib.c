/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*

2.4 kbps MELP Proposed Federal Standard mf_speech coder

version 1.2

Copyright (c) 1996, Texas Instruments, Inc.  

Texas Instruments has intellectual property rights on the MELP
algorithm.  The Texas Instruments contact for licensing issues for
commercial and non-government use is William Gordon, Director,
Government Contracts, Texas Instruments Incorporated, Semiconductor
Group (phone 972 480 7442).


*/

/*

  mat_lib.c: Matrix and vector manipulation library

*/

#include "spbstd.h"
#include "mat.h"

/* V_ADD- vector addition */
float *mf_v_add(float *v1, float *v2, int n)
{
	int i;

	for (i = 0; i < n; i++)
		v1[i] += v2[i];
	return (v1);
}

/* V_EQU- vector equate */
float *mf_v_equ(float *v1, float *v2, int n)
{
	int i;

	for (i = 0; i < n; i++)
		v1[i] = v2[i];
	return (v1);
}

int *mf_mf_v_equ_int(int *v1, int *v2, int n)
{
	int i;

	for (i = 0; i < n; i++)
		v1[i] = v2[i];
	return (v1);
}

/* V_INNER- inner product */
float mf_v_inner(float *v1, float *v2, int n)
{
	int i;
	float innerprod;

	for (i = 0, innerprod = 0.0; i < n; i++)
		innerprod += v1[i] * v2[i];
	return (innerprod);
}

/* v_mf_magsq - sum of squares */

float v_mf_magsq(float *v, int n)
{
	int i;
	float mf_magsq;

	for (i = 0, mf_magsq = 0.0; i < n; i++)
		mf_magsq += v[i] * v[i];
	return (mf_magsq);
}				/* V_MAGSQ */

/* V_SCALE- vector scale */
float *mf_v_scale(float *v, float scale, int n)
{
	int i;

	for (i = 0; i < n; i++)
		v[i] *= scale;
	return (v);
}

/* V_SUB- vector difference */
float *mf_v_sub(float *v1, float *v2, int n)
{
	int i;

	for (i = 0; i < n; i++)
		v1[i] -= v2[i];
	return (v1);
}

/* mf_v_zap - clear vector */

float *mf_v_zap(float *v, int n)
{
	int i;

	for (i = 0; i < n; i++)
		v[i] = 0.0;
	return (v);
}				/* V_ZAP */

int *mf_mf_v_zap_int(int *v, int n)
{
	int i;

	for (i = 0; i < n; i++)
		v[i] = 0;
	return (v);
}				/* V_ZAP */

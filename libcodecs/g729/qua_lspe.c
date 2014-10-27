/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/

/*
 File : QUA_LSPE.C
*/

/*----------------------------------------------------------*
 *  qua_lsp.c                                               *
 *  ~~~~~~~~                                                *
 * Functions related to the quantization of LSP's           *
 *----------------------------------------------------------*/
#include <math.h>
#include "ld8k.h"
#include "tab_ld8k.h"
#include "ld8cp.h"
#include "tabld8cp.h"

/* Prototype definitions of static functions */
void get_wegt(float flsp[], float wegt[]);
static void lsp_pre_select(float rbuf[], float lspcb1[][M], int *cand);
static void lsp_select_1(float rbuf[], float lspcb1[], float wegt[],
			 float lspcb2[][M], int *index);
static void lsp_select_2(float rbuf[], float lspcb1[], float wegt[],
			 float lspcb2[][M], int *index);
static void lsp_last_select(float tdist[MODE], int *mode_index);
static void lsp_get_tdist(float wegt[], float buf[],
			  float * tdist, float rbuf[], float fg_sum[]);
void qua_lspe(float lsp[],	/* (i) : Unquantized LSP            */
	      float lsp_q[],	/* (o) : Quantized LSP              */
	      int ana[],	/* (o) : indexes                    */
	      float freq_prev[MA_NP][M],	/* (i) : previous LSP MA vector        */
	      float freq_cur[]	/* (o) : current LSP MA vector        */
    )
{
	int i;
	float lsf[M], lsf_q[M];	/* domain 0.0<= lsf <PI */

	/* Convert LSPs to LSFs */
	for (i = 0; i < M; i++)
		lsf[i] = (float) acos(lsp[i]);

	lsp_qua_cse(lsf, lsf_q, ana, freq_prev, freq_cur);

	/* Convert LSFs to LSPs */
	for (i = 0; i < M; i++)
		lsp_q[i] = (float) cos(lsf_q[i]);

	return;
}

/*----------------------------------------------------------------------------
* lsp_encw_reset - set the previous LSP vector
*----------------------------------------------------------------------------
*/
void lsp_encw_resete(float freq_prev[MA_NP][M]	/* (i) : previous LSP MA vector        */
    )
{
	int i;
	for (i = 0; i < MA_NP; i++)
		copy(&freq_prev_reset[0], &freq_prev[i][0], M);
	return;
}

/*----------------------------------------------------------------------------
* lsp_qua_cs - lsp quantizer
*----------------------------------------------------------------------------
*/
void lsp_qua_cse(float * flsp_in,	/*  input : Original LSP parameters      */
		 float * lspq_out,	/*  output: Quantized LSP parameters     */
		 int *code,	/*  output: codes of the selected LSP    */
		 float freq_prev[MA_NP][M],	/* (i)  : previous LSP MA vector        */
		 float freq_cur[]	/* (o)  : current LSP MA vector        */
    )
{
	float wegt[M];		/* weight coef. */

	get_wegt(flsp_in, wegt);

	relspwede(flsp_in, wegt, lspq_out, lspcb1, lspcb2, fg,
		  freq_prev, fg_sum, fg_sum_inv, code, freq_cur);
	return;
}

/*----------------------------------------------------------------------------
* relspwed -
*----------------------------------------------------------------------------
*/
void relspwede(float lsp[],	/*input: unquantized LSP parameters  */
	       float wegt[],	/*input: weight coef.                */
	       float lspq[],	/*output:quantized LSP parameters    */
	       float lspcb1[][M],	/*input: first stage LSP codebook    */
	       float lspcb2[][M],	/*input: Second stage LSP codebook   */
	       float fg[MODE][MA_NP][M],	/*input: MA prediction coef.         */
	       float freq_prev[MA_NP][M],	/*input: previous LSP vector         */
	       float fg_sum[MODE][M],	/*input: present MA prediction coef. */
	       float fg_sum_inv[MODE][M],	/*input: inverse coef.               */
	       int code_ana[],	/*output:codes of the selected LSP   */
	       float freq_cur[]	/* (o) : current LSP MA vector        */
    )
{
	int mode, j;
	int index, mode_index;
	int cand[MODE], cand_cur;
	int tindex1[MODE], tindex2[MODE];
	float tdist[MODE];
	float rbuf[M];
	float buf[M];

	for (mode = 0; mode < MODE; mode++) {

		lsp_prev_extract(lsp, rbuf, fg[mode], freq_prev,
				 fg_sum_inv[mode]);

	/*----- search the first stage lsp codebook -----*/
		lsp_pre_select(rbuf, lspcb1, &cand_cur);
		cand[mode] = cand_cur;

	/*----- search the second stage lsp codebook (lower 0-4) ----- */
		lsp_select_1(rbuf, lspcb1[cand_cur], wegt, lspcb2, &index);

		tindex1[mode] = index;

		for (j = 0; j < NC; j++)
			buf[j] = lspcb1[cand_cur][j] + lspcb2[index][j];

		lsp_expand_1(buf, GAP1);	/* check */

	/*----- search the second stage lsp codebook (Higher 5-9) ----- */
		lsp_select_2(rbuf, lspcb1[cand_cur], wegt, lspcb2, &index);

		tindex2[mode] = index;

		for (j = NC; j < M; j++)
			buf[j] = lspcb1[cand_cur][j] + lspcb2[index][j];
		lsp_expand_2(buf, GAP1);	/* check */

		/* check */
		lsp_expand_1_2(buf, GAP2);

		lsp_get_tdist(wegt, buf, &tdist[mode], rbuf, fg_sum[mode]);	/* calculate the distortion */

	}			/* mode */

	lsp_last_select(tdist, &mode_index);	/* select the codes */

	/* pack codes for lsp parameters */
	code_ana[0] = (mode_index << NC0_B) | cand[mode_index];
	code_ana[1] = (tindex1[mode_index] << NC1_B) | tindex2[mode_index];

	/* reconstruct quantized LSP parameter and check the stabilty */
	lsp_get_quante(lspcb1, lspcb2, cand[mode_index],
		       tindex1[mode_index], tindex2[mode_index],
		       fg[mode_index],
		       freq_prev, lspq, fg_sum[mode_index], freq_cur);

	return;
}

/*----------------------------------------------------------------------------
* lsp_pre_select - select the code of first stage lsp codebook
*----------------------------------------------------------------------------
*/
static void lsp_pre_select(float rbuf[],	/*input : target vetor             */
			   float lspcb1[][M],	/*input : first stage lsp codebook */
			   int *cand	/*output: selected code            */
    )
{
	int i, j;
	float dmin, dist, temp;

	/* calculate the distortion */
	*cand = 0;
	dmin = FLT_MAX_G729;
	for (i = 0; i < NC0; i++) {
		dist = (float) 0.;
		for (j = 0; j < M; j++) {
			temp = rbuf[j] - lspcb1[i][j];
			dist += temp * temp;
		}

		if (dist < dmin) {
			dmin = dist;
			*cand = i;
		}
	}
	return;
}

/*----------------------------------------------------------------------------
* lsp_pre_select_1 - select the code of second stage lsp codebook (lower 0-4)
*----------------------------------------------------------------------------
*/
static void lsp_select_1(float rbuf[],	/*input : target vector            */
			 float lspcb1[],	/*input : first stage lsp codebook */
			 float wegt[],	/*input : weight coef.             */
			 float lspcb2[][M],	/*input : second stage lsp codebook */
			 int *index	/*output: selected codebook index     */
    )
{
	int j, k1;
	float buf[M];
	float dist, dmin, tmp;

	for (j = 0; j < NC; j++)
		buf[j] = rbuf[j] - lspcb1[j];

	*index = 0;
	dmin = FLT_MAX_G729;
	for (k1 = 0; k1 < NC1; k1++) {
		/* calculate the distortion */
		dist = (float) 0.;
		for (j = 0; j < NC; j++) {
			tmp = buf[j] - lspcb2[k1][j];
			dist += wegt[j] * tmp * tmp;
		}

		if (dist < dmin) {
			dmin = dist;
			*index = k1;
		}
	}
	return;
}

/*----------------------------------------------------------------------------
* lsp_pre_select_2 - select the code of second stage lsp codebook (higher 5-9)
*----------------------------------------------------------------------------
*/
static void lsp_select_2(float rbuf[],	/*input : target vector            */
			 float lspcb1[],	/*input : first stage lsp codebook */
			 float wegt[],	/*input : weighting coef.             */
			 float lspcb2[][M],	/*input : second stage lsp codebook */
			 int *index	/*output: selected codebook index    */
    )
{
	int j, k1;
	float buf[M];
	float dist, dmin, tmp;

	for (j = NC; j < M; j++)
		buf[j] = rbuf[j] - lspcb1[j];

	*index = 0;
	dmin = FLT_MAX_G729;
	for (k1 = 0; k1 < NC1; k1++) {
		dist = (float) 0.0;
		for (j = NC; j < M; j++) {
			tmp = buf[j] - lspcb2[k1][j];
			dist += wegt[j] * tmp * tmp;
		}

		if (dist < dmin) {
			dmin = dist;
			*index = k1;
		}
	}
	return;
}

/*----------------------------------------------------------------------------
* lsp_get_tdist - calculate the distortion
*----------------------------------------------------------------------------
*/
static void lsp_get_tdist(float wegt[],	/*input : weight coef.          */
			  float buf[],	/*input : candidate LSP vector  */
			  float * tdist,	/*output: distortion            */
			  float rbuf[],	/*input : target vector         */
			  float fg_sum[]	/*input : present MA prediction coef.  */
    )
{
	int j;
	float tmp;

	*tdist = (float) 0.0;
	for (j = 0; j < M; j++) {
		tmp = (buf[j] - rbuf[j]) * fg_sum[j];
		*tdist += wegt[j] * tmp * tmp;
	}
	return;
}

/*----------------------------------------------------------------------------
* lsp_last_select - select the mode
*----------------------------------------------------------------------------
*/
static void lsp_last_select(float tdist[],	/*input : distortion         */
			    int *mode_index	/*output: the selected mode  */
    )
{
	*mode_index = 0;
	if (tdist[1] < tdist[0])
		*mode_index = 1;
	return;
}

/*----------------------------------------------------------------------------
* get_wegt - compute lsp weights
*----------------------------------------------------------------------------
*/
void get_wegt(float flsp[],	/* input : M LSP parameters */
	      float wegt[]	/* output: M weighting coefficients */
    )
{
	int i;
	float tmp;

	tmp = flsp[1] - PI04 - (float) 1.0;
	if (tmp > (float) 0.0)
		wegt[0] = (float) 1.0;
	else
		wegt[0] = tmp * tmp * (float) 10. + (float) 1.0;

	for (i = 1; i < M - 1; i++) {
		tmp = flsp[i + 1] - flsp[i - 1] - (float) 1.0;
		if (tmp > (float) 0.0)
			wegt[i] = (float) 1.0;
		else
			wegt[i] = tmp * tmp * (float) 10. + (float) 1.0;
	}

	tmp = PI92 - flsp[M - 2] - (float) 1.0;
	if (tmp > (float) 0.0)
		wegt[M - 1] = (float) 1.0;
	else
		wegt[M - 1] = tmp * tmp * (float) 10. + (float) 1.0;

	wegt[4] *= CONST12;
	wegt[5] *= CONST12;
	return;
}

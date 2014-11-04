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
/* File:		qnt12.c 											*/
/*																	*/
/* Description: quantization for 1200bps							*/
/*																	*/
/*------------------------------------------------------------------*/

#include "sc1200.h"
#include "lpc_lib.h"
#include "vq_lib.h"
#include "global.h"
#include "macro.h"
#include "qnt12_cb.h"
#include "mat_lib.h"
#include "math_lib.h"
#include "qnt12.h"
#include "constant.h"
#include "mathhalf.h"
#include "msvq_cb.h"
#include "fsvq_cb.h"
#include "dsp_sub.h"
#include "melp_sub.h"

#define	LSP_INP_CAND		5
#define NF_X_NUM_GAINFR		(NF * NUM_GAINFR)

#define X0333_Q15			10923	/* (1/3) * (1 << 15) */
#define X0667_Q15			21845	/* (2/3) * (1 << 15) */

/* ------ Local prototypes ------ */
static void wvq1(int16_t target[], int16_t weights[],
		 const int16_t codebook[], int16_t dim,
		 int16_t cbsize, int16_t index[], int32_t dist[],
		 int16_t cand);
static int16_t InsertCand(int16_t c1, int16_t s1, int16_t dMin[],
			    int16_t distortion, int16_t entry,
			    int16_t nextIndex[], int16_t index[]);
static int16_t wvq2(int16_t target[], int16_t weights[],
		      int16_t codebook[], int16_t dim,
		      int16_t index[], int32_t dist[], int16_t cand);
static int16_t WeightedMSE(int16_t n, int16_t weight[],
			     const int16_t x[], int16_t target[],
			     int16_t max_dMin);
static void lspVQ(int16_t target[], int16_t weight[], int16_t qout[],
		  const int16_t codebook[], int16_t tos,
		  const int16_t cb_size[], int16_t cb_index[],
		  int16_t dim, BOOLEAN flag);

/****************************************************************************
**
** Function:		pitch_vq
**
** Description: 	Pitch values of three frames are vector quantized
**
** Arguments:
**
**	melp_param *par ---- input/output melp parameters
**
** Return value:	None
**
*****************************************************************************/
void pitch_vq(struct melp_param *par)
{
	register int16_t i;
	static BOOLEAN prev_uv_flag = TRUE;
	static int16_t prev_pitch = LOG_UV_PITCH_Q12;	/* Q12 */
	static int16_t prev_qpitch = LOG_UV_PITCH_Q12;	/* Q12 */
	const int16_t *codebook;
	int16_t cnt, size, pitch_index;
	int16_t temp1, temp2;
	int32_t L_temp;
	int16_t dcb[PITCH_VQ_CAND * NF];	/* Q12 */
	int16_t target[NF], deltp[NF];	/* Q12 */
	int16_t deltw[NF];	/* Q0 */
	int16_t weights[NF];	/* Q0 */
	int16_t indexlist[PITCH_VQ_CAND];
	int32_t distlist[PITCH_VQ_CAND];	/* Q25 */

	/* ---- Compute pitch in log domain ---- */
	for (i = 0; i < NF; i++)
		target[i] = log10_fxp(par[i].pitch, 7);	/* Q12 */

	cnt = 0;
	for (i = 0; i < NF; i++) {
		if (par[i].uv_flag)
			weights[i] = 0;	/* Q0 */
		else {
			weights[i] = 1;
			cnt++;
		}
	}

	/* ---- calculate delta ---- */
	for (i = 0; i < NF; i++) {
		if (prev_uv_flag || par[i].uv_flag) {
			deltp[i] = 0;
			deltw[i] = 0;
		} else {
			deltp[i] = melpe_sub(target[i], prev_pitch);
			deltw[i] = DELTA_PITCH_WEIGHT_Q0;
		}
		prev_pitch = target[i];
		prev_uv_flag = par[i].uv_flag;
	}

	if (cnt == 0) {

		for (i = 0; i < NF; i++)
			par[i].pitch = UV_PITCH;
		prev_qpitch = LOG_UV_PITCH_Q12;

	} else if (cnt == 1) {

		for (i = 0; i < NF; i++) {
			if (!par[i].uv_flag) {
				quant_u(&target[i], &(quant_par.pitch_index),
					PIT_QLO_Q12, PIT_QUP_Q12, PIT_QLEV_M1,
					PIT_QLEV_M1_Q8, TRUE, 7);
				quant_u_dec(quant_par.pitch_index,
					    &par[i].pitch, PIT_QLO_Q12,
					    PIT_QUP_Q12, PIT_QLEV_M1_Q8, 7);
			} else
				par[i].pitch = LOG_UV_PITCH_Q12;
		}

		/* At this point par[].pitch temporarily holds the pitches in the     */
		/* log domain with Q12.                                               */

		prev_qpitch = par[NF - 1].pitch;	/* Q12 */

		for (i = 0; i < NF; i++)
			par[i].pitch = pow10_fxp(par[i].pitch, 7);	/* Q7 */

	} else if (cnt > 1) {	/* cnt == 2, 3, ......, (NF - 1) */
		/* ----- set pointer ----- */
		if (cnt == NF) {	/* All NF frames are voiced. */
			codebook = pitch_vq_cb_vvv;
			size = PITCH_VQ_LEVEL_VVV;
		} else {
			codebook = pitch_vq_cb_uvv;
			size = PITCH_VQ_LEVEL_UVV;
		}		/* This part changed !!! (12/13/99) */

		/* ---- select candidate using static pitch distortion ---- */
		wvq1(target, weights, codebook, NF, size, indexlist, distlist,
		     PITCH_VQ_CAND);

		/* -- select index using static and delta pitch distortion -- */
		temp1 = 0;
		for (i = 0; i < PITCH_VQ_CAND; i++) {
			L_temp = melpe_L_mult(indexlist[i], NF);
			L_temp = melpe_L_shr(L_temp, 1);
			temp2 = melpe_extract_l(L_temp);

			/* Now temp1 is (i*NF) and temp2 is (indexlist[i]*NF).            */

			dcb[temp1] = melpe_sub(codebook[temp2], prev_qpitch);	/* Q12 */
			v_equ(&dcb[temp1 + 1], &codebook[temp2 + 1], NF - 1);
			v_sub(&dcb[temp1 + 1], &codebook[temp2], NF - 1);
			temp1 = melpe_add(temp1, NF);
		}

		pitch_index = wvq2(deltp, deltw, dcb, NF, indexlist, distlist,
				   PITCH_VQ_CAND);

		if (par[NF - 1].uv_flag)
			prev_qpitch = LOG_UV_PITCH_Q12;
		else
			prev_qpitch = codebook[pitch_index * NF + NF - 1];	/* Q12 */

		for (i = 0; i < NF; i++) {
			if (par[i].uv_flag)
				par[i].pitch = UV_PITCH_Q7;
			else
				par[i].pitch =
				    pow10_fxp(codebook[pitch_index * NF + i],
					      7);
		}

		quant_par.pitch_index = pitch_index;
	}
}

/****************************************************************************
**
** Function:		wvq1
**
** Description: 	Pitch vq the first stage
**
**    The purpose of wvq1() is to loop through all the "cbsize" entries of
**    codebook[] (PITCH_VQ_LEVEL_VVV or PITCH_VQ_LEVEL_UVV, 512 or 2048) and
**    record the "cand" entries (PITCH_VQ_CAND, 16) which yields the minimum
**    errors.
**
** Arguments:
**	int16_t target[] 	: target vector (Q12)
**	int16_t weights[]	: weighting vector (Q0)
**	int16_t codebook[]: codebook (Q12)
**	int16_t dim 		: vector dimension
**	int16_t cbsize	: codebook size
**	int16_t index[]	: output candidate index list
**	int32_t dist[]		: output candidate distortion list (Q25)
**	int16_t cand		: number of output candidates
**
** Return value:	None
**
*****************************************************************************/
static void wvq1(int16_t target[], int16_t weights[],
		 const int16_t codebook[], int16_t dim,
		 int16_t cbsize, int16_t index[], int32_t dist[],
		 int16_t cand)
{
	register int16_t i, j;
	int16_t maxindex;
	int32_t err, maxdist;	/* Q25 */
	int32_t L_temp;
	int16_t temp;		/* Q12 */

	/* ------ Initialize the distortion ------ */
	L_fill(dist, LW_MAX, cand);

	maxdist = LW_MAX;
	maxindex = 0;

	/* ------ Search the codebook ------ */
	for (i = 0; i < cbsize; i++) {

		err = 0;

		/* Here the for loop computes the distortion between target[] and     */
		/* codebook[] and stores the result in int32_t err.  If              */
		/* (err < maxdist) then we execute some actions.  If err is already   */
		/* larger than or equal to maxdist, there is no need to keep          */
		/* computing the distortion.  This improvement is only marginal       */
		/* because "dim" is small (NF == 3).                                  */

		for (j = 0; j < dim; j++) {
			if (weights[j] > 0) {	/* weights[] is either 1 or 0. */
				/*      err += SQR(target[j] - codebook[j]); */
				temp = melpe_sub(target[j], codebook[j]);	/* Q12 */
				L_temp = melpe_L_mult(temp, temp);	/* Q25 */
				L_temp = melpe_L_shr(L_temp, 2);	/* Q23 */
				err = melpe_L_add(err, L_temp);
				if (err >= maxdist)
					break;
			}
		}
		if (err < maxdist) {
			index[maxindex] = i;
			dist[maxindex] = err;

			/* The following loop forgets maxindex and maxdist and finds them */
			/* from scratch.  This is very inefficient because "cand" is      */
			/* PITCH_VQ_CAND (== 16) and we know the maximum is always        */
			/* replaced just now by the new "err".  However, an attempt of    */
			/* keeping dist[] sorted (so the following loop is not needed     */
			/* every time we update maxdist) only shows minimal improvement.  */

			maxdist = 0;
			for (j = 0; j < cand; j++) {
				if (dist[j] > maxdist) {
					maxdist = dist[j];
					maxindex = j;
				}
			}
		}
		codebook += dim;	/* Pointer arithmetics. */
	}
}

/****************************************************************************
**
** Function:		wvq2
**
** Description: 	Pitch vq the second stage
**
** Arguments:
**	int16_t target[] 	: target vector (Q12)
**	int16_t weights[]	: weighting vector (Q0)
**	int16_t codebook[]: codebook (Q12)
**	int16_t dim 		: vector dimension
**	int16_t index[]	: codebook index
**	int32_t dist[]		: distortion (Q25)
**	int16_t cand		: number of input candidates
**
** Return value:	int16_t ---- the final codebook index
**
*****************************************************************************/
static int16_t wvq2(int16_t target[], int16_t weights[],
		      int16_t codebook[], int16_t dim,
		      int16_t index[], int32_t dist[], int16_t cand)
{
	register int16_t i, j;
	int16_t ind;
	int32_t err, min;
	int32_t L_temp;
	int16_t temp;

	/* To reduce the complexity, we should try to increase the opportunity of */
	/* making (err >= min).  In other words, set "min" as small as possible   */
	/* before the loop begins.  One idea is to find the i which minimizes     */
	/* dist[] and use it to compute "min" and "ind" first, then loop through  */
	/* all candidates except this i.  This scheme only reduces about 1/100 of */
	/* the execution time, so it is not implemented here.                     */

	min = LW_MAX;
	ind = 0;

	for (i = 0; i < cand; i++) {
		err = dist[i];

		for (j = 0; j < dim; j++) {
			if (weights[j] > 0) {

				/* weights[] are either 0 or a positive constant              */
				/* DELTA_PITCH_WEIGHT_Q0 == 1.  Note that the following code  */
				/* segment no longer works correctly if DELTA_PITCH_WEIGHT_Q0 */
				/* is changed to other value.                                 */

				/*      err += weights[j] * SQR(target[j] - codebook[j]); */

				temp = melpe_sub(target[j], codebook[j]);	/* Q12 */
				L_temp = melpe_L_mult(temp, temp);	/* Q25 */
				L_temp = melpe_L_shr(L_temp, 2);	/* Q23 */
				err = melpe_L_add(err, L_temp);	/* Q23 */

				/* Exit the loop if (err >= min). */
				if (err >= min)
					break;
			}
		}
		if (err < min) {
			min = err;
			ind = index[i];
		}
		codebook += dim;	/* Pointer arithmetics. */
	}

	return (ind);
}

/****************************************************************************
**
** Function:		gain_vq
**
** Description: 	Gain quantization for 1200bps
**
** Arguments:
**
**	melp_param *par ---- input/output melp parameters
**
** Return value:	None
**
*****************************************************************************/
void gain_vq(struct melp_param *par)
{
	register int16_t i, j;
	int16_t index;
	int16_t temp, temp2;
	int16_t gain_target[NF_X_NUM_GAINFR];
	int32_t err, minErr;	/* Q17 */
	int32_t L_temp;

	/* Reshape par[i].gain[j] into a one-dimensional vector gain_target[]. */
	temp = 0;
	for (i = 0; i < NF; i++) {
		v_equ(&(gain_target[temp]), par[i].gain, NUM_GAINFR);
		temp = melpe_add(temp, NUM_GAINFR);
	}

	minErr = LW_MAX;
	index = 0;
	temp2 = 0;
	for (i = 0; i < GAIN_VQ_SIZE; i++) {

		/*      temp2 = i * NF * NUM_GAINFR; */

		err = 0;

		/* j = 0 for the following for loop. */
		temp = melpe_sub(gain_target[0], gain_vq_cb[temp2]);	/* Q8 */
		L_temp = melpe_L_mult(temp, temp);	/* Q17 */
		L_temp = melpe_L_shr(L_temp, 3);	/* Q14 */
		err = melpe_L_add(err, L_temp);	/* Q14 */

		/* For the sum of 6 terms, if the first term already exceeds minErr,  */
		/* there is no need to keep computing.                                */

		if (err < minErr) {
			for (j = 1; j < NF_X_NUM_GAINFR; j++) {
				/*      err += SQR(par[j].gain[k] -
				   gain_vq_cb[i*NUM_GAINFR*NF + j*NUM_GAINFR + k]); */
				temp = melpe_sub(gain_target[j], gain_vq_cb[temp2 + j]);	/* Q8 */
				L_temp = melpe_L_mult(temp, temp);	/* Q17 */
				L_temp = melpe_L_shr(L_temp, 3);	/* Q14 */
				err = melpe_L_add(err, L_temp);	/* Q14 */
			}
			if (err < minErr) {
				minErr = err;
				index = i;
			}
		}
		temp2 = melpe_add(temp2, NF_X_NUM_GAINFR);
	}

	/*      temp2 = index * NF * NUM_GAINFR; */
	L_temp = melpe_L_mult(index, NF_X_NUM_GAINFR);
	L_temp = melpe_L_shr(L_temp, 1);
	temp2 = melpe_extract_l(L_temp);
	for (i = 0; i < NF; i++) {
		/*      v_equ(par[i].gain, &(gain_vq_cb[index*NUM_GAINFR*NF +
		   i*NUM_GAINFR]), NUM_GAINFR); */
		v_equ(par[i].gain, &(gain_vq_cb[temp2]), NUM_GAINFR);
		temp2 = melpe_add(temp2, NUM_GAINFR);
	}

	quant_par.gain_index[0] = index;
}

/****************************************************************************
**
** Function:		quant_bp
**
** Description: 	Quantization the band-pass voicing for 1200bps
**
** Arguments:
**
**	melp_param *par ---- input/output melp parameters
**
** Return value:	None
**
*****************************************************************************/

void quant_bp(struct melp_param *par, int16_t num_frames)
{
	register int16_t i;

	for (i = 0; i < num_frames; i++) {
		par[i].uv_flag = q_bpvc(par[i].bpvc, &(quant_par.bpvc_index[i]),
					NUM_BANDS);
		quant_par.bpvc_index[i] = bp_index_map[quant_par.bpvc_index[i]];
	}
}

/********************************************************************
**
** Function: lspVQ ()
**
** Description:
**		Vector quantizes a set of int term filter coefficients
**		using a multi-stage M-L tree search algorithm.
**
** Arguments:
**
**	int16_t	target[]	: the target coefficients to be quantized (Q15/Q17)
**	int16_t	weight[]	: weights for mse calculation (Q11)
**	int16_t	qout[]		: the output array (Q15/Q17)
**	int16_t	codebook[]	: codebooks,   cb[0..numStages-1] (Q15/Q17)
**	int16_t	tos 		: the number of stages
**  int16_t	cb_size[]	: codebook size (multistages)
**	int16_t	cb_index[]	: codebook indeces; cb_index[0..numStages-1]
**                            (output)
**  int16_t	dim
**  BOOLEAN		flag
**
** Return value:	None
**
***********************************************************************/
static void lspVQ(int16_t target[], int16_t weight[], int16_t qout[],
		  const int16_t codebook[], int16_t tos,
		  const int16_t cb_size[], int16_t cb_index[],
		  int16_t dim, BOOLEAN flag)
{
	register int16_t i, entry;
	register int16_t c1, s1;
	const int16_t *cdbk_ptr, *cdbk_ptr2, *ptr1;
	int16_t index[LSP_VQ_CAND][LSP_VQ_STAGES];
	int16_t nextIndex[LSP_VQ_CAND][LSP_VQ_STAGES];
	int16_t ncPrev;
	int16_t cand[LSP_VQ_CAND][2 * LPC_ORD];
	int16_t max_dMin, dMin[LSP_VQ_CAND], distortion;
	int16_t *cand_target;
	int32_t L_temp;
	int16_t ptr_offset = 0;
	int16_t temp1, temp2;

	/*==================================================================*
	*	Initialize the data before starting the tree search.			*
	*	  - the number of candidates from the "previous" stage is set	*
	*		to 1 since there is no previous stage!						*
	*	  - the candidate vector from the previous stage is set to zero *
	*	  - the list of indeces for each candidate is set to 1			*
	*==================================================================*/
	for (i = 0; i < LSP_VQ_CAND; i++) {
		v_zap(cand[i], dim);
		v_zap(index[i], LSP_VQ_STAGES);
		v_zap(nextIndex[i], LSP_VQ_STAGES);
	}
	cand_target = v_get(dim);
	ncPrev = 1;

	/*==================================================================*
	*	Now we start the search:										*
	*		For each stage												*
	*			For each candidate from the previous stage				*
	*				For each entry in the current stage codebook		*
	*					* add codebook vector to current candidate		*
	*					* compute the distortion with the target		*
	*					* retain candidate if one of the best so far	*
	*==================================================================*/
	cdbk_ptr = codebook;

	/* An observation for lspVQ() shows that if "flag" is FALSE, then we only */
	/* need to keep track of the best one (instead of the best LSP_VQ_CAND,   */
	/* 8) cand[][] and index[][].  This has significant influence on          */
	/* execution speed.                                                       */

	for (s1 = 0; s1 < tos; s1++) {
		/* set the distortions to huge values */
		fill(dMin, SW_MAX, LSP_VQ_CAND);
		max_dMin = SW_MAX;

		/* Loop for each previous candidate selected, and try each entry */
		for (c1 = 0; c1 < ncPrev; c1++) {
			ptr_offset = 0;

			/* cand_target[] is the target vector with cand[c1] removed.      */
			/* This moves some operations from the for-entry loop here.       */
			/* save_saturation(); */
			v_equ(cand_target, target, dim);
			v_sub(cand_target, cand[c1], dim);
			/* restore_saturation(); */

			for (entry = 0; entry < cb_size[s1]; entry++) {
				ptr1 = cdbk_ptr + ptr_offset;	/* Pointer arithmetics. */

				/* compute the distortion */
				distortion =
				    WeightedMSE(dim, weight, ptr1, cand_target,
						max_dMin);

				/*======================================================*
				* If the error for this entry is less than the worst	*
				* retained candidate so far, keep it. Note that the 	*
				* error list is maintained in order of best to worst.	*
				*=======================================================*/
				if (distortion < max_dMin) {
					max_dMin =
					    InsertCand(c1, s1, dMin, distortion,
						       entry, nextIndex[0],
						       index[0]);
				}
				ptr_offset = melpe_add(ptr_offset, dim);
			}
		}

		/* At this point ptr_offset is (cb_size[s1]*dim).                     */

		/*==================================================================*
		*	Compute the number of candidate vectors which we kept for the	*
		*	next stage. Note that if the size of the stages is less than	*
		*	the number of candidates, we build them up using all entries	*
		*	until we have kept numCand candidates.  On the other hand, if   *
		*   flag is FALSE and (s1 == tos - 1), then we only need to use     *
		*   ncPrev = 1 because we only copy the best candidate before       *
		*   exiting lspVQ().                                                *
		*==================================================================*/

		if (!flag && s1 == tos - 1)
			ncPrev = 1;
		else {
			/* ncPrev = Min(ncPrev*cb_size[s1], LSP_VQ_CAND) for regular      */
			/* loops, and ncPrev = Min(ncPrev*cb_size[s1], LSP_INP_CAND) for  */
			/* the last lap.  Explanations are available near the end of this *//* function.                                                      */

			L_temp = melpe_L_mult(ncPrev, cb_size[s1]);
			L_temp = melpe_L_shr(L_temp, 1);
			temp1 = melpe_extract_l(L_temp);	/* temp1 = ncPrev * cb_size[s1] */
			if (s1 == tos - 1)
				temp2 = LSP_INP_CAND;
			else
				temp2 = LSP_VQ_CAND;
			if (temp1 < temp2)
				ncPrev = temp1;
			else
				ncPrev = temp2;
		}

		/*==================================================================*
		*	We now have the  best indices for the stage just completed, so	*
		*	compute the new candidate vectors for the next stage... 		*
		*==================================================================*/
		for (c1 = 0; c1 < ncPrev; c1++) {
			v_zap(cand[c1], dim);
			cdbk_ptr2 = codebook;
			temp1 = melpe_add(s1, 1);
			v_equ(index[c1], nextIndex[c1], temp1);
			for (i = 0; i < temp1; i++) {
				/*      v_add(cand[c1], cdbk_ptr2 + index[c1][i]*dim, dim); */
				L_temp = melpe_L_mult(index[c1][i], dim);
				L_temp = melpe_L_shr(L_temp, 1);
				temp2 = melpe_extract_l(L_temp);
				ptr1 = cdbk_ptr2 + temp2;
				v_add(cand[c1], ptr1, dim);
				/*      cdbk_ptr2 += cb_size[i]*dim; */
				L_temp = melpe_L_mult(cb_size[i], dim);
				L_temp = melpe_L_shr(L_temp, 1);
				temp2 = melpe_extract_l(L_temp);
				cdbk_ptr2 += temp2;
			}
		}

		/*      cdbk_ptr += cb_size[s1] * dim; */
		cdbk_ptr += ptr_offset;
	}

	/* Copy best candidate and indices into output.  Here we use temp1 and    */
	/* temp2 to compute (c1*tos) and (c1*dim).                                */

	/* Originally this function copies LSP_VQ_CAND (== 8) vectors before      */
	/* exiting if flag is TRUE.  However, in the calling environment of       */
	/* lspVQ() when flag is passed in as TRUE, we only used LSP_INP_CAND      */
	/* (== 5).                                                                */

	temp1 = 0;
	temp2 = 0;
	for (i = 0; i < ncPrev; i++) {
		v_equ(&(cb_index[temp1]), index[i], tos);
		v_equ(&qout[temp2], cand[i], dim);
		temp1 = melpe_add(temp1, tos);
		temp2 = melpe_add(temp2, dim);
	}

	v_free(cand_target);
}

/********************************************************************
**
** Function: WeightedMSE
**
** Description:
**	Given a weighting function,  computes the weighted mean squared
**	error between two vectors.
**
** Arguments:
**	int16_t n		: number of coefficients in the two vectors
**	int16_t weight[] 	: weighting function; weight[1..n] (Q11)
**	int16_t x[]		: first vector (Q15/Q17)
**	int16_t target[]	: second vector (Q15/Q17)
**
** Return value:
**
**	int16_t WeightedMSE : distortion returned as function value (Q15/Q17)
**
***********************************************************************/
static int16_t WeightedMSE(int16_t n, int16_t weight[],
			     const int16_t x[], int16_t target[],
			     int16_t max_dMin)
{
	register int16_t i;
	int32_t distortion;
	int16_t temp, half_n;

	/* x[] and target[] are either Q15 or Q17.  Since the only issue          */
	/* mattering is the relative magnitude of WeightedMSE() among different   */
	/* x[]'s, it will be okay to simply treat both x[] and target[] as Q15.   */
	/* This will make the returned value incorrect in scaling, but it will    */
	/* not affect the relative magnitudes.  Returning a int16_t in Q11      */
	/* seems to be fine according to some rough statistics collected.         */

	distortion = 0;
	half_n = melpe_shr(n, 1);
	for (i = 0; i < half_n; i++) {
		save_saturation();
		temp = melpe_sub(x[i], target[i]);	/* Q15 */
		temp = melpe_mult(temp, temp);	/* Q15 */
		restore_saturation();
		distortion = melpe_L_mac(distortion, weight[i], temp);	/* Q27 */
	}

	if (melpe_r_ound(distortion) >= max_dMin)	/* if this situation takes place, */
		return (SW_MAX);	/* distortion will exceed max_dMin */
	/* and we can leave. */

	for (i = half_n; i < n; i++) {
		save_saturation();
		temp = melpe_sub(x[i], target[i]);	/* Q15 */
		temp = melpe_mult(temp, temp);	/* Q15 */
		restore_saturation();
		distortion = melpe_L_mac(distortion, weight[i], temp);	/* Q27 */
	}

	temp = melpe_r_ound(distortion);
	return (temp);
}

/********************************************************************
**
** Function: InsertCand ()
**
** Description:
**
**	Inserts the indeces corresponding to a candidate into the
**	candidate index list, which is sorted in order of increasing
**	distortion.
**
** Arguments:
**
**	int16_t c1		: index of candidate to insert into list
**	int16_t s1		: index of current stage we are searching
**	int16_t dMin[]	: list of distortions of best nc candidates (Q11)
**	int16_t distortion[]	: distortion of candidate c when used with
**							  "entry" from current stage (Q11)
**	int16_t entry		: current stage entry which results in lower
**						  distortion
**	int16_t	**index	: list of past indices for each candidate
**	int16_t	**nextIndex : indices for next stage (output)
**
** Return value:	int16_t
**
***********************************************************************/
static int16_t InsertCand(int16_t c1, int16_t s1, int16_t dMin[],
			    int16_t distortion, int16_t entry,
			    int16_t nextIndex[], int16_t index[])
{
	register int16_t i, j;
	int16_t ptr_offset;
	int16_t temp1, temp2;
	int16_t *ptr1, *ptr2;
	int32_t L_temp;

	/*==================================================================*
	*	First find the index into the distortion array where this		*
	*	candidate fits. Note that we assume it has been previously		*
	*	verified that this error falls in the range of the candidate	*
	*	errors. 														*
	*==================================================================*/
	for (i = 0; (i < LSP_VQ_CAND) && (distortion > dMin[i]); i++) ;

	/* shift the distortions and indices down to make room for the new one */
	/*      ptr_offset = (LSP_VQ_CAND - 1) * vq_stages; */

	L_temp = melpe_L_mult((LSP_VQ_CAND - 1), LSP_VQ_STAGES);
	L_temp = melpe_L_shr(L_temp, 1);
	ptr_offset = melpe_extract_l(L_temp);
	temp2 = melpe_add(s1, 1);
	for (j = (LSP_VQ_CAND - 1); j > i; j--) {
		dMin[j] = dMin[j - 1];
		temp1 = melpe_sub(ptr_offset, LSP_VQ_STAGES);
		ptr1 = nextIndex + ptr_offset;	/* Pointer arithmetics. */
		ptr2 = nextIndex + temp1;
		/*      v_equ(nextIndex + j * vq_stages, nextIndex + (j - 1)*vq_stages,
		   s1 + 1); */
		v_equ(ptr1, ptr2, temp2);
		ptr_offset = temp1;
	}

	/* insert the index and distortion into the ith candidate */
	dMin[i] = distortion;
	/*      v_equ(nextIndex + i * vq_stages, index + c1 * vq_stages, s1); */
	L_temp = melpe_L_mult(i, LSP_VQ_STAGES);	/* temp1 = i * vq_stages; */
	L_temp = melpe_L_shr(L_temp, 1);
	temp1 = melpe_extract_l(L_temp);
	L_temp = melpe_L_mult(c1, LSP_VQ_STAGES);
	L_temp = melpe_L_shr(L_temp, 1);
	temp2 = melpe_extract_l(L_temp);
	ptr1 = nextIndex + temp1;	/* Pointer arithmetics. */
	ptr2 = index + temp2;
	v_equ(ptr1, ptr2, s1);
	/*      *(nextIndex + i*vq_stages + s1) = entry; */
	ptr1 += s1;		/* Pointer arithmetics. */
	*ptr1 = entry;

	return (dMin[LSP_VQ_CAND - 1]);
}

/*********************************************************************
** NAME: lspStable
**
** DESCRIPTION:
**	This routines checks the stability of a set of LSP parameters
**	by ensuring that all parameters are in the correct order. For
**	LSPs, the LSP frequencies must be monotonically increasing.
**
** INPUTS:
**	int16_t lsp[]	: the LSP coefficients lsp[0..order - 1] (Q15)
**	int16_t order	: order of the LSP coeffs
**
** OUTPUTS: BOOLEAN : TRUE == stable;	FALSE == unstable
**
**********************************************************************/
BOOLEAN lspStable(int16_t lsp[], int16_t order)
{
	register int16_t i;
	BOOLEAN stable;
	int16_t temp;

	/* The following loop attempts to ensure lsp[0] is at least 6.37,         */
	/* lsp[order - 1] is at most 3992.0, and each consecutive pair of lsp[]   */
	/* is separated by at least 25.0.  The sampling frequency is assumed to   */
	/* be 8000.0.                                                             */

	if (lsp[0] < 52)	/* 52 == (6.37/4000.0 * (1 << 15)) */
		lsp[0] = (int16_t) 52;
	for (i = 0; i < order - 1; i++) {
		temp = melpe_add(lsp[i], 205);
		if (lsp[i + 1] < temp)
			lsp[i + 1] = temp;
	}
	/* 205 == (25.0/4000.0 * (1 << 15)) */
	if (lsp[order - 1] > 32702)
		lsp[order - 1] = (int16_t) 32702;
	/* 32702 == (3992.0/4000.0 * (1 << 15)) */

	/* Previously here we use a loop checking whether (lsp[i] < lsp[i - 1])   */
	/* for any of the pairs from i = 1 to i < order.  It is not needed.  The  */
	/* for loop above essentially guarantees the monotonic ascending of       */
	/* lsp[]'s (with a guaranteed gap of 25.0 (Hz)).  The only possible       */
	/* violation is between lsp[order - 2] and lsp[order - 1] because of the  */
	/* modification of lsp[order - 1] after the loop.  So now we only check   */
	/* this pair.                                                             */

	if (lsp[order - 1] < lsp[order - 2])
		stable = FALSE;
	else
		stable = TRUE;

	if (!stable)		/* Warning message moved from lspSort() to lspStable(). */
		fprintf(stderr, "Unstable filter found in lspStable()...\n");

	return (stable);
}

/*********************************************************************
**
** Name:  lspSort()
**
** Description:
**
**	Uses the very slow Straight Insertion technique...so only
**	use for, say, n < 50.  This routine is taken from the
**	Numerical Recipes in C book.
**
** Arguments:
**
**	int16_t lsp[]	: array to be sorted  arr[1..n] (input/output) (Q15)
**	int16_t n		: number of samples to sort
**
** Return value:	None
**
***********************************************************************/
void lspSort(int16_t lsp[], int16_t order)
{
	register int16_t i, j;
	int16_t temp;		/* Q15 */

	for (j = 1; j < order; j++) {
		temp = lsp[j];
		i = (int16_t) (j - 1);
		while (i >= 0 && lsp[i] > temp) {
			lsp[i + 1] = lsp[i];
			i--;
		}
		lsp[i + 1] = temp;
	}
}

/****************************************************************************
**
** Function:		lsf_vq
**
** Description: 	lsfs of three frames are vector quantized
**
** Arguments:
**
**	melp_param *par ---- input/output melp parameters
**
** Return value:	None
**
*****************************************************************************/

void lsf_vq(struct melp_param *par)
{
	register int16_t i, j, k;
	static BOOLEAN firstTime = TRUE;
	static int16_t qplsp[LPC_ORD];	/* Q15 */
	const int16_t melp_cb_size[4] = { 256, 64, 32, 32 };	/* !!! (12/15/99) */
	const int16_t res_cb_size[4] = { 256, 64, 64, 64 };
	const int16_t melp_uv_cb_size[1] = { 512 };
	int16_t uv_config;	/* Bits of uv_config replace uv1, uv2 and cuv. */
	int16_t *lsp[NF];
	int32_t err, minErr, acc, bcc;	/* !!! (12/15/99), Q11 */
	int16_t temp1, temp2;
	int16_t lpc[LPC_ORD];	/* Q12 */
	int16_t wgt[NF][LPC_ORD];	/* Q11 */
	int16_t mwgt[2 * LPC_ORD];	/* Q11 */
	int16_t bestlsp0[LPC_ORD], bestlsp1[LPC_ORD];	/* Q15 */
	int16_t res[2 * LPC_ORD];	/* Q17 */

	/* The original program declares lsp_cand[LSP_VQ_CAND][] and              */
	/* lsp_index_cand[LSP_VQ_CAND*LSP_VQ_STAGES] with LSP_VQ_CAND == 8.  The  */
	/* program only uses up to LSP_INP_CAND == 5 and the declaration is       */
	/* modified.                                                              */

	int16_t lsp_cand[LSP_INP_CAND][LPC_ORD];	/* Q15 */
	int16_t lsp_index_cand[LSP_INP_CAND * LSP_VQ_STAGES];
	int16_t ilsp0[LPC_ORD], ilsp1[LPC_ORD];	/* Q15 */
	int16_t cand, inp_index_cand, tos, intfact;

	if (firstTime) {
		temp2 = melpe_shl(LPC_ORD, 10);	/* Q10 */
		temp1 = X08_Q10;	/* Q10 */
		for (i = 0; i < LPC_ORD; i++) {
			/*      qplsp[i] = (i+1)*0.8/LPC_ORD; */
			qplsp[i] = melpe_divide_s(temp1, temp2);
			temp1 = melpe_add(temp1, X08_Q10);
		}
		firstTime = FALSE;
	}

	/* ==== Compute weights ==== */
	for (i = 0; i < NF; i++) {
		lsp[i] = par[i].lsf;
		lpc_lsp2pred(lsp[i], lpc, LPC_ORD);
		vq_lspw(wgt[i], lsp[i], lpc, LPC_ORD);
	}

	uv_config = 0;
	for (i = 0; i < NF; i++) {
		uv_config = melpe_shl(uv_config, 1);
		if (par[i].uv_flag) {
			uv_config |= 0x0001;

			/* ==== Adjust weights ==== */
			if (i == 0)	/* Testing for par[0].uv_flag == 1 */
				v_scale(wgt[0], X02_Q15, LPC_ORD);
			else if (i == 1)
				v_scale(wgt[1], X02_Q15, LPC_ORD);
		}
	}

	/* ==== Quantize the lsp according to the UV decisions ==== */
	switch (uv_config) {
	case 7:		/* 111, all frames are NOT voiced ---- */
		lspVQ(lsp[0], wgt[0], lsp[0], lsp_uv_9, 1, melp_uv_cb_size,
		      quant_par.lsf_index[0], LPC_ORD, FALSE);
		lspVQ(lsp[1], wgt[1], lsp[1], lsp_uv_9, 1, melp_uv_cb_size,
		      quant_par.lsf_index[1], LPC_ORD, FALSE);
		lspVQ(lsp[2], wgt[2], lsp[2], lsp_uv_9, 1, melp_uv_cb_size,
		      quant_par.lsf_index[2], LPC_ORD, FALSE);
		break;
	case 6:		/* 110 */
		lspVQ(lsp[0], wgt[0], lsp[0], lsp_uv_9, 1, melp_uv_cb_size,
		      quant_par.lsf_index[0], LPC_ORD, FALSE);
		lspVQ(lsp[1], wgt[1], lsp[1], lsp_uv_9, 1, melp_uv_cb_size,
		      quant_par.lsf_index[1], LPC_ORD, FALSE);
		lspVQ(lsp[2], wgt[2], lsp[2], lsp_v_256x64x32x32, 4, melp_cb_size,	/* !!! (12/15/99) */
		      quant_par.lsf_index[2], LPC_ORD, FALSE);
		break;
	case 5:		/* 101 */
		lspVQ(lsp[0], wgt[0], lsp[0], lsp_uv_9, 1, melp_uv_cb_size,
		      quant_par.lsf_index[0], LPC_ORD, FALSE);
		lspVQ(lsp[1], wgt[1], lsp[1], lsp_v_256x64x32x32, 4, melp_cb_size,	/* !!! (12/15/99) */
		      quant_par.lsf_index[1], LPC_ORD, FALSE);
		lspVQ(lsp[2], wgt[2], lsp[2], lsp_uv_9, 1, melp_uv_cb_size,
		      quant_par.lsf_index[2], LPC_ORD, FALSE);
		break;
	case 3:		/* 011 */
		lspVQ(lsp[0], wgt[0], lsp[0], lsp_v_256x64x32x32, 4, melp_cb_size,	/* !!! (12/15/99) */
		      quant_par.lsf_index[0], LPC_ORD, FALSE);
		lspVQ(lsp[1], wgt[1], lsp[1], lsp_uv_9, 1, melp_uv_cb_size,
		      quant_par.lsf_index[1], LPC_ORD, FALSE);
		lspVQ(lsp[2], wgt[2], lsp[2], lsp_uv_9, 1, melp_uv_cb_size,
		      quant_par.lsf_index[2], LPC_ORD, FALSE);
		break;
	default:
		if (uv_config == 1) {	/* 001 case, if (!uv1 && !uv2 && uv3). */
			/* ---- Interpolation [4 inp + (8+6+6+6) res + 9 uv] ---- */
			tos = 1;
			lspVQ(lsp[2], wgt[2], lsp_cand[0], lsp_uv_9, tos,
			      melp_uv_cb_size, lsp_index_cand, LPC_ORD, TRUE);
		} else {
			tos = 4;
			lspVQ(lsp[2], wgt[2], lsp_cand[0], lsp_v_256x64x32x32, tos,	/* !!! (12/15/99) */
			      melp_cb_size, lsp_index_cand, LPC_ORD, TRUE);
		}

		minErr = LW_MAX;
		cand = 0;
		inp_index_cand = 0;
		for (k = 0; k < LSP_INP_CAND; k++) {
			for (i = 0; i < 16; i++) {

				err = 0;

				/* Originally we have two for loops here.  One computes       */
				/* ilsp0[] and ilsp1[] and the other one computes "err".  If  */
				/* "err" already exceeds minErr, we can stop the loop and     */
				/* there is no need to compute the remaining ilsp0[] and      */
				/* ilsp1[] entries.  Hence the two for loops are joined.      */

				for (j = 0; j < LPC_ORD; j++) {

					/*      ilsp0[j] = (inpCoef[i][j] * qplsp[j] +
					   (1.0 - inpCoef[i][j]) * lsp_cand[k][j]); */
					intfact = inpCoef[i][j];	/* Q14 */
					acc = melpe_L_mult(intfact, qplsp[j]);	/* Q30 */
					intfact = melpe_sub(ONE_Q14, intfact);	/* Q14 */
					acc = melpe_L_mac(acc, intfact, lsp_cand[k][j]);	/* Q30 */
					ilsp0[j] = melpe_extract_h(melpe_L_shl(acc, 1));
					acc =
					    melpe_L_sub(acc,
						  melpe_L_shl(melpe_L_deposit_l(lsp[0][j]),
							15));

					/*      ilsp1[j] = inpCoef[i][j + LPC_ORD] * qplsp[j] +
					   (1.0 - inpCoef[i][j + LPC_ORD]) * lsp_cand[k][j]; */
					intfact = inpCoef[i][j + LPC_ORD];	/* Q14 */
					bcc = melpe_L_mult(intfact, qplsp[j]);
					intfact = melpe_sub(ONE_Q14, intfact);
					bcc = melpe_L_mac(bcc, intfact, lsp_cand[k][j]);	/* Q30 */
					ilsp1[j] = melpe_extract_h(melpe_L_shl(bcc, 1));
					bcc =
					    melpe_L_sub(bcc,
						  melpe_L_shl(melpe_L_deposit_l(lsp[1][j]),
							15));

					/*      err += wgt0[j]*(lsp0[j] - ilsp0[j])*
					   (lsp0[j] - ilsp0[j]); */
					temp1 = melpe_norm_l(acc);
					temp2 = melpe_extract_h(melpe_L_shl(acc, temp1));
					if (temp2 == MONE_Q15)
						temp2 = -32767;
					temp2 = melpe_mult(temp2, temp2);
					acc = melpe_L_mult(temp2, wgt[0][j]);
					temp1 = melpe_shl(melpe_sub(1, temp1), 1);
					acc = melpe_L_shl(acc, melpe_sub(temp1, 3));	/* Q24 */
					err = melpe_L_add(err, acc);

					/*      err += wgt1[j]*(lsp1[j] - ilsp1[j])*
					   (lsp1[j] - ilsp1[j]); */
					temp1 = melpe_norm_l(bcc);
					temp2 = melpe_extract_h(melpe_L_shl(bcc, temp1));
					if (temp2 == MONE_Q15)
						temp2 = -32767;
					temp2 = melpe_mult(temp2, temp2);
					bcc = melpe_L_mult(temp2, wgt[1][j]);
					temp1 = melpe_shl(melpe_sub(1, temp1), 1);
					bcc = melpe_L_shl(bcc, melpe_sub(temp1, 3));	/* Q24 */
					err = melpe_L_add(err, bcc);

					/* computer the err for the last frame */
					acc = melpe_L_shl(melpe_L_deposit_l(lsp[2][j]), 15);
					acc =
					    melpe_L_sub(acc,
						  melpe_L_shl(melpe_L_deposit_l
							(lsp_cand[k][j]), 15));
					temp1 = melpe_norm_l(acc);
					temp2 = melpe_extract_h(melpe_L_shl(acc, temp1));
					if (temp2 == MONE_Q15)
						temp2 = -32767;
					temp2 = melpe_mult(temp2, temp2);
					acc = melpe_L_mult(temp2, wgt[2][j]);
					temp1 = melpe_shl(melpe_sub(1, temp1), 1);
					acc = melpe_L_shl(acc, melpe_sub(temp1, 3));	/* Q24 */
					err = melpe_L_add(err, acc);
				}

				if (err < minErr) {
					minErr = err;
					cand = k;
					inp_index_cand = i;
					v_equ(bestlsp0, ilsp0, LPC_ORD);
					v_equ(bestlsp1, ilsp1, LPC_ORD);
				}
			}
		}

		v_equ(lsp[2], lsp_cand[cand], LPC_ORD);
		v_equ(quant_par.lsf_index[0], &(lsp_index_cand[cand * tos]),
		      tos);
		quant_par.lsf_index[1][0] = inp_index_cand;

		for (i = 0; i < LPC_ORD; i++) {
			temp1 = melpe_sub(lsp[0][i], bestlsp0[i]);	/* Q15 */
			temp2 = melpe_sub(lsp[1][i], bestlsp1[i]);	/* Q15 */
			res[i] = melpe_shl(temp1, 2);	/* Q17 */
			res[i + LPC_ORD] = melpe_shl(temp2, 2);	/* Q17 */
		}
		v_equ(mwgt, wgt[0], LPC_ORD);
		v_equ(mwgt + LPC_ORD, wgt[1], LPC_ORD);

		/* Note that in the following IF block, the lspVQ() is quantizing on  */
		/* res[] and res256x64x64x64[], and both of them are Q17 instead of   */
		/* Q15, unlike the other calling instances in this function.          */

		if (uv_config == 1)	/* if (!uv1 && !uv2 && uv3) */
			lspVQ(res, mwgt, res, res256x64x64x64, 4, res_cb_size,
			      quant_par.lsf_index[2], 2 * LPC_ORD, FALSE);
		else
			lspVQ(res, mwgt, res, res256x64x64x64, 2, res_cb_size,
			      quant_par.lsf_index[2], 2 * LPC_ORD, FALSE);

		/* ---- reconstruct lsp for later stability check ---- */
		for (i = 0; i < LPC_ORD; i++) {
			temp1 = melpe_shr(res[i], 2);
			lsp[0][i] = melpe_add(temp1, bestlsp0[i]);
			temp2 = melpe_shr(res[i + LPC_ORD], 2);
			lsp[1][i] = melpe_add(temp2, bestlsp1[i]);
		}
		break;
	}

	/* ---- Stability checking ---- */
	/* The sortings on lsp[0] and lsp[1] are not necessary because they are   */
	/* variables local to this function and they are discarded upon exit.     */
	/* We only check whether they fit the stability test and issue a warning. */

	(void)lspStable(lsp[0], LPC_ORD);
	(void)lspStable(lsp[1], LPC_ORD);
	if (!lspStable(lsp[2], LPC_ORD))
		lspSort(lsp[2], LPC_ORD);

	v_equ(qplsp, lsp[2], LPC_ORD);
}

/*********************************************************************
**
** Name:  deqnt_msvq()
**
** Description:
**
**	Dequantization using codebook indices with multi-stages
**
** Arguments:
**
**	int16_t	qout[]	---- (output) quantized data (Q15/Q17)
**	int16_t	codebook[] 	---- codebooks,	 cb[0..numStages-1] (Q15/Q17)
**	int16_t 	tos 	----	the number of stages
**	short	*index	----	codebook index
**
** Return value:	None
**
***********************************************************************/
void deqnt_msvq(int16_t qout[], const int16_t codebook[], int16_t tos,
		const int16_t cb_size[], int16_t index[], int16_t dim)
{
	register int16_t i;
	const int16_t *cdbk_ptr;
	int16_t temp;
	int32_t L_temp;

	/* ====== Clear output ====== */
	v_zap(qout, dim);

	/* ====== Add each stage ====== */
	cdbk_ptr = codebook;
	for (i = 0; i < tos; i++) {
		/*      v_add(qout, cdbk_ptr + index[i]*dim, dim); */
		L_temp = melpe_L_mult(index[i], dim);
		L_temp = melpe_L_shr(L_temp, 1);
		temp = melpe_extract_l(L_temp);
		v_add(qout, cdbk_ptr + temp, dim);
		/*      cdbk_ptr += cb_size[i] * dim; */
		L_temp = melpe_L_mult(cb_size[i], dim);
		L_temp = melpe_L_shr(L_temp, 1);
		temp = melpe_extract_l(L_temp);
		cdbk_ptr += temp;
	}
}

/****************************************************************************
**
** Function:		quant_jitter
**
** Description: 	Jitter of three frames are quantized
**
** Arguments:
**
**	melp_param *par ---- input/output melp parameters
**
** Return value:	None
**
*****************************************************************************/
void quant_jitter(struct melp_param *par)
{
	register int16_t i;
	int16_t uv_config;
	BOOLEAN flag_jitter;
	int16_t cnt_jitter;
	int16_t jitter[NF];	/* Q15 */

	/* Previously we use a BOOLEAN array uv[] for par[].uv_flag.  Now we use  */
	/* a int16_t uv_config and use its bits for BOOLEAN values.             */

	uv_config = 0;
	for (i = 0; i < NF; i++) {
		uv_config = melpe_shl(uv_config, 1);
		uv_config |= par[i].uv_flag;
		jitter[i] = par[i].jitter;
	}

	flag_jitter = FALSE;

	switch (uv_config) {
	case 6:		/* uv_config == 110, UUV */
		if (jitter[2] == MAX_JITTER_Q15)
			flag_jitter = TRUE;
		break;
	case 5:		/* uv_config == 101, UVU */
	case 4:		/* uv_config == 100, UVV */
	case 1:		/* uv_config == 001, VVU */
		if (jitter[1] == MAX_JITTER_Q15)
			flag_jitter = TRUE;
		break;
	case 3:		/* uv_config == 011, VUU */
		if (jitter[0] == MAX_JITTER_Q15)
			flag_jitter = TRUE;
		break;
	case 0:		/* uv_config == 000, VVV */
		cnt_jitter = 0;
		for (i = 0; i < NF; i++)
			if (jitter[i] == MAX_JITTER_Q15)
				cnt_jitter++;

		if (cnt_jitter >= 2)
			flag_jitter = TRUE;
		break;
	}

	/* Decoding jitter flag, note that this is not exactly the inverse        */
	/* operation of the encoding part.                                        */

	for (i = 0; i < NF; i++) {
		if (par[i].uv_flag)
			jitter[i] = MAX_JITTER_Q15;
		else
			jitter[i] = 0;
	}

	if (flag_jitter && (uv_config == 0))	/* uv_config == 000, VVV */
		jitter[0] = jitter[1] = jitter[2] = MAX_JITTER_Q15;

	for (i = 0; i < NF; i++)
		par[i].jitter = jitter[i];

	quant_par.jit_index[0] = flag_jitter;
}

/****************************************************************************
**
** Function:		quant_fsmag
**
** Description: 	Fourier Magnitude of three frames are vector quantized
**
** Arguments:
**
**	melp_param *par ---- input/output melp parameters
**
** Return value:	None
**
*****************************************************************************/

void quant_fsmag(struct melp_param *par)
{
	static BOOLEAN prev_uv = TRUE;
	register int16_t i;
	static int16_t prev_fsmag[NUM_HARM];
	int16_t qmag[NUM_HARM];	/* Q13 */
	int16_t temp1, temp2;
	int16_t p_value, q_value;
	int16_t count, last;

	count = 0;
	last = -1;
	for (i = 0; i < NF; i++) {
		if (par[i].uv_flag)
			fill(par[i].fs_mag, ONE_Q13, NUM_HARM);
		else {
			window_Q(par[i].fs_mag, w_fs, par[i].fs_mag, NUM_HARM,
				 14);
			last = i;
			count++;
		}
	}

	/*      fsvq_enc(par[last].fs_mag, qmag, fs_vq_par); */
	/* Later it is found that we do not need the structured variable          */
	/* fs_vq_par at all.  References to its individual fields can be replaced */
	/* directly with constants or other variables.                            */

	if (count > 0)
		vq_enc(fsvq_cb, par[last].fs_mag, FS_LEVELS, NUM_HARM, qmag,
		       &quant_par.fs_index);

	if (count > 1) {
		if (prev_uv || par[0].uv_flag) {
			for (i = 0; i <= last; i++) {
				if (!par[i].uv_flag)
					v_equ(par[i].fs_mag, qmag, NUM_HARM);
			}
		} else {
			if (par[1].uv_flag) {	/* V VUV */
				v_equ(par[0].fs_mag, prev_fsmag, NUM_HARM);
				v_equ(par[last].fs_mag, qmag, NUM_HARM);
			} else if (par[2].uv_flag) {	/* V VVU */
				v_equ(par[1].fs_mag, qmag, NUM_HARM);
				for (i = 0; i < NUM_HARM; i++) {
					/*      par[0].fs_mag[i] = 0.5*(qmag[i] + prev_fsmag[i]); */
					temp1 = melpe_shr(qmag[i], 1);	/* 0.5*qmag[i], Q13 */
					temp2 = melpe_shr(prev_fsmag[i], 1);	/* Q13 */
					par[0].fs_mag[i] = melpe_add(temp1, temp2);	/* Q13 */
				}
			} else {	/* V VVV */
				v_equ(par[2].fs_mag, qmag, NUM_HARM);
				for (i = 0; i < NUM_HARM; i++) {
					p_value = prev_fsmag[i];
					q_value = qmag[i];	/* Q13 */

					/* Note that (par[0].fs_mag[i] + par[1].fs_mag[i]) ==     */
					/* (p + q).  We might replace some multiplications with   */
					/* additions.                                             */

					/*      par[0].fs_mag[i] = (p + p + q)/3.0; */
					temp1 = melpe_mult(p_value, X0667_Q15);	/* Q13 */
					temp2 = melpe_mult(q_value, X0333_Q15);	/* Q13 */
					par[0].fs_mag[i] = melpe_add(temp1, temp2);
					/*      par[1].fs_mag[i] = (p + q + q)/3.0; */
					temp1 = melpe_mult(p_value, X0333_Q15);
					temp2 = melpe_mult(q_value, X0667_Q15);
					par[1].fs_mag[i] = melpe_add(temp1, temp2);
				}
			}
		}
	} else if (count == 1)
		v_equ(par[last].fs_mag, qmag, NUM_HARM);

	prev_uv = par[NF - 1].uv_flag;
	if (prev_uv)
		fill(prev_fsmag, ONE_Q13, NUM_HARM);
	else
		v_equ(prev_fsmag, par[NF - 1].fs_mag, NUM_HARM);
}

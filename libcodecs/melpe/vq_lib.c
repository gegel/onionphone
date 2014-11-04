/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*

2.4 kbps MELP Proposed Federal Standard speech coder

Fixed-point C code, version 1.0

Copyright (c) 1998, Texas Instruments, Inc.

Texas Instruments has intellectual property rights on the MELP
algorithm.	The Texas Instruments contact for licensing issues for
commercial and non-government use is William Gordon, Director,
Government Contracts, Texas Instruments Incorporated, Semiconductor
Group (phone 972 480 7442).

The fixed-point version of the voice codec Mixed Excitation Linear
Prediction (MELP) is based on specifications on the C-language software
simulation contained in GSM 06.06 which is protected by copyright and
is the property of the European Telecommunications Standards Institute
(ETSI). This standard is available from the ETSI publication office
tel. +33 (0)4 92 94 42 58. ETSI has granted a license to United States
Department of Defense to use the C-language software simulation contained
in GSM 06.06 for the purposes of the development of a fixed-point
version of the voice codec Mixed Excitation Linear Prediction (MELP).
Requests for authorization to make other use of the GSM 06.06 or
otherwise distribute or modify them need to be addressed to the ETSI
Secretariat fax: +33 493 65 47 16.

*/

#include "sc1200.h"
#include "macro.h"
#include "mathhalf.h"
#include "mat_lib.h"
#include "math_lib.h"
#include "lpc_lib.h"
#include "constant.h"

#define MAXWT		4096	/* w[i] < 2.0 to avoid saturation */
#define MAXWT2		(MAXWT*2)
#define MAXWT4		(MAXWT*4)
#define ONE_Q28		268435456L	/* (1 << 28) */
#define EIGHT_Q11   16384	/* 8 * (1 << 11) */
#define X016_Q15	5242	/* 0.16 * (1 << 15) */
#define X064_Q15	20971	/* 0.64 * (1 << 15) */
#define X069_Q15	22609	/* 0.69 * (1 << 15) */
#define X14_Q14		22937	/* 1.4 * (1 << 14) */
#define X25_Q6		1600	/* 25 * (1 << 6) */
#define X75_Q8		19200	/* 75 * (1 << 8) */
#define X117_Q5		3744	/* 117 * (1 << 5) */
#define XN03_Q15	-9830	/* -0.3 * (1 << 15) */

#define P_SWAP(x, y, type)	{type u__p; u__p = x; x = y; y = u__p;}

/* VQ_LSPW - compute LSP weighting vector                                     */
/*                                                                            */
/* Atal's method:                                                             */
/*      From Atal and Paliwal (ICASSP 1991)                                   */
/*      (Note: Paliwal and Atal used w(k)^2(u(k)-u_hat(k))^2,                 */
/*      and we use w(k)(u(k)-u_hat(k))^2 so our weights are different         */
/*      but the method (i.e. the weighted MSE) is the same.                   */
/*                                                                            */
/* Q values:                                                                  */
/*      weight[] is Q11, lsp[] is Q15, lpc[] is Q12                           */
/*                                                                            */
/* Note:                                                                      */
/*      The coder does not use the returned value from vq_lspw() at all.      */

int16_t *vq_lspw(int16_t weight[], int16_t lsp[], int16_t lpc[],
		   int16_t order)
{
	register int16_t i;
	int32_t L_temp;

	for (i = 0; i < order; i++) {
		L_temp = lpc_aejw(lpc, lsp[i], order);	/* L_temp in Q19 */
		weight[i] = L_pow_fxp(L_temp, XN03_Q15, 19, 11);
	}

	/* Modifying weight[8] and weight[9]. */

	weight[8] = melpe_mult(weight[8], X064_Q15);
	weight[9] = melpe_mult(weight[9], X016_Q15);
	return (weight);
}

/* VQ_MS4-                                                                    */
/*		Tree search multi-stage VQ encoder (optimized for speed               */
/*		should be compatible with VQ_MS)                                      */
/*                                                                            */
/*	Synopsis: vq_ms4(cb, u, u_est, levels, ma, stages, order, w, u_hat,       */
/*                   a_indices)                                               */
/*		Input:                                                                */
/*			cb- one dimensional linear codebook array (codebook is structured */
/*				as [stages][levels for each stage][p])                        */
/*			u- dimension p, the parameters to be encoded (u[0..p-1])          */
/*			u_est- dimension p, the estimated parameters or mean (if NULL,    */
/*                 assume estimate is the all zero vector) (u_est[0..p-1])    */
/*			levels- the number of levels at each stage (levels[0..stages-1])  */
/*			ma- the tree search size (keep ma candidates from one stage to    */
/*              the next)                                                     */
/*			stages- the number of stages of msvq                              */
/*			order (p)- the predictor order                                    */
/*			weights (w)- the weighting vector (w[0..p-1])                     */
/*			max_inner- the maximum number of times the swapping procedure     */
/*					   in the inner loop can be executed                      */
/*		Output:                                                               */
/*			u_hat-	 the reconstruction vector (if non null)                  */
/* 			a_indices- the codebook indices (for each stage)                  */
/*                     a_indices[0..stages-1]                                 */
/*                                                                            */
/* cb is Q17, u is Q15, u_est is Q15, weights is Q11, u_hat is Q15            */
/*                                                                            */
/*      Note:                                                                 */
/*          The coder does not use the returned value from vq_lspw() at all.  */

int16_t vq_ms4(const int16_t * cb, int16_t * u, const int16_t * u_est,
		 const int16_t levels[], int16_t ma, int16_t stages,
		 int16_t order, int16_t weights[], int16_t * u_hat,
		 int16_t * a_indices, int16_t max_inner)
{
	const int16_t *cbp, *cb_currentstage, *cb_table;
	int16_t tmp, *u_tmp, *uhatw, uhatw_sq;
	int16_t d_cj, d_opt;
	int16_t *d, *p_d, *n_d, *p_distortion;
	int16_t *errors, *p_errors, *n_errors, *p_e;
	int16_t i, j, m, s, c, p_max, inner_counter;
	int16_t *indices, *p_indices, *n_indices;
	int16_t *parents, *p_parents, *n_parents;
	int16_t *tmp_p_e;
	int16_t temp;
	int32_t L_temp, L_temp1;

	/* make sure weights don't get too big */
	j = 0;
	for (i = 0; i < order; i++) {
		if (weights[i] > MAXWT4) {
			j = 3;
			break;
		} else if (weights[i] > MAXWT2) {
			j = 2;
		} else if (weights[i] > MAXWT) {
			if (j == 0)
				j = 1;
		}
	}
	for (i = 0; i < order; i++) {
		weights[i] = melpe_shr(weights[i], j);
	}

	/* allocate memory for the current node and parent node (thus, the        */
	/* factors of two everywhere) The parents and current nodes are allocated */
	/* contiguously */
	indices = v_get((int16_t) (2 * ma * stages));
	errors = v_get((int16_t) (2 * ma * order));
	uhatw = v_get(order);
	d = v_get((int16_t) (2 * ma));
	parents = v_get((int16_t) (2 * ma));
	tmp_p_e = v_get((int16_t) (ma * order));

	/* initialize memory */
	v_zap(indices, (int16_t) (2 * stages * ma));
	v_zap(parents, (int16_t) (2 * ma));

	/* initialize inner loop counter */
	inner_counter = 0;

	/* set up memory */
	p_indices = &indices[0];
	n_indices = &indices[ma * stages];
	p_errors = &errors[0];
	n_errors = &errors[ma * order];
	p_d = &d[0];
	n_d = &d[ma];
	p_parents = &parents[0];
	n_parents = &parents[ma];

	/* u_tmp is the input vector (i.e. if u_est is non-null, it is subtracted */
	/* off) */
	u_tmp = v_get((int16_t) (order + 1));	/* u_tmp is Q15 */
	(void)v_equ(u_tmp, u, order);
	if (u_est)
		(void)v_sub(u_tmp, u_est, order);

	/* change u_tmp from Q15 to Q17 */
	for (j = 0; j < order; j++)
		u_tmp[j] = melpe_shl(u_tmp[j], 2);

	/* L_temp is Q31 */
	L_temp = 0;
	for (j = 0; j < order; j++) {
		temp = melpe_mult(u_tmp[j], weights[j]);	/* temp = Q13 */
		L_temp = melpe_L_mac(L_temp, temp, u_tmp[j]);
	}

	/* tmp in Q15 */
	tmp = melpe_extract_h(L_temp);

	/* set up inital error vectors (i.e. error vectors = u_tmp) */
	for (c = 0; c < ma; c++) {
		/* n_errors is Q17, n_d is Q15 */
		(void)v_equ(&n_errors[c * order], u_tmp, order);
		n_d[c] = tmp;
	}

	/* no longer need memory so free it here */
	v_free(u_tmp);

	/* codebook pointer is set to point to first stage */
	cbp = cb;

	/* set m to 1 for the first stage and loop over all stages */

	for (m = 1, s = 0; s < stages; s++) {
		/* Save the pointer to the beginning of the current stage.  Note: cbp */
		/* is only incremented in one spot, and it is incremented all the way */
		/* through all the stages. */
		cb_currentstage = cbp;

		/* set up pointers to the parent and current nodes */
		P_SWAP(p_indices, n_indices, int16_t *);
		P_SWAP(p_parents, n_parents, int16_t *);
		P_SWAP(p_errors, n_errors, int16_t *);
		P_SWAP(p_d, n_d, int16_t *);

		/* p_max is the pointer to the maximum distortion node over all       */
		/* candidates.  The so-called worst of the best. */
		p_max = 0;

		/* store errors in Q15 in tmp_p_e */
		for (i = 0; i < m * order; i++) {
			tmp_p_e[i] = melpe_shr(p_errors[i], 2);
		}

		/* set the distortions to a large value */
		for (c = 0; c < ma; c++) {
			n_d[c] = SW_MAX;
		}
		for (j = 0; j < levels[s]; j++) {
			/* compute weighted codebook element, increment codebook pointer */
			/* L_temp is Q31 */
			L_temp = 0;
			for (i = 0; i < order; i++, cbp++) {
				/* Q17*Q11 << 1 = Q29 */
				L_temp1 = melpe_L_mult(*cbp, weights[i]);

				/* uhatw[i] = -2*tmp */
				/* uhatw is Q15 (shift 3 to take care of *2) */
				uhatw[i] = melpe_negate(melpe_extract_h(melpe_L_shl(L_temp1, 3)));

				/* tmp is now Q13 */
				tmp = melpe_extract_h(L_temp1);
				L_temp = melpe_L_mac(L_temp, *cbp, tmp);
			}
			/* uhatw_sq is Q15 */
			uhatw_sq = melpe_extract_h(L_temp);

			/* p_e points to the error vectors and p_distortion points to the */
			/* node distortions.  Note: the error vectors are contiguous in   */
			/* memory, as are the distortions.  Thus, the error vector for    */
			/* the 2nd node comes immediately after the error for the first   */
			/* node.  (This saves on repeated initializations) */
			/* p_e is Q15 */
			p_e = tmp_p_e;
			p_distortion = p_d;

			/* iterate over all parent nodes */
			for (c = 0; c < m; c++) {
				/* L_temp is Q31, p_distortion is same Q as n_d, p_e is Q15 */
				L_temp =
				    melpe_L_deposit_h(melpe_add(*p_distortion++, uhatw_sq));
				for (i = 0; i < order; i++)
					L_temp =
					    melpe_L_mac(L_temp, *p_e++, uhatw[i]);

				/* d_cj is Q15 */
				d_cj = melpe_extract_h(L_temp);

				/* determine if d is less than the maximum candidate          */
				/* distortion.  i.e., is the distortion found better than the */
				/* so-called worst of the best */
				if (d_cj <= n_d[p_max]) {
					/* replace the worst with the values just found */
					/* n_d is now a Q16 */
					n_d[p_max] = d_cj;

					i = melpe_add(melpe_shr
						(melpe_extract_l
						 (melpe_L_mult(p_max, stages)), 1),
						s);
					n_indices[i] = j;
					n_parents[p_max] = c;

					/* want to limit the number of times the inner loop is    */
					/* entered (to reduce the *maximum* complexity) */
					if (inner_counter < max_inner) {
						inner_counter =
						    melpe_add(inner_counter, 1);
						if (inner_counter < max_inner) {
							p_max = 0;
							/* find the new maximum */
							for (i = 1; i < ma; i++) {
								if (n_d[i] >
								    n_d[p_max])
									p_max =
									    i;
							}
						} else {	/* inner_counter == max_inner */
							/* The inner loop counter now exceeds the         */
							/* maximum, and the inner loop will now not be    */
							/* entered.  Instead of quitting the search or    */
							/* doing something drastic, we simply keep track  */
							/* of the best candidate (rather than the M best) */
							/* by setting p_max to equal the index of the     */
							/* minimum distortion i.e. only keep one          */
							/* candidate ar_ound the MINIMUM distortion */
							for (i = 1; i < ma; i++) {
								if (n_d[i] <
								    n_d[p_max])
									p_max =
									    i;
							}
						}
					}
				}
			}	/* for c */
		}		/* for j */

		/* compute the error vectors for each node */
		for (c = 0; c < ma; c++) {
			/* get the error from the parent node and subtract off the        */
			/* codebook value */
			(void)v_equ(&n_errors[c * order],
				    &p_errors[n_parents[c] * order], order);
			(void)v_sub(&n_errors[c * order],
				    &cb_currentstage[n_indices[c * stages + s] *
						     order], order);
			/* get the indices that were used for the parent node */
			(void)v_equ(&n_indices[c * stages],
				    &p_indices[n_parents[c] * stages], s);
		}

		m = (int16_t) (m * levels[s]);
		if (m > ma)
			m = ma;
	}			/* for s */

	/* find the optimum candidate c */
	for (i = 1, c = 0; i < ma; i++) {
		if (n_d[i] < n_d[c])
			c = i;
	}

	d_opt = n_d[c];

	if (a_indices) {
		(void)v_equ(a_indices, &n_indices[c * stages], stages);
	}
	if (u_hat) {
		if (u_est)
			(void)v_equ(u_hat, u_est, order);
		else
			(void)v_zap(u_hat, order);

		cb_currentstage = cb;
		for (s = 0; s < stages; s++) {
			cb_table =
			    &cb_currentstage[n_indices[c * stages + s] * order];
			for (i = 0; i < order; i++)
				u_hat[i] = melpe_add(u_hat[i], melpe_shr(cb_table[i], 2));
			cb_currentstage += levels[s] * order;
		}
	}

	v_free(tmp_p_e);
	v_free(parents);
	v_free(d);
	v_free(uhatw);
	v_free(errors);
	v_free(indices);

	return (d_opt);
}

/*  VQ_MSD2 -                                                                 */
/*      Tree search multi-stage VQ decoder                                    */
/*                                                                            */
/*  Synopsis: vq_msd(cb, u, u_est, a, indices, levels, stages, p, conversion) */
/*      Input:                                                                */
/*          cb- one dimensional linear codebook array (codebook is structured */
/*              as [stages][levels for each stage][p])                        */
/*          indices- the codebook indices (for each stage)                    */
/*                   indices[0..stages-1]                                     */
/*          levels- the number of levels (for each stage) levels[0..stages-1] */
/*          u_est- dimension p, the estimated parameters or mean (if NULL,    */
/*                 assume estimate is the all zero vector) (u_est[0..p-1])    */
/*          stages- the number of stages of msvq                              */
/*          p- the predictor order                                            */
/*          conversion- the conversion constant (see lpc.h, lpc_conv.c)       */
/*          diff_Q- the difference between Q value of codebook and u_est      */
/*      Output:                                                               */
/*          u- dimension p, the decoded parameters (if NULL, use alternate    */
/*             temporary storage) (u[0..p-1])                                 */
/*          a- predictor parameters (a[0..p]), if NULL, don't compute         */
/*      Returns:                                                              */
/*          pointer to reconstruction vector (u)                              */
/*      Parameters:                                                           */
/*                                                                            */
/*      Note:                                                                 */
/*          The coder does not use the returned value from vq_lspw() at all.  */

void vq_msd2(const int16_t * cb, int16_t * u_hat, const int16_t * u_est,
	     int16_t * indices, const int16_t levels[], int16_t stages,
	     int16_t p, int16_t diff_Q)
{
	register int16_t i, j;
	const int16_t *cb_currentstage, *cb_table;
	int32_t *L_u_hat, L_temp;

	/* allocate memory (if required) */
	L_u_hat = L_v_get(p);

	/* add estimate on (if non-null), or clear vector */
	if (u_est)
		(void)v_equ(u_hat, u_est, p);
	else
		(void)v_zap(u_hat, p);

	/* put u_hat to a long buffer */
	for (i = 0; i < p; i++)
		L_u_hat[i] = melpe_L_shl(melpe_L_deposit_l(u_hat[i]), diff_Q);

	/* add the contribution of each stage */
	cb_currentstage = cb;
	for (i = 0; i < stages; i++) {
		/*      (void) v_add(u_hat, &cb_currentstage[indices[i]*p], p);           */
		cb_table = &cb_currentstage[indices[i] * p];
		for (j = 0; j < p; j++) {
			L_temp = melpe_L_deposit_l(cb_table[j]);
			L_u_hat[j] = melpe_L_add(L_u_hat[j], L_temp);
		}
		cb_currentstage += levels[i] * p;
	}

	/* convert long buffer back to u_hat */
	for (i = 0; i < p; i++)
		u_hat[i] = melpe_extract_l(melpe_L_shr(L_u_hat[i], diff_Q));

	v_free(L_u_hat);
}

/* VQ_ENC -                                                                   */
/*  encode vector with full VQ using unweighted Euclidean distance            */
/*  Synopsis: vq_enc(codebook, u, levels, order, u_hat, indices)              */
/*      Input:                                                                */
/*          codebook- one dimensional linear codebook array                   */
/*          u- dimension order, the parameters to be encoded                  */
/*             (u[0 .. order-1])                                              */
/*          levels- the number of levels                                      */
/*          order- the predictor order                                        */
/*      Output:                                                               */
/*          u_hat-	 the reconstruction vector (if non null)                  */
/*          a_indices- the codebook indices (for each stage)                  */
/*                     a_indices[0..stages-1]                                 */
/*      Parameters:                                                           */
/*                                                                            */
/*      Q values:                                                             */
/*          codebook - Q13, u - Q13, u_hat - Q13                              */
/*                                                                            */
/*      Note:                                                                 */
/*          The coder does not use the returned value from vq_lspw() at all.  */

int32_t vq_enc(const int16_t codebook[], int16_t u[], int16_t levels,
		int16_t order, int16_t u_hat[], int16_t * indices)
{
	register int16_t i, j;
	const int16_t *p_cb;
	int16_t index;
	int32_t d, dmin;
	int16_t temp;

	/* Search codebook for minimum distance */
	index = 0;
	dmin = LW_MAX;
	p_cb = codebook;
	for (i = 0; i < levels; i++) {
		d = 0;
		for (j = 0; j < order; j++) {
			temp = melpe_sub(u[j], *p_cb);
			d = melpe_L_mac(d, temp, temp);
			p_cb++;
		}
		if (d < dmin) {
			dmin = d;
			index = i;
		}
	}

	/* Update index and quantized value, and return minimum distance */
	*indices = index;
	v_equ(u_hat, &codebook[order * index], order);

	return (dmin);
}

/* ========================================================================== */
/* vq_fsw() computes the weights for Euclidean distance of Fourier harmonics. */
/*    Q values:                                                               */
/*       w_fs - Q14, pitch - Q9                                               */
/* ========================================================================== */
void vq_fsw(int16_t w_fs[], int16_t num_harm, int16_t pitch)
{
	register int16_t i;
	register int16_t temp;
	int16_t tempw0, denom;
	int32_t L_temp;

	/* Calculate fundamental frequency */
	/* w0 = TWOPI/pitch */
	/* tempw0 = w0/(0.25*PI) = 8/pitch */

	tempw0 = melpe_divide_s(EIGHT_Q11, pitch);	/* tempw0 in Q17 */
	for (i = 0; i < num_harm; i++) {

		/* Bark-scale weighting */
		/* w_fs[i] = 117.0/(25.0 + 75.0* pow(1.0 + */
		/*           1.4*SQR(w0*(i+1)/(0.25*PI)),0.69)) */

		temp = melpe_add(i, 1);
		temp = melpe_shl(temp, 11);	/* (i+1), Q11 */
		L_temp = melpe_L_mult(tempw0, temp);	/* Q29 */
		L_temp = melpe_L_shl(L_temp, 1);	/* Q30 */
		temp = melpe_extract_h(L_temp);	/* w0*(i+1)/0.25*PI, Q14 */
		temp = melpe_mult(temp, temp);	/* SQR(*), Q13 */
		/* Q28 for L_temp = 1.0 + 1.4*SQR(*) */
		L_temp = melpe_L_mult(X14_Q14, temp);	/* Q28 */
		L_temp = melpe_L_add(ONE_Q28, L_temp);	/* Q28 */
		temp = L_pow_fxp(L_temp, X069_Q15, 28, 13);	/* Q13 */
		temp = melpe_mult(X75_Q8, temp);	/* Q6 */
		denom = melpe_add(X25_Q6, temp);	/* Q6 */
		w_fs[i] = melpe_divide_s(X117_Q5, denom);	/* Q14 */
	}
}

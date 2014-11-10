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

  vq_lib.c: vector quantization subroutines 

*/

#include <assert.h>
#include <stdio.h>
#include <math.h>
#include "spbstd.h"
#include "vq.h"
#include "mat.h"
#include "lpc.h"

#define BIGVAL 1E20

/* VQ_LSPW- compute LSP weighting vector-

    Atal's method:
        From Atal and Paliwal (ICASSP 1991)
        (Note: Paliwal and Atal used w(k)^2(u(k)-u_hat(k))^2,
         and we use w(k)(u(k)-u_hat(k))^2 so our weights are different
         but the method (i.e. the weighted MSE) is the same.

                */

float *mf_vq_lspw(float *w, float *lsp, float *a, int p)
{
	int j;

	for (j = 0; j < p; j++) {
		w[j] =
		    (float)pow((double)mf_lpc_aejw(a, lsp[j] * M_PI, p),
			       (double)-0.3);
		if (j == 8)
			w[j] *= 0.64;
		else if (j == 9)
			w[j] *= 0.16;
	}

	return (w);
}				/* VQ_LSPW */

/*
    VQ_MS4-
        Tree search multi-stage VQ encoder 

    Synopsis: mf_vq_ms4(cb,u,u_est,levels,ma,stages,p,w,u_hat,a_indices)
        Input:
            cb- one dimensional linear codebook array (codebook is structured 
                as [stages][levels for each stage][p])
            u- dimension p, the parameters to be encoded (u[0..p-1])
            u_est- dimension p, the estimated parameters or mean (if NULL, assume
               estimate is the all zero vector) (u_est[0..p-1])
            levels- the number of levels at each stage (levels[0..stages-1])
            ma- the tree search size (keep ma candidates from one stage to the
               next)
            stages- the number of stages of msvq
            p- the predictor order
            w- the weighting vector (w[0..p-1])
            max_inner- the maximum number of times the swapping procedure
                       in the inner loop can be executed
        Output:
            u_hat-   the reconstruction vector (if non null)
            a_indices- the codebook indices (for each stage) a_indices[0..stages-1]
        Parameters:

*/

#define P_SWAP(x,y,type) do{type u__p;u__p = x;x = y;y = u__p;}while(0)

float mf_vq_ms4(float *cb, float *u, float *u_est, int *levels, int ma,
		int stages, int p, float *w, float *u_hat, int *a_indices,
		int max_inner)
{
	float tmp, *u_tmp, *uhatw, uhatw_sq;
	float d_cj, d_opt;
	float *d, *p_d, *n_d, *p_distortion, *cb_currentstage, *cbp;
	float *errors, *p_errors, *n_errors, *p_e;
	int i, j, m, s, c, p_max, inner_counter;
	int *indices, *p_indices, *n_indices;
	int *parents, *p_parents, *n_parents;

	/* allocate memory for the current node and
	   parent node (thus, the factors of two everywhere)
	   The parents and current nodes are allocated contiguously */
	indices = calloc(1, (2 * ma * stages) * sizeof(int));
	assert(indices);
	errors = calloc(1, (2 * ma * p) * sizeof(float));
	assert(errors);
	uhatw = calloc(1, (p) * sizeof(float));
	assert(uhatw);
	d = calloc(1, (2 * ma) * sizeof(float));
	assert(d);
	parents = calloc(1, (2 * ma) * sizeof(int));
	assert(parents);

	/* initialize memory */
	mf_mf_v_zap_int(indices, 2 * stages * ma);
	mf_mf_v_zap_int(parents, 2 * ma);

	/* initialize inner loop counter */
	inner_counter = 0;

	/* set up memory */
	p_indices = &indices[0];
	n_indices = &indices[ma * stages];
	p_errors = &errors[0];
	n_errors = &errors[ma * p];
	p_d = &d[0];
	n_d = &d[ma];
	p_parents = &parents[0];
	n_parents = &parents[ma];

	/* u_tmp is the input vector (i.e. if u_est is non-null, it
	   is subtracted off) */
	u_tmp = calloc(1, (p + 1) * sizeof(float));
	assert(u_tmp);
	(void)mf_v_equ(u_tmp, u, p);
	if (u_est) {
		(void)mf_v_sub(u_tmp, u_est, p);
	}

	for (j = 0, tmp = 0.0; j < p; j++) {
		tmp += u_tmp[j] * u_tmp[j] * w[j];
	}

	/* set up inital error vectors (i.e. error vectors = u_tmp) */
	for (c = 0; c < ma; c++) {
		(void)mf_v_equ(&n_errors[c * p], u_tmp, p);
		n_d[c] = tmp;
	}

	/* no longer need memory so free it here */
	if (u_tmp)
		free(u_tmp);

	/* codebook pointer is set to point to first stage */
	cbp = cb;

	/* set m to 1 for the first stage
	   and loop over all stages */

	for (m = 1, s = 0; s < stages; s++) {
		/* Save the pointer to the beginning of the
		   current stage.  Note: cbp is only incremented in
		   one spot, and it is incremented all the way through
		   all the stages. */
		cb_currentstage = cbp;

		/* set up pointers to the parent and current nodes */
		P_SWAP(p_indices, n_indices, int *);
		P_SWAP(p_parents, n_parents, int *);
		P_SWAP(p_errors, n_errors, float *);
		P_SWAP(p_d, n_d, float *);

		/* p_max is the pointer to the maximum distortion
		   node over all candidates.  The so-called worst
		   of the best. */
		p_max = 0;

		/* set the distortions to a large value */
		for (c = 0; c < ma; c++)
			n_d[c] = BIGVAL;

		for (j = 0; j < levels[s]; j++) {
			/* compute weighted codebook element, increment codebook pointer */
			for (i = 0, uhatw_sq = 0.0; i < p; i++, cbp++) {
				uhatw_sq += *cbp * (tmp = *cbp * w[i]);
				uhatw[i] = -2.0 * tmp;
			}

			/* p_e points to the error vectors and p_distortion
			   points to the node distortions.  Note: the error
			   vectors are contiguous in memory, as are the distortions.
			   Thus, the error vector for the 2nd node comes immediately
			   after the error for the first node.  (This saves on
			   repeated initializations) */
			p_e = p_errors;
			p_distortion = p_d;

			/* iterate over all parent nodes */
			for (c = 0; c < m; c++) {
				d_cj = *p_distortion++ + uhatw_sq;
				for (i = 0; i < p; i++)
					d_cj += *p_e++ * uhatw[i];

				/* determine if d is less than the maximum candidate distortion                   i.e., is the distortion found better than the so-called
				   worst of the best */
				if (d_cj <= n_d[p_max]) {
					/* replace the worst with the values just found */
					n_d[p_max] = d_cj;
					n_indices[p_max * stages + s] = j;
					n_parents[p_max] = c;

					/* want to limit the number of times the inner loop
					   is entered (to reduce the *maximum* complexity) */
					if (inner_counter < max_inner) {
						inner_counter++;
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

							/* The inner loop counter now exceeds the
							   maximum, and the inner loop will now not
							   be entered.  Instead of quitting the search
							   or doing something drastic, we simply keep
							   track of the best candidate (rather than the
							   M best) by setting p_max to equal the index
							   of the minimum distortion
							   i.e. only keep one candidate around
							   the MINIMUM distortion */
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
			/* get the error from the parent node and subtract off
			   the codebook value */
			(void)mf_v_equ(&n_errors[c * p],
				       &p_errors[n_parents[c] * p], p);
			(void)mf_v_sub(&n_errors[c * p],
				       &cb_currentstage[n_indices
							[c * stages + s] * p],
				       p);

			/* get the indices that were used for the parent node */
			(void)mf_mf_v_equ_int(&n_indices[c * stages],
					      &p_indices[n_parents[c] * stages],
					      s);
		}

		m = (m * levels[s] > ma) ? ma : m * levels[s];
	}			/* for s */

	/* find the optimum candidate c */
	for (i = 1, c = 0; i < ma; i++) {
		if (n_d[i] < n_d[c])
			c = i;
	}

	d_opt = n_d[c];

	if (a_indices) {
		(void)mf_mf_v_equ_int(a_indices, &n_indices[c * stages],
				      stages);
	}
	if (u_hat) {
		if (u_est)
			(void)mf_v_equ(u_hat, u_est, p);
		else
			(void)mf_v_zap(u_hat, p);

		cb_currentstage = cb;
		for (s = 0; s < stages; s++) {
			(void)mf_v_add(u_hat,
				       &cb_currentstage[n_indices
							[c * stages + s] * p],
				       p);
			cb_currentstage += levels[s] * p;
		}
	}

	if (parents)
		free(parents);
	if (d)
		free(d);
	if (uhatw)
		free(uhatw);
	if (errors)
		free(errors);
	if (indices)
		free(indices);

	return (d_opt);
}

/*
    VQ_MSD2-
        Tree search multi-stage VQ decoder

    Synopsis: vq_msd(cb,u,u_est,a,indices,levels,stages,p,conversion)
        Input:
            cb- one dimensional linear codebook array (codebook is structured 
                as [stages][levels for each stage][p])
            indices- the codebook indices (for each stage) indices[0..stages-1]
            levels- the number of levels (for each stage) levels[0..stages-1]
            u_est- dimension p, the estimated parameters or mean (if NULL, assume
               estimate is the all zero vector) (u_est[0..p-1])
            stages- the number of stages of msvq
            p- the predictor order
            conversion- the conversion constant (see lpc.h, lpc_conv.c)
        Output:
            u- dimension p, the decoded parameters (if NULL, use alternate
               temporary storage) (u[0..p-1])
            a- predictor parameters (a[0..p]), if NULL, don't compute
        Returns:
            pointer to reconstruction vector (u)
        Parameters:

*/

float *mf_vq_msd2(float *cb, float *u, float *u_est, float *a, int *indices,
		  int *levels, int stages, int p, int conversion)
{
	(void)a;
	(void)conversion;

	float *u_hat, *cb_currentstage;
	int i;

	/* allocate memory (if required) */
	if (u == (float *)NULL) {
		u_hat = calloc(1, (p) * sizeof(float));
		assert(u_hat);
	} else
		u_hat = u;

	/* add estimate on (if non-null), or clear vector */
	if (u_est) {
		(void)mf_v_equ(u_hat, u_est, p);
	} else {
		(void)mf_v_zap(u_hat, p);
	}

	/* add the contribution of each stage */
	cb_currentstage = cb;
	for (i = 0; i < stages; i++) {
		(void)mf_v_add(u_hat, &cb_currentstage[indices[i] * p], p);
		cb_currentstage += levels[i] * p;
	}

	return (u);
}

/* VQ_ENC -
   encode vector with full VQ using unweighted Euclidean distance
    Synopsis: mf_vq_enc(cb, u, levels, p, u_hat, indices)
        Input:
            cb- one dimensional linear codebook array
            u- dimension p, the parameters to be encoded (u[0..p-1])
            levels- the number of levels
            p- the predictor order
        Output:
            u_hat-   the reconstruction vector (if non null)
            a_indices- the codebook indices (for each stage) a_indices[0..stages-1]
        Parameters:
*/

float mf_vq_enc(float *cb, float *u, int levels, int p, float *u_hat,
		int *indices)
{
	int i, j, index;
	float d, dmin;
	float *p_cb;

	/* Search codebook for minimum distance */
	index = 0;
	dmin = BIGVAL;
	p_cb = cb;
	for (i = 0; i < levels; i++) {
		d = 0.0;
		for (j = 0; j < p; j++) {
			d += SQR(u[j] - *p_cb);
			p_cb++;
		}
		if (d < dmin) {
			dmin = d;
			index = i;
		}
	}

	/* Update index and quantized value, and return minimum distance */
	*indices = index;
	mf_v_equ(u_hat, &cb[p * index], p);
	return (dmin);
}

/* VQ_FSW - 
   compute the weights for Euclidean distance of Fourier harmonics  */

void mf_vq_fsw(float *mf_w_fs, int num_harm, float pitch)
{

	int j;
	float w0;

	/* Calculate fundamental frequency */
	w0 = TWOPI / pitch;
	for (j = 0; j < num_harm; j++) {

		/* Bark-scale weighting */
		mf_w_fs[j] = 117.0 / (25.0 + 75.0 *
				      pow(1.0 +
					  1.4 * SQR(w0 * (j + 1) / (0.25 * PI)),
					  0.69));
	}

}

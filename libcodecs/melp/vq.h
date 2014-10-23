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
   vq.h     VQ include file.
            (Search/decode/distortion/weighting functions for VQ)

   Copyright (c) 1995 by Texas Instruments Incorporated.  All rights reserved.
*/

#ifndef _vq_h_
#define _vq_h_

float mf_vq_ms4(float *cb, float *u, float *u_est, int *levels, int ma,
		int stages, int p, float *w, float *u_hat, int *indices,
		int max_inner);
float *mf_vq_msd2(float *cb, float *u, float *u_est, float *a, int *indices,
		  int *levels, int stages, int p, int conversion);
float *mf_vq_lspw(float *w, float *lsp, float *a, int p);

/* Structure definition */
struct msmf_vq_param {		/* Multistage VQ parameters */
	int num_stages;
	int *num_levels;
	int *num_bits;
	int dimension;
	int num_best;
	int *indices;
	char *fname_cb;
	float *cb;
};

/* External function definitions */

#define msmf_vq_enc(u,w,u_hat,par)\
    mf_vq_ms4(par.cb,u,(float*)NULL,par.num_levels,\
	  par.num_best,par.num_stages,par.dimension,w,u_hat,\
	  par.indices,MSVQ_MAXCNT)

#define fsmf_vq_enc(u,u_hat,par)\
    mf_vq_enc(par.cb,u,*(par.num_levels),\
	  par.dimension,u_hat,\
	  par.indices)

#define msvq_dec(u,par)\
    mf_vq_msd2(par.cb,u,(float*)NULL,(float*)NULL,par.indices,par.num_levels,par.num_stages,par.dimension,0)

void msvq_init(struct msmf_vq_param *par);

float mf_vq_enc(float *cb, float *u, int levels, int p, float *u_hat,
		int *indices);

void mf_vq_fsw(float *mf_w_fs, int num_harm, float pitch);

#endif

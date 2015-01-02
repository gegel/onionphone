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
    Name: mf_melp_syn.c
    Description: MELP synthesis
      This program takes the new parameters for a mf_speech
      frame and synthesizes the output mf_speech.  It keeps
      an internal record of the previous frame parameters
      to use for interpolation.			
    Inputs:
      *par - MELP parameter structure
    Outputs: 
      mf_speech[] - output mf_speech signal
    Returns: void
*/

/* compiler include files */

#include <assert.h>
#include <stdio.h>
#include <math.h>
#include "melp.h"
#include "spbstd.h"
#include "lpc.h"
#include "mat.h"
#include "vq.h"
#include "fs.h"

/* compiler constants */

#if (MIX_ORD > DISP_ORD)
#define BEGIN MIX_ORD
#else
#define BEGIN DISP_ORD
#endif

#define TILT_ORD 1
#define SYN_GAIN 1000.0
#define	SCALEOVER	10
#define PDEL SCALEOVER

/* external memory references */

extern float mf_bp_cof[mf_NUM_BANDS][MIX_ORD + 1];
extern float mf_disp_cof[DISP_ORD + 1];
extern float mf_msvq_cb[];
extern float mf_fsvq_cb[];
extern int mf_fsvq_weighted;

/* temporary memory */

static float mf_sigbuf[BEGIN + mf_PITCHMAX];
static float mf_sig2[BEGIN + mf_PITCHMAX];
static float mf_fs_real[mf_PITCHMAX];

/* permanent memory */

static int mf_firstcall = 1;	/* Just used for noise gain init */
static int mf_mf_prev_gain_err;
static float mf_sigsave[mf_PITCHMAX];
static struct mf_melp_param mf_prev_par;
static int mf_syn_begin;
static float mf_premf_v_scale;
static float mf_noise_gain = MIN_NOISE;
static float mf_pulse_del[MIX_ORD], mf_noise_del[MIX_ORD];
static float mf_lpc_del[LPC_ORD], mf_ase_del[LPC_ORD], mf_tilt_del[TILT_ORD];
static float mf_disp_del[DISP_ORD];

static struct msmf_vq_param mf_vq_par;	/* MSVQ parameters */

static struct msmf_vq_param mf_fs_mf_vq_par;	/* Fourier series VQ parameters */
static float mf_mf_w_fs_inv[NUM_HARM];

/* these can be saved or recomputed */
static float mf_prev_pcof[MIX_ORD + 1], mf_prev_ncof[MIX_ORD + 1];
static float mf_prev_tilt;
static float mf_prev_gain;

void mf_melp_syn(struct mf_melp_param *par, float sp_out[])
{

	int i, gaincnt;
	int erase;
	int length;
	float intfact, ifact, ifact_gain;
	float gain, pulse_gain, pitch, jitter;
	float curr_tilt, tilt_cof[TILT_ORD + 1];
	float temp, sig_prob;
	float lsf[LPC_ORD + 1];
	float lpc[LPC_ORD + 1];
	float ase_num[LPC_ORD + 1], ase_den[LPC_ORD + 1];
	float curr_pcof[MIX_ORD + 1], curr_ncof[MIX_ORD + 1];
	float pulse_cof[MIX_ORD + 1], noise_cof[MIX_ORD + 1];

	/* Copy previous period of processed mf_speech to output array */
	if (mf_syn_begin > 0) {
		if (mf_syn_begin > mf_FRAME) {
			mf_v_equ(&sp_out[0], &mf_sigsave[0], mf_FRAME);
			/* past end: save remainder in mf_sigsave[0] */
			mf_v_equ(&mf_sigsave[0], &mf_sigsave[mf_FRAME],
				 mf_syn_begin - mf_FRAME);
		} else
			mf_v_equ(&sp_out[0], &mf_sigsave[0], mf_syn_begin);
	}

	/* Update MSVQ information */
	par->msvq_stages = mf_vq_par.num_stages;
	par->msvq_bits = mf_vq_par.num_bits;
	par->msvq_levels = mf_vq_par.num_levels;
	par->msvq_index = mf_vq_par.indices;
	par->fsvq_index = mf_fs_mf_vq_par.indices;

	/*  Read and decode channel input buffer    */
	erase = mf_melp_chn_read(par, &mf_prev_par);

	if (par->uv_flag != 1 && !erase) {
		/* Un-weight Fourier mf_magnitudes */
		mf_window(&par->fs_mf_mag[0], mf_mf_w_fs_inv,
			  &par->fs_mf_mag[0], NUM_HARM);
	}

	/* Update adaptive noise level estimate based on last gain  */
	if (mf_firstcall) {
		mf_firstcall = 0;
		mf_noise_gain = par->gain[NUM_GAINFR - 1];
	}

	else if (!erase) {
		for (i = 0; i < NUM_GAINFR; i++) {
			mf_noise_est(par->gain[i], &mf_noise_gain, UPCONST,
				     DOWNCONST, MIN_NOISE, MAX_NOISE);

			/* Adjust gain based on noise level (noise suppression) */
			mf_noise_sup(&par->gain[i], mf_noise_gain, MAX_NS_SUP,
				     MAX_NS_ATT, NFACT);

		}

	}

	/* Clamp LSP bandwidths to avoid sharp LPC filters */
	lpc_clamp(par->lsf, BWMIN, LPC_ORD);

	/* Calculate spectral tilt for current frame for spectral enhancement */
	tilt_cof[0] = 1.0;
	mf_lpc_lsp2pred(par->lsf, lpc, LPC_ORD);
	mf_lpc_pred2refl(lpc, mf_sig2, LPC_ORD);
	if (mf_sig2[1] < 0.0)
		curr_tilt = 0.5 * mf_sig2[1];
	else
		curr_tilt = 0.0;

	/* Disable pitch interpolation for high-pitched onsets */
	if (par->pitch < 0.5 * mf_prev_par.pitch &&
	    par->gain[0] > 6.0 + mf_prev_par.gain[NUM_GAINFR - 1]) {

		/* copy current pitch into previous */
		mf_prev_par.pitch = par->pitch;
	}

	/* Set pulse and noise coefficients based on voicing strengths */
	mf_v_zap(curr_pcof, MIX_ORD + 1);
	mf_v_zap(curr_ncof, MIX_ORD + 1);
	for (i = 0; i < mf_NUM_BANDS; i++) {
		if (par->bpvc[i] > 0.5)
			mf_v_add(curr_pcof, &mf_bp_cof[i][0], MIX_ORD + 1);
		else
			mf_v_add(curr_ncof, &mf_bp_cof[i][0], MIX_ORD + 1);
	}

	/* Process each pitch period */

	while (mf_syn_begin < mf_FRAME) {

		/* interpolate previous and current parameters */

		ifact = ((float)mf_syn_begin) / mf_FRAME;

		if (mf_syn_begin >= GAINFR) {
			gaincnt = 2;
			ifact_gain = ((float)mf_syn_begin - GAINFR) / GAINFR;
		} else {
			gaincnt = 1;
			ifact_gain = ((float)mf_syn_begin) / GAINFR;
		}

		/* interpolate gain */
		if (gaincnt > 1) {
			gain = ifact_gain * par->gain[gaincnt - 1] +
			    (1.0 - ifact_gain) * par->gain[gaincnt - 2];
		} else {
			gain = ifact_gain * par->gain[gaincnt - 1] +
			    (1.0 - ifact_gain) * mf_prev_par.gain[NUM_GAINFR -
								  1];
		}

		/* Set overall interpolation path based on gain change */

		temp =
		    par->gain[NUM_GAINFR - 1] - mf_prev_par.gain[NUM_GAINFR -
								 1];
		if (fabs(temp) > 6) {

			/* Power surge: use gain adjusted interpolation */
			intfact =
			    (gain - mf_prev_par.gain[NUM_GAINFR - 1]) / temp;
			if (intfact > 1.0)
				intfact = 1.0;
			if (intfact < 0.0)
				intfact = 0.0;
		} else
			/* Otherwise, linear interpolation */
			intfact = ((float)mf_syn_begin) / mf_FRAME;

		/* interpolate LSF's and convert to LPC filter */
		mf_interp_array(mf_prev_par.lsf, par->lsf, lsf, intfact,
				LPC_ORD + 1);
		mf_lpc_lsp2pred(lsf, lpc, LPC_ORD);

		/* Check signal probability for adaptive spectral enhancement filter */
		sig_prob =
		    mf_lin_int_bnd(gain, mf_noise_gain + 12.0,
				   mf_noise_gain + 30.0, 0.0, 1.0);

		/* Calculate adaptive spectral enhancement filter coefficients */
		ase_num[0] = 1.0;
		lpc_bw_expand(lpc, ase_num, sig_prob * ASE_NUM_BW, LPC_ORD);
		lpc_bw_expand(lpc, ase_den, sig_prob * ASE_DEN_BW, LPC_ORD);
		tilt_cof[1] = sig_prob * (intfact * curr_tilt +
					  (1.0 - intfact) * mf_prev_tilt);

		/* interpolate pitch and pulse gain */
		pitch =
		    intfact * par->pitch + (1.0 - intfact) * mf_prev_par.pitch;
		pulse_gain = SYN_GAIN * sqrt(pitch);

		/* interpolate pulse and noise coefficients */
		temp = sqrt(ifact);
		mf_interp_array(mf_prev_pcof, curr_pcof, pulse_cof, temp,
				MIX_ORD + 1);
		mf_interp_array(mf_prev_ncof, curr_ncof, noise_cof, temp,
				MIX_ORD + 1);

		/* interpolate jitter */
		jitter = ifact * par->jitter +
		    (1.0 - ifact) * mf_prev_par.jitter;

		/* convert gain to linear */
		gain = pow(10.0, 0.05 * gain);

		/* Set period length based on pitch and jitter */
		mf_rand_num(&temp, 1.0, 1);
		length = pitch * (1.0 - jitter * temp) + 0.5;
		if (length < mf_PITCHMIN)
			length = mf_PITCHMIN;
		if (length > mf_PITCHMAX)
			length = mf_PITCHMAX;

		/* Use inverse DFT for pulse excitation */
		mf_fill(mf_fs_real, 1.0, length);
		mf_fs_real[0] = 0.0;
		mf_interp_array(mf_prev_par.fs_mf_mag, par->fs_mf_mag,
				&mf_fs_real[1], intfact, NUM_HARM);
		mf_idft_real(mf_fs_real, &mf_sigbuf[BEGIN], length);

		/* Delay overall signal by PDEL samples (circular shift) */
		/* use mf_fs_real as a scratch buffer */
		mf_v_equ(mf_fs_real, &mf_sigbuf[BEGIN], length);
		mf_v_equ(&mf_sigbuf[BEGIN + PDEL], mf_fs_real, length - PDEL);
		mf_v_equ(&mf_sigbuf[BEGIN], &mf_fs_real[length - PDEL], PDEL);

		/* Scale by pulse gain */
		mf_v_scale(&mf_sigbuf[BEGIN], pulse_gain, length);

		/* Filter and scale pulse excitation */
		mf_v_equ(&mf_sigbuf[BEGIN - MIX_ORD], mf_pulse_del, MIX_ORD);
		mf_v_equ(mf_pulse_del, &mf_sigbuf[length + BEGIN - MIX_ORD],
			 MIX_ORD);
		mf_zerflt(&mf_sigbuf[BEGIN], pulse_cof, &mf_sigbuf[BEGIN],
			  MIX_ORD, length);

		/* Get scaled noise excitation */
		mf_rand_num(&mf_sig2[BEGIN], (1.732 * SYN_GAIN), length);

		/* Filter noise excitation */
		mf_v_equ(&mf_sig2[BEGIN - MIX_ORD], mf_noise_del, MIX_ORD);
		mf_v_equ(mf_noise_del, &mf_sig2[length + BEGIN - MIX_ORD],
			 MIX_ORD);
		mf_zerflt(&mf_sig2[BEGIN], noise_cof, &mf_sig2[BEGIN], MIX_ORD,
			  length);

		/* Add two excitation signals (mixed excitation) */
		mf_v_add(&mf_sigbuf[BEGIN], &mf_sig2[BEGIN], length);

		/* Adaptive spectral enhancement */
		mf_v_equ(&mf_sigbuf[BEGIN - LPC_ORD], mf_ase_del, LPC_ORD);
		mf_lpc_synthesis(&mf_sigbuf[BEGIN], &mf_sigbuf[BEGIN], ase_den,
				 LPC_ORD, length);
		mf_v_equ(mf_ase_del, &mf_sigbuf[BEGIN + length - LPC_ORD],
			 LPC_ORD);
		mf_zerflt(&mf_sigbuf[BEGIN], ase_num, &mf_sigbuf[BEGIN],
			  LPC_ORD, length);
		mf_v_equ(&mf_sigbuf[BEGIN - TILT_ORD], mf_tilt_del, TILT_ORD);
		mf_v_equ(mf_tilt_del, &mf_sigbuf[length + BEGIN - TILT_ORD],
			 TILT_ORD);
		mf_zerflt(&mf_sigbuf[BEGIN], tilt_cof, &mf_sigbuf[BEGIN],
			  TILT_ORD, length);

		/* Perform LPC synthesis filtering */
		mf_v_equ(&mf_sigbuf[BEGIN - LPC_ORD], mf_lpc_del, LPC_ORD);
		mf_lpc_synthesis(&mf_sigbuf[BEGIN], &mf_sigbuf[BEGIN], lpc,
				 LPC_ORD, length);
		mf_v_equ(mf_lpc_del, &mf_sigbuf[length + BEGIN - LPC_ORD],
			 LPC_ORD);

		/* Adjust scaling of synthetic mf_speech */
		mf_scale_adj(&mf_sigbuf[BEGIN], gain, &mf_premf_v_scale, length,
			     SCALEOVER);

		/* Implement pulse dispersion filter */
		mf_v_equ(&mf_sigbuf[BEGIN - DISP_ORD], mf_disp_del, DISP_ORD);
		mf_v_equ(mf_disp_del, &mf_sigbuf[length + BEGIN - DISP_ORD],
			 DISP_ORD);
		mf_zerflt(&mf_sigbuf[BEGIN], mf_disp_cof, &mf_sigbuf[BEGIN],
			  DISP_ORD, length);

		/* Copy processed mf_speech to output array (not past frame end) */
		if (mf_syn_begin + length > mf_FRAME) {
			mf_v_equ(&sp_out[mf_syn_begin], &mf_sigbuf[BEGIN],
				 mf_FRAME - mf_syn_begin);

			/* past end: save remainder in mf_sigsave[0] */
			mf_v_equ(&mf_sigsave[0],
				 &mf_sigbuf[BEGIN + mf_FRAME - mf_syn_begin],
				 length - (mf_FRAME - mf_syn_begin));
		}

		else
			mf_v_equ(&sp_out[mf_syn_begin], &mf_sigbuf[BEGIN],
				 length);

		/* Update mf_syn_begin for next period */
		mf_syn_begin += length;

	}

	/* Save previous pulse and noise filters for next frame */
	mf_v_equ(mf_prev_pcof, curr_pcof, MIX_ORD + 1);
	mf_v_equ(mf_prev_ncof, curr_ncof, MIX_ORD + 1);

	/* Copy current parameters to previous parameters for next time */
	mf_prev_par = *par;
	mf_prev_tilt = curr_tilt;

	/* Update mf_syn_begin for next frame */
	mf_syn_begin -= mf_FRAME;

}

/* 
 *
 * Subroutine mf_mf_melp_syn_init(): perform initialization for melp 
 *	synthesis
 *
 */

void mf_mf_melp_syn_init()
{
	int i;
	float mf_w_fs[NUM_HARM];

	mf_v_zap(mf_prev_par.gain, NUM_GAINFR);
	mf_prev_par.pitch = UV_PITCH;
	mf_prev_par.lsf[0] = 0.0;
	for (i = 1; i < LPC_ORD + 1; i++)
		mf_prev_par.lsf[i] =
		    mf_prev_par.lsf[i - 1] + (1.0 / (LPC_ORD + 1));
	mf_prev_par.jitter = 0.0;
	mf_v_zap(&mf_prev_par.bpvc[0], mf_NUM_BANDS);
	mf_prev_tilt = 0.0;
	mf_prev_gain = 0.0;
	mf_premf_v_scale = 0.0;
	mf_syn_begin = 0;
	mf_noise_gain = MIN_NOISE;
	mf_firstcall = 1;
	mf_mf_prev_gain_err = 0;
	mf_v_zap(mf_pulse_del, MIX_ORD);
	mf_v_zap(mf_noise_del, MIX_ORD);
	mf_v_zap(mf_lpc_del, LPC_ORD);
	mf_v_zap(mf_ase_del, LPC_ORD);
	mf_v_zap(mf_tilt_del, TILT_ORD);
	mf_v_zap(mf_disp_del, DISP_ORD);
	mf_v_zap(mf_sig2, BEGIN + mf_PITCHMAX);
	mf_v_zap(mf_sigbuf, BEGIN + mf_PITCHMAX);
	mf_v_zap(mf_sigsave, mf_PITCHMAX);
	mf_v_zap(mf_prev_pcof, MIX_ORD + 1);
	mf_v_zap(mf_prev_ncof, MIX_ORD + 1);
	mf_prev_ncof[MIX_ORD / 2] = 1.0;

	mf_fill(mf_prev_par.fs_mf_mag, 1.0, NUM_HARM);

	/* 
	 * Initialize multi-stage vector quantization (read codebook) 
	 */

	mf_vq_par.num_best = MSVQ_M;
	mf_vq_par.num_stages = 4;
	mf_vq_par.dimension = 10;

	/* 
	 * Allocate memory for number of levels per stage and indices
	 * and for number of bits per stage 
	 */

	mf_vq_par.num_levels = calloc(1, (mf_vq_par.num_stages) * sizeof(int));
	assert(mf_vq_par.num_levels);
	mf_vq_par.indices = calloc(1, (mf_vq_par.num_stages) * sizeof(int));
	assert(mf_vq_par.indices);
	mf_vq_par.num_bits = calloc(1, (mf_vq_par.num_stages) * sizeof(int));
	assert(mf_vq_par.num_bits);

	mf_vq_par.num_levels[0] = 128;
	mf_vq_par.num_levels[1] = 64;
	mf_vq_par.num_levels[2] = 64;
	mf_vq_par.num_levels[3] = 64;

	mf_vq_par.num_bits[0] = 7;
	mf_vq_par.num_bits[1] = 6;
	mf_vq_par.num_bits[2] = 6;
	mf_vq_par.num_bits[3] = 6;

	mf_vq_par.cb = mf_msvq_cb;

	/* Scale codebook to 0 to 1 */
	if (mf_fsvq_weighted == 0)
		mf_v_scale(mf_vq_par.cb, (2.0 / FSAMP), 3200);

	/* 
	 * Initialize Fourier mf_magnitude vector quantization (read codebook) 
	 */

	mf_fs_mf_vq_par.num_best = 1;
	mf_fs_mf_vq_par.num_stages = 1;
	mf_fs_mf_vq_par.dimension = NUM_HARM;

	/* 
	 * Allocate memory for number of levels per stage and indices
	 * and for number of bits per stage 
	 */

	mf_fs_mf_vq_par.num_levels = calloc(1,
					    (mf_fs_mf_vq_par.num_stages) *
					    sizeof(int));
	assert(mf_fs_mf_vq_par.num_levels);
	mf_fs_mf_vq_par.indices = calloc(1,
					 (mf_fs_mf_vq_par.num_stages) *
					 sizeof(int));
	assert(mf_fs_mf_vq_par.indices);
	mf_fs_mf_vq_par.num_bits = calloc(1,
					  (mf_fs_mf_vq_par.num_stages) *
					  sizeof(int));
	assert(mf_fs_mf_vq_par.num_bits);

	mf_fs_mf_vq_par.num_levels[0] = FS_LEVELS;

	mf_fs_mf_vq_par.num_bits[0] = FS_BITS;

	mf_fs_mf_vq_par.cb = mf_fsvq_cb;

	/* 
	 * Initialize fixed MSE weighting and inverse of weighting 
	 */

	mf_vq_fsw(mf_w_fs, NUM_HARM, 60.0);
	for (i = 0; i < NUM_HARM; i++)
		mf_mf_w_fs_inv[i] = 1.0 / mf_w_fs[i];

	/* 
	 * Pre-weight codebook (assume single stage only) 
	 */

	if (mf_fsvq_weighted == 0) {
		mf_fsvq_weighted = 1;
		for (i = 0; i < mf_fs_mf_vq_par.num_levels[0]; i++)
			mf_window(&mf_fs_mf_vq_par.cb[i * NUM_HARM], mf_w_fs,
				  &mf_fs_mf_vq_par.cb[i * NUM_HARM], NUM_HARM);
	}

}

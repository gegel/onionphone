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

  melp_sub.c: MELP-specific subroutines

*/

#include <stdio.h>
#include <math.h>
#include "spbstd.h"
#include "mat.h"
#include "melp_sub.h"
#include "dsp_sub.h"
#include "pit.h"

/*
    Name: mf_bpvc_ana.c
    Description: Bandpass voicing analysis
    Inputs:
      mf_speech[] - input mf_speech signal
      mf_fpitch[] - initial (floating point) pitch estimates
    Outputs: 
      bpvc[] - bandpass voicing decisions
      pitch[] - frame pitch estimates
    Returns: void 

    Copyright (c) 1995 by Texas Instruments, Inc.  All rights reserved.
*/

/* Filter orders */
#define ENV_ORD 2
#define BPF_ORD 6

/* Constants */
static int mf_PITCHMAX;
static int mf_PITCHMIN;
static int mf_FRAME;
static int mf_NUM_BANDS;
static int mf_PIT_BEG;
static int mf_PIT_P_FR;
static int mf_PIT_FR_BEG;
static int mf_FIRST_CNTR;
static int mf_BPF_BEG;
static int mf_NUM_PITCHES;
static int mf_LMIN;

/* Static memory */
static float **mf_bpfdel;
static float **mf_envdel;
static float *mf_mf_envdel2;
static float *mf_sigbuf;

/* External variables */
extern float mf_bpf_num[], mf_bpf_den[];

void mf_bpvc_ana(float mf_speech[], float mf_fpitch[], float bpvc[],
		 float pitch[])
{

	float pcorr, temp;
	int j;

	/* Filter lowest band and estimate pitch */
	mf_v_equ(&mf_sigbuf[mf_PIT_BEG - BPF_ORD], &mf_bpfdel[0][0], BPF_ORD);
	mf_polflt(&mf_speech[mf_PIT_FR_BEG], &mf_bpf_den[0],
		  &mf_sigbuf[mf_PIT_BEG], BPF_ORD, mf_PIT_P_FR);
	mf_v_equ(&mf_bpfdel[0][0], &mf_sigbuf[mf_PIT_BEG + mf_FRAME - BPF_ORD],
		 BPF_ORD);
	mf_zerflt(&mf_sigbuf[mf_PIT_BEG], &mf_bpf_num[0],
		  &mf_sigbuf[mf_PIT_BEG], BPF_ORD, mf_PIT_P_FR);

	*pitch = mf_frac_pch(&mf_sigbuf[mf_FIRST_CNTR],
			     &bpvc[0], mf_fpitch[0], 5, mf_PITCHMIN,
			     mf_PITCHMAX, mf_LMIN);

	for (j = 1; j < mf_NUM_PITCHES; j++) {
		temp = mf_frac_pch(&mf_sigbuf[mf_FIRST_CNTR],
				   &pcorr, mf_fpitch[j], 5, mf_PITCHMIN,
				   mf_PITCHMAX, mf_LMIN);

		/* choose largest correlation value */
		if (pcorr > bpvc[0]) {
			*pitch = temp;
			bpvc[0] = pcorr;
		}

	}

	/* Calculate bandpass voicing for frames */

	for (j = 1; j < mf_NUM_BANDS; j++) {

		/* Bandpass filter input mf_speech */
		mf_v_equ(&mf_sigbuf[mf_PIT_BEG - BPF_ORD], &mf_bpfdel[j][0],
			 BPF_ORD);
		mf_polflt(&mf_speech[mf_PIT_FR_BEG],
			  &mf_bpf_den[j * (BPF_ORD + 1)],
			  &mf_sigbuf[mf_PIT_BEG], BPF_ORD, mf_PIT_P_FR);
		mf_v_equ(&mf_bpfdel[j][0],
			 &mf_sigbuf[mf_PIT_BEG + mf_FRAME - BPF_ORD], BPF_ORD);
		mf_zerflt(&mf_sigbuf[mf_PIT_BEG],
			  &mf_bpf_num[j * (BPF_ORD + 1)],
			  &mf_sigbuf[mf_PIT_BEG], BPF_ORD, mf_PIT_P_FR);

		/* Check correlations for each frame */
		mf_frac_pch(&mf_sigbuf[mf_FIRST_CNTR],
			    &bpvc[j], *pitch, 0, mf_PITCHMIN, mf_PITCHMAX,
			    mf_LMIN);

		/* Calculate mf_envelope of bandpass filtered input mf_speech */
		temp = mf_mf_envdel2[j];
		mf_mf_envdel2[j] = mf_sigbuf[mf_PIT_BEG + mf_FRAME - 1];
		mf_v_equ(&mf_sigbuf[mf_PIT_BEG - ENV_ORD], &mf_envdel[j][0],
			 ENV_ORD);
		mf_envelope(&mf_sigbuf[mf_PIT_BEG], temp,
			    &mf_sigbuf[mf_PIT_BEG], mf_PIT_P_FR);
		mf_v_equ(&mf_envdel[j][0],
			 &mf_sigbuf[mf_PIT_BEG + mf_FRAME - ENV_ORD], ENV_ORD);

		/* Check correlations for each frame */
		mf_frac_pch(&mf_sigbuf[mf_FIRST_CNTR], &pcorr,
			    *pitch, 0, mf_PITCHMIN, mf_PITCHMAX, mf_LMIN);

		/* reduce mf_envelope correlation */
		pcorr -= 0.1;

		if (pcorr > bpvc[j])
			bpvc[j] = pcorr;
	}
}

void mf_mf_bpvc_ana_init(int fr, int pmin, int pmax, int nbands, int num_p,
			 int lmin)
{

	/* Initialize constants */
	mf_FRAME = fr;
	mf_PITCHMIN = pmin;
	mf_PITCHMAX = pmax;
	mf_NUM_BANDS = nbands;
	mf_NUM_PITCHES = num_p;
	mf_LMIN = lmin;
	mf_PIT_BEG = BPF_ORD;
	mf_PIT_P_FR = ((2 * mf_PITCHMAX) + 1);
	mf_PIT_FR_BEG = (-mf_PITCHMAX);
	mf_FIRST_CNTR = (mf_PIT_BEG + mf_PITCHMAX);
	mf_BPF_BEG = (mf_PIT_BEG + mf_PIT_P_FR - mf_FRAME);

	/* Allocate memory */
	mf_bpfdel = calloc(1, (mf_NUM_BANDS) * sizeof(float *));
	if (!mf_bpfdel)
		program_abort(__FILE__, "calloc", 0, __LINE__);
	else {
		for (int u__i = 0; u__i < mf_NUM_BANDS; u__i++) {
			mf_bpfdel[u__i] = calloc(1, BPF_ORD * sizeof(float));
			if (!mf_bpfdel[u__i])
				program_abort(__FILE__, "calloc", 0, __LINE__);
		}
	}
	mf_v_zap(&mf_bpfdel[0][0], mf_NUM_BANDS * BPF_ORD);

	mf_envdel = calloc(1, (mf_NUM_BANDS) * sizeof(float *));
	if (!mf_envdel)
		program_abort(__FILE__, "calloc", 0, __LINE__);
	else {
		for (int u__i = 0; u__i < mf_NUM_BANDS; u__i++) {
			mf_envdel[u__i] = calloc(1, ENV_ORD * sizeof(float));
			if (!mf_envdel[u__i])
				program_abort(__FILE__, "calloc", 0, __LINE__);
		}
	}
	mf_v_zap(&mf_envdel[0][0], mf_NUM_BANDS * ENV_ORD);
	mf_mf_envdel2 = calloc(1, (mf_NUM_BANDS) * sizeof(float));
	if (!mf_mf_envdel2)
		program_abort(__FILE__, "calloc", 0, __LINE__);
	mf_v_zap(mf_mf_envdel2, mf_NUM_BANDS);

	/* Allocate scratch buffer */
	mf_sigbuf = calloc(1, (mf_PIT_BEG + mf_PIT_P_FR) * sizeof(float));
	if (!mf_sigbuf)
		program_abort(__FILE__, "calloc", 0, __LINE__);
}

/*
    Name: mf_dc_rmv.c
    Description: remove DC from input signal
    Inputs: 
      sigin[] - input signal
      mf_dcdel[] - filter delay history (size DC_ORD)
      frame - number of samples to filter
    Outputs: 
      sigout[] - output signal
      mf_dcdel[] - updated filter delay history
    Returns: void 
    See_Also:

    Copyright (c) 1995 by Texas Instruments, Inc.  All rights reserved.
*/

/* Filter order */
#define DC_ORD 4

/* DC removal filter */
/* 4th order Chebychev Type II 60 Hz removal filter */
/* cutoff=60 Hz, stop=-30 dB */
static float mf_dc_num[DC_ORD + 1] = {
	0.92692416,
	-3.70563834,
	5.55742893,
	-3.70563834,
	0.92692416
};

static float mf_dc_den[DC_ORD + 1] = {
	1.00000000,
	-3.84610723,
	5.55209760,
	-3.56516069,
	0.85918839
};

void mf_dc_rmv(float sigin[], float sigout[], float mf_dcdel[], int frame)
{
	float *mf_sigbuf;

	/* Allocate scratch buffer */
	mf_sigbuf = calloc(1, (frame + DC_ORD) * sizeof(float));
	if (!mf_sigbuf)
		program_abort(__FILE__, "calloc", 0, __LINE__);

	/* Remove DC from input mf_speech */
	mf_v_equ(mf_sigbuf, mf_dcdel, DC_ORD);
	mf_polflt(sigin, mf_dc_den, &mf_sigbuf[DC_ORD], DC_ORD, frame);
	mf_v_equ(mf_dcdel, &mf_sigbuf[frame], DC_ORD);
	mf_zerflt(&mf_sigbuf[DC_ORD], mf_dc_num, sigout, DC_ORD, frame);

	/* Free scratch buffer */
	if (mf_sigbuf)
		free(mf_sigbuf);

}

/*
    Name: mf_gain_ana.c
    Description: analyze gain level for input signal
    Inputs: 
      sigin[] - input signal
      pitch - pitch value (for pitch synchronous mf_window)
      minlength - minimum mf_window length
      maxlength - maximum mf_window length
    Outputs: 
    Returns: log gain in dB
    See_Also:

    Copyright (c) 1995 by Texas Instruments, Inc.  All rights reserved.
*/

#define MINGAIN 0.0

float mf_gain_ana(float sigin[], float pitch, int minlength, int maxlength)
{
	int length;
	float flength, gain;

	/* Find shortest pitch multiple mf_window length (floating point) */
	flength = pitch;
	while (flength < minlength)
		flength += pitch;

	/* Convert mf_window length to integer and check against maximum */
	length = flength + 0.5;
	if (length > maxlength)
		length = (length / 2);

	/* Calculate RMS gain in dB */
	gain =
	    10.0 * log10(0.01 +
			 (v_mf_magsq(&sigin[-(length / 2)], length) / length));
	if (gain < MINGAIN)
		gain = MINGAIN;

	return (gain);

}

/*
    Name: mf_lin_int_bnd.c
    Description: Linear interpolation within bounds
    Inputs:
      x - input X value
      xmin - minimum X value
      xmax - maximum X value
      ymin - minimum Y value
      ymax - maximum Y value
    Returns: y - interpolated and bounded y value

    Copyright (c) 1995 by Texas Instruments, Inc.  All rights reserved.
*/

float mf_lin_int_bnd(float x, float xmin, float xmax, float ymin, float ymax)
{
	float y;

	if (x <= xmin)
		y = ymin;

	else if (x >= xmax)
		y = ymax;

	else
		y = ymin + (x - xmin) * (ymax - ymin) / (xmax - xmin);

	return (y);
}

/*
    Name: mf_noise_est.c
    Description: Estimate long-term noise floor
    Inputs:
      gain - input gain (in dB)
      mf_noise_gain - current noise gain estimate
      up - maximum up stepsize
      down - maximum down stepsize
      min - minimum allowed gain
      max - maximum allowed gain
    Outputs:
      mf_noise_gain - updated noise gain estimate
    Returns: void

    Copyright (c) 1995 by Texas Instruments, Inc.  All rights reserved.
*/

void mf_noise_est(float gain, float *mf_noise_gain, float up, float down,
		  float min, float max)
{

	/* Update mf_noise_gain */
	if (gain > *mf_noise_gain + up)
		*mf_noise_gain = *mf_noise_gain + up;

	else if (gain < *mf_noise_gain + down)
		*mf_noise_gain = *mf_noise_gain + down;

	else
		*mf_noise_gain = gain;

	/* Constrain total range of mf_noise_gain */
	if (*mf_noise_gain < min)
		*mf_noise_gain = min;
	if (*mf_noise_gain > max)
		*mf_noise_gain = max;

}

/*
    Name: mf_noise_sup.c
    Description: Perform noise suppression on mf_speech gain
    Inputs: (all in dB)
      gain - input gain (in dB)
      mf_noise_gain - current noise gain estimate (in dB)
      max_noise - maximum allowed noise gain 
      max_atten - maximum allowed attenuation
      nfact - noise floor boost
    Outputs:
      gain - updated gain 
    Returns: void

    Copyright (c) 1995 by Texas Instruments, Inc.  All rights reserved.
*/

void mf_noise_sup(float *gain, float mf_noise_gain, float max_noise,
		  float max_atten, float nfact)
{
	float gain_lev, suppress;

	/* Reduce effect for louder background noise */
	if (mf_noise_gain > max_noise)
		mf_noise_gain = max_noise;

	/* Calculate suppression factor */
	gain_lev = *gain - (mf_noise_gain + nfact);
	if (gain_lev > 0.001) {
		suppress = -10.0 * log10(1.0 - pow(10.0, -0.1 * gain_lev));
		if (suppress > max_atten)
			suppress = max_atten;
	} else
		suppress = max_atten;

	/* Apply suppression to input gain */
	*gain -= suppress;

}

/*
    Name: mf_q_bpvc.c, mf_mf_q_bpvc_dec.c
    Description: Quantize/decode bandpass voicing
    Inputs:
      bpvc, bpvc_index
      bpthresh - threshold
      mf_NUM_BANDS - number of bands
    Outputs: 
      bpvc, bpvc_index
    Returns: uv_flag - flag if unvoiced

    Copyright (c) 1995 by Texas Instruments, Inc.  All rights reserved.
*/

/* Compile constants */
#define INVALID_BPVC 0001

int mf_q_bpvc(float *bpvc, int *bpvc_index, float bpthresh, int mf_NUM_BANDS)
{
	int j, uv_flag;

	uv_flag = 1;

	if (bpvc[0] > bpthresh) {

		/* Voiced: pack bandpass voicing */
		uv_flag = 0;
		*bpvc_index = 0;
		bpvc[0] = 1.0;

		for (j = 1; j < mf_NUM_BANDS; j++) {
			*bpvc_index <<= 1;	/* left shift */
			if (bpvc[j] > bpthresh) {
				bpvc[j] = 1.0;
				*bpvc_index |= 1;
			} else {
				bpvc[j] = 0.0;
				*bpvc_index |= 0;
			}
		}

		/* Don't use invalid code (only top band voiced) */
		if (*bpvc_index == INVALID_BPVC) {
			bpvc[(mf_NUM_BANDS - 1)] = 0.0;
			*bpvc_index = 0;
		}
	}

	else {

		/* Unvoiced: force all bands unvoiced */
		*bpvc_index = 0;
		for (j = 0; j < mf_NUM_BANDS; j++)
			bpvc[j] = 0.0;
	}

	return (uv_flag);
}

void mf_mf_q_bpvc_dec(float *bpvc, int *bpvc_index, int uv_flag,
		      int mf_NUM_BANDS)
{
	int j;

	if (uv_flag) {

		/* Unvoiced: set all bpvc to 0 */
		*bpvc_index = 0;
		bpvc[0] = 0.0;
	}

	else {

		/* Voiced: set bpvc[0] to 1.0 */
		bpvc[0] = 1.0;
	}

	if (*bpvc_index == INVALID_BPVC) {

		/* Invalid code received: set higher band voicing to zero */
		*bpvc_index = 0;
	}

	/* Decode remaining bands */
	for (j = mf_NUM_BANDS - 1; j > 0; j--) {
		if ((*bpvc_index & 1) == 1)
			bpvc[j] = 1.0;
		else
			bpvc[j] = 0.0;
		*bpvc_index >>= 1;
	}
}

/*
    Name: mf_q_gain.c, mf_mf_q_gain_dec.c
    Description: Quantize/decode two gain terms using quasi-differential coding
    Inputs:
      gain[2],gain_index[2]
      GN_QLO,GN_QUP,GN_QLEV for second gain term
    Outputs: 
      gain[2],gain_index[2]
    Returns: void

    Copyright (c) 1995 by Texas Instruments, Inc.  All rights reserved.
*/

/* Compile constants */
#define GAIN_INT_DB 5.0

void mf_q_gain(float *gain, int *gain_index, float GN_QLO, float GN_QUP,
	       float GN_QLEV)
{
	static float mf_prev_gain = 0.0;
	float temp, temp2;

	/* Quantize second gain term with uniform quantizer */
	mf_quant_u(&gain[1], &gain_index[1], GN_QLO, GN_QUP, GN_QLEV);

	/* Check for intermediate gain interpolation */
	if (gain[0] < GN_QLO)
		gain[0] = GN_QLO;
	if (gain[0] > GN_QUP)
		gain[0] = GN_QUP;
	if (fabs(gain[1] - mf_prev_gain) < GAIN_INT_DB &&
	    fabs(gain[0] - 0.5 * (gain[1] + mf_prev_gain)) < 3.0) {

		/* interpolate and set special code */
		gain[0] = 0.5 * (gain[1] + mf_prev_gain);
		gain_index[0] = 0;
	} else {

		/* Code intermediate gain with 7-levels */
		if (mf_prev_gain < gain[1]) {
			temp = mf_prev_gain;
			temp2 = gain[1];
		} else {
			temp = gain[1];
			temp2 = mf_prev_gain;
		}
		temp -= 6.0;
		temp2 += 6.0;
		if (temp < GN_QLO)
			temp = GN_QLO;
		if (temp2 > GN_QUP)
			temp2 = GN_QUP;
		mf_quant_u(&gain[0], &gain_index[0], temp, temp2, 7);

		/* Skip all-zero code */
		gain_index[0]++;
	}

	/* Update previous gain for next time */
	mf_prev_gain = gain[1];

}

void mf_mf_q_gain_dec(float *gain, int *gain_index, float GN_QLO, float GN_QUP,
		      float GN_QLEV)
{

	static float mf_prev_gain = 0.0;
	static int mf_mf_prev_gain_err = 0;
	float temp, temp2;

	/* Decode second gain term */
	mf_mf_quant_u_dec(gain_index[1], &gain[1], GN_QLO, GN_QUP, GN_QLEV);

	if (gain_index[0] == 0) {

		/* interpolation bit code for intermediate gain */
		if (fabs(gain[1] - mf_prev_gain) > GAIN_INT_DB) {
			/* Invalid received data (bit error) */
			if (mf_mf_prev_gain_err == 0) {
				/* First time: don't allow gain excursion */
				gain[1] = mf_prev_gain;
			}
			mf_mf_prev_gain_err = 1;
		} else
			mf_mf_prev_gain_err = 0;

		/* Use interpolated gain value */
		gain[0] = 0.5 * (gain[1] + mf_prev_gain);
	}

	else {

		/* Decode 7-bit quantizer for first gain term */
		mf_mf_prev_gain_err = 0;
		gain_index[0]--;
		if (mf_prev_gain < gain[1]) {
			temp = mf_prev_gain;
			temp2 = gain[1];
		} else {
			temp = gain[1];
			temp2 = mf_prev_gain;
		}
		temp -= 6.0;
		temp2 += 6.0;
		if (temp < GN_QLO)
			temp = GN_QLO;
		if (temp2 > GN_QUP)
			temp2 = GN_QUP;
		mf_mf_quant_u_dec(gain_index[0], &gain[0], temp, temp2, 7);
	}

	/* Update previous gain for next time */
	mf_prev_gain = gain[1];

}

/*
    Name: mf_scale_adj.c
    Description: Adjust scaling of output mf_speech signal.
    Inputs:
      mf_speech - mf_speech signal
      gain - desired RMS gain
      mf_premf_v_scale - previous scale factor
      length - number of samples in signal
      SCALEOVER - number of points to interpolate scale factor
    Warning: SCALEOVER is assumed to be less than length.
    Outputs: 
      mf_speech - scaled mf_speech signal
      mf_premf_v_scale - updated previous scale factor
    Returns: void

    Copyright (c) 1995 by Texas Instruments, Inc.  All rights reserved.
*/

void mf_scale_adj(float *mf_speech, float gain, float *mf_premf_v_scale,
		  int length, int SCALEOVER)
{
	int i;
	float scale;

	/* Calculate desired scaling factor to match gain level */
	scale = gain / (sqrt(v_mf_magsq(&mf_speech[0], length) / length) + .01);

	/* interpolate scale factors for first SCALEOVER points */
	for (i = 1; i < SCALEOVER; i++) {
		mf_speech[i - 1] *=
		    ((scale * i + *mf_premf_v_scale * (SCALEOVER - i))
		     * (1.0 / SCALEOVER));
	}

	/* Scale rest of signal */
	mf_v_scale(&mf_speech[SCALEOVER - 1], scale, length - SCALEOVER + 1);

	/* Update previous scale factor for next call */
	*mf_premf_v_scale = scale;

}

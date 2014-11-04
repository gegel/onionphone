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

/* compiler include files */

#include "sc1200.h"
#include "mathhalf.h"
#include "macro.h"
#include "lpc_lib.h"
#include "mat_lib.h"
#include "vq_lib.h"
#include "fs_lib.h"
#include "fft_lib.h"
#include "pit_lib.h"
#include "math_lib.h"
#include "constant.h"
#include "cprv.h"
#include "global.h"
#include "pitch.h"
#include "qnt12_cb.h"
#include "qnt12.h"
#include "msvq_cb.h"
#include "fsvq_cb.h"
#include "melp_sub.h"
#include "dsp_sub.h"
#include "coeff.h"

/* compiler constants */

#define BWFACT_Q15			32571

#define HF_CORR_Q15			4	/* 0.0001220703125 in Q15 */

#define PEAK_THRESH_Q12		5488	/* 1.34 * (1 << 12) */
#define PEAK_THR2_Q12		6553	/* 1.6 * (1 << 12) */
#define SILENCE_DB_Q8		7680	/* 30.0 * (1 << 8) */
#define SIG_LENGTH			(LPF_ORD + PITCH_FR)	/* 327 */

#define X01_Q15				3277	/* 0.1 * (1 << 15) */
#define X03_Q11				614	/* 0.3 * (1 << 11) */
#define X04_Q14				6554	/* 0.4 * (1 << 14) */
#define X045_Q14			7273	/* 0.45 * (1 << 14) */
#define X05_Q11				1024	/* 0.5 * (1 << 11) */
#define X07_Q14				11469	/* 0.7 * (1 << 11) */
#define X08_Q11				1638	/* 0.8 * (1 << 11) */
#define X12_Q11				2458	/* 1.2 * (1 << 11) */
#define X28_Q11				5734	/* 2.8 * (1 << 11) */
#define MX01_Q15			-3277	/* -0.1 * (1 << 15) */
#define MX02_Q15			-6554	/* -0.2 * (1 << 15) */

/* memory definitions */

static int16_t sigbuf[SIG_LENGTH];
static classParam classStat[TRACK_NUM];	/* class of subframes */
static pitTrackParam pitTrack[TRACK_NUM];	/* pitch of subframes */

int16_t top_lpc[LPC_ORD];

/* Prototype */

static void melp_ana(int16_t sp_in[], struct melp_param *par,
		     int16_t subnum);
void sc_ana(struct melp_param *par);
static BOOLEAN subenergyRelation1(classParam classStat[], int16_t curTrack);
static BOOLEAN subenergyRelation2(classParam classStat[], int16_t curTrack);

/****************************************************************************
**
** Function:		analysis
**
** Description: 	The analysis routine for the sc1200 coder
**
** Arguments:
**
**	int16_t	sp_in[] ---- input speech data buffer (Q0)
**	melp_param *par ---- output encoded melp parameters
**
** Return value:	None
**
*****************************************************************************/
void analysis(int16_t sp_in[], struct melp_param *par)
{
	register int16_t i;
	int16_t lpc[LPC_ORD + 1], weights[LPC_ORD];
	int16_t num_frames;

	num_frames = (int16_t) ((rate == RATE2400) ? 1 : NF);

	/* ======== Compute melp parameters ======== */
	for (i = 0; i < num_frames; i++) {

		/* Remove DC from input speech */

		/* Previously the floating point version of dc_rmv() for RATE1200     */
		/* uses the following filter:                                         */
		/*                                                                    */
		/*    H(z) = N(z)/D(z), with                                          */
		/*    N(z) = 0.92692416 - 3.70563834 z^{-1} + 5.55742893 z^{-2} -     */
		/*           3.70563834 z^{-3} + 0.92642416 z^{-4},                   */
		/*    D(z) = 1 - 3.84610723 z^{-1} + 5.5520976 z^{-2} -               */
		/*           3.56516069 z^{-3} + 0.85918839 z^{-4}.                   */
		/* It can be shown that this is equivalent to the fixed point filter  */
		/* (Q13) we use for RATE 2400.                                        */

		if (rate == RATE2400)
			dc_rmv(&sp_in[i * FRAME], &hpspeech[OLD_IN_BEG],
			       dcdelin, dcdelout_hi, dcdelout_lo, FRAME);
		else
			dc_rmv(&sp_in[i * FRAME], &hpspeech[IN_BEG + i * FRAME],
			       dcdelin, dcdelout_hi, dcdelout_lo, FRAME);

		melp_ana(&hpspeech[i * FRAME], &par[i], i);
	}

	/* ---- New routine to refine the parameters for block ---- */

	if (rate == RATE1200)
		sc_ana(par);

	/* ======== Quantization ======== */

	lpc[0] = ONE_Q12;
	if (rate == RATE2400) {
		/* -- Quantize MELP parameters to 2400 bps and generate bitstream --  */

		/* Quantize LSF's with MSVQ */
		v_equ(&(lpc[1]), top_lpc, LPC_ORD);
		vq_lspw(weights, par->lsf, &(lpc[1]), LPC_ORD);

		/*      msvq_enc(par->lsf, weights, par->lsf, vq_par); */
		vq_ms4(msvq_cb, par->lsf, msvq_cb_mean, msvq_levels, MSVQ_M, 4,
		       LPC_ORD, weights, par->lsf, quant_par.msvq_index,
		       MSVQ_MAXCNT);

		/* Force minimum LSF bandwidth (separation) */
		lpc_clamp(par->lsf, BWMIN_Q15, LPC_ORD);

	} else
		lsf_vq(par);

	if (rate == RATE2400) {
		/* Quantize logarithmic pitch period */
		/* Reserve all zero code for completely unvoiced */
		par->pitch = log10_fxp(par->pitch, 7);	/* par->pitch in Q12 */
		quant_u(&par->pitch, &(quant_par.pitch_index), PIT_QLO_Q12,
			PIT_QUP_Q12, PIT_QLEV_M1, PIT_QLEV_M1_Q8, 1, 7);
		/* convert pitch back to linear in Q7 */
		par->pitch = pow10_fxp(par->pitch, 7);
	} else
		pitch_vq(par);

	if (rate == RATE2400) {
		/* Quantize gain terms with uniform log quantizer   */
		q_gain(par->gain, quant_par.gain_index, GN_QLO_Q8, GN_QUP_Q8,
		       GN_QLEV_M1, GN_QLEV_M1_Q10, 0, 5);
	} else
		gain_vq(par);

	if (rate == RATE2400) {	/* Quantize jitter */
		/*      quant_u(&par->jitter, &par->jit_index, 0, MAX_JITTER_Q15, 2);     */
		if (par->jitter < melpe_shr(MAX_JITTER_Q15, 1)) {
			par->jitter = 0;
			quant_par.jit_index[0] = 0;
		} else {
			par->jitter = MAX_JITTER_Q15;
			quant_par.jit_index[0] = 1;
		}
	} else {
		for (i = 0; i < NF; i++)
			quant_u(&par[i].jitter, &(quant_par.jit_index[i]), 0,
				MAX_JITTER_Q15, 2, ONE_Q15, 1, 7);
	}

	/* Quantize bandpass voicing */
	if (rate == RATE2400)
		par[0].uv_flag = q_bpvc(par[0].bpvc, &(quant_par.bpvc_index[0]),
					NUM_BANDS);
	else
		quant_bp(par, num_frames);

	if (rate == RATE1200)
		quant_jitter(par);

	/* Calculate Fourier coeffs of residual from quantized LPC */
	for (i = 0; i < num_frames; i++) {

		/* The following fill() action is believed to be unnecessary. */
		fill(par[i].fs_mag, ONE_Q13, NUM_HARM);
		if (!par[i].uv_flag) {
			lpc_lsp2pred(par[i].lsf, &(lpc[1]), LPC_ORD);
			zerflt(&hpspeech
			       [(i * FRAME + FRAME_END - (LPC_FRAME / 2))], lpc,
			       sigbuf, LPC_ORD, LPC_FRAME);
			window(sigbuf, win_cof, sigbuf, LPC_FRAME);
			find_harm(sigbuf, par[i].fs_mag, par[i].pitch, NUM_HARM,
				  LPC_FRAME);
		}
	}

	if (rate == RATE2400) {
		/* quantize Fourier coefficients */
		/* pre-weight vector, then use Euclidean distance */
		window_Q(par->fs_mag, w_fs, par->fs_mag, NUM_HARM, 14);

		/*      fsvq_enc(par->fs_mag, par->fs_mag, fs_vq_par); */
		/* Later it is found that we do not need the structured variable      */
		/* fs_vq_par at all.  References to its individual fields can be      */
		/* replaced directly with constants or other variables.               */

		vq_enc(fsvq_cb, par->fs_mag, FS_LEVELS, NUM_HARM, par->fs_mag,
		       &(quant_par.fsvq_index));
	} else
		quant_fsmag(par);

	for (i = 0; i < num_frames; i++)
		quant_par.uv_flag[i] = par[i].uv_flag;

	/* Write channel bitstream */
	if (rate == RATE2400)
		melp_chn_write(&quant_par, chbuf);
#if !SKIP_CHANNEL
	else
		low_rate_chn_write(&quant_par);
#endif

	/* Update delay buffers for next block */
	v_equ(hpspeech, &(hpspeech[num_frames * FRAME]), IN_BEG);

}

/* ========================================================================== */
/*  Name: melp_ana.c                                                          */
/*  Description: MELP analysis                                                */
/*  Inputs:                                                                   */
/*    speech[] - input speech signal                                          */
/*    subnum   - subframe index                                               */
/*  Outputs:                                                                  */
/*    *par - MELP parameter structure                                         */
/*  Returns: void                                                             */
/* ========================================================================== */

static void melp_ana(int16_t speech[], struct melp_param *par,
		     int16_t subnum)
{
	register int16_t i;
	static BOOLEAN firstTime = TRUE;
	static int16_t lpfsp_delin[LPF_ORD];
	static int16_t lpfsp_delout[LPF_ORD];
	static int16_t pitch_avg;
	static int16_t fpitch[NUM_PITCHES];
	int16_t cur_track, begin;
	int16_t sub_pitch;
	int16_t temp, dontcare, pcorr;
	int16_t auto_corr[EN_FILTER_ORDER], lpc[LPC_ORD + 1];
	int16_t section;
	int16_t temp_delin[LPF_ORD];
	int16_t temp_delout[LPF_ORD];

	if (firstTime) {
		v_zap(lpfsp_delin, LPF_ORD);	/* Release V2 only use "lpfsp_del". */
		v_zap(lpfsp_delout, LPF_ORD);
		pitch_avg = DEFAULT_PITCH_Q7;
		fill(fpitch, DEFAULT_PITCH_Q7, 2);
		firstTime = FALSE;
	}

	/* Copy input speech to pitch window and lowpass filter */

	/* The following filter block was in use in the fixed point implementa-   */
	/* tion of RATE2400.  As a comparision, the floating point version of     */
	/* RATE1200 uses the following filter:                                    */
	/*                                                                        */
	/*    H(z) = N(z)/D(z), with                                              */
	/*        N(z) = 0.00105165 + 0.00630988 z^{-1} + 0.01577470 z^{-2} +     */
	/*               0.02103294 z^{-3} + 0.01577470 z^{-4} +                  */
	/*               0.00630988 z^{-5} + 0.00105165 z^{-6},                   */
	/*        D(z) = 1 - 2.97852993 z^{-1} + 4.136081 z^{-2} -                */
	/*               3.25976428 z^{-3} + 1.51727884 z^{-4} -                  */
	/*               0.39111723 z^{-5} + 0.04335699 z^{-6}.                   */
	/*                                                                        */
	/* It can be shown that the following fixed point filter used by RATE     */
	/* 2400 is the same as the one mentione above (except for some numerical  */
	/* truncation errors) under Q13.  Therefore the floating point version of */
	/* the filter is completely replaced with the RATE2400 filter.            */

	v_equ(&sigbuf[LPF_ORD], &speech[PITCH_BEG], PITCH_FR);
	for (section = 0; section < LPF_ORD / 2; section++) {
		for (i = (int16_t) (section * 2);
		     i < (int16_t) (section * 2 + 2); i++) {
			temp_delin[i] =
			    sigbuf[LPF_ORD + FRAME - 1 - i + section * 2];
		}
		iir_2nd_s(&sigbuf[LPF_ORD], &lpf_den[section * 3],
			  &lpf_num[section * 3], &sigbuf[LPF_ORD],
			  &lpfsp_delin[section * 2], &lpfsp_delout[section * 2],
			  FRAME);
		v_equ(&(temp_delout[2 * section]), &(lpfsp_delout[2 * section]),
		      2);
		iir_2nd_s(&sigbuf[LPF_ORD + FRAME], &lpf_den[section * 3],
			  &lpf_num[section * 3], &sigbuf[LPF_ORD + FRAME],
			  &lpfsp_delin[section * 2], &lpfsp_delout[section * 2],
			  PITCH_FR - FRAME);
		/* restore delay buffers for the next overlapping frame */
		v_equ(&(lpfsp_delin[2 * section]), &(temp_delin[2 * section]),
		      2);
		v_equ(&(lpfsp_delout[2 * section]), &(temp_delout[2 * section]),
		      2);
	}

	f_pitch_scale(&sigbuf[LPF_ORD], &sigbuf[LPF_ORD], PITCH_FR);

	/* Perform global pitch search at frame end on lowpass speech signal */
	/* Note: avoid short pitches due to formant tracking */
	fpitch[1] = find_pitch(&sigbuf[LPF_ORD + (PITCH_FR / 2)], &dontcare,
			       (2 * PITCHMIN), PITCHMAX, PITCHMAX);
	fpitch[1] = melpe_shl(fpitch[1], 7);	/* fpitch in Q7 */

	/* Perform bandpass voicing analysis for end of frame */
	bpvc_ana(&speech[FRAME_END], fpitch, par->bpvc, &sub_pitch);

	/* Force jitter if lowest band voicing strength is weak */
	if (par->bpvc[0] < VJIT_Q14)	/* par->bpvc in Q14 */
		par->jitter = MAX_JITTER_Q15;	/* par->jitter in Q15 */
	else
		par->jitter = 0;

	/* Calculate LPC for end of frame.  Note that we compute (LPC_ORD + 1)    */
	/* entries of auto_corr[] for RATE2400 but EN_FILTER_ORDER entries for    */
	/* RATE1200 because bandEn() for classify() of "classify.c" needs so many */
	/* entries for computation.  lpc_schur() needs (LPC_ORD + 1) entries.     */

	if (rate == RATE2400)
		lpc_autocorr(&speech[(FRAME_END - (LPC_FRAME / 2))], win_cof,
			     auto_corr, HF_CORR_Q15, LPC_ORD, LPC_FRAME);
	else
		lpc_autocorr(&speech[(FRAME_END - (LPC_FRAME / 2))], win_cof,
			     auto_corr, HF_CORR_Q15, EN_FILTER_ORDER - 1,
			     LPC_FRAME);

	lpc[0] = ONE_Q12;
	lpc_schur(auto_corr, &(lpc[1]), LPC_ORD);	/* lpc in Q12 */

	if (rate == RATE2400) {	/* Calculate Line Spectral Frequencies */
		lpc_pred2lsp(&(lpc[1]), par->lsf, LPC_ORD);
		v_equ(top_lpc, &(lpc[1]), LPC_ORD);
	} else {
		lpc_bw_expand(&(lpc[1]), &(lpc[1]), BWFACT_Q15, LPC_ORD);

		/* Calculate Line Spectral Frequencies */
		lpc_pred2lsp(&(lpc[1]), par->lsf, LPC_ORD);

		/* Force minimum LSF bandwidth (separation) */
		lpc_clamp(par->lsf, BWMIN_Q15, LPC_ORD);
	}

	/* Calculate LPC residual */
	zerflt(&speech[PITCH_BEG], lpc, &sigbuf[LPF_ORD], LPC_ORD, PITCH_FR);

	/* Check peakiness of residual signal */
	begin = LPF_ORD + (PITCHMAX / 2);
	temp = peakiness(&sigbuf[begin], PITCHMAX);	/* temp in Q12 */

	/* Peakiness: force lowest band to be voiced  */
	if (temp > PEAK_THRESH_Q12)
		par->bpvc[0] = ONE_Q14;

	/* Extreme peakiness: force second and third bands to be voiced */
	if (temp > PEAK_THR2_Q12) {
		par->bpvc[1] = ONE_Q14;
		par->bpvc[2] = ONE_Q14;
	}

	if (rate == RATE1200) {
		/* ======== Compute pre-classification and pitch parameters ========  */
		cur_track = (int16_t) (CUR_TRACK + subnum * PIT_SUBNUM);
		for (i = 0; i < PIT_SUBNUM; i++) {
			pitchAuto(&speech
				  [FRAME_END + i * PIT_SUBFRAME +
				   PIT_COR_LEN / 2],
				  &pitTrack[cur_track + i + 1],
				  &classStat[cur_track + i + 1]);

			classify(&speech
				 [FRAME_END + i * PIT_SUBFRAME +
				  PIT_SUBFRAME / 2],
				 &classStat[cur_track + i + 1], auto_corr);
		}
	}

	/* Calculate overall frame pitch using lowpass filtered residual */
	par->pitch = pitch_ana(&speech[FRAME_END], &sigbuf[LPF_ORD + PITCHMAX],
			       sub_pitch, pitch_avg, &pcorr);

	/* Calculate gain of input speech for each gain subframe */
	for (i = 0; i < NUM_GAINFR; i++) {
		if (par->bpvc[0] > BPTHRESH_Q14) {
			/* voiced mode: pitch synchronous window length */
			temp = sub_pitch;
			par->gain[i] =
			    gain_ana(&speech[FRAME_BEG + (i + 1) * GAINFR],
				     temp, MIN_GAINFR, PITCHMAX_X2);
		} else {
			if (rate == RATE2400)
				temp = (int16_t) GAIN_PITCH_Q7;
			else
				temp = (int16_t) 15258;
			/* (1.33*GAINFR - 0.5) << 7 */
			par->gain[i] =
			    gain_ana(&speech[FRAME_BEG + (i + 1) * GAINFR],
				     temp, 0, PITCHMAX_X2);
		}
	}

	/* Update average pitch value */
	if (par->gain[NUM_GAINFR - 1] > SILENCE_DB_Q8)	/* par->gain in Q8 */
		temp = pcorr;
	else
		temp = 0;
	/* pcorr in Q14 */
	pitch_avg = p_avg_update(par->pitch, temp, VMIN_Q14);

	if (rate == RATE1200) {
		if (par->bpvc[0] > BPTHRESH_Q14)
			par->uv_flag = FALSE;
		else
			par->uv_flag = TRUE;
	}

	/* Update delay buffers for next frame */
	fpitch[0] = fpitch[1];
}

/* ======================================= */
/* melp_ana_init(): perform initialization */
/* ======================================= */

void melp_ana_init()
{
	register int16_t i;

	v_zap(hpspeech, IN_BEG + BLOCK);	/* speech[] is declared IN_BEG + BLOCK */

	/* Initialize fixed MSE weighting and inverse of weighting */

	if (!w_fs_init) {
		vq_fsw(w_fs, NUM_HARM, X60_Q9);
		for (i = 0; i < NUM_HARM; i++)
			w_fs_inv[i] = melpe_divide_s(ONE_Q13, w_fs[i]);	/* w_fs_inv in Q14 */
		w_fs_init = TRUE;
	}

	/* Initialize wr_array and wi_array */
	fs_init();

	/* Initialization of the pitch, classification and voicing paramters.  It */
	/* is believed that the following for loop is not necessary.              */

	for (i = 0; i < TRACK_NUM; i++) {
		fill(pitTrack[i].pit, FIFTY_Q0, NODE);	/* !!! (12/10/99) */
		fill(pitTrack[i].weight, ONE_Q15, NODE);

		classStat[i].classy = UNVOICED;
		classStat[i].subEnergy = X28_Q11;
		classStat[i].zeroCrosRate = X05_Q15;
		classStat[i].peakiness = ONE_Q11;
		classStat[i].corx = X02_Q15;
	}
}

/****************************************************************************
**
** Function:		sc_ana
**
** Description: 	The analysis routine for block (1200bps only)
**
** Arguments:
**
**	melp_param	*par	---- output encoded melp parameters
**
** Return value:	None
**
*****************************************************************************/

void sc_ana(struct melp_param *par)
{
	register int16_t i;
	static int16_t prev_sbp3;
	static BOOLEAN prev_uv = UNVOICED;
	static int16_t prev_pitch = FIFTY_Q7;	/* Note that prev_pitch is Q7 */
	/* but it is always an integer in Q7 as the */
	/* former floating point version specifies. */
	int16_t pitCand, npitch;	/* Q7 */
	int16_t bpvc_copy[NUM_BANDS];
	int16_t sbp[NF + 1];
	BOOLEAN uv[NF + 1];
	int16_t curTrack;
	int16_t index, index1, index2;
	int16_t temp1, temp2;
	int16_t w1_w2;	/* Q15 */

	/* ======== Update silence energy ======== */
	for (i = 0; i < NF; i++) {

		curTrack = (int16_t) (i * PIT_SUBNUM + CUR_TRACK);

		/* ---- update silence energy ---- */
		if ((classStat[curTrack].classy == SILENCE) &&
		    (classStat[curTrack - 1].classy == SILENCE)) {
			/*      silenceEn = log10(EN_UP_RATE * pow(10.0, silenceEn) +
			   (1.0 - EN_UP_RATE) * 
			   pow(10.0, classStat[curTrack].subEnergy)); */
			silenceEn = updateEn(silenceEn, EN_UP_RATE_Q15,
					     classStat[curTrack].subEnergy);
		}
	}

	uv[0] = prev_uv;
	uv[1] = par[0].uv_flag;
	uv[2] = par[1].uv_flag;
	uv[3] = par[2].uv_flag;

	curTrack = CUR_TRACK;
	if (!uv[1])
		voicedCnt++;
	else
		voicedCnt = 0;

	if (!uv[1] && !uv[2] && !uv[3]
	    && !subenergyRelation1(classStat, curTrack)) {
		/* ---- process only voiced frames ---- */
		/* check if it is offset first */
		if ((voicedCnt < 2) || subenergyRelation2(classStat, curTrack)) {

			/* Got onset position, should look forward for pitch */
			pitCand = pitLookahead(&pitTrack[curTrack], 3);
			if (ratio(par[0].pitch, pitCand) > X015_Q15) {
				if (ratio(pitCand, par[1].pitch) < X015_Q15)
					par[0].pitch = pitCand;
				else if ((ratio(par[1].pitch, par[2].pitch) <
					  X015_Q15)
					 && (ratio(par[0].pitch, par[1].pitch) >
					     X015_Q15))
					par[0].pitch = par[1].pitch;
				else if (ratio(par[0].pitch, par[1].pitch) >
					 X015_Q15)
					par[0].pitch = pitCand;
			}
		} else if (!uv[0]) {
			/* not onset not offset, just check pitch smooth */
			temp1 = melpe_sub(par[0].pitch, prev_pitch);
			index1 = melpe_shr(temp1, 7);	/* Q0 */
			temp2 = melpe_sub(par[1].pitch, par[0].pitch);
			index2 = melpe_shr(temp2, 7);	/* Q0 */
			if ((melpe_abs_s(index1) > 5) && (melpe_abs_s(index2) > 5) &&
			    (index1 * index2 < 0)) {
				/* here is a pitch jump */
				pitCand = pitLookahead(&pitTrack[curTrack], 3);
				if ((ratio(prev_pitch, pitCand) < X015_Q15) ||
				    (ratio(pitCand, par[1].pitch) < X02_Q15))
					par[0].pitch = pitCand;
				else {
					/*      par[0].pitch = (prev_pitch + par[1].pitch)/2; */
					temp1 = melpe_shr(prev_pitch, 1);
					temp2 = melpe_shr(par[1].pitch, 1);
					par[0].pitch = melpe_add(temp1, temp2);	/* Q7 */
				}
			} else if ((ratio(par[0].pitch, prev_pitch) > X015_Q15)
				   &&
				   ((ratio(par[1].pitch, prev_pitch) < X015_Q15)
				    || (ratio(par[2].pitch, prev_pitch) <
					X015_Q15))) {
				index1 =
				    trackPitch(prev_pitch, &pitTrack[curTrack]);
				pitCand = melpe_shl(pitTrack[curTrack].pit[index1], 7);	/* !!! (12/10/99) */
				index2 =
				    trackPitch(par[0].pitch,
					       &pitTrack[curTrack]);
				w1_w2 =
				    melpe_sub(pitTrack[curTrack].weight[index1],
					pitTrack[curTrack].weight[index2]);

				if (multiCheck(par[0].pitch, pitCand) <
				    X008_Q15) {
					if (((par[0].pitch > pitCand)
					     && (w1_w2 > MX02_Q15))
					    || (w1_w2 > X02_Q15))
						par[0].pitch = pitCand;
				} else if (w1_w2 > MX01_Q15) {
					par[0].pitch = pitCand;
				}
			} else
			    if ((L_ratio
				 (par[0].pitch,
				  (int32_t) (prev_pitch * 2)) < X008_Q15)
				||
				(L_ratio
				 (par[0].pitch,
				  (int32_t) (prev_pitch * 3)) < X008_Q15)) {
				/* possible it is a double pitch */
				pitCand = pitLookahead(&pitTrack[curTrack], 4);
				if (ratio(pitCand, prev_pitch) < X01_Q15)
					par[0].pitch = pitCand;
			}
		}
	}

	/* ======== The second frame ======== */
	prev_pitch = melpe_shl(melpe_shr(par[0].pitch, 7), 7);	/* r_ounding to Q7 integer. */
	curTrack = CUR_TRACK + 2;
	if (!uv[2])
		voicedCnt++;
	else
		voicedCnt = 0;

	if (!uv[2] && !subenergyRelation1(classStat, curTrack)) {
		if ((voicedCnt < 2) || subenergyRelation2(classStat, curTrack)) {

			/* Got onset position, should look forward for pitch */
			pitCand = pitLookahead(&pitTrack[curTrack], 3);
			if (ratio(par[1].pitch, pitCand) > X015_Q15) {
				if (ratio(pitCand, par[2].pitch) < X015_Q15)
					par[1].pitch = pitCand;
				else {
					npitch =
					    pitLookahead(&pitTrack
							 [curTrack + 1], 3);
					if (ratio(npitch, par[2].pitch) <
					    X015_Q15) {
						if (ratio(pitCand, npitch) <
						    X015_Q15)
							par[1].pitch = pitCand;
						else if (ratio
							 (par[1].pitch,
							  npitch) > X015_Q15)
							par[1].pitch = npitch;
					} else if (ratio(pitCand, npitch) <
						   X015_Q15)
						par[1].pitch = pitCand;
				}
			}
		} else if (!uv[1]) {
			/* not onset not offset, just check pitch smooth */
			temp1 = melpe_sub(par[1].pitch, prev_pitch);
			index1 = melpe_shr(temp1, 7);
			temp2 = melpe_sub(par[2].pitch, par[1].pitch);
			index2 = melpe_shr(temp2, 7);
			pitCand = pitLookahead(&pitTrack[curTrack], 3);
			if ((melpe_abs_s(index1) > 5) && (melpe_abs_s(index2) > 5) &&
			    (index1 * index2 < 0)) {
				/* here is a pitch jump */
				if ((ratio(prev_pitch, pitCand) < X015_Q15) ||
				    (ratio(pitCand, par[2].pitch) < X02_Q15))
					par[1].pitch = pitCand;
				else {
					/*      par[1].pitch = (prev_pitch + par[2].pitch)/2; */
					temp1 = melpe_shr(prev_pitch, 1);
					temp2 = melpe_shr(par[2].pitch, 1);
					par[1].pitch = melpe_add(temp1, temp2);	/* Q7 */
				}
			} else if ((ratio(par[1].pitch, prev_pitch) > X015_Q15)
				   &&
				   ((ratio(par[2].pitch, prev_pitch) < X015_Q15)
				    || (ratio(pitCand, prev_pitch) <
					X015_Q15))) {
				if (ratio(pitCand, prev_pitch) < X015_Q15)
					par[1].pitch = pitCand;
				else {
					index1 =
					    trackPitch(prev_pitch,
						       &pitTrack[curTrack]);
					pitCand = melpe_shl(pitTrack[curTrack].pit[index1], 7);	/* !!! (12/10/99) */
					index2 =
					    trackPitch(par[1].pitch,
						       &pitTrack[curTrack]);
					w1_w2 =
					    melpe_sub(pitTrack[curTrack].
						weight[index1],
						pitTrack[curTrack].
						weight[index2]);

					if (multiCheck(par[1].pitch, pitCand) <
					    X008_Q15) {
						if (((par[1].pitch > pitCand)
						     && (w1_w2 > MX02_Q15))
						    || (w1_w2 > X02_Q15))
							par[1].pitch = pitCand;
					} else if (w1_w2 > MX01_Q15)
						par[1].pitch = pitCand;
				}
			} else
			    if ((L_ratio
				 (par[1].pitch,
				  (int32_t) (prev_pitch * 2)) < X008_Q15)
				||
				(L_ratio
				 (par[1].pitch,
				  (int32_t) (prev_pitch * 3)) < X008_Q15)) {
				/* possible it is a double pitch */
				pitCand = pitLookahead(&pitTrack[curTrack], 4);
				if (ratio(pitCand, prev_pitch) < X01_Q15)
					par[1].pitch = pitCand;
			}
		}
	}

	/* ======== The third frame ======== */
	prev_pitch = melpe_shl(melpe_shr(par[1].pitch, 7), 7);
	curTrack = CUR_TRACK + 4;
	if (!uv[3])
		voicedCnt++;
	else
		voicedCnt = 0;

	if (!uv[3] && (classStat[curTrack + 1].classy == VOICED) &&
	    (classStat[curTrack + 2].classy == VOICED) &&
	    !subenergyRelation1(classStat, curTrack)) {
		if ((voicedCnt < 2) || subenergyRelation2(classStat, curTrack)) {

			/* Got onset position, should look forward for pitch */
			pitCand = pitLookahead(&pitTrack[curTrack], 2);
			if (ratio(par[2].pitch, pitCand) > X015_Q15) {
				npitch =
				    pitLookahead(&pitTrack[curTrack + 1], 1);
				if (ratio(npitch, pitCand) < X015_Q15)
					par[2].pitch = pitCand;
				else if (ratio(par[2].pitch, npitch) >=
					 X015_Q15)
					par[2].pitch = npitch;
			}
		} else if (!uv[2]) {
			/* not onset not offset, just check pitch smooth */
			pitCand = pitLookahead(&pitTrack[curTrack], 2);
			temp1 = melpe_sub(par[2].pitch, prev_pitch);
			index1 = melpe_shr(temp1, 7);
			temp2 = melpe_sub(pitCand, par[2].pitch);
			index2 = melpe_shr(temp2, 7);
			if ((melpe_abs_s(index1) > 5) && (melpe_abs_s(index2) > 5) &&
			    (index1 * index2 < 0)) {
				/* here is a pitch jump */
				if (ratio(prev_pitch, pitCand) < X015_Q15)
					par[2].pitch = pitCand;
				else {
					index1 =
					    trackPitch(pitCand,
						       &pitTrack[curTrack]);
					index2 =
					    trackPitch(par[2].pitch,
						       &pitTrack[curTrack]);
					w1_w2 =
					    melpe_sub(pitTrack[curTrack].
						weight[index1],
						pitTrack[curTrack].
						weight[index2]);

					if (multiCheck(par[2].pitch, pitCand) <
					    X008_Q15) {
						if (((par[2].pitch > pitCand)
						     && (w1_w2 > MX02_Q15))
						    || (w1_w2 > X02_Q15))
							par[2].pitch = pitCand;
					} else {
						index1 =
						    trackPitch(prev_pitch,
							       &pitTrack
							       [curTrack]);
						pitCand = melpe_shl(pitTrack[curTrack].pit[index1], 7);	/* !!! (12/10/99) */

						/* Note that w1 = pitTrack[curTrack].weight[index1]   */
						/* has ben modified from the value outside of the if  */
						/* condition.                                         */

						w1_w2 =
						    melpe_sub(pitTrack[curTrack].
							weight[index1],
							pitTrack[curTrack].
							weight[index2]);
						if (multiCheck
						    (par[2].pitch,
						     pitCand) < X008_Q15) {
							if (((par[2].pitch >
							      pitCand)
							     && (w1_w2 >
								 MX02_Q15))
							    || (w1_w2 >
								X02_Q15))
								par[2].pitch =
								    pitCand;
						} else {
							/*      par[2].pitch = (prev_pitch + pitCand)/2; */
							temp1 =
							    melpe_shr(prev_pitch, 1);
							temp2 = melpe_shr(pitCand, 1);
							par[2].pitch = melpe_add(temp1, temp2);	/* Q7 */
						}
					}
				}
			}
		}
	}

	/* ======== Try smooth voicing information ======== */

	sbp[0] = prev_sbp3;
	for (i = 0; i < NF; i++) {

		/* Make a copy of par[i].bpvc because the function q_bpvc() will      */
		/* change them.  Hence we use the copy for q_bpvc().                  */

		v_equ(bpvc_copy, par[i].bpvc, NUM_BANDS);

		/* We call q_bpvc() to determine index from the quantization.         */
		/* However, whether par[i].bpvc[0] > BPTHRESH_Q14 or not cannot be    */
		/* judged based on the returned index.  Therefore we use the returned */
		/* value of the function call q_bpvc().                               */

		if (q_bpvc(bpvc_copy, &index, NUM_BANDS))
			sbp[i + 1] = -1;
		else
			sbp[i + 1] = inv_bp_index_map[bp_index_map[index]];
	}

	/* At this point sbp[] is either 0, 8, 12, 15 or -1.  Note that each bit  */
	/* of sbp[i] corresponds to the entries of par[i].bpvc[].  The inequality */
	/* sbp[] > 12 implies it has to be 15, or par[i].bpvc[] has to satisfy a  */
	/* certain relationship about whether they are greater or small than      */
	/* BPTHRESH_Q14.  Similarly, when we apply a Max() or Min() operation on  */
	/* sbp[], more or less we are saying "set par[i].bpvc[] to ONE_Q14" or    */
	/* "reset par[i].bpvc[] to 0".                                            */

	for (i = 1; i < NF; i++) {
		curTrack = (int16_t) (CUR_TRACK + (i - 1) * 2);
		if ((sbp[i - 1] > 12) && (sbp[i + 1] > 12)) {
			if ((classStat[curTrack].subEnergy > voicedEn - X05_Q11)
			    || ((par[i - 1].bpvc[2] > X05_Q14)
				&& (par[i - 1].bpvc[3] > X05_Q14))) {
				if (sbp[i] < 12)
					sbp[i] = 12;
			} else {
				if (sbp[i] < 8)
					sbp[i] = 8;
			}
		} else if ((sbp[i - 1] > 8) && (sbp[i + 1] > 8)) {
			if ((classStat[curTrack].subEnergy > voicedEn - ONE_Q11)
			    || ((par[i - 1].bpvc[2] > X04_Q14)
				&& (par[i - 1].bpvc[3] > X04_Q14))) {
				if (sbp[i] < 8)
					sbp[i] = 8;
			}
		} else if ((sbp[i - 1] < 8) && (sbp[i + 1] < 8)) {
			if ((classStat[curTrack].subEnergy < voicedEn - X05_Q11)
			    && (par[i - 1].bpvc[3] < X07_Q14)) {
				if (sbp[i] > 12)
					sbp[i] = 12;
			}
		}
	}

	curTrack = CUR_TRACK + 4;
	if ((classStat[curTrack].subEnergy > voicedEn - X03_Q11) &&
	    (sbp[2] > 12) && (par[1].bpvc[2] > X05_Q14) &&
	    (par[1].bpvc[3] > X05_Q14)) {
		if (sbp[3] < 12)
			sbp[3] = 12;
	} else if ((classStat[curTrack].subEnergy > voicedEn - X05_Q11) &&
		   (sbp[2] > 8) && (par[1].bpvc[2] > X045_Q14) &&
		   (par[1].bpvc[3] > X045_Q14)) {
		if (sbp[3] < 8)
			sbp[3] = 8;
	}

	/* ======== Depack and save back bpvc information ======== */
	for (i = 0; i < NF; i++) {
		/* sbp[i + 1] and it can only be 0, 8, 12, 15 or -1, and it is never  */
		/* INVALID_BPVC (== 1).  On the other hand, we can call q_bpvc_dec()  */
		/* with its uv_flag input argument passed in as FALSE, so par[i].bpvc */
		/* will be completely determined by index.  Note that passing uv_flag */
		/* as FALSE will alter par[i].bpvc[0], so we save it and restore it.  */

		temp1 = par[i].bpvc[0];
		q_bpvc_dec(par[i].bpvc, sbp[i + 1], FALSE, NUM_BANDS);
		par[i].bpvc[0] = temp1;
	}

	prev_sbp3 = sbp[3];

	/* ======== Update voiced and unvoiced counters ======== */
	for (i = 0; i < NF; i++) {
		curTrack = (int16_t) (i * PIT_SUBNUM + CUR_TRACK);

		/* ---- update voiced energy ---- */
		if (voicedCnt > 2) {
			/*      voicedEn = log10(EN_UP_RATE * pow(10.0, voicedEn) +
			   (1.0 - EN_UP_RATE) * 
			   pow(10.0, classStat[curTrack].subEnergy)); */
			voicedEn = updateEn(voicedEn, EN_UP_RATE_Q15,
					    classStat[curTrack].subEnergy);
		}
		if (voicedEn < classStat[curTrack].subEnergy)
			voicedEn = classStat[curTrack].subEnergy;
	}

	/* ======== Update class and pitch structures ======== */
	for (i = 0; i < TRACK_NUM - NF * PIT_SUBNUM; i++) {
		classStat[i] = classStat[i + NF * PIT_SUBNUM];
		pitTrack[i] = pitTrack[i + NF * PIT_SUBNUM];
	}

	prev_uv = par[NF - 1].uv_flag;

	/* Set prev_pitch to par[NF - 1].pitch so that it is an integer for its   */
	/* Q value.  We first use shr() to truncate par[NF - 1].pitch and then    */
	/* use shl() to correct the Q value.                                      */

	prev_pitch = melpe_shl(melpe_shr(par[NF - 1].pitch, 7), 7);
}

static BOOLEAN subenergyRelation1(classParam classStat[], int16_t curTrack)
{
	BOOLEAN result;
	int16_t prevg, lastg, orig, nextg, futureg;	/* Q11 */

	prevg = classStat[curTrack - 2].subEnergy;
	lastg = classStat[curTrack - 1].subEnergy;
	orig = classStat[curTrack].subEnergy;
	nextg = classStat[curTrack + 1].subEnergy;
	futureg = classStat[curTrack + 2].subEnergy;

	result = (BOOLEAN)
	    (((lastg - prevg < X05_Q11) && (orig - lastg < X05_Q11) &&
	      (nextg - orig < X05_Q11) &&
	      ((prevg - lastg > X12_Q11) || (lastg - orig > X12_Q11) ||
	       (orig - nextg > X12_Q11))) ||
	     ((orig - lastg < X03_Q11) && (nextg - orig < X03_Q11) &&
	      (((lastg - prevg < X03_Q11) &&
		((prevg - orig > X08_Q11) || (lastg - nextg > X08_Q11))) ||
	       ((futureg - nextg < X03_Q11) &&
		((lastg - nextg > X08_Q11) || (orig - futureg > X08_Q11))))));

	return (result);
}

static BOOLEAN subenergyRelation2(classParam classStat[], int16_t curTrack)
{
	BOOLEAN result;
	int16_t prevg, lastg, orig, nextg, futureg;	/* Q11 */

	prevg = classStat[curTrack - 2].subEnergy;
	lastg = classStat[curTrack - 1].subEnergy;
	orig = classStat[curTrack].subEnergy;
	nextg = classStat[curTrack + 1].subEnergy;
	futureg = classStat[curTrack + 2].subEnergy;

	result = (BOOLEAN)
	    (((lastg - orig < X03_Q11) && (orig - nextg < X03_Q11) &&
	      (((prevg - lastg < X03_Q11) &&
		((orig - prevg > X08_Q11) || (nextg - lastg > X08_Q11))) ||
	       ((nextg - futureg < X03_Q11) &&
		((nextg - lastg > X08_Q11) || (futureg - orig > X08_Q11))))) ||
	     ((prevg - lastg < X05_Q11) && (lastg - orig < X05_Q11) &&
	      (orig - nextg < X05_Q11) &&
	      ((lastg - prevg > X12_Q11) || (orig - lastg > X12_Q11) ||
	       (nextg - orig > X12_Q11))));

	return (result);
}

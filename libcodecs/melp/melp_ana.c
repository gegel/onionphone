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
    Name: mf_melp_ana.c
    Description: MELP analysis
    Inputs:
      mf_speech[] - input mf_speech signal
    Outputs: 
      *par - MELP parameter structure
    Returns: void
*/

/* compiler include files */

#include <stdio.h>
#include <math.h>
#include "melp.h"
#include "spbstd.h"
#include "lpc.h"
#include "mat.h"
#include "vq.h"
#include "fs.h"
#include "pit.h"

/* compiler constants */

#define BEGIN 0
#define END 1
#define BWFACT 0.994
#define mf_PDECAY 0.95
#define PEAK_THRESH 1.34
#define PEAK_THR2 1.6
#define SILENCE_DB 30.0
#define MAX_ORD mf_LPF_ORD
#define mf_FRAME_BEG (mf_PITCHMAX-(mf_FRAME/2))
#define mf_FRAME_END (mf_FRAME_BEG+mf_FRAME)
#define PITCH_BEG (mf_FRAME_END-mf_PITCHMAX)
#define mf_PITCH_FR ((2*mf_PITCHMAX)+1)
#define IN_BEG (PITCH_BEG+mf_PITCH_FR-mf_FRAME)
#define SIG_LENGTH (mf_LPF_ORD+mf_PITCH_FR)

/* external memory references */
 
extern float mf_win_cof[LPC_mf_FRAME];
extern float mf_lpf_num[mf_LPF_ORD+1];
extern float mf_lpf_den[mf_LPF_ORD+1];
extern float mf_msvq_cb[];
extern float mf_fsvq_cb[];
extern int mf_fsvq_weighted;

/* memory definitions */
 
static float mf_sigbuf[SIG_LENGTH];
static float mf_speech[IN_BEG+mf_FRAME];
static float mf_dcdel[DC_ORD];
static float lpfsp_del[mf_LPF_ORD];
static float mf_pitch_avg;
static float mf_fpitch[2];
static struct msmf_vq_param mf_vq_par;  /* MSVQ parameters */
static struct msmf_vq_param mf_fs_mf_vq_par;  /* Fourier series VQ parameters */
static float mf_w_fs[NUM_HARM];

	
void mf_melp_ana(float sp_in[],struct mf_melp_param *par)
{

    int i;
    int begin;
    float sub_pitch;
    float temp,pcorr,bpthresh;
    float r[LPC_ORD+1],refc[LPC_ORD+1],lpc[LPC_ORD+1];
    float weights[LPC_ORD];
        
    /* Remove DC from input mf_speech */
    mf_dc_rmv(sp_in,&mf_speech[IN_BEG],mf_dcdel,mf_FRAME);
    
    /* Copy input mf_speech to pitch mf_window and lowpass filter */
    mf_v_equ(&mf_sigbuf[mf_LPF_ORD],&mf_speech[PITCH_BEG],mf_PITCH_FR);
    mf_v_equ(mf_sigbuf,lpfsp_del,mf_LPF_ORD);
    mf_polflt(&mf_sigbuf[mf_LPF_ORD],mf_lpf_den,&mf_sigbuf[mf_LPF_ORD],mf_LPF_ORD,mf_PITCH_FR);
    mf_v_equ(lpfsp_del,&mf_sigbuf[mf_FRAME],mf_LPF_ORD);
    mf_zerflt(&mf_sigbuf[mf_LPF_ORD],mf_lpf_num,&mf_sigbuf[mf_LPF_ORD],mf_LPF_ORD,mf_PITCH_FR);
    
    /* Perform global pitch search at frame end on lowpass mf_speech signal */
    /* Note: avoid short pitches due to formant tracking */
    mf_fpitch[END] = mf_find_pitch(&mf_sigbuf[mf_LPF_ORD+(mf_PITCH_FR/2)],&temp,
			     (2*mf_PITCHMIN),mf_PITCHMAX,mf_PITCHMAX);
    
    /* Perform bandpass voicing analysis for end of frame */
    mf_bpvc_ana(&mf_speech[mf_FRAME_END], mf_fpitch, &par->bpvc[0], &sub_pitch);
    
    /* Force jitter if lowest band voicing strength is weak */    
    if (par->bpvc[0] < VJIT)
	par->jitter = MAX_JITTER;
    else
	par->jitter = 0.0;
    
    /* Calculate LPC for end of frame */
    mf_window(&mf_speech[(mf_FRAME_END-(LPC_mf_FRAME/2))],mf_win_cof,mf_sigbuf,LPC_mf_FRAME);
    mf_autocorr(mf_sigbuf,r,LPC_ORD,LPC_mf_FRAME);
    lpc[0] = 1.0;
    lpc_schur(r,lpc,refc,LPC_ORD);
    lpc_bw_expand(lpc,lpc,BWFACT,LPC_ORD);
    
    /* Calculate LPC residual */
    mf_zerflt(&mf_speech[PITCH_BEG],lpc,&mf_sigbuf[mf_LPF_ORD],LPC_ORD,mf_PITCH_FR);
        
    /* Check mf_peakiness of residual signal */
    begin = (mf_LPF_ORD+(mf_PITCHMAX/2));
    temp = mf_peakiness(&mf_sigbuf[begin],mf_PITCHMAX);
    
    /* Peakiness: force lowest band to be voiced  */
    if (temp > PEAK_THRESH) {
	par->bpvc[0] = 1.0;
    }
    
    /* Extreme mf_peakiness: force second and third bands to be voiced */
    if (temp > PEAK_THR2) {
	par->bpvc[1] = 1.0;
	par->bpvc[2] = 1.0;
    }
		
    /* Calculate overall frame pitch using lowpass filtered residual */
    par->pitch = mf_pitch_ana(&mf_speech[mf_FRAME_END], &mf_sigbuf[mf_LPF_ORD+mf_PITCHMAX], 
			   sub_pitch,mf_pitch_avg,&pcorr);
    bpthresh = BPTHRESH;
    
    /* Calculate gain of input mf_speech for each gain subframe */
    for (i = 0; i < NUM_GAINFR; i++) {
	if (par->bpvc[0] > bpthresh) {

	    /* voiced mode: pitch synchronous mf_window length */
	    temp = sub_pitch;
	    par->gain[i] = mf_gain_ana(&mf_speech[mf_FRAME_BEG+(i+1)*GAINFR],
				    temp,MIN_GAINFR,2*mf_PITCHMAX);
	}
	else {
	    temp = 1.33*GAINFR - 0.5;
	    par->gain[i] = mf_gain_ana(&mf_speech[mf_FRAME_BEG+(i+1)*GAINFR],
				    temp,0,2*mf_PITCHMAX);
	}
    }
    
    /* Update average pitch value */
    if (par->gain[NUM_GAINFR-1] > SILENCE_DB)
      temp = pcorr;
    else
      temp = 0.0;
    mf_pitch_avg = mf_p_avg_update(par->pitch, temp, VMIN);
    
    /* Calculate Line Spectral Frequencies */
    mf_lpc_pred2lsp(lpc,par->lsf,LPC_ORD);
    
    /* Force minimum LSF bandwidth (separation) */
    lpc_clamp(par->lsf,BWMIN,LPC_ORD);
    
    /* Quantize MELP parameters to 2400 bps and generate bitstream */
    
    /* Quantize LSF's with MSVQ */
    mf_vq_lspw(weights, &par->lsf[1], lpc, LPC_ORD);
    msmf_vq_enc(&par->lsf[1], weights, &par->lsf[1], mf_vq_par);
    par->msvq_index = mf_vq_par.indices;
    
    /* Force minimum LSF bandwidth (separation) */
    lpc_clamp(par->lsf,BWMIN,LPC_ORD);
    
    /* Quantize logarithmic pitch period */
    /* Reserve all zero code for completely unvoiced */
    par->pitch = log10(par->pitch);
    mf_quant_u(&par->pitch,&par->pitch_index,PIT_QLO,PIT_QUP,PIT_QLEV);
    par->pitch = pow(10.0,par->pitch);
    
    /* Quantize gain terms with uniform log quantizer	*/
    mf_q_gain(par->gain, par->gain_index,GN_QLO,GN_QUP,GN_QLEV);
    
    /* Quantize jitter and bandpass voicing */
    mf_quant_u(&par->jitter,&par->jit_index,0.0,MAX_JITTER,2);
    par->uv_flag = mf_q_bpvc(&par->bpvc[0],&par->bpvc_index,bpthresh,
			  mf_NUM_BANDS);
    
    /*	Calculate Fourier coefficients of residual signal from quantized LPC */
    mf_fill(par->fs_mf_mag,1.0,NUM_HARM);
    if (par->bpvc[0] > bpthresh) {
	mf_lpc_lsp2pred(par->lsf,lpc,LPC_ORD);
	mf_zerflt(&mf_speech[(mf_FRAME_END-(LPC_mf_FRAME/2))],lpc,mf_sigbuf,
	       LPC_ORD,LPC_mf_FRAME);
	mf_window(mf_sigbuf,mf_win_cof,mf_sigbuf,LPC_mf_FRAME);
	mf_find_harm(mf_sigbuf, par->fs_mf_mag, par->pitch, NUM_HARM, LPC_mf_FRAME);
    }
    
    /* quantize Fourier coefficients */
    /* pre-weight vector, then use Euclidean distance */
    mf_window(&par->fs_mf_mag[0],mf_w_fs,&par->fs_mf_mag[0],NUM_HARM);
    fsmf_vq_enc(&par->fs_mf_mag[0], &par->fs_mf_mag[0], mf_fs_mf_vq_par);
    
    /* Set MELP indeces to point to same array */
    par->fsvq_index = mf_fs_mf_vq_par.indices;

    /* Update MSVQ information */
    par->msvq_stages = mf_vq_par.num_stages;
    par->msvq_bits = mf_vq_par.num_bits;

    /* Write channel bitstream */
    mf_melp_chn_write(par);

    /* Update delay buffers for next frame */
    mf_v_equ(&mf_speech[0],&mf_speech[mf_FRAME],IN_BEG);
    mf_fpitch[BEGIN] = mf_fpitch[END];
}



/* 
 * mf_mf_melp_ana_init: perform initialization 
 */


void mf_mf_melp_ana_init()
{

    int j;

    mf_mf_bpvc_ana_init(mf_FRAME,mf_PITCHMIN,mf_PITCHMAX,mf_NUM_BANDS,2,MINLENGTH);
    mf_mf_pitch_ana_init(mf_PITCHMIN,mf_PITCHMAX,mf_FRAME,mf_LPF_ORD,MINLENGTH);
    mf_p_avg_init(mf_PDECAY,mf_DEFAULT_PITCH,3);

    mf_v_zap(mf_speech,IN_BEG+mf_FRAME);
    mf_pitch_avg=mf_DEFAULT_PITCH;
    mf_fill(mf_fpitch,mf_DEFAULT_PITCH,2);
    mf_v_zap(lpfsp_del,mf_LPF_ORD);
	
    /* Initialize multi-stage vector quantization (read codebook) */
	
    mf_vq_par.num_best = MSVQ_M;
    mf_vq_par.num_stages = 4;
    mf_vq_par.dimension = 10;

    /* 
     * Allocate memory for number of levels per stage and indices
     * and for number of bits per stage 
     */
 
    mf_vq_par.num_levels = calloc(1, (mf_vq_par.num_stages) * sizeof(int));
    if (!mf_vq_par.num_levels)
	  program_abort(__FILE__, "calloc", 0, __LINE__);
    mf_vq_par.indices = calloc(1, (mf_vq_par.num_stages) * sizeof(int));
    if (!mf_vq_par.indices)
	  program_abort(__FILE__, "calloc", 0, __LINE__);
    mf_vq_par.num_bits = calloc(1, (mf_vq_par.num_stages) * sizeof(int));
    if (!mf_vq_par.num_bits)
	  program_abort(__FILE__, "calloc", 0, __LINE__);
	
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
    mf_v_scale(mf_vq_par.cb,(2.0/FSAMP),3200);

    /* Initialize Fourier mf_magnitude vector quantization (read codebook) */
	
    mf_fs_mf_vq_par.num_best = 1;
    mf_fs_mf_vq_par.num_stages = 1;
    mf_fs_mf_vq_par.dimension = NUM_HARM;

    /* 
     * Allocate memory for number of levels per stage and indices
     * and for number of bits per stage 
     */
 
    mf_fs_mf_vq_par.num_levels = calloc(1,
                                        (mf_fs_mf_vq_par.num_stages) * sizeof(int));
    if (!mf_fs_mf_vq_par.num_levels)
	  program_abort(__FILE__, "calloc", 0, __LINE__);
    mf_fs_mf_vq_par.indices = calloc(1,
                                     (mf_fs_mf_vq_par.num_stages) * sizeof(int));
    if (!mf_fs_mf_vq_par.indices)
	  program_abort(__FILE__, "calloc", 0, __LINE__);
    mf_fs_mf_vq_par.num_bits = calloc(1,
                                      (mf_fs_mf_vq_par.num_stages) * sizeof(int));
    if (!mf_fs_mf_vq_par.num_bits)
	  program_abort(__FILE__, "calloc", 0, __LINE__);

    mf_fs_mf_vq_par.num_levels[0] = FS_LEVELS;
    mf_fs_mf_vq_par.num_bits[0] = FS_BITS;
    mf_fs_mf_vq_par.cb = mf_fsvq_cb;
	
    /* Initialize fixed MSE weighting and inverse of weighting */
	
    mf_vq_fsw(mf_w_fs, NUM_HARM, 60.0);
	
    /* Pre-weight codebook (assume single stage only) */
	
    if (mf_fsvq_weighted == 0)
      {
	  mf_fsvq_weighted = 1;
	  for (j = 0; j < mf_fs_mf_vq_par.num_levels[0]; j++)
	    mf_window(&mf_fs_mf_vq_par.cb[j*NUM_HARM],mf_w_fs,&mf_fs_mf_vq_par.cb[j*NUM_HARM],
		   NUM_HARM);
      }

}

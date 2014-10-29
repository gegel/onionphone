/***********************************************************************
Copyright (c) 2006-2010, Skype Limited. All rights reserved. 
Redistribution and use in source and binary forms, with or without 
modification, (subject to the limitations in the disclaimer below) 
are permitted provided that the following conditions are met:
- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright 
notice, this list of conditions and the following disclaimer in the 
documentation and/or other materials provided with the distribution.
- Neither the name of Skype Limited, nor the names of specific 
contributors, may be used to endorse or promote products derived from 
this software without specific prior written permission.
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED 
BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
CONTRIBUTORS ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF 
USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

#ifndef SKP_SILK_MAIN_FIX_H
#define SKP_SILK_MAIN_FIX_H

#include <stdlib.h>
#include "SKP_Silk_SigProc_FIX.h"
#include "SKP_Silk_structs_FIX.h"
#include "SKP_Silk_main.h"
#include "SKP_Silk_define_FIX.h"
#include "SKP_Silk_PLC.h"
#define TIC(TAG_NAME)
#define TOC(TAG_NAME)

#ifndef FORCE_CPP_BUILD
#ifdef __cplusplus
extern "C"
{
#endif
#endif

/*********************/
/* Encoder Functions */
/*********************/

/* Initializes the Silk encoder state */
int SKP_Silk_init_encoder_FIX(
    SKP_Silk_encoder_state_FIX  *psEnc              /* I/O  Pointer to Silk FIX encoder state           */
);

/* Control the Silk encoder */
int SKP_Silk_control_encoder_FIX( 
    SKP_Silk_encoder_state_FIX  *psEnc,             /* I/O  Pointer to Silk FIX encoder state               */
    const int               API_fs_kHz,         /* I    External (API) sampling rate (kHz)              */
    const int               PacketSize_ms,      /* I    Packet length (ms)                              */
    int32_t                   TargetRate_bps,     /* I    Target max bitrate (bps) (used if SNR_dB == 0)  */
    const int               PacketLoss_perc,    /* I    Packet loss rate (in percent)                   */
    const int               INBandFec_enabled,  /* I    Enable (1) / disable (0) inband FEC             */
    const int               DTX_enabled,        /* I    Enable / disable DTX                            */
    const int               InputFramesize_ms,  /* I    Inputframe in ms                                */
    const int               Complexity          /* I    Complexity (0->low; 1->medium; 2->high)         */
);

/* Encoder main function */
int SKP_Silk_encode_frame_FIX( 
    SKP_Silk_encoder_state_FIX      *psEnc,             /* I/O  Pointer to Silk FIX encoder state           */
    uint8_t                       *pCode,             /* O    Pointer to payload                          */
    int16_t                       *pnBytesOut,        /* I/O  Pointer to number of payload bytes;         */
                                                        /*      input: max length; output: used             */
    const int16_t                 *pIn                /* I    Pointer to input speech frame               */
);

/* Low BitRate Redundancy encoding functionality. Reuse all parameters but encode with lower bitrate */
void SKP_Silk_LBRR_encode_FIX(
    SKP_Silk_encoder_state_FIX      *psEnc,         /* I/O  Pointer to Silk FIX encoder state           */
    SKP_Silk_encoder_control_FIX    *psEncCtrl,     /* I/O  Pointer to Silk FIX encoder control struct  */
    uint8_t                       *pCode,         /* O    Pointer to payload                          */
    int16_t                       *pnBytesOut,    /* I/O  Pointer to number of payload bytes          */
    int16_t                       xfw[]           /* I    Input signal                                */
);

/* High-pass filter with cutoff frequency adaptation based on pitch lag statistics */
void SKP_Silk_HP_variable_cutoff_FIX(
    SKP_Silk_encoder_state_FIX      *psEnc,         /* I/O  Encoder state                               */
    SKP_Silk_encoder_control_FIX    *psEncCtrl,     /* I/O  Encoder control                             */
    int16_t                       *out,           /* O    high-pass filtered output signal            */
    const int16_t                 *in             /* I    input signal                                */
);

/****************/
/* Prefiltering */
/****************/
void SKP_Silk_prefilter_FIX(
    SKP_Silk_encoder_state_FIX          *psEnc,         /* I/O  Encoder state                               */
    const SKP_Silk_encoder_control_FIX  *psEncCtrl,     /* I    Encoder control                             */
    int16_t                           xw[],           /* O    Weighted signal                             */
    const int16_t                     x[]             /* I    Speech signal                               */
);

/**************************************************************/
/* Compute noise shaping coefficients and initial gain values */
/**************************************************************/
void SKP_Silk_noise_shape_analysis_FIX(
    SKP_Silk_encoder_state_FIX      *psEnc,         /* I/O  Encoder state                               */
    SKP_Silk_encoder_control_FIX    *psEncCtrl,     /* I/O  Encoder control                             */
    const int16_t                 *pitch_res,     /* I    LPC residual from pitch analysis            */
    const int16_t                 *x              /* I    Input signal [ 2 * frame_length + la_shape ]*/
);

/* Processing of gains */
void SKP_Silk_process_gains_FIX(
    SKP_Silk_encoder_state_FIX      *psEnc,         /* I/O  Encoder state                               */
    SKP_Silk_encoder_control_FIX    *psEncCtrl      /* I/O  Encoder control                             */
);


/* Control low bitrate redundancy usage */
void SKP_Silk_LBRR_ctrl_FIX(
    SKP_Silk_encoder_state_FIX      *psEnc,         /* I/O  encoder state                               */
    SKP_Silk_encoder_control_FIX    *psEncCtrl      /* I/O  encoder control                             */
);

/* Calculation of LTP state scaling */
void SKP_Silk_LTP_scale_ctrl_FIX(
    SKP_Silk_encoder_state_FIX      *psEnc,         /* I/O  encoder state                               */
    SKP_Silk_encoder_control_FIX    *psEncCtrl      /* I/O  encoder control                             */
);

/**********************************************/
/* Prediction Analysis                        */
/**********************************************/

/* Find pitch lags */
void SKP_Silk_find_pitch_lags_FIX(
    SKP_Silk_encoder_state_FIX      *psEnc,         /* I/O  encoder state                               */
    SKP_Silk_encoder_control_FIX    *psEncCtrl,     /* I/O  encoder control                             */
    int16_t                       res[],          /* O    residual                                    */
    const int16_t                 x[]             /* I    Speech signal                               */
);

void SKP_Silk_find_pred_coefs_FIX(
    SKP_Silk_encoder_state_FIX      *psEnc,         /* I/O  encoder state                               */
    SKP_Silk_encoder_control_FIX    *psEncCtrl,     /* I/O  encoder control                             */
    const int16_t                 res_pitch[]     /* I    Residual from pitch analysis                */
);

void SKP_Silk_find_LPC_FIX(
    int             NLSF_Q15[],             /* O    LSFs                                                                        */
    int             *interpIndex,           /* O    LSF interpolation index, only used for LSF interpolation                    */
    const int       prev_NLSFq_Q15[],       /* I    previous LSFs, only used for LSF interpolation                              */
    const int       useInterpolatedLSFs,    /* I    Flag                                                                        */
    const int       LPC_order,              /* I    LPC order                                                                   */
    const int16_t     x[],                    /* I    Input signal                                                                */
    const int       subfr_length            /* I    Input signal subframe length including preceeding samples                   */
);

void SKP_Silk_LTP_analysis_filter_FIX(
    int16_t       *LTP_res,                           /* O:   LTP residual signal of length NB_SUBFR * ( pre_length + subfr_length )  */
    const int16_t *x,                                 /* I:   Pointer to input signal with at least max( pitchL ) preceeding samples  */
    const int16_t LTPCoef_Q14[ LTP_ORDER * NB_SUBFR ],/* I:   LTP_ORDER LTP coefficients for each NB_SUBFR subframe                   */
    const int   pitchL[ NB_SUBFR ],                 /* I:   Pitch lag, one for each subframe                                        */
    const int32_t invGains_Qxx[ NB_SUBFR ],           /* I:   Inverse quantization gains, one for each subframe                       */
    const int   Qxx,                                /* I:   Inverse quantization gains Q domain                                     */
    const int   subfr_length,                       /* I:   Length of each subframe                                                 */
    const int   pre_length                          /* I:   Length of the preceeding samples starting at &x[0] for each subframe    */
);

/* Finds LTP vector from correlations */
void SKP_Silk_find_LTP_FIX(
    int16_t           b_Q14[ NB_SUBFR * LTP_ORDER ],              /* O    LTP coefs                                                   */
    int32_t           WLTP[ NB_SUBFR * LTP_ORDER * LTP_ORDER ],   /* O    Weight for LTP quantization                                 */
    int             *LTPredCodGain_Q7,                          /* O    LTP coding gain                                             */
    const int16_t     r_first[],                                  /* I    residual signal after LPC signal + state for first 10 ms    */
    const int16_t     r_last[],                                   /* I    residual signal after LPC signal + state for last 10 ms     */
    const int       lag[ NB_SUBFR ],                            /* I    LTP lags                                                    */
    const int32_t     Wght_Q15[ NB_SUBFR ],                       /* I    weights                                                     */
    const int       subfr_length,                               /* I    subframe length                                             */
    const int       mem_offset,                                 /* I    number of samples in LTP memory                             */
    int             corr_rshifts[ NB_SUBFR ]                    /* O    right shifts applied to correlations                        */
);

/* LTP tap quantizer */
void SKP_Silk_quant_LTP_gains_FIX(
    int16_t               B_Q14[],                /* I/O  (un)quantized LTP gains     */
    int                 cbk_index[],            /* O    Codebook Index              */
    int                 *periodicity_index,     /* O    Periodicity Index           */
    const int32_t         W_Q18[],                /* I    Error Weights in Q18        */
    int                 mu_Q8,                  /* I    Mu value (R/D tradeoff)     */
    int                 lowComplexity           /* I    Flag for low complexity     */
);

/******************/
/* NLSF Quantizer */
/******************/

/* Limit, stabilize, convert and quantize NLSFs.    */ 
void SKP_Silk_process_NLSFs_FIX(
    SKP_Silk_encoder_state_FIX      *psEnc,     /* I/O  encoder state                               */
    SKP_Silk_encoder_control_FIX    *psEncCtrl, /* I/O  encoder control                             */
    int                         *pNLSF_Q15  /* I/O  Normalized LSFs (quant out) (0 - (2^15-1))  */
);

/* LSF vector encoder */
void SKP_Silk_NLSF_MSVQ_encode_FIX(
          int                   *NLSFIndices,           /* O    Codebook path vector [ CB_STAGES ]      */
          int                   *pNLSF_Q15,             /* I/O  Quantized NLSF vector [ LPC_ORDER ]     */
    const SKP_Silk_NLSF_CB_struct   *psNLSF_CB,             /* I    Codebook object                         */
    const int                   *pNLSF_q_Q15_prev,      /* I    Prev. quantized NLSF vector [LPC_ORDER] */
    const int                   *pW_Q6,                 /* I    NLSF weight vector [ LPC_ORDER ]        */
    const int                   NLSF_mu_Q15,            /* I    Rate weight for the RD optimization     */
    const int                   NLSF_mu_fluc_red_Q16,   /* I    Fluctuation reduction error weight      */
    const int                   NLSF_MSVQ_Survivors,    /* I    Max survivors from each stage           */
    const int                   LPC_order,              /* I    LPC order                               */
    const int                   deactivate_fluc_red     /* I    Deactivate fluctuation reduction        */
);

/* Rate-Distortion calculations for multiple input data vectors */
void SKP_Silk_NLSF_VQ_rate_distortion_FIX(
    int32_t                       *pRD_Q20,           /* O    Rate-distortion values [psNLSF_CBS->nVectors*N] */
    const SKP_Silk_NLSF_CBS         *psNLSF_CBS,        /* I    NLSF codebook stage struct                      */
    const int                   *in_Q15,            /* I    Input vectors to be quantized                   */
    const int                   *w_Q6,              /* I    Weight vector                                   */
    const int32_t                 *rate_acc_Q5,       /* I    Accumulated rates from previous stage           */
    const int                   mu_Q15,             /* I    Weight between weighted error and rate          */
    const int                   N,                  /* I    Number of input vectors to be quantized         */
    const int                   LPC_order           /* I    LPC order                                       */
);

/* Compute weighted quantization errors for an LPC_order element input vector, over one codebook stage */
void SKP_Silk_NLSF_VQ_sum_error_FIX(
    int32_t                       *err_Q20,           /* O    Weighted quantization errors  [N*K]         */
    const int                   *in_Q15,            /* I    Input vectors to be quantized [N*LPC_order] */
    const int                   *w_Q6,              /* I    Weighting vectors             [N*LPC_order] */
    const int16_t                 *pCB_Q15,           /* I    Codebook vectors              [K*LPC_order] */
    const int                   N,                  /* I    Number of input vectors                     */
    const int                   K,                  /* I    Number of codebook vectors                  */
    const int                   LPC_order           /* I    Number of LPCs                              */
);

/* Entropy constrained MATRIX-weighted VQ, for a single input data vector */
void SKP_Silk_VQ_WMat_EC_FIX(
    int                         *ind,               /* O    index of best codebook vector               */
    int32_t                       *rate_dist_Q14,     /* O    best weighted quantization error + mu * rate*/
    const int16_t                 *in_Q14,            /* I    input vector to be quantized                */
    const int32_t                 *W_Q18,             /* I    weighting matrix                            */
    const int16_t                 *cb_Q14,            /* I    codebook                                    */
    const int16_t                 *cl_Q6,             /* I    code length for each codebook vector        */
    const int                   mu_Q8,              /* I    tradeoff between weighted error and rate    */
    int                         L                   /* I    number of vectors in codebook               */
);

/******************/
/* Linear Algebra */
/******************/

/* Calculates correlation matrix X'*X */
void SKP_Silk_corrMatrix_FIX(
    const int16_t                 *x,         /* I    x vector [L + order - 1] used to form data matrix X */
    const int                   L,          /* I    Length of vectors                                   */
    const int                   order,      /* I    Max lag for correlation                             */
    int32_t                       *XX,        /* O    Pointer to X'*X correlation matrix [ order x order ]*/
    int                         *rshifts    /* I/O  Right shifts of correlations                        */
);

/* Calculates correlation vector X'*t */
void SKP_Silk_corrVector_FIX(
    const int16_t                 *x,         /* I    x vector [L + order - 1] used to form data matrix X */
    const int16_t                 *t,         /* I    target vector [L]                                   */
    const int                   L,          /* I    Length of vectors                                   */
    const int                   order,      /* I    Max lag for correlation                             */
    int32_t                       *Xt,        /* O    Pointer to X'*t correlation vector [order]          */
    const int                   rshifts     /* I    Right shifts of correlations                        */
);

/* Add noise to matrix diagonal */
void SKP_Silk_regularize_correlations_FIX(
    int32_t                       *XX,                /* I/O  Correlation matrices                        */
    int32_t                       *xx,                /* I/O  Correlation values                          */
    int32_t                       noise,              /* I    Noise to add                                */
    int                         D                   /* I    Dimension of XX                             */
);

/* Solves Ax = b, assuming A is symmetric */
void SKP_Silk_solve_LDL_FIX(
    int32_t                       *A,                 /* I    Pointer to symetric square matrix A         */
    int                         M,                  /* I    Size of matrix                              */
    const int32_t                 *b,                 /* I    Pointer to b vector                         */
    int32_t                       *x_Q16              /* O    Pointer to x solution vector                */
);

/* Residual energy: nrg = wxx - 2 * wXx * c + c' * wXX * c */
int32_t SKP_Silk_residual_energy16_covar_FIX(
    const int16_t                 *c,                 /* I    Prediction vector                           */
    const int32_t                 *wXX,               /* I    Correlation matrix                          */
    const int32_t                 *wXx,               /* I    Correlation vector                          */
    int32_t                       wxx,                /* I    Signal energy                               */
    int                         D,                  /* I    Dimension                                   */
    int                         cQ                  /* I    Q value for c vector 0 - 15                 */
);

/* Calculates residual energies of input subframes where all subframes have LPC_order   */
/* of preceeding samples                                                                */
void SKP_Silk_residual_energy_FIX(
          int32_t nrgs[ NB_SUBFR ],           /* O    Residual energy per subframe    */
          int   nrgsQ[ NB_SUBFR ],          /* O    Q value per subframe            */
    const int16_t x[],                        /* I    Input signal                    */
    const int16_t a_Q12[ 2 ][ MAX_LPC_ORDER ],/* I    AR coefs for each frame half    */
    const int32_t gains_Qx[ NB_SUBFR ],       /* I    Quantization gains in Qx        */
    const int   Qx,                         /* I    Quantization gains Q value      */
    const int   subfr_length,               /* I    Subframe length                 */
    const int   LPC_order                   /* I    LPC order                       */
);

#ifndef FORCE_CPP_BUILD
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* FORCE_CPP_BUILD */
#endif /* SKP_SILK_MAIN_FIX_H */

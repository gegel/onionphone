/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

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

#ifndef _SKP_SILK_SIGPROC_FIX_H_
#define _SKP_SILK_SIGPROC_FIX_H_

#ifdef  __cplusplus
extern "C" {
#endif

#define SigProc_MAX_ORDER_LPC            16	/* max order of the LPC analysis in schur() and k2a()    */
#define SigProc_MAX_CORRELATION_LENGTH   640	/* max input length to the correlation                    */
#include "SKP_Silk_typedef.h"
#include <string.h>
#include <stdlib.h>		/* for abs() */
#include "SKP_Silk_macros.h"
#include "SKP_Silk_resample_rom.h"

/********************************************************************/
/*                    SIGNAL PROCESSING FUNCTIONS                   */
/********************************************************************/

/* downsample by a factor 2 */
	void SKP_Silk_resample_1_2(const int16_t * in,	/* I:   16 kHz signal [2*len]    */
				   int32_t * S,	/* I/O: State vector [6]         */
				   int16_t * out,	/* O:   8 kHz signal [len]       */
				   int32_t * scratch,	/* I:   Scratch memory [4*len]   */
				   const int32_t len	/* I:   Number of OUTPUT samples */
	    );

/*! 
 * downsample by a factor 2, coarser (good for resampling audio) 
 */
	void SKP_Silk_resample_1_2_coarse(const int16_t * in,	/* I:   16 kHz signal [2*len]    */
					  int32_t * S,	/* I/O: state vector [4]         */
					  int16_t * out,	/* O:   8 kHz signal [len]       */
					  int32_t * scratch,	/* I:   scratch memory [3*len]   */
					  const int32_t len	/* I:   number of OUTPUT samples */
	    );

/*! 
 * downsample by a factor 2, coarsest (good for signals that are already oversampled, or for analysis purposes) 
 */
	void SKP_Silk_resample_1_2_coarsest(const int16_t * in,	/* I:   16 kHz signal [2*len]    */
					    int32_t * S,	/* I/O: State vector [2]         */
					    int16_t * out,	/* O:   8 kHz signal [len]       */
					    int32_t * scratch,	/* I:   Scratch memory [3*len]   */
					    const int32_t len	/* I:   Number of OUTPUT samples */
	    );

/*! 
 * upsample by a factor 2, coarser (good for resampling audio) 
 */
	void SKP_Silk_resample_2_1_coarse(const int16_t * in,	/* I:   8 kHz signal [len]      */
					  int32_t * S,	/* I/O: State vector [4]        */
					  int16_t * out,	/* O:   16 kHz signal [2*len]   */
					  int32_t * scratch,	/* I:   Scratch memory [3*len]  */
					  const int32_t len	/* I:   Number of INPUT samples */
	    );

/*!
 * Resamples by a factor 1/3 
 */
	void SKP_Silk_resample_1_3(int16_t * out,	/* O:   Fs_low signal  [inLen/3]              */
				   int32_t * S,	/* I/O: State vector   [7]                    */
				   const int16_t * in,	/* I:   Fs_high signal [inLen]                */
				   const int32_t inLen	/* I:   Input length, must be a multiple of 3 */
	    );

/*!
 * Resamples by a factor 3/1
 */
	void SKP_Silk_resample_3_1(int16_t * out,	/* O:   Fs_high signal [inLen*3]          */
				   int32_t * S,	/* I/O: State vector   [7]                */
				   const int16_t * in,	/* I:   Fs_low signal  [inLen]            */
				   const int32_t inLen	/* I:   Input length                      */
	    );

/*!
 * Resamples by a factor 2/3
 */
	void SKP_Silk_resample_2_3(int16_t * out,	/* O:   Fs_low signal    [inLen * 2/3]        */
				   int32_t * S,	/* I/O: State vector    [7+4]                 */
				   const int16_t * in,	/* I:   Fs_high signal    [inLen]             */
				   const int inLen	/* I:   Input length, must be a multiple of 3 */
	    );

/*!
 * Resamples by a factor 3/2
 */
	void SKP_Silk_resample_3_2(int16_t * out,	/*   O: Fs_high signal  [inLen*3/2]              */
				   int32_t * S,	/* I/O: State vector    [7+4]                    */
				   const int16_t * in,	/* I:   Fs_low signal   [inLen]                  */
				   int inLen	/* I:   Input length, must be a multiple of 2    */
	    );

/*!
 * Resamples by a factor 4/3
 */
	void SKP_Silk_resample_4_3(int16_t * out,	/* O:   Fs_low signal    [inLen * 4/3]           */
				   int32_t * S,	/* I/O: State vector    [7+4+4]                  */
				   const int16_t * in,	/* I:   Fs_high signal    [inLen]                */
				   const int inLen	/* I:   input length, must be a multiple of 3    */
	    );

/*!
 * Resamples by a factor 3/4
 */
	void SKP_Silk_resample_3_4(int16_t * out,	/* O:   Fs_high signal  [inLen*3/4]              */
				   int32_t * S,	/* I/O: State vector    [7+2+6]                  */
				   const int16_t * in,	/* I:   Fs_low signal   [inLen]                  */
				   int inLen	/* I:   Input length, must be a multiple of 4    */
	    );

/*! 
 * resample with a factor 2/3 coarse
 */
	void SKP_Silk_resample_2_3_coarse(int16_t * out,	/* O:   Output signal                                                                 */
					  int16_t * S,	/* I/O: Resampler state [ SigProc_Resample_2_3_coarse_NUM_FIR_COEFS - 1 ]             */
					  const int16_t * in,	/* I:   Input signal                                                                  */
					  const int frameLenIn,	/* I:   Number of input samples                                                       */
					  int16_t * scratch	/* I:   Scratch memory [ frameLenIn + SigProc_Resample_2_3_coarse_NUM_FIR_COEFS - 1 ] */
	    );

/*! 
 * resample with a factor 2/3 coarsest
 */
	void SKP_Silk_resample_2_3_coarsest(int16_t * out,	/* O:   Output signal                                                                   */
					    int16_t * S,	/* I/O: Resampler state [ SigProc_Resample_2_3_coarsest_NUM_FIR_COEFS - 1 ]             */
					    const int16_t * in,	/* I:   Input signal                                                                    */
					    const int frameLenIn,	/* I:   Number of input samples                                                         */
					    int16_t * scratch	/* I:   Scratch memory [ frameLenIn + SigProc_Resample_2_3_coarsest_NUM_FIR_COEFS - 1 ] */
	    );

/*! 
 * First order low-pass filter, with input as int16_t, running at 48 kHz 
 */
	void SKP_Silk_lowpass_short(const int16_t * in,	/* I:   Q15 48 kHz signal; [len]    */
				    int32_t * S,	/* I/O: Q25 state; length = 1       */
				    int32_t * out,	/* O:   Q25 48 kHz signal; [len]    */
				    const int32_t len	/* O:   Signal length               */
	    );

/*! 
 * First order low-pass filter, with input as int32_t, running at 48 kHz 
 */
	void SKP_Silk_lowpass_int(const int32_t * in,	/* I:   Q25 48 kHz signal; length = len  */
				  int32_t * S,	/* I/O: Q25 state; length = 1            */
				  int32_t * out,	/* O:   Q25 48 kHz signal; length = len  */
				  const int32_t len	/* I:   Number of samples                */
	    );

/*! 
 * First-order allpass filter 
 */
	void SKP_Silk_allpass_int(const int32_t * in,	/* I:   Q25 input signal [len]               */
				  int32_t * S,	/* I/O: Q25 state [1]                        */
				  int A,	/* I:   Q15 coefficient    (0 <= A < 32768)  */
				  int32_t * out,	/* O:   Q25 output signal [len]              */
				  const int32_t len	/* I:   Number of samples                    */
	    );

/*! 
 * second order ARMA filter
 * can handle (slowly) varying coefficients 
 */
	void SKP_Silk_biquad(const int16_t * in,	/* I:   input signal                */
			     const int16_t * B,	/* I:   MA coefficients, Q13 [3]    */
			     const int16_t * A,	/* I:   AR coefficients, Q13 [2]    */
			     int32_t * S,	/* I/O: state vector [2]            */
			     int16_t * out,	/* O:   output signal               */
			     const int32_t len	/* I:   signal length               */
	    );
/*!
 * second order ARMA filter; 
 * slower than biquad() but uses more precise coefficients
 * can handle (slowly) varying coefficients 
 */
	void SKP_Silk_biquad_alt(const int16_t * in,	/* I:    input signal                 */
				 const int32_t * B_Q28,	/* I:    MA coefficients [3]          */
				 const int32_t * A_Q28,	/* I:    AR coefficients [2]          */
				 int32_t * S,	/* I/O: state vector [2]              */
				 int16_t * out,	/* O:    output signal                */
				 const int32_t len	/* I:    signal length (must be even) */
	    );

/*! 
 * variable order MA filter. Prediction error filter implementation. Coeficients negated and starting with coef to x[n - 1]
 */
	void SKP_Silk_MA_Prediction(const int16_t * in,	/* I:   Input signal                                */
				    const int16_t * B,	/* I:   MA prediction coefficients, Q12 [order]     */
				    int32_t * S,	/* I/O: State vector [order]                        */
				    int16_t * out,	/* O:   Output signal                               */
				    const int32_t len,	/* I:   Signal length                               */
				    const int32_t order	/* I:  Filter order                                 */
	    );

	void SKP_Silk_MA_Prediction_Q13(const int16_t * in,	/* I:   input signal                                */
					const int16_t * B,	/* I:   MA prediction coefficients, Q13 [order]     */
					int32_t * S,	/* I/O: state vector [order]                        */
					int16_t * out,	/* O:   output signal                               */
					int32_t len,	/* I:   signal length                               */
					int32_t order	/* I:   filter order                                */
	    );

/*!
 * 16th order AR filter for LPC synthesis, coefficients are in Q12
 */
	void SKP_Silk_LPC_synthesis_order16(const int16_t * in,	/* I:   excitation signal                            */
					    const int16_t * A_Q12,	/* I:   AR coefficients [16], between -8_Q0 and 8_Q0 */
					    const int32_t Gain_Q26,	/* I:   gain                                         */
					    int32_t * S,	/* I/O: state vector [16]                            */
					    int16_t * out,	/* O:   output signal                                */
					    const int32_t len	/* I:   signal length, must be multiple of 16        */
	    );

/* variable order MA prediction error filter. */
/* Inverse filter of SKP_Silk_LPC_synthesis_filter */
	void SKP_Silk_LPC_analysis_filter(const int16_t * in,	/* I:   Input signal                                */
					  const int16_t * B,	/* I:   MA prediction coefficients, Q12 [order]     */
					  int16_t * S,	/* I/O: State vector [order]                        */
					  int16_t * out,	/* O:   Output signal                               */
					  const int32_t len,	/* I:   Signal length                               */
					  const int32_t Order	/* I:   Filter order                                */
	    );

/* even order AR filter */
	void SKP_Silk_LPC_synthesis_filter(const int16_t * in,	/* I:   excitation signal                               */
					   const int16_t * A_Q12,	/* I:   AR coefficients [Order], between -8_Q0 and 8_Q0 */
					   const int32_t Gain_Q26,	/* I:   gain                                            */
					   int32_t * S,	/* I/O: state vector [Order]                            */
					   int16_t * out,	/* O:   output signal                                   */
					   const int32_t len,	/* I:   signal length                                   */
					   const int Order	/* I:   filter order, must be even                      */
	    );

/* Chirp (bandwidth expand) LP AR filter */
	void SKP_Silk_bwexpander(int16_t * ar,	/* I/O  AR filter to be expanded (without leading 1)    */
				 const int d,	/* I    Length of ar                                    */
				 int32_t chirp_Q16	/* I    Chirp factor (typically in the range 0 to 1)    */
	    );

/* Chirp (bandwidth expand) LP AR filter */
	void SKP_Silk_bwexpander_32(int32_t * ar,	/* I/O  AR filter to be expanded (without leading 1)    */
				    const int d,	/* I    Length of ar                                    */
				    int32_t chirp_Q16	/* I    Chirp factor in Q16                             */
	    );

/* Compute inverse of LPC prediction gain, and                           */
/* test if LPC coefficients are stable (all poles within unit circle)    */
	int SKP_Silk_LPC_inverse_pred_gain(	/* O:  Returns 1 if unstable, otherwise 0          */
						  int32_t * invGain_Q30,	/* O:  Inverse prediction gain, Q30 energy domain  */
						  const int16_t * A_Q12,	/* I:  Prediction coefficients, Q12 [order]        */
						  const int order	/* I:  Prediction order                            */
	    );

	int SKP_Silk_LPC_inverse_pred_gain_Q13(	/* O:  returns 1 if unstable, otherwise 0      */
						      int32_t * invGain_Q30,	/* O:  Inverse prediction gain, Q30 energy domain  */
						      const int16_t * A_Q13,	/* I:  Prediction coefficients, Q13 [order]        */
						      const int order	/* I:  Prediction order                            */
	    );

/* split signal in two decimated bands using first-order allpass filters */
	void SKP_Silk_ana_filt_bank_1(const int16_t * in,	/* I:   Input signal [N]        */
				      int32_t * S,	/* I/O: State vector [2]        */
				      int16_t * outL,	/* O:   Low band [N/2]          */
				      int16_t * outH,	/* O:   High band [N/2]         */
				      int32_t * scratch,	/* I:   Scratch memory [3*N/2]  */
				      const int32_t N	/* I:   Number of input samples */
	    );

/********************************************************************/
/*                        SCALAR FUNCTIONS                            */
/********************************************************************/

/* approximation of 128 * log2() (exact inverse of approx 2^() below) */
/* convert input to a log scale    */
	int32_t SKP_Silk_lin2log(const int32_t inLin);	/* I: input in linear scale        */

/* Approximation of a sigmoid function */
	int SKP_Silk_sigm_Q15(int in_Q5);

/* approximation of 2^() (exact inverse of approx log2() above) */
/* convert input to a linear scale    */
	int32_t SKP_Silk_log2lin(const int32_t inLog_Q7);	/* I: input on log scale */

/* Function that returns the maximum absolut value of the input vector */
	int16_t SKP_Silk_int16_array_maxabs(	/* O   Maximum absolute value, max: 2^15-1   */
						   const int16_t * vec,	/* I   Input vector  [len]                   */
						   const int32_t len	/* I   Length of input vector                */
	    );

/* Compute number of bits to right shift the sum of squares of a vector    */
/* of int16s to make it fit in an int32                                    */
	void SKP_Silk_sum_sqr_shift(int32_t * energy,	/* O   Energy of x, after shifting to the right            */
				    int *shift,	/* O   Number of bits right shift applied to energy        */
				    const int16_t * x,	/* I   Input vector                                        */
				    int len	/* I   Length of input vector                              */
	    );

/* Calculates the reflection coefficients from the correlation sequence    */
/* Faster than schur64(), but much less accurate.                          */
/* Uses SMLAWB(), requiring armv5E and higher.                             */
	void SKP_Silk_schur(int16_t * rc_Q15,	/* O:  reflection coefficients [order] Q15         */
			    const int32_t * c,	/* I:  correlations [order+1]                      */
			    const int32_t order	/* I:  prediction order                            */
	    );

/* Calculates the reflection coefficients from the correlation sequence    */
/* Slower than schur(), but more accurate.                                 */
/* Uses SMULL(), available on armv4                                        */
	int32_t SKP_Silk_schur64(	/* O:  returns residual energy                     */
					int32_t rc_Q16[],	/* O:  Reflection coefficients [order] Q16         */
					const int32_t c[],	/* I:  Correlations [order+1]                      */
					int32_t order	/* I:  Prediction order                            */
	    );

/* Step up function, converts reflection coefficients to prediction coefficients */
	void SKP_Silk_k2a(int32_t * A_Q24,	/* O:  Prediction coefficients [order] Q24         */
			  const int16_t * rc_Q15,	/* I:  Reflection coefficients [order] Q15         */
			  const int32_t order	/* I:  Prediction order                            */
	    );

/* Step up function, converts reflection coefficients to prediction coefficients */
	void SKP_Silk_k2a_Q16(int32_t * A_Q24,	/* O:  Prediction coefficients [order] Q24         */
			      const int32_t * rc_Q16,	/* I:  Reflection coefficients [order] Q16         */
			      const int32_t order	/* I:  Prediction order                            */
	    );

/* Apply sine window to signal vector.                                      */
/* Window types:                                                            */
/*    0 -> sine window from 0 to pi                                         */
/*    1 -> sine window from 0 to pi/2                                       */
/*    2 -> sine window from pi/2 to pi                                      */
/* every other sample of window is linearly interpolated, for speed         */
	void SKP_Silk_apply_sine_window(int16_t px_win[],	/* O  Pointer to windowed signal                  */
					const int16_t px[],	/* I  Pointer to input signal                     */
					const int win_type,	/* I  Selects a window type                       */
					const int length	/* I  Window length, multiple of 4                */
	    );

/* Compute autocorrelation */
	void SKP_Silk_autocorr(int32_t * results,	/* O  Result (length correlationCount)            */
			       int32_t * scale,	/* O  Scaling of the correlation vector           */
			       const int16_t * inputData,	/* I  Input data to correlate                     */
			       const int inputDataSize,	/* I  Length of input                             */
			       const int correlationCount	/* I  Number of correlation taps to compute      */
	    );

/* Pitch estimator */
#define SigProc_PITCH_EST_MIN_COMPLEX        0
#define SigProc_PITCH_EST_MID_COMPLEX        1
#define SigProc_PITCH_EST_MAX_COMPLEX        2

	void SKP_Silk_decode_pitch(int lagIndex,	/* I                                              */
				   int contourIndex,	/* O                                              */
				   int pitch_lags[],	/* O 4 pitch values                               */
				   int Fs_kHz	/* I sampling frequency (kHz)                     */
	    );

	int SKP_Silk_pitch_analysis_core(	/* O    Voicing estimate: 0 voiced, 1 unvoiced                      */
						const int16_t * signal,	/* I    Signal of length PITCH_EST_FRAME_LENGTH_MS*Fs_kHz           */
						int *pitch_out,	/* O    4 pitch lag values                                          */
						int *lagIndex,	/* O    Lag Index                                                   */
						int *contourIndex,	/* O    Pitch contour Index                                         */
						int *LTPCorr_Q15,	/* I/O  Normalized correlation; input: value from previous frame    */
						int prevLag,	/* I    Last lag of previous frame; set to zero is unvoiced         */
						const int32_t search_thres1_Q16,	/* I    First stage threshold for lag candidates 0 - 1              */
						const int search_thres2_Q15,	/* I    Final threshold for lag candidates 0 - 1                    */
						const int Fs_kHz,	/* I    Sample frequency (kHz)                                      */
						const int complexity	/* I    Complexity setting, 0-2, where 2 is highest                 */
	    );

/* parameter defining the size and accuracy of the piecewise linear    */
/* cosine approximatin table.                                        */

#define LSF_COS_TAB_SZ_FIX      128
/* rom table with cosine values */
	extern const int SKP_Silk_LSFCosTab_FIX_Q12[LSF_COS_TAB_SZ_FIX + 1];

	void SKP_Silk_LPC_fit(int16_t * a_QQ,	/* O    stabilized LPC vector, Q(24-rshift) [L]         */
			      int32_t * a_Q24,	/* I    LPC vector [L]                                  */
			      const int QQ,	/* I    Q domain of output LPC vector                   */
			      const int L	/* I    Number of LPC parameters in the input vector    */
	    );

/* Compute Normalized Line Spectral Frequencies (NLSFs) from whitening filter coefficients        */
/* If not all roots are found, the a_Q16 coefficients are bandwidth expanded until convergence.    */
	void SKP_Silk_A2NLSF(int *NLSF,	/* O    Normalized Line Spectral Frequencies, Q15 (0 - (2^15-1)), [d] */
			     int32_t * a_Q16,	/* I/O  Monic whitening filter coefficients in Q16 [d]                */
			     const int d	/* I    Filter order (must be even)                                   */
	    );

/* compute whitening filter coefficients from normalized line spectral frequencies */
	void SKP_Silk_NLSF2A(int16_t * a,	/* o    monic whitening filter coefficients in Q12,  [d]    */
			     const int *NLSF,	/* i    normalized line spectral frequencies in Q15, [d]    */
			     const int d	/* i    filter order (should be even)                       */
	    );

	void SKP_Silk_insertion_sort_increasing(int32_t * a,	/* I/O   Unsorted / Sorted vector                */
						int *index,	/* O:    Index vector for the sorted elements    */
						const int L,	/* I:    Vector length                           */
						const int K	/* I:    Number of correctly sorted positions    */
	    );

	void SKP_Silk_insertion_sort_decreasing(int *a,	/* I/O:  Unsorted / Sorted vector                */
						int *index,	/* O:    Index vector for the sorted elements    */
						const int L,	/* I:    Vector length                           */
						const int K	/* I:    Number of correctly sorted positions    */
	    );

	void SKP_Silk_insertion_sort_decreasing_int16(int16_t * a,	/* I/O:  Unsorted / Sorted vector                */
						      int *index,	/* O:    Index vector for the sorted elements    */
						      const int L,	/* I:    Vector length                           */
						      const int K	/* I:    Number of correctly sorted positions    */
	    );

	void SKP_Silk_insertion_sort_increasing_all_values(int *a,	/* I/O:  Unsorted / Sorted vector                */
							   const int L	/* I:    Vector length                           */
	    );

/* NLSF stabilizer, for a single input data vector */
	void SKP_Silk_NLSF_stabilize(int *NLSF_Q15,	/* I/O:  Unstable/stabilized normalized LSF vector in Q15 [L]                    */
				     const int *NDeltaMin_Q15,	/* I:    Normalized delta min vector in Q15, NDeltaMin_Q15[L] must be >= 1 [L+1] */
				     const int L	/* I:    Number of NLSF parameters in the input vector                           */
	    );

/* NLSF stabilizer, over multiple input column data vectors */
	void SKP_Silk_NLSF_stabilize_multi(int *NLSF_Q15,	/* I/O:  Unstable/stabilized normalized LSF vectors in Q15 [LxN]                 */
					   const int *NDeltaMin_Q15,	/* I:    Normalized delta min vector in Q15, NDeltaMin_Q15[L] must be >= 1 [L+1] */
					   const int N,	/* I:    Number of input vectors to be stabilized                                */
					   const int L	/* I:    NLSF vector dimension                                                   */
	    );

/* Laroia low complexity NLSF weights */
	void SKP_Silk_NLSF_VQ_weights_laroia(int *pNLSFW_Q6,	/* O:    Pointer to input vector weights            [D x 1]       */
					     const int *pNLSF_Q15,	/* I:    Pointer to input vector                    [D x 1]       */
					     const int D	/* I:    Input vector dimension (even)                            */
	    );

/* Compute reflection coefficients from input signal */
	void SKP_Silk_burg_modified(int32_t * res_nrg,	/* O   residual energy                                                 */
				    int *res_nrgQ,	/* O   residual energy Q value                                         */
				    int32_t A_Q16[],	/* O   prediction coefficients (length order)                          */
				    const int16_t x[],	/* I   input signal, length: nb_subfr * ( D + subfr_length )           */
				    const int subfr_length,	/* I   input signal subframe length (including D preceeding samples)   */
				    const int nb_subfr,	/* I   number of subframes stacked in x                                */
				    const int32_t WhiteNoiseFrac_Q32,	/* I   fraction added to zero-lag autocorrelation                      */
				    const int D	/* I   order                                                           */
	    );

/* Multiply a vector by a constant */
	void SKP_Silk_scale_vector16_Q14(int16_t * data1, int gain_Q14,	/* Gain in Q14 */
					 int dataSize);

/* Copy and multiply a vector by a constant */
	void SKP_Silk_scale_copy_vector16(int16_t * data_out, const int16_t * data_in, int32_t gain_Q16,	/* I:   gain in Q16   */
					  const int dataSize	/* I:   length        */
	    );

	void SKP_Silk_scale_vector32_16_Q14(int32_t * data1,	/* I/O: Q0/Q0         */
					    int gain_Q14,	/* I:   Q14           */
					    int dataSize	/* I:   length        */
	    );

/* Multiply a vector by a constant, does not saturate output data */
	void SKP_Silk_scale_vector32_Q16(int32_t * data1,	/* I/O: Q0/Q0         */
					 int32_t gain_Q16,	/* I:   gain in Q16 ( int16_t_MIN <= gain_Q16 <= int16_t_MAX + 65536 ) */
					 const int dataSize	/* I:   length        */
	    );

/* Some for the LTP related function requires Q26 to work.*/
	void SKP_Silk_scale_vector32_Q26_lshift_18(int32_t * data1,	/* I/O: Q0/Q18        */
						   int32_t gain_Q26,	/* I:   Q26           */
						   int dataSize	/* I:   length        */
	    );

/********************************************************************/
/*                        INLINE ARM MATH                             */
/********************************************************************/

/*    return sum(inVec1[i]*inVec2[i])    */
/*    inVec1 and inVec2 should be increasing ordered, and starting address should be 4 byte aligned. (a factor of 4)*/
	int32_t SKP_Silk_inner_prod_aligned(const int16_t * const inVec1,	/* I   input vector 1    */
					    const int16_t * const inVec2,	/* I   input vector 2    */
					    const int len	/* I   vector lengths    */
	    );

	int32_t SKP_Silk_inner_prod16_aligned_sat(const int16_t * const inVec1,	/* I   input vector 1  */
						  const int16_t * const inVec2,	/* I   input vector 2  */
						  const int len	/* I   vector lengths  */
	    );

	int64_t SKP_Silk_inner_prod_aligned_64(const int32_t * inVec1,	/* I   input vector 1    */
					       const int32_t * inVec2,	/* I   input vector 2    */
					       const int len	/* I   vector lengths    */
	    );

	int64_t SKP_Silk_inner_prod16_aligned_64(const int16_t * inVec1,	/* I   input vector 1    */
						 const int16_t * inVec2,	/* I   input vector 2    */
						 const int len	/* I   vector lengths    */
	    );
/********************************************************************/
/*                                MACROS                                */
/********************************************************************/

/* Define 4-byte aligned array of int16_t */
#define SKP_array_of_int16_4_byte_aligned( arrayName, nElements )    \
    int32_t dummy_int32 ## arrayName;                                \
    int16_t arrayName[ (nElements) ]

/* Useful Macros that can be adjusted to other platforms */
#define SKP_memcpy(a, b, c)                memcpy((a), (b), (c))	/* Dest, Src, ByteCount */
#define SKP_memset(a, b, c)                memset((a), (b), (c))	/* Dest, value, ByteCount */
#define SKP_memmove(a, b, c)               memmove((a), (b), (c))	/* Dest, Src, ByteCount */
/* fixed point macros */

// (a32 * b32) output have to be 32bit int
#define SKP_MUL(a32, b32)                  ((a32) * (b32))

// (a32 * b32) output have to be 32bit uint
#define SKP_MUL_uint(a32, b32)             SKP_MUL(a32, b32)

// a32 + (b32 * c32) output have to be 32bit int
#define SKP_MLA(a32, b32, c32)             SKP_ADD32((a32),((b32) * (c32)))

// a32 + (b32 * c32) output have to be 32bit uint
#define SKP_MLA_uint(a32, b32, c32)        SKP_MLA(a32, b32, c32)

// ((a32 >> 16)  * (b32 >> 16)) output have to be 32bit int
#define SKP_SMULTT(a32, b32)               (((a32) >> 16) * ((b32) >> 16))

// a32 + ((a32 >> 16)  * (b32 >> 16)) output have to be 32bit int
#define SKP_SMLATT(a32, b32, c32)          SKP_ADD32((a32),((b32) >> 16) * ((c32) >> 16))

#define SKP_SMLALBB(a64, b16, c16)         SKP_ADD64((a64),(int64_t)((int32_t)(b16) * (int32_t)(c16)))

// (a32 * b32)
#define SKP_SMULL(a32, b32)                ((int64_t)(a32) * /*(int64_t)*/(b32))

// multiply-accumulate macros that allow overflow in the addition (ie, no asserts in debug mode)
#define SKP_MLA_ovflw(a32, b32, c32)       SKP_MLA(a32, b32, c32)
#ifndef SKP_SMLABB_ovflw
#define SKP_SMLABB_ovflw(a32, b32, c32)    SKP_SMLABB(a32, b32, c32)
#endif
#define SKP_SMLABT_ovflw(a32, b32, c32)    SKP_SMLABT(a32, b32, c32)
#define SKP_SMLATT_ovflw(a32, b32, c32)    SKP_SMLATT(a32, b32, c32)
#define SKP_SMLAWB_ovflw(a32, b32, c32)    SKP_SMLAWB(a32, b32, c32)
#define SKP_SMLAWT_ovflw(a32, b32, c32)    SKP_SMLAWT(a32, b32, c32)

#define SKP_DIV64_32(a64, b32)             ((a64)/(b32))	/* TODO: rewrite it as a set of SKP_DIV32. */

#define SKP_DIV32_16(a32, b16)             ((int32_t)((a32) / (b16)))
#define SKP_DIV32(a32, b32)                ((int32_t)((a32) / (b32)))

// These macros enables checking for overflow in SKP_Silk_API_Debug.h
#define SKP_ADD16(a, b)                    ((a) + (b))
#define SKP_ADD32(a, b)                    ((a) + (b))
#define SKP_ADD64(a, b)                    ((a) + (b))

#define SKP_SUB16(a, b)                    ((a) - (b))
#define SKP_SUB32(a, b)                    ((a) - (b))
#define SKP_SUB64(a, b)                    ((a) - (b))

#define SKP_SAT8(a)                        ((a) > int8_t_MAX ? int8_t_MAX  : \
                                           ((a) < int8_t_MIN ? int8_t_MIN  : (a)))
#define SKP_SAT16(a)                       ((a) > int16_t_MAX ? int16_t_MAX : \
                                           ((a) < int16_t_MIN ? int16_t_MIN : (a)))
#define SKP_SAT32(a)                       ((a) > int32_t_MAX ? int32_t_MAX : \
                                           ((a) < int32_t_MIN ? int32_t_MIN : (a)))

#define SKP_CHECK_FIT8(a)                  (a)
#define SKP_CHECK_FIT16(a)                 (a)
#define SKP_CHECK_FIT32(a)                 (a)

#define SKP_ADD_SAT16(a, b)                (int16_t)SKP_SAT16( SKP_ADD32( (int32_t)(a), (b) ) )
#define SKP_ADD_SAT64(a, b)                ((((a) + (b)) & 0x8000000000000000LL) == 0 ?                            \
                                           ((((a) & (b)) & 0x8000000000000000LL) != 0 ? int64_t_MIN : (a)+(b)) :    \
                                           ((((a) | (b)) & 0x8000000000000000LL) == 0 ? int64_t_MAX : (a)+(b)) )

#define SKP_SUB_SAT16(a, b)                (int16_t)SKP_SAT16( SKP_SUB32( (int32_t)(a), (b) ) )
#define SKP_SUB_SAT64(a, b)                ((((a)-(b)) & 0x8000000000000000LL) == 0 ?                                                    \
                                           (( (a) & ((b)^0x8000000000000000LL) & 0x8000000000000000LL) ? int64_t_MIN : (a)-(b)) :    \
                                           ((((a)^0x8000000000000000LL) & (b)  & 0x8000000000000000LL) ? int64_t_MAX : (a)-(b)) )

/* Saturation for positive input values */
#define SKP_POS_SAT32(a)                   ((a) > int32_t_MAX ? int32_t_MAX : (a))

/* Add with saturation for positive input values */
#define SKP_ADD_POS_SAT8(a, b)             ((((a)+(b)) & 0x80)                 ? int8_t_MAX  : ((a)+(b)))
#define SKP_ADD_POS_SAT16(a, b)            ((((a)+(b)) & 0x8000)               ? int16_t_MAX : ((a)+(b)))
#define SKP_ADD_POS_SAT32(a, b)            ((((a)+(b)) & 0x80000000)           ? int32_t_MAX : ((a)+(b)))
#define SKP_ADD_POS_SAT64(a, b)            ((((a)+(b)) & 0x8000000000000000LL) ? int64_t_MAX : ((a)+(b)))

#define SKP_LSHIFT8(a, shift)              ((a)<<(shift))	// shift >= 0, shift < 8
#define SKP_LSHIFT16(a, shift)             ((a)<<(shift))	// shift >= 0, shift < 16
#define SKP_LSHIFT32(a, shift)             ((a)<<(shift))	// shift >= 0, shift < 32
#define SKP_LSHIFT64(a, shift)             ((a)<<(shift))	// shift >= 0, shift < 64
#define SKP_LSHIFT(a, shift)               SKP_LSHIFT32(a, shift)	// shift >= 0, shift < 32

#define SKP_RSHIFT8(a, shift)              ((a)>>(shift))	// shift >= 0, shift < 8
#define SKP_RSHIFT16(a, shift)             ((a)>>(shift))	// shift >= 0, shift < 16
#define SKP_RSHIFT32(a, shift)             ((a)>>(shift))	// shift >= 0, shift < 32
#define SKP_RSHIFT64(a, shift)             ((a)>>(shift))	// shift >= 0, shift < 64
#define SKP_RSHIFT(a, shift)               SKP_RSHIFT32(a, shift)	// shift >= 0, shift < 32

/* saturates before shifting */
#define SKP_LSHIFT_SAT16(a, shift)         (SKP_LSHIFT16( SKP_LIMIT( (a), SKP_RSHIFT16( int16_t_MIN, (shift) ),    \
                                                                          SKP_RSHIFT16( int16_t_MAX, (shift) ) ), (shift) ))
#define SKP_LSHIFT_SAT32(a, shift)         (SKP_LSHIFT32( SKP_LIMIT( (a), SKP_RSHIFT32( int32_t_MIN, (shift) ),    \
                                                                          SKP_RSHIFT32( int32_t_MAX, (shift) ) ), (shift) ))

#define SKP_LSHIFT_ovflw(a, shift)        ((a)<<(shift))	// shift >= 0, allowed to overflow
#define SKP_LSHIFT_uint(a, shift)         ((a)<<(shift))	// shift >= 0
#define SKP_RSHIFT_uint(a, shift)         ((a)>>(shift))	// shift >= 0

#define SKP_ADD_LSHIFT(a, b, shift)       ((a) + SKP_LSHIFT((b), (shift)))	// shift >= 0
#define SKP_ADD_LSHIFT32(a, b, shift)     SKP_ADD32((a), SKP_LSHIFT32((b), (shift)))	// shift >= 0
#define SKP_ADD_LSHIFT_uint(a, b, shift)  ((a) + SKP_LSHIFT_uint((b), (shift)))	// shift >= 0
#define SKP_ADD_RSHIFT(a, b, shift)       ((a) + SKP_RSHIFT((b), (shift)))	// shift >= 0
#define SKP_ADD_RSHIFT32(a, b, shift)     SKP_ADD32((a), SKP_RSHIFT32((b), (shift)))	// shift >= 0
#define SKP_ADD_RSHIFT_uint(a, b, shift)  ((a) + SKP_RSHIFT_uint((b), (shift)))	// shift >= 0
#define SKP_SUB_LSHIFT32(a, b, shift)     SKP_SUB32((a), SKP_LSHIFT32((b), (shift)))	// shift >= 0
#define SKP_SUB_RSHIFT32(a, b, shift)     SKP_SUB32((a), SKP_RSHIFT32((b), (shift)))	// shift >= 0

/* Requires that shift > 0 */
#define SKP_RSHIFT_ROUND(a, shift)        ((shift) == 1 ? ((a) >> 1) + ((a) & 1) : (((a) >> ((shift) - 1)) + 1) >> 1)
#define SKP_RSHIFT_ROUND64(a, shift)      ((shift) == 1 ? ((a) >> 1) + ((a) & 1) : (((a) >> ((shift) - 1)) + 1) >> 1)

/* Number of rightshift required to fit the multiplication */
#define SKP_NSHIFT_MUL_32_32(a, b)        ( -(31- (32-SKP_Silk_CLZ32(SKP_abs(a)) + (32-SKP_Silk_CLZ32(SKP_abs(b))))) )
#define SKP_NSHIFT_MUL_16_16(a, b)        ( -(15- (16-SKP_Silk_CLZ16(SKP_abs(a)) + (16-SKP_Silk_CLZ16(SKP_abs(b))))) )

#define SKP_min(a, b)                     (((a) < (b)) ? (a) : (b))
#define SKP_max(a, b)                     (((a) > (b)) ? (a) : (b))

/* Macro to convert floating-point constants to fixed-point */
#define SKP_FIX_CONST( C, Q )             ((int32_t)((C) * (1 << (Q)) + 0.5))

/* SKP_min() versions with typecast in the function call */
	SKP_INLINE int SKP_min_int(int a, int b) {
		return (((a) < (b)) ? (a) : (b));
	} SKP_INLINE int16_t SKP_min_16(int16_t a, int16_t b) {
		return (((a) < (b)) ? (a) : (b));
	}
	SKP_INLINE int32_t SKP_min_32(int32_t a, int32_t b) {
		return (((a) < (b)) ? (a) : (b));
	}
	SKP_INLINE int64_t SKP_min_64(int64_t a, int64_t b) {
		return (((a) < (b)) ? (a) : (b));
	}

/* SKP_min() versions with typecast in the function call */
	SKP_INLINE int SKP_max_int(int a, int b) {
		return (((a) > (b)) ? (a) : (b));
	}
	SKP_INLINE int16_t SKP_max_16(int16_t a, int16_t b) {
		return (((a) > (b)) ? (a) : (b));
	}
	SKP_INLINE int32_t SKP_max_32(int32_t a, int32_t b) {
		return (((a) > (b)) ? (a) : (b));
	}
	SKP_INLINE int64_t SKP_max_64(int64_t a, int64_t b) {
		return (((a) > (b)) ? (a) : (b));
	}

#define SKP_LIMIT( a, limit1, limit2)    ((limit1) > (limit2) ? ((a) > (limit1) ? (limit1) : ((a) < (limit2) ? (limit2) : (a))) \
                                                             : ((a) > (limit2) ? (limit2) : ((a) < (limit1) ? (limit1) : (a))))

//#define SKP_non_neg(a)                 ((a) & ((-(a)) >> (8 * sizeof(a) - 1)))   /* doesn't seem faster than SKP_max(0, a);

#define SKP_abs(a)                       (((a) >  0)  ? (a) : -(a))	// Be careful, SKP_abs returns wrong when input equals to intXX_MIN
#define SKP_abs_int(a)                   (((a) ^ ((a) >> (8 * sizeof(a) - 1))) - ((a) >> (8 * sizeof(a) - 1)))
#define SKP_abs_int32(a)                 (((a) ^ ((a) >> 31)) - ((a) >> 31))
#define SKP_abs_int64(a)                 (((a) >  0)  ? (a) : -(a))

#define SKP_sign(a)                      ((a) > 0 ? 1 : ( (a) < 0 ? -1 : 0 ))

#define SKP_sqrt(a)                      (sqrt(a))

/* PSEUDO-RANDOM GENERATOR                                                          */
/* Make sure to store the result as the seed for the next call (also in between     */
/* frames), otherwise result won't be random at all. When only using some of the    */
/* bits, take the most significant bits by right-shifting. Do not just mask off     */
/* the lowest bits.                                                                 */
#define SKP_RAND(seed)                   (SKP_MLA_ovflw(907633515, (seed), 196314165))

// Add some multiplication functions that can be easily mapped to ARM.

//    SKP_SMMUL: Signed top word multiply. 
//        ARMv6        2 instruction cycles. 
//        ARMv3M+        3 instruction cycles. use SMULL and ignore LSB registers.(except xM) 
//#define SKP_SMMUL(a32, b32)            (int32_t)SKP_RSHIFT(SKP_SMLAL(SKP_SMULWB((a32), (b32)), (a32), SKP_RSHIFT_ROUND((b32), 16)), 16)
// the following seems faster on x86
#define SKP_SMMUL(a32, b32)              (int32_t)SKP_RSHIFT64(SKP_SMULL((a32), (b32)), 32)

#include "SKP_Silk_Inlines.h"

#ifdef  __cplusplus
}
#endif

#endif

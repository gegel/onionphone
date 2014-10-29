/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/***********************************************************************
Copyright (c) 2006-2011, Skype Limited. All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
- Neither the name of Internet Society, IETF or IETF Trust, nor the
names of specific contributors, may be used to endorse or promote
products derived from this software without specific prior written
permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

#ifndef SILK_SIGPROC_FIX_H
#define SILK_SIGPROC_FIX_H

#ifdef  __cplusplus
extern "C" {
#endif

	/*#define silk_MACRO_COUNT *//* Used to enable WMOPS counting */

#define SILK_MAX_ORDER_LPC            16	/* max order of the LPC analysis in schur() and k2a() */

#include <string.h>		/* for memset(), memcpy(), memmove() */
#include "typedef.h"
#include "resampler_structs.h"
#include "macros.h"

/********************************************************************/
/*                    SIGNAL PROCESSING FUNCTIONS                   */
/********************************************************************/

/*!
 * Initialize/reset the resampler state for a given pair of input/output sampling rates
*/
	int silk_resampler_init(silk_resampler_state_struct * S,	/* I/O  Resampler state                                             */
				     int32_t Fs_Hz_in,	/* I    Input sampling rate (Hz)                                    */
				     int32_t Fs_Hz_out,	/* I    Output sampling rate (Hz)                                   */
				     int forEnc	/* I    If 1: encoder; if 0: decoder                                */
	    );

/*!
 * Resampler: convert from one sampling rate to another
 */
	int silk_resampler(silk_resampler_state_struct * S,	/* I/O  Resampler state                                             */
				int16_t out[],	/* O    Output signal                                               */
				const int16_t in[],	/* I    Input signal                                                */
				int32_t inLen	/* I    Number of input samples                                     */
	    );

/*!
* Downsample 2x, mediocre quality
*/
	void silk_resampler_down2(int32_t * S,	/* I/O  State vector [ 2 ]                                          */
				  int16_t * out,	/* O    Output signal [ len ]                                       */
				  const int16_t * in,	/* I    Input signal [ floor(len/2) ]                               */
				  int32_t inLen	/* I    Number of input samples                                     */
	    );

/*!
 * Downsample by a factor 2/3, low quality
*/
	void silk_resampler_down2_3(int32_t * S,	/* I/O  State vector [ 6 ]                                          */
				    int16_t * out,	/* O    Output signal [ floor(2*inLen/3) ]                          */
				    const int16_t * in,	/* I    Input signal [ inLen ]                                      */
				    int32_t inLen	/* I    Number of input samples                                     */
	    );

/*!
 * second order ARMA filter;
 * slower than biquad() but uses more precise coefficients
 * can handle (slowly) varying coefficients
 */
	void silk_biquad_alt(const int16_t * in,	/* I     input signal                                               */
			     const int32_t * B_Q28,	/* I     MA coefficients [3]                                        */
			     const int32_t * A_Q28,	/* I     AR coefficients [2]                                        */
			     int32_t * S,	/* I/O   State vector [2]                                           */
			     int16_t * out,	/* O     output signal                                              */
			     const int32_t len,	/* I     signal length (must be even)                               */
			     int stride	/* I     Operate on interleaved signal if > 1                       */
	    );

/* Variable order MA prediction error filter. */
	void silk_LPC_analysis_filter(int16_t * out,	/* O    Output signal                                               */
				      const int16_t * in,	/* I    Input signal                                                */
				      const int16_t * B,	/* I    MA prediction coefficients, Q12 [order]                     */
				      const int32_t len,	/* I    Signal length                                               */
				      const int32_t d	/* I    Filter order                                                */
	    );

/* Chirp (bandwidth expand) LP AR filter */
	void silk_bwexpander(int16_t * ar,	/* I/O  AR filter to be expanded (without leading 1)                */
			     const int d,	/* I    Length of ar                                                */
			     int32_t chirp_Q16	/* I    Chirp factor (typically in the range 0 to 1)                */
	    );

/* Chirp (bandwidth expand) LP AR filter */
	void silk_bwexpander_32(int32_t * ar,	/* I/O  AR filter to be expanded (without leading 1)                */
				const int d,	/* I    Length of ar                                                */
				int32_t chirp_Q16	/* I    Chirp factor in Q16                                         */
	    );

/* Compute inverse of LPC prediction gain, and                           */
/* test if LPC coefficients are stable (all poles within unit circle)    */
	int32_t silk_LPC_inverse_pred_gain(	/* O   Returns inverse prediction gain in energy domain, Q30        */
						     const int16_t * A_Q12,	/* I   Prediction coefficients, Q12 [order]                         */
						     const int order	/* I   Prediction order                                             */
	    );

/* For input in Q24 domain */
	int32_t silk_LPC_inverse_pred_gain_Q24(	/* O    Returns inverse prediction gain in energy domain, Q30       */
							 const int32_t * A_Q24,	/* I    Prediction coefficients [order]                             */
							 const int order	/* I    Prediction order                                            */
	    );

/* Split signal in two decimated bands using first-order allpass filters */
	void silk_ana_filt_bank_1(const int16_t * in,	/* I    Input signal [N]                                            */
				  int32_t * S,	/* I/O  State vector [2]                                            */
				  int16_t * outL,	/* O    Low band [N/2]                                              */
				  int16_t * outH,	/* O    High band [N/2]                                             */
				  const int32_t N	/* I    Number of input samples                                     */
	    );

/********************************************************************/
/*                        SCALAR FUNCTIONS                          */
/********************************************************************/

/* Approximation of 128 * log2() (exact inverse of approx 2^() below) */
/* Convert input to a log scale    */
	int32_t silk_lin2log(const int32_t inLin	/* I  input in linear scale                                         */
	    );

/* Approximation of a sigmoid function */
	int silk_sigm_Q15(int in_Q5	/* I                                                                */
	    );

/* Approximation of 2^() (exact inverse of approx log2() above) */
/* Convert input to a linear scale */
	int32_t silk_log2lin(const int32_t inLog_Q7	/* I  input on log scale                                            */
	    );

/* Compute number of bits to right shift the sum of squares of a vector    */
/* of int16s to make it fit in an int32                                    */
	void silk_sum_sqr_shift(int32_t * energy,	/* O   Energy of x, after shifting to the right                     */
				int * shift,	/* O   Number of bits right shift applied to energy                 */
				const int16_t * x,	/* I   Input vector                                                 */
				int len	/* I   Length of input vector                                       */
	    );

/* Calculates the reflection coefficients from the correlation sequence    */
/* Faster than schur64(), but much less accurate.                          */
/* uses SMLAWB(), requiring armv5E and higher.                             */
	int32_t silk_schur(	/* O    Returns residual energy                                     */
				     int16_t * rc_Q15,	/* O    reflection coefficients [order] Q15                         */
				     const int32_t * c,	/* I    correlations [order+1]                                      */
				     const int32_t order	/* I    prediction order                                            */
	    );

/* Calculates the reflection coefficients from the correlation sequence    */
/* Slower than schur(), but more accurate.                                 */
/* Uses SMULL(), available on armv4                                        */
	int32_t silk_schur64(	/* O    returns residual energy                                     */
				       int32_t rc_Q16[],	/* O    Reflection coefficients [order] Q16                         */
				       const int32_t c[],	/* I    Correlations [order+1]                                      */
				       int32_t order	/* I    Prediction order                                            */
	    );

/* Step up function, converts reflection coefficients to prediction coefficients */
	void silk_k2a(int32_t * A_Q24,	/* O    Prediction coefficients [order] Q24                         */
		      const int16_t * rc_Q15,	/* I    Reflection coefficients [order] Q15                         */
		      const int32_t order	/* I    Prediction order                                            */
	    );

/* Step up function, converts reflection coefficients to prediction coefficients */
	void silk_k2a_Q16(int32_t * A_Q24,	/* O    Prediction coefficients [order] Q24                         */
			  const int32_t * rc_Q16,	/* I    Reflection coefficients [order] Q16                         */
			  const int32_t order	/* I    Prediction order                                            */
	    );

/* Apply sine window to signal vector.                              */
/* Window types:                                                    */
/*    1 -> sine window from 0 to pi/2                               */
/*    2 -> sine window from pi/2 to pi                              */
/* every other sample of window is linearly interpolated, for speed */
	void silk_apply_sine_window(int16_t px_win[],	/* O    Pointer to windowed signal                                  */
				    const int16_t px[],	/* I    Pointer to input signal                                     */
				    const int win_type,	/* I    Selects a window type                                       */
				    const int length	/* I    Window length, multiple of 4                                */
	    );

/* Compute autocorrelation */
	void silk_autocorr(int32_t * results,	/* O    Result (length correlationCount)                            */
			   int * scale,	/* O    Scaling of the correlation vector                           */
			   const int16_t * inputData,	/* I    Input data to correlate                                     */
			   const int inputDataSize,	/* I    Length of input                                             */
			   const int correlationCount,	/* I    Number of correlation taps to compute                       */
			   int arch	/* I    Run-time architecture                                       */
	    );

	void silk_decode_pitch(int16_t lagIndex,	/* I                                                                */
			       int8_t contourIndex,	/* O                                                                */
			       int pitch_lags[],	/* O    4 pitch values                                              */
			       const int Fs_kHz,	/* I    sampling frequency (kHz)                                    */
			       const int nb_subfr	/* I    number of sub frames                                        */
	    );

	int silk_pitch_analysis_core(	/* O    Voicing estimate: 0 voiced, 1 unvoiced                      */
						 const int16_t * frame,	/* I    Signal of length PE_FRAME_LENGTH_MS*Fs_kHz                  */
						 int * pitch_out,	/* O    4 pitch lag values                                          */
						 int16_t * lagIndex,	/* O    Lag Index                                                   */
						 int8_t * contourIndex,	/* O    Pitch contour Index                                         */
						 int * LTPCorr_Q15,	/* I/O  Normalized correlation; input: value from previous frame    */
						 int prevLag,	/* I    Last lag of previous frame; set to zero is unvoiced         */
						 const int32_t search_thres1_Q16,	/* I    First stage threshold for lag candidates 0 - 1              */
						 const int search_thres2_Q13,	/* I    Final threshold for lag candidates 0 - 1                    */
						 const int Fs_kHz,	/* I    Sample frequency (kHz)                                      */
						 const int complexity,	/* I    Complexity setting, 0-2, where 2 is highest                 */
						 const int nb_subfr,	/* I    number of 5 ms subframes                                    */
						 int arch	/* I    Run-time architecture                                       */
	    );

/* Compute Normalized Line Spectral Frequencies (NLSFs) from whitening filter coefficients      */
/* If not all roots are found, the a_Q16 coefficients are bandwidth expanded until convergence. */
	void silk_A2NLSF(int16_t * NLSF,	/* O    Normalized Line Spectral Frequencies in Q15 (0..2^15-1) [d] */
			 int32_t * a_Q16,	/* I/O  Monic whitening filter coefficients in Q16 [d]              */
			 const int d	/* I    Filter order (must be even)                                 */
	    );

/* compute whitening filter coefficients from normalized line spectral frequencies */
	void silk_NLSF2A(int16_t * a_Q12,	/* O    monic whitening filter coefficients in Q12,  [ d ]          */
			 const int16_t * NLSF,	/* I    normalized line spectral frequencies in Q15, [ d ]          */
			 const int d	/* I    filter order (should be even)                               */
	    );

	void silk_insertion_sort_increasing(int32_t * a,	/* I/O   Unsorted / Sorted vector                                   */
					    int * idx,	/* O     Index vector for the sorted elements                       */
					    const int L,	/* I     Vector length                                              */
					    const int K	/* I     Number of correctly sorted positions                       */
	    );

	void silk_insertion_sort_decreasing_int16(int16_t * a,	/* I/O   Unsorted / Sorted vector                                   */
						  int * idx,	/* O     Index vector for the sorted elements                       */
						  const int L,	/* I     Vector length                                              */
						  const int K	/* I     Number of correctly sorted positions                       */
	    );

	void silk_insertion_sort_increasing_all_values_int16(int16_t * a,	/* I/O   Unsorted / Sorted vector                                   */
							     const int L	/* I     Vector length                                              */
	    );

/* NLSF stabilizer, for a single input data vector */
	void silk_NLSF_stabilize(int16_t * NLSF_Q15,	/* I/O   Unstable/stabilized normalized LSF vector in Q15 [L]       */
				 const int16_t * NDeltaMin_Q15,	/* I     Min distance vector, NDeltaMin_Q15[L] must be >= 1 [L+1]   */
				 const int L	/* I     Number of NLSF parameters in the input vector              */
	    );

/* Laroia low complexity NLSF weights */
	void silk_NLSF_VQ_weights_laroia(int16_t * pNLSFW_Q_OUT,	/* O     Pointer to input vector weights [D]                        */
					 const int16_t * pNLSF_Q15,	/* I     Pointer to input vector         [D]                        */
					 const int D	/* I     Input vector dimension (even)                              */
	    );

/* Compute reflection coefficients from input signal */
	void silk_burg_modified(int32_t * res_nrg,	/* O    Residual energy                                             */
				int * res_nrg_Q,	/* O    Residual energy Q value                                     */
				int32_t A_Q16[],	/* O    Prediction coefficients (length order)                      */
				const int16_t x[],	/* I    Input signal, length: nb_subfr * ( D + subfr_length )       */
				const int32_t minInvGain_Q30,	/* I    Inverse of max prediction gain                              */
				const int subfr_length,	/* I    Input signal subframe length (incl. D preceding samples)    */
				const int nb_subfr,	/* I    Number of subframes stacked in x                            */
				const int D,	/* I    Order                                                       */
				int arch	/* I    Run-time architecture                                       */
	    );

/* Copy and multiply a vector by a constant */
	void silk_scale_copy_vector16(int16_t * data_out, const int16_t * data_in, int32_t gain_Q16,	/* I    Gain in Q16                                                 */
				      const int dataSize	/* I    Length                                                      */
	    );

/* Some for the LTP related function requires Q26 to work.*/
	void silk_scale_vector32_Q26_lshift_18(int32_t * data1,	/* I/O  Q0/Q18                                                      */
					       int32_t gain_Q26,	/* I    Q26                                                         */
					       int dataSize	/* I    length                                                      */
	    );

/********************************************************************/
/*                        INLINE ARM MATH                           */
/********************************************************************/

/*    return sum( inVec1[i] * inVec2[i] ) */
	int32_t silk_inner_prod_aligned(const int16_t * const inVec1,	/*    I input vector 1                                              */
					   const int16_t * const inVec2,	/*    I input vector 2                                              */
					   const int len	/*    I vector lengths                                              */
	    );

	int32_t silk_inner_prod_aligned_scale(const int16_t * const inVec1,	/*    I input vector 1                                              */
						 const int16_t * const inVec2,	/*    I input vector 2                                              */
						 const int scale,	/*    I number of bits to shift                                     */
						 const int len	/*    I vector lengths                                              */
	    );

	int64_t silk_inner_prod16_aligned_64(const int16_t * inVec1,	/*    I input vector 1                                              */
						const int16_t * inVec2,	/*    I input vector 2                                              */
						const int len	/*    I vector lengths                                              */
	    );

/********************************************************************/
/*                                MACROS                            */
/********************************************************************/

/* Rotate a32 right by 'rot' bits. Negative rot values result in rotating
   left. Output is 32bit int.
   Note: contemporary compilers recognize the C expression below and
   compile it into a 'ror' instruction if available. No need for inline ASM! */
	static inline int32_t silk_ROR32(int32_t a32, int rot) {
		uint32_t x = (uint32_t) a32;
		uint32_t r = (uint32_t) rot;
		uint32_t m = (uint32_t) - rot;
		if (rot == 0) {
			return a32;
		} else if (rot < 0) {
			return (int32_t) ((x << m) | (x >> (32 - m)));
		} else {
			return (int32_t) ((x << (32 - r)) | (x >> r));
		}
	}

/* Allocate int16_t aligned to 4-byte memory address */
#if EMBEDDED_ARM
#define silk_DWORD_ALIGN __attribute__((aligned(4)))
#else
#define silk_DWORD_ALIGN
#endif

/* Useful Macros that can be adjusted to other platforms */
#define silk_memcpy(dest, src, size)        memcpy((dest), (src), (size))
#define silk_memmove(dest, src, size)       memmove((dest), (src), (size))

/* Fixed point macros */

/* (a32 * b32) output have to be 32bit int */
#define silk_MUL(a32, b32)                  ((a32) * (b32))

/* (a32 * b32) output have to be 32bit uint */
#define silk_MUL_uint(a32, b32)             silk_MUL(a32, b32)

/* a32 + (b32 * c32) output have to be 32bit int */
#define silk_MLA(a32, b32, c32)             silk_ADD32((a32),((b32) * (c32)))

/* a32 + (b32 * c32) output have to be 32bit uint */
#define silk_MLA_uint(a32, b32, c32)        silk_MLA(a32, b32, c32)

/* ((a32 >> 16)  * (b32 >> 16)) output have to be 32bit int */
#define silk_SMULTT(a32, b32)               (((a32) >> 16) * ((b32) >> 16))

/* a32 + ((a32 >> 16)  * (b32 >> 16)) output have to be 32bit int */
#define silk_SMLATT(a32, b32, c32)          silk_ADD32((a32),((b32) >> 16) * ((c32) >> 16))

#define silk_SMLALBB(a64, b16, c16)         silk_ADD64((a64),(int64_t)((int32_t)(b16) * (int32_t)(c16)))

/* (a32 * b32) */
#define silk_SMULL(a32, b32)                ((int64_t)(a32) * /*(int64_t)*/(b32))

/* Adds two signed 32-bit values in a way that can overflow, while not relying on undefined behaviour
   (just standard two's complement implementation-specific behaviour) */
#define silk_ADD32_ovflw(a, b)              ((int32_t)((uint32_t)(a) + (uint32_t)(b)))
/* Subtractss two signed 32-bit values in a way that can overflow, while not relying on undefined behaviour
   (just standard two's complement implementation-specific behaviour) */
#define silk_SUB32_ovflw(a, b)              ((int32_t)((uint32_t)(a) - (uint32_t)(b)))

/* Multiply-accumulate macros that allow overflow in the addition (ie, no asserts in debug mode) */
#define silk_MLA_ovflw(a32, b32, c32)       silk_ADD32_ovflw((a32), (uint32_t)(b32) * (uint32_t)(c32))
#define silk_SMLABB_ovflw(a32, b32, c32)    (silk_ADD32_ovflw((a32) , ((int32_t)((int16_t)(b32))) * (int32_t)((int16_t)(c32))))

#define silk_DIV32_16(a32, b16)             ((int32_t)((a32) / (b16)))
#define silk_DIV32(a32, b32)                ((int32_t)((a32) / (b32)))

/* These macros enables checking for overflow in silk_API_Debug.h*/
#define silk_ADD16(a, b)                    ((a) + (b))
#define silk_ADD32(a, b)                    ((a) + (b))
#define silk_ADD64(a, b)                    ((a) + (b))

#define silk_SUB16(a, b)                    ((a) - (b))
#define silk_SUB32(a, b)                    ((a) - (b))
#define silk_SUB64(a, b)                    ((a) - (b))

#define silk_SAT8(a)                        ((a) > silk_int8_MAX ? silk_int8_MAX  :       \
                                            ((a) < silk_int8_MIN ? silk_int8_MIN  : (a)))
#define silk_SAT16(a)                       ((a) > silk_int16_MAX ? silk_int16_MAX :      \
                                            ((a) < silk_int16_MIN ? silk_int16_MIN : (a)))
#define silk_SAT32(a)                       ((a) > silk_int32_MAX ? silk_int32_MAX :      \
                                            ((a) < silk_int32_MIN ? silk_int32_MIN : (a)))

#define silk_CHECK_FIT8(a)                  (a)
#define silk_CHECK_FIT16(a)                 (a)
#define silk_CHECK_FIT32(a)                 (a)

#define silk_ADD_SAT16(a, b)                (int16_t)silk_SAT16( silk_ADD32( (int32_t)(a), (b) ) )
#define silk_ADD_SAT64(a, b)                ((((a) + (b)) & 0x8000000000000000LL) == 0 ?                            \
                                            ((((a) & (b)) & 0x8000000000000000LL) != 0 ? silk_int64_MIN : (a)+(b)) : \
                                            ((((a) | (b)) & 0x8000000000000000LL) == 0 ? silk_int64_MAX : (a)+(b)) )

#define silk_SUB_SAT16(a, b)                (int16_t)silk_SAT16( silk_SUB32( (int32_t)(a), (b) ) )
#define silk_SUB_SAT64(a, b)                ((((a)-(b)) & 0x8000000000000000LL) == 0 ?                                               \
                                            (( (a) & ((b)^0x8000000000000000LL) & 0x8000000000000000LL) ? silk_int64_MIN : (a)-(b)) : \
                                            ((((a)^0x8000000000000000LL) & (b)  & 0x8000000000000000LL) ? silk_int64_MAX : (a)-(b)) )

/* Saturation for positive input values */
#define silk_POS_SAT32(a)                   ((a) > silk_int32_MAX ? silk_int32_MAX : (a))

/* Add with saturation for positive input values */
#define silk_ADD_POS_SAT8(a, b)             ((((a)+(b)) & 0x80)                 ? silk_int8_MAX  : ((a)+(b)))
#define silk_ADD_POS_SAT16(a, b)            ((((a)+(b)) & 0x8000)               ? silk_int16_MAX : ((a)+(b)))
#define silk_ADD_POS_SAT32(a, b)            ((((a)+(b)) & 0x80000000)           ? silk_int32_MAX : ((a)+(b)))
#define silk_ADD_POS_SAT64(a, b)            ((((a)+(b)) & 0x8000000000000000LL) ? silk_int64_MAX : ((a)+(b)))

#define silk_LSHIFT8(a, shift)              ((int8_t)((uint8_t)(a)<<(shift)))	/* shift >= 0, shift < 8  */
#define silk_LSHIFT16(a, shift)             ((int16_t)((uint16_t)(a)<<(shift)))	/* shift >= 0, shift < 16 */
#define silk_LSHIFT32(a, shift)             ((int32_t)((uint32_t)(a)<<(shift)))	/* shift >= 0, shift < 32 */
#define silk_LSHIFT64(a, shift)             ((int64_t)((uint64_t)(a)<<(shift)))	/* shift >= 0, shift < 64 */
#define silk_LSHIFT(a, shift)               silk_LSHIFT32(a, shift)	/* shift >= 0, shift < 32 */

#define silk_RSHIFT8(a, shift)              ((a)>>(shift))	/* shift >= 0, shift < 8  */
#define silk_RSHIFT16(a, shift)             ((a)>>(shift))	/* shift >= 0, shift < 16 */
#define silk_RSHIFT32(a, shift)             ((a)>>(shift))	/* shift >= 0, shift < 32 */
#define silk_RSHIFT64(a, shift)             ((a)>>(shift))	/* shift >= 0, shift < 64 */
#define silk_RSHIFT(a, shift)               silk_RSHIFT32(a, shift)	/* shift >= 0, shift < 32 */

/* saturates before shifting */
#define silk_LSHIFT_SAT32(a, shift)         (silk_LSHIFT32( silk_LIMIT( (a), silk_RSHIFT32( silk_int32_MIN, (shift) ), \
                                                    silk_RSHIFT32( silk_int32_MAX, (shift) ) ), (shift) ))

#define silk_LSHIFT_ovflw(a, shift)         ((int32_t)((uint32_t)(a) << (shift)))	/* shift >= 0, allowed to overflow */
#define silk_LSHIFT_uint(a, shift)          ((a) << (shift))	/* shift >= 0 */
#define silk_RSHIFT_uint(a, shift)          ((a) >> (shift))	/* shift >= 0 */

#define silk_ADD_LSHIFT(a, b, shift)        ((a) + silk_LSHIFT((b), (shift)))	/* shift >= 0 */
#define silk_ADD_LSHIFT32(a, b, shift)      silk_ADD32((a), silk_LSHIFT32((b), (shift)))	/* shift >= 0 */
#define silk_ADD_LSHIFT_uint(a, b, shift)   ((a) + silk_LSHIFT_uint((b), (shift)))	/* shift >= 0 */
#define silk_ADD_RSHIFT(a, b, shift)        ((a) + silk_RSHIFT((b), (shift)))	/* shift >= 0 */
#define silk_ADD_RSHIFT32(a, b, shift)      silk_ADD32((a), silk_RSHIFT32((b), (shift)))	/* shift >= 0 */
#define silk_ADD_RSHIFT_uint(a, b, shift)   ((a) + silk_RSHIFT_uint((b), (shift)))	/* shift >= 0 */
#define silk_SUB_LSHIFT32(a, b, shift)      silk_SUB32((a), silk_LSHIFT32((b), (shift)))	/* shift >= 0 */
#define silk_SUB_RSHIFT32(a, b, shift)      silk_SUB32((a), silk_RSHIFT32((b), (shift)))	/* shift >= 0 */

/* Requires that shift > 0 */
#define silk_RSHIFT_ROUND(a, shift)         ((shift) == 1 ? ((a) >> 1) + ((a) & 1) : (((a) >> ((shift) - 1)) + 1) >> 1)
#define silk_RSHIFT_ROUND64(a, shift)       ((shift) == 1 ? ((a) >> 1) + ((a) & 1) : (((a) >> ((shift) - 1)) + 1) >> 1)

/* Number of rightshift required to fit the multiplication */
#define silk_NSHIFT_MUL_32_32(a, b)         ( -(31- (32-silk_CLZ32(silk_abs(a)) + (32-silk_CLZ32(silk_abs(b))))) )
#define silk_NSHIFT_MUL_16_16(a, b)         ( -(15- (16-silk_CLZ16(silk_abs(a)) + (16-silk_CLZ16(silk_abs(b))))) )

#define silk_min(a, b)                      (((a) < (b)) ? (a) : (b))
#define silk_max(a, b)                      (((a) > (b)) ? (a) : (b))

/* Macro to convert floating-point constants to fixed-point */
#define SILK_FIX_CONST( C, Q )              ((int32_t)((C) * ((int64_t)1 << (Q)) + 0.5))

/* silk_min() versions with typecast in the function call */
	static inline int silk_min_int(int a, int b) {
		return (((a) < (b)) ? (a) : (b));
	}
	static inline int16_t silk_min_16(int16_t a, int16_t b) {
		return (((a) < (b)) ? (a) : (b));
	}
	static inline int32_t silk_min_32(int32_t a, int32_t b) {
		return (((a) < (b)) ? (a) : (b));
	}
	static inline int64_t silk_min_64(int64_t a, int64_t b) {
		return (((a) < (b)) ? (a) : (b));
	}

/* silk_min() versions with typecast in the function call */
	static inline int silk_max_int(int a, int b) {
		return (((a) > (b)) ? (a) : (b));
	}
	static inline int16_t silk_max_16(int16_t a, int16_t b) {
		return (((a) > (b)) ? (a) : (b));
	}
	static inline int32_t silk_max_32(int32_t a, int32_t b) {
		return (((a) > (b)) ? (a) : (b));
	}
	static inline int64_t silk_max_64(int64_t a, int64_t b) {
		return (((a) > (b)) ? (a) : (b));
	}

#define silk_LIMIT( a, limit1, limit2)      ((limit1) > (limit2) ? ((a) > (limit1) ? (limit1) : ((a) < (limit2) ? (limit2) : (a))) \
                                                                 : ((a) > (limit2) ? (limit2) : ((a) < (limit1) ? (limit1) : (a))))

#define silk_LIMIT_int                      silk_LIMIT
#define silk_LIMIT_16                       silk_LIMIT
#define silk_LIMIT_32                       silk_LIMIT

#define silk_abs(a)                         (((a) >  0)  ? (a) : -(a))	/* Be careful, silk_abs returns wrong when input equals to silk_intXX_MIN */
#define silk_abs_int(a)                     (((a) ^ ((a) >> (8 * sizeof(a) - 1))) - ((a) >> (8 * sizeof(a) - 1)))
#define silk_abs_int32(a)                   (((a) ^ ((a) >> 31)) - ((a) >> 31))
#define silk_abs_int64(a)                   (((a) >  0)  ? (a) : -(a))

#define silk_sign(a)                        ((a) > 0 ? 1 : ( (a) < 0 ? -1 : 0 ))

/* PSEUDO-RANDOM GENERATOR                                                          */
/* Make sure to store the result as the seed for the next call (also in between     */
/* frames), otherwise result won't be random at all. When only using some of the    */
/* bits, take the most significant bits by right-shifting.                          */
#define silk_RAND(seed)                     (silk_MLA_ovflw(907633515, (seed), 196314165))

/*  Add some multiplication functions that can be easily mapped to ARM. */

/*    silk_SMMUL: Signed top word multiply.
          ARMv6        2 instruction cycles.
          ARMv3M+      3 instruction cycles. use SMULL and ignore LSB registers.(except xM)*/
/*#define silk_SMMUL(a32, b32)                (int32_t)silk_RSHIFT(silk_SMLAL(silk_SMULWB((a32), (b32)), (a32), silk_RSHIFT_ROUND((b32), 16)), 16)*/
/* the following seems faster on x86 */
#define silk_SMMUL(a32, b32)                (int32_t)silk_RSHIFT64(silk_SMULL((a32), (b32)), 32)

#include "Inlines.h"
#include "MacroCount.h"
#include "MacroDebug.h"

#ifdef OPUS_ARM_INLINE_ASM
#include "arm/SigProc_FIX_armv4.h"
#endif

#ifdef OPUS_ARM_INLINE_EDSP
#include "arm/SigProc_FIX_armv5e.h"
#endif

#ifdef  __cplusplus
}
#endif

#endif				/* SILK_SIGPROC_FIX_H */

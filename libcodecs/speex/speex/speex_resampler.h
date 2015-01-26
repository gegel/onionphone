/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* Copyright (C) 2007 Jean-Marc Valin
      
   File: speex_resampler.h
   Resampling code
      
   The design goals of this code are:
      - Very fast algorithm
      - Low memory requirement
      - Good *perceptual* quality (and not best SNR)

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

   1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SPEEX_RESAMPLER_H
#define SPEEX_RESAMPLER_H

#ifdef OUTSIDE_SPEEX

/********* WARNING: MENTAL SANITY ENDS HERE *************/

/* If the resampler is defined outside of Speex, we change the symbol names so that 
   there won't be any clash if linking with Speex later on. */

/* #define RANDOM_PREFIX your software name here */
#ifndef RANDOM_PREFIX
#error "Please define RANDOM_PREFIX (above) to something specific to your project to prevent symbol name clashes"
#endif

#define CAT_PREFIX2(a,b) a ## b
#define CAT_PREFIX(a,b) CAT_PREFIX2(a, b)

#define speex_resampler_init CAT_PREFIX(RANDOM_PREFIX,_resampler_init)
#define speex_resampler_init_frac CAT_PREFIX(RANDOM_PREFIX,_resampler_init_frac)
#define speex_resampler_destroy CAT_PREFIX(RANDOM_PREFIX,_resampler_destroy)
#define speex_resampler_process_int CAT_PREFIX(RANDOM_PREFIX,_resampler_process_int)
#define speex_resampler_set_rate CAT_PREFIX(RANDOM_PREFIX,_resampler_set_rate)
#define speex_resampler_set_rate_frac CAT_PREFIX(RANDOM_PREFIX,_resampler_set_rate_frac)
#define speex_resampler_set_quality CAT_PREFIX(RANDOM_PREFIX,_resampler_set_quality)

#define int16_t short
#define int32_t int
#define uint16_t unsigned short
#define uint32_t unsigned int

#else				/* OUTSIDE_SPEEX */

#include <stdint.h>

#endif				/* OUTSIDE_SPEEX */

#ifdef __cplusplus
extern "C" {
#endif

#define SPEEX_RESAMPLER_QUALITY_MAX 10
#define SPEEX_RESAMPLER_QUALITY_MIN 0
#define SPEEX_RESAMPLER_QUALITY_DEFAULT 4
#define SPEEX_RESAMPLER_QUALITY_VOIP 3
#define SPEEX_RESAMPLER_QUALITY_DESKTOP 5

	enum {
		RESAMPLER_ERR_SUCCESS = 0,
		RESAMPLER_ERR_ALLOC_FAILED = 1,
		RESAMPLER_ERR_BAD_STATE = 2,
		RESAMPLER_ERR_INVALID_ARG = 3,
		RESAMPLER_ERR_PTR_OVERLAP = 4,

		RESAMPLER_ERR_MAX_ERROR
	};

	struct SpeexResamplerState_;
	typedef struct SpeexResamplerState_ SpeexResamplerState;

/** Create a new resampler with integer input and output rates.
 * @param nb_channels Number of channels to be processed
 * @param in_rate Input sampling rate (integer number of Hz).
 * @param out_rate Output sampling rate (integer number of Hz).
 * @param quality Resampling quality between 0 and 10, where 0 has poor quality
 * and 10 has very high quality.
 * @return Newly created resampler state
 * @retval NULL Error: not enough memory
 */
	SpeexResamplerState *speex_resampler_init(uint32_t nb_channels,
						  uint32_t in_rate,
						  uint32_t out_rate,
						  int quality, int *err);

/** Create a new resampler with fractional input/output rates. The sampling 
 * rate ratio is an arbitrary rational number with both the numerator and 
 * denominator being 32-bit integers.
 * @param nb_channels Number of channels to be processed
 * @param ratio_num Numerator of the sampling rate ratio
 * @param ratio_den Denominator of the sampling rate ratio
 * @param in_rate Input sampling rate rounded to the nearest integer (in Hz).
 * @param out_rate Output sampling rate rounded to the nearest integer (in Hz).
 * @param quality Resampling quality between 0 and 10, where 0 has poor quality
 * and 10 has very high quality.
 * @return Newly created resampler state
 * @retval NULL Error: not enough memory
 */
	SpeexResamplerState *speex_resampler_init_frac(uint32_t nb_channels,
						       uint32_t ratio_num,
						       uint32_t ratio_den,
						       uint32_t in_rate,
						       uint32_t out_rate,
						       int quality, int *err);

/** Destroy a resampler state.
 * @param st Resampler state
 */
	void speex_resampler_destroy(SpeexResamplerState * st);

/** Resample an int array. The input and output buffers must *not* overlap.
 * @param st Resampler state
 * @param channel_index Index of the channel to process for the multi-channel 
 * base (0 otherwise)
 * @param in Input buffer
 * @param in_len Number of input samples in the input buffer. Returns the number
 * of samples processed
 * @param out Output buffer
 * @param out_len Size of the output buffer. Returns the number of samples written
 */
	int speex_resampler_process_int(SpeexResamplerState * st,
					uint32_t channel_index,
					const int16_t * in,
					uint32_t * in_len,
					int16_t * out,
					uint32_t * out_len);

/** Resample an interleaved float array. The input and output buffers must *not* overlap.
 * @param st Resampler state
 * @param in Input buffer
 * @param in_len Number of input samples in the input buffer. Returns the number
 * of samples processed. This is all per-channel.
 * @param out Output buffer
 * @param out_len Size of the output buffer. Returns the number of samples written.
 * This is all per-channel.
 */
	int speex_resampler_process_interleaved_float(SpeexResamplerState * st,
						      const float *in,
						      uint32_t * in_len,
						      float *out,
						      uint32_t * out_len);

/** Set (change) the input/output sampling rates (integer value).
 * @param st Resampler state
 * @param in_rate Input sampling rate (integer number of Hz).
 * @param out_rate Output sampling rate (integer number of Hz).
 */
	int speex_resampler_set_rate(SpeexResamplerState * st,
				     uint32_t in_rate,
				     uint32_t out_rate);

/** Set (change) the input/output sampling rates and resampling ratio 
 * (fractional values in Hz supported).
 * @param st Resampler state
 * @param ratio_num Numerator of the sampling rate ratio
 * @param ratio_den Denominator of the sampling rate ratio
 * @param in_rate Input sampling rate rounded to the nearest integer (in Hz).
 * @param out_rate Output sampling rate rounded to the nearest integer (in Hz).
 */
	int speex_resampler_set_rate_frac(SpeexResamplerState * st,
					  uint32_t ratio_num,
					  uint32_t ratio_den,
					  uint32_t in_rate,
					  uint32_t out_rate);

/** Set (change) the conversion quality.
 * @param st Resampler state
 * @param quality Resampling quality between 0 and 10, where 0 has poor 
 * quality and 10 has very high quality.
 */
	int speex_resampler_set_quality(SpeexResamplerState * st, int quality);

#ifdef __cplusplus
}
#endif
#endif

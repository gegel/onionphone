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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "SigProc_FIX.h"
#include "resampler_private.h"

#define ORDER_FIR                   4

/* Downsample by a factor 2/3, low quality */
void silk_resampler_down2_3(int32_t * S,	/* I/O  State vector [ 6 ]                                          */
			    int16_t * out,	/* O    Output signal [ floor(2*inLen/3) ]                          */
			    const int16_t * in,	/* I    Input signal [ inLen ]                                      */
			    int32_t inLen	/* I    Number of input samples                                     */
    )
{
	int32_t nSamplesIn, counter, res_Q6;

	int32_t *buf_ptr;

	int32_t buf[RESAMPLER_MAX_BATCH_SIZE_IN + ORDER_FIR];

	/* Copy buffered samples to start of buffer */
	memcpy(buf, S, ORDER_FIR * sizeof(int32_t));

	/* Iterate over blocks of frameSizeIn input samples */
	while (1) {
		nSamplesIn = silk_min(inLen, RESAMPLER_MAX_BATCH_SIZE_IN);

		/* Second-order AR filter (output in Q8) */
		silk_resampler_private_AR2(&S[ORDER_FIR], &buf[ORDER_FIR], in,
					   silk_Resampler_2_3_COEFS_LQ,
					   nSamplesIn);

		/* Interpolate filtered signal */
		buf_ptr = buf;
		counter = nSamplesIn;
		while (counter > 2) {
			/* Inner product */
			res_Q6 =
			    silk_SMULWB(buf_ptr[0],
					silk_Resampler_2_3_COEFS_LQ[2]);
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[1],
					silk_Resampler_2_3_COEFS_LQ[3]);
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[2],
					silk_Resampler_2_3_COEFS_LQ[5]);
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[3],
					silk_Resampler_2_3_COEFS_LQ[4]);

			/* Scale down, saturate and store in output array */
			*out++ =
			    (int16_t)
			    silk_SAT16(silk_RSHIFT_ROUND(res_Q6, 6));

			res_Q6 =
			    silk_SMULWB(buf_ptr[1],
					silk_Resampler_2_3_COEFS_LQ[4]);
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[2],
					silk_Resampler_2_3_COEFS_LQ[5]);
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[3],
					silk_Resampler_2_3_COEFS_LQ[3]);
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[4],
					silk_Resampler_2_3_COEFS_LQ[2]);

			/* Scale down, saturate and store in output array */
			*out++ =
			    (int16_t)
			    silk_SAT16(silk_RSHIFT_ROUND(res_Q6, 6));

			buf_ptr += 3;
			counter -= 3;
		}

		in += nSamplesIn;
		inLen -= nSamplesIn;

		if (inLen > 0) {
			/* More iterations to do; copy last part of filtered signal to beginning of buffer */
			memcpy(buf, &buf[nSamplesIn],
				    ORDER_FIR * sizeof(int32_t));
		} else {
			break;
		}
	}

	/* Copy last part of filtered signal to the state for the next call */
	memcpy(S, &buf[nSamplesIn], ORDER_FIR * sizeof(int32_t));

}

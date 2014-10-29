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

/*                                                                      *
 * SKP_Silk_resample_3_1.c                                            *
 *                                                                      *
 * Upsamples by a factor 3                                              *
 *                                                                      *
 * Copyright 2008 (c), Skype Limited                                    *
 * Date: 081113                                                         *
 *                                                                      */
#include "SKP_Silk_SigProc_FIX.h"

#define IN_SUBFR_LEN_RESAMPLE_3_1       40

/* Resamples by a factor 3/1 */
void SKP_Silk_resample_3_1(int16_t * out,	/* O:   Fs_high signal [inLen*3]        */
			   int32_t * S,	/* I/O: State vector   [7]              */
			   const int16_t * in,	/* I:   Fs_low signal  [inLen]          */
			   const int32_t inLen	/* I:   Input length                    */
    )
{
	int k, LSubFrameIn, LSubFrameOut;
	int32_t out_tmp, idx, inLenTmp = inLen;
	int32_t scratch00[IN_SUBFR_LEN_RESAMPLE_3_1];
	int32_t scratch0[3 * IN_SUBFR_LEN_RESAMPLE_3_1];
	int32_t scratch1[3 * IN_SUBFR_LEN_RESAMPLE_3_1];

	/* Coefficients for 3-fold resampling */
	const int16_t A30[2] = { 1773, 17818 };
	const int16_t A31[2] = { 4942, 25677 };
	const int16_t A32[2] = { 11786, 29304 };

	while (inLenTmp > 0) {
		LSubFrameIn = SKP_min_int(IN_SUBFR_LEN_RESAMPLE_3_1, inLenTmp);
		LSubFrameOut = SKP_SMULBB(3, LSubFrameIn);

		/* Convert Q15 -> Q25 */
		for (k = 0; k < LSubFrameIn; k++) {
			scratch00[k] = SKP_LSHIFT((int32_t) in[k], 10);
		}

		/* Allpass filtering */
		/* Scratch size: 2 * 3* LSubFrame * sizeof(int32_t) */
		SKP_Silk_allpass_int(scratch00, S + 1, A30[0], scratch1,
				     LSubFrameIn);
		SKP_Silk_allpass_int(scratch1, S + 2, A30[1], scratch0,
				     LSubFrameIn);

		SKP_Silk_allpass_int(scratch00, S + 3, A31[0], scratch1,
				     LSubFrameIn);
		SKP_Silk_allpass_int(scratch1, S + 4, A31[1],
				     scratch0 + IN_SUBFR_LEN_RESAMPLE_3_1,
				     LSubFrameIn);

		SKP_Silk_allpass_int(scratch00, S + 5, A32[0], scratch1,
				     LSubFrameIn);
		SKP_Silk_allpass_int(scratch1, S + 6, A32[1],
				     scratch0 + 2 * IN_SUBFR_LEN_RESAMPLE_3_1,
				     LSubFrameIn);

		/* Interleave three allpass outputs */
		for (k = 0; k < LSubFrameIn; k++) {
			idx = SKP_SMULBB(3, k);
			scratch1[idx] = scratch0[k];
			scratch1[idx + 1] =
			    scratch0[k + IN_SUBFR_LEN_RESAMPLE_3_1];
			scratch1[idx + 2] =
			    scratch0[k + 2 * IN_SUBFR_LEN_RESAMPLE_3_1];
		}

		/* Low-pass filtering */
		SKP_Silk_lowpass_int(scratch1, S, scratch0, LSubFrameOut);

		/* Saturate and convert to int16_t */
		for (k = 0; k < LSubFrameOut; k++) {
			out_tmp = scratch0[k];
			out[k] =
			    (int16_t) SKP_SAT16(SKP_RSHIFT_ROUND(out_tmp, 10));
		}

		in += LSubFrameIn;
		inLenTmp -= LSubFrameIn;
		out += LSubFrameOut;
	}
}

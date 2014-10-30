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
 * File Name:   SKP_Silk_resample_2_3_coarse.c                        *
 *                                                                      *
 * Description: Linear phase FIR polyphase implementation of resampling *
 *                                                                      *
 * Copyright 2009 (c), Skype Limited                                    *
 * All rights reserved.                                                 *
 *                                                                      *
 * Date: 090423                                                         *
 *                                                                      */

#include "SKP_Silk_SigProc_FIX.h"
#include "SKP_Silk_resample_rom.h"

/* Resamples input data with a factor 2/3 */
void SKP_Silk_resample_2_3_coarse(int16_t * out,	/* O:   Output signal                                                                   */
				  int16_t * S,	/* I/O: Resampler state [ SigProc_Resample_2_3_coarse_NUM_FIR_COEFS - 1 ]               */
				  const int16_t * in,	/* I:   Input signal                                                                    */
				  const int frameLenIn,	/* I:   Number of input samples                                                         */
				  int16_t * scratch	/* I:   Scratch memory [ frameLenIn + SigProc_Resample_2_3_coarse_NUM_FIR_COEFS - 1 ]   */
    )
{
	int32_t n, ind, interpol_ind, tmp, index_Q16;
	int16_t *in_ptr;
	int frameLenOut;
	const int16_t *interpol_ptr;

	/* Copy buffered samples to start of scratch */
	SKP_memcpy(scratch, S,
		   (SigProc_Resample_2_3_coarse_NUM_FIR_COEFS -
		    1) * sizeof(int16_t));

	/* Then append by the input signal */
	SKP_memcpy(&scratch[SigProc_Resample_2_3_coarse_NUM_FIR_COEFS - 1], in,
		   frameLenIn * sizeof(int16_t));

	frameLenOut = SKP_DIV32_16(SKP_MUL(2, frameLenIn), 3);
	index_Q16 = 0;

	assert(frameLenIn == ((frameLenOut * 3) / 2));

	/* Interpolate */
	for (n = frameLenOut; n > 0; n--) {

		/* Integer part */
		ind = SKP_RSHIFT(index_Q16, 16);

		/* Pointer to buffered input */
		in_ptr = scratch + ind;

		/* Fractional part */
		interpol_ind =
		    (SKP_SMULWB
		     (index_Q16,
		      SigProc_Resample_2_3_coarse_NUM_INTERPOLATORS) &
		     (SigProc_Resample_2_3_coarse_NUM_INTERPOLATORS - 1));

		/* Pointer to FIR taps */
		interpol_ptr =
		    SigProc_Resample_2_3_coarse_INTERPOL[interpol_ind];

		/* Interpolate */
		/* Hardcoded for 32 FIR taps */
		assert(SigProc_Resample_2_3_coarse_NUM_FIR_COEFS == 32);
		tmp =
		    (int32_t) interpol_ptr[0] * in_ptr[0] +
		    (int32_t) interpol_ptr[1] * in_ptr[1] +
		    (int32_t) interpol_ptr[2] * in_ptr[2] +
		    (int32_t) interpol_ptr[3] * in_ptr[3] +
		    (int32_t) interpol_ptr[4] * in_ptr[4] +
		    (int32_t) interpol_ptr[5] * in_ptr[5] +
		    (int32_t) interpol_ptr[6] * in_ptr[6] +
		    (int32_t) interpol_ptr[7] * in_ptr[7] +
		    (int32_t) interpol_ptr[8] * in_ptr[8] +
		    (int32_t) interpol_ptr[9] * in_ptr[9] +
		    (int32_t) interpol_ptr[10] * in_ptr[10] +
		    (int32_t) interpol_ptr[11] * in_ptr[11] +
		    (int32_t) interpol_ptr[12] * in_ptr[12] +
		    (int32_t) interpol_ptr[13] * in_ptr[13] +
		    (int32_t) interpol_ptr[14] * in_ptr[14] +
		    (int32_t) interpol_ptr[15] * in_ptr[15] +
		    (int32_t) interpol_ptr[16] * in_ptr[16] +
		    (int32_t) interpol_ptr[17] * in_ptr[17] +
		    (int32_t) interpol_ptr[18] * in_ptr[18] +
		    (int32_t) interpol_ptr[19] * in_ptr[19] +
		    (int32_t) interpol_ptr[20] * in_ptr[20] +
		    (int32_t) interpol_ptr[21] * in_ptr[21] +
		    (int32_t) interpol_ptr[22] * in_ptr[22] +
		    (int32_t) interpol_ptr[23] * in_ptr[23] +
		    (int32_t) interpol_ptr[24] * in_ptr[24] +
		    (int32_t) interpol_ptr[25] * in_ptr[25] +
		    (int32_t) interpol_ptr[26] * in_ptr[26] +
		    (int32_t) interpol_ptr[27] * in_ptr[27] +
		    (int32_t) interpol_ptr[28] * in_ptr[28] +
		    (int32_t) interpol_ptr[29] * in_ptr[29];

		/* Round, saturate and store to output array */
		*out++ = (int16_t) SKP_SAT16(SKP_RSHIFT_ROUND(tmp, 15));

		/* Update index */
		index_Q16 += ((1 << 16) + (1 << 15));	// (3/2)_Q0;
	}

	/* Move last part of input signal to the sample buffer to prepare for the next call */
	SKP_memcpy(S,
		   &in[frameLenIn -
		       (SigProc_Resample_2_3_coarse_NUM_FIR_COEFS - 1)],
		   (SigProc_Resample_2_3_coarse_NUM_FIR_COEFS -
		    1) * sizeof(int16_t));
}

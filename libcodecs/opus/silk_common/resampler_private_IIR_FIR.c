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

static inline int16_t *silk_resampler_private_IIR_FIR_INTERPOL(int16_t *
								  out,
								  int16_t *
								  buf,
								  int32_t
								  max_index_Q16,
								  int32_t
								  index_increment_Q16)
{
	int32_t index_Q16, res_Q15;
	int16_t *buf_ptr;
	int32_t table_index;

	/* Interpolate upsampled signal and store in output array */
	for (index_Q16 = 0; index_Q16 < max_index_Q16;
	     index_Q16 += index_increment_Q16) {
		table_index = silk_SMULWB(index_Q16 & 0xFFFF, 12);
		buf_ptr = &buf[index_Q16 >> 16];

		res_Q15 =
		    silk_SMULBB(buf_ptr[0],
				silk_resampler_frac_FIR_12[table_index][0]);
		res_Q15 =
		    silk_SMLABB(res_Q15, buf_ptr[1],
				silk_resampler_frac_FIR_12[table_index][1]);
		res_Q15 =
		    silk_SMLABB(res_Q15, buf_ptr[2],
				silk_resampler_frac_FIR_12[table_index][2]);
		res_Q15 =
		    silk_SMLABB(res_Q15, buf_ptr[3],
				silk_resampler_frac_FIR_12[table_index][3]);
		res_Q15 =
		    silk_SMLABB(res_Q15, buf_ptr[4],
				silk_resampler_frac_FIR_12[11 -
							   table_index][3]);
		res_Q15 =
		    silk_SMLABB(res_Q15, buf_ptr[5],
				silk_resampler_frac_FIR_12[11 -
							   table_index][2]);
		res_Q15 =
		    silk_SMLABB(res_Q15, buf_ptr[6],
				silk_resampler_frac_FIR_12[11 -
							   table_index][1]);
		res_Q15 =
		    silk_SMLABB(res_Q15, buf_ptr[7],
				silk_resampler_frac_FIR_12[11 -
							   table_index][0]);
		*out++ =
		    (int16_t) silk_SAT16(silk_RSHIFT_ROUND(res_Q15, 15));
	}
	return out;
}

/* Upsample using a combination of allpass-based 2x upsampling and FIR interpolation */
void silk_resampler_private_IIR_FIR(void *SS,	/* I/O  Resampler state             */
				    int16_t out[],	/* O    Output signal               */
				    const int16_t in[],	/* I    Input signal                */
				    int32_t inLen	/* I    Number of input samples     */
    )
{
	silk_resampler_state_struct *S = (silk_resampler_state_struct *) SS;
	int32_t nSamplesIn;
	int32_t max_index_Q16, index_increment_Q16;

	int16_t buf[2 * S->batchSize + RESAMPLER_ORDER_FIR_12];

	/* Copy buffered samples to start of buffer */
	memcpy(buf, S->sFIR.i16,
		    RESAMPLER_ORDER_FIR_12 * sizeof(int16_t));

	/* Iterate over blocks of frameSizeIn input samples */
	index_increment_Q16 = S->invRatio_Q16;
	while (1) {
		nSamplesIn = silk_min(inLen, S->batchSize);

		/* Upsample 2x */
		silk_resampler_private_up2_HQ(S->sIIR,
					      &buf[RESAMPLER_ORDER_FIR_12], in,
					      nSamplesIn);

		max_index_Q16 = silk_LSHIFT32(nSamplesIn, 16 + 1);	/* + 1 because 2x upsampling */
		out =
		    silk_resampler_private_IIR_FIR_INTERPOL(out, buf,
							    max_index_Q16,
							    index_increment_Q16);
		in += nSamplesIn;
		inLen -= nSamplesIn;

		if (inLen > 0) {
			/* More iterations to do; copy last part of filtered signal to beginning of buffer */
			memcpy(buf, &buf[nSamplesIn << 1],
				    RESAMPLER_ORDER_FIR_12 *
				    sizeof(int16_t));
		} else {
			break;
		}
	}

	/* Copy last part of filtered signal to the state for the next call */
	memcpy(S->sFIR.i16, &buf[nSamplesIn << 1],
		    RESAMPLER_ORDER_FIR_12 * sizeof(int16_t));

}

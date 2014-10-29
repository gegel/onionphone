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

static inline int16_t *silk_resampler_private_down_FIR_INTERPOL(int16_t *
								   out,
								   int32_t *
								   buf,
								   const
								   int16_t *
								   FIR_Coefs,
								   int
								   FIR_Order,
								   int
								   FIR_Fracs,
								   int32_t
								   max_index_Q16,
								   int32_t
								   index_increment_Q16)
{
	int32_t index_Q16, res_Q6;
	int32_t *buf_ptr;
	int32_t interpol_ind;
	const int16_t *interpol_ptr;

	switch (FIR_Order) {
	case RESAMPLER_DOWN_ORDER_FIR0:
		for (index_Q16 = 0; index_Q16 < max_index_Q16;
		     index_Q16 += index_increment_Q16) {
			/* Integer part gives pointer to buffered input */
			buf_ptr = buf + silk_RSHIFT(index_Q16, 16);

			/* Fractional part gives interpolation coefficients */
			interpol_ind =
			    silk_SMULWB(index_Q16 & 0xFFFF, FIR_Fracs);

			/* Inner product */
			interpol_ptr =
			    &FIR_Coefs[RESAMPLER_DOWN_ORDER_FIR0 / 2 *
				       interpol_ind];
			res_Q6 = silk_SMULWB(buf_ptr[0], interpol_ptr[0]);
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[1], interpol_ptr[1]);
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[2], interpol_ptr[2]);
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[3], interpol_ptr[3]);
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[4], interpol_ptr[4]);
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[5], interpol_ptr[5]);
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[6], interpol_ptr[6]);
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[7], interpol_ptr[7]);
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[8], interpol_ptr[8]);
			interpol_ptr =
			    &FIR_Coefs[RESAMPLER_DOWN_ORDER_FIR0 / 2 *
				       (FIR_Fracs - 1 - interpol_ind)];
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[17], interpol_ptr[0]);
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[16], interpol_ptr[1]);
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[15], interpol_ptr[2]);
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[14], interpol_ptr[3]);
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[13], interpol_ptr[4]);
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[12], interpol_ptr[5]);
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[11], interpol_ptr[6]);
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[10], interpol_ptr[7]);
			res_Q6 =
			    silk_SMLAWB(res_Q6, buf_ptr[9], interpol_ptr[8]);

			/* Scale down, saturate and store in output array */
			*out++ =
			    (int16_t)
			    silk_SAT16(silk_RSHIFT_ROUND(res_Q6, 6));
		}
		break;
	case RESAMPLER_DOWN_ORDER_FIR1:
		for (index_Q16 = 0; index_Q16 < max_index_Q16;
		     index_Q16 += index_increment_Q16) {
			/* Integer part gives pointer to buffered input */
			buf_ptr = buf + silk_RSHIFT(index_Q16, 16);

			/* Inner product */
			res_Q6 =
			    silk_SMULWB(silk_ADD32(buf_ptr[0], buf_ptr[23]),
					FIR_Coefs[0]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[1], buf_ptr[22]),
					FIR_Coefs[1]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[2], buf_ptr[21]),
					FIR_Coefs[2]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[3], buf_ptr[20]),
					FIR_Coefs[3]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[4], buf_ptr[19]),
					FIR_Coefs[4]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[5], buf_ptr[18]),
					FIR_Coefs[5]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[6], buf_ptr[17]),
					FIR_Coefs[6]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[7], buf_ptr[16]),
					FIR_Coefs[7]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[8], buf_ptr[15]),
					FIR_Coefs[8]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[9], buf_ptr[14]),
					FIR_Coefs[9]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[10], buf_ptr[13]),
					FIR_Coefs[10]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[11], buf_ptr[12]),
					FIR_Coefs[11]);

			/* Scale down, saturate and store in output array */
			*out++ =
			    (int16_t)
			    silk_SAT16(silk_RSHIFT_ROUND(res_Q6, 6));
		}
		break;
	case RESAMPLER_DOWN_ORDER_FIR2:
		for (index_Q16 = 0; index_Q16 < max_index_Q16;
		     index_Q16 += index_increment_Q16) {
			/* Integer part gives pointer to buffered input */
			buf_ptr = buf + silk_RSHIFT(index_Q16, 16);

			/* Inner product */
			res_Q6 =
			    silk_SMULWB(silk_ADD32(buf_ptr[0], buf_ptr[35]),
					FIR_Coefs[0]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[1], buf_ptr[34]),
					FIR_Coefs[1]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[2], buf_ptr[33]),
					FIR_Coefs[2]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[3], buf_ptr[32]),
					FIR_Coefs[3]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[4], buf_ptr[31]),
					FIR_Coefs[4]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[5], buf_ptr[30]),
					FIR_Coefs[5]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[6], buf_ptr[29]),
					FIR_Coefs[6]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[7], buf_ptr[28]),
					FIR_Coefs[7]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[8], buf_ptr[27]),
					FIR_Coefs[8]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[9], buf_ptr[26]),
					FIR_Coefs[9]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[10], buf_ptr[25]),
					FIR_Coefs[10]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[11], buf_ptr[24]),
					FIR_Coefs[11]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[12], buf_ptr[23]),
					FIR_Coefs[12]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[13], buf_ptr[22]),
					FIR_Coefs[13]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[14], buf_ptr[21]),
					FIR_Coefs[14]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[15], buf_ptr[20]),
					FIR_Coefs[15]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[16], buf_ptr[19]),
					FIR_Coefs[16]);
			res_Q6 =
			    silk_SMLAWB(res_Q6,
					silk_ADD32(buf_ptr[17], buf_ptr[18]),
					FIR_Coefs[17]);

			/* Scale down, saturate and store in output array */
			*out++ =
			    (int16_t)
			    silk_SAT16(silk_RSHIFT_ROUND(res_Q6, 6));
		}
		break;
	default:
		silk_assert(0);
	}
	return out;
}

/* Resample with a 2nd order AR filter followed by FIR interpolation */
void silk_resampler_private_down_FIR(void *SS,	/* I/O  Resampler state             */
				     int16_t out[],	/* O    Output signal               */
				     const int16_t in[],	/* I    Input signal                */
				     int32_t inLen	/* I    Number of input samples     */
    )
{
	silk_resampler_state_struct *S = (silk_resampler_state_struct *) SS;
	int32_t nSamplesIn;
	int32_t max_index_Q16, index_increment_Q16;

	const int16_t *FIR_Coefs;

	int32_t buf[S->batchSize + S->FIR_Order];

	/* Copy buffered samples to start of buffer */
	memcpy(buf, S->sFIR.i32, S->FIR_Order * sizeof(int32_t));

	FIR_Coefs = &S->Coefs[2];

	/* Iterate over blocks of frameSizeIn input samples */
	index_increment_Q16 = S->invRatio_Q16;
	while (1) {
		nSamplesIn = silk_min(inLen, S->batchSize);

		/* Second-order AR filter (output in Q8) */
		silk_resampler_private_AR2(S->sIIR, &buf[S->FIR_Order], in,
					   S->Coefs, nSamplesIn);

		max_index_Q16 = silk_LSHIFT32(nSamplesIn, 16);

		/* Interpolate filtered signal */
		out =
		    silk_resampler_private_down_FIR_INTERPOL(out, buf,
							     FIR_Coefs,
							     S->FIR_Order,
							     S->FIR_Fracs,
							     max_index_Q16,
							     index_increment_Q16);

		in += nSamplesIn;
		inLen -= nSamplesIn;

		if (inLen > 1) {
			/* More iterations to do; copy last part of filtered signal to beginning of buffer */
			memcpy(buf, &buf[nSamplesIn],
				    S->FIR_Order * sizeof(int32_t));
		} else {
			break;
		}
	}

	/* Copy last part of filtered signal to the state for the next call */
	memcpy(S->sFIR.i32, &buf[nSamplesIn],
		    S->FIR_Order * sizeof(int32_t));

}

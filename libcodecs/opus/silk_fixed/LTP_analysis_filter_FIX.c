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

#include "main_FIX.h"

void silk_LTP_analysis_filter_FIX(int16_t * LTP_res,	/* O    LTP residual signal of length MAX_NB_SUBFR * ( pre_length + subfr_length )  */
				  const int16_t * x,	/* I    Pointer to input signal with at least max( pitchL ) preceding samples       */
				  const int16_t LTPCoef_Q14[LTP_ORDER * MAX_NB_SUBFR],	/* I    LTP_ORDER LTP coefficients for each MAX_NB_SUBFR subframe                   */
				  const int pitchL[MAX_NB_SUBFR],	/* I    Pitch lag, one for each subframe                                            */
				  const int32_t invGains_Q16[MAX_NB_SUBFR],	/* I    Inverse quantization gains, one for each subframe                           */
				  const int subfr_length,	/* I    Length of each subframe                                                     */
				  const int nb_subfr,	/* I    Number of subframes                                                         */
				  const int pre_length	/* I    Length of the preceding samples starting at &x[0] for each subframe         */
    )
{
	const int16_t *x_ptr, *x_lag_ptr;
	int16_t Btmp_Q14[LTP_ORDER];
	int16_t *LTP_res_ptr;
	int k, i, j;
	int32_t LTP_est;

	x_ptr = x;
	LTP_res_ptr = LTP_res;
	for (k = 0; k < nb_subfr; k++) {

		x_lag_ptr = x_ptr - pitchL[k];
		for (i = 0; i < LTP_ORDER; i++) {
			Btmp_Q14[i] = LTPCoef_Q14[k * LTP_ORDER + i];
		}

		/* LTP analysis FIR filter */
		for (i = 0; i < subfr_length + pre_length; i++) {
			LTP_res_ptr[i] = x_ptr[i];

			/* Long-term prediction */
			LTP_est =
			    silk_SMULBB(x_lag_ptr[LTP_ORDER / 2], Btmp_Q14[0]);
			for (j = 1; j < LTP_ORDER; j++) {
				LTP_est =
				    silk_SMLABB_ovflw(LTP_est,
						      x_lag_ptr[LTP_ORDER / 2 -
								j],
						      Btmp_Q14[j]);
			}
			LTP_est = silk_RSHIFT_ROUND(LTP_est, 14);	/* round and -> Q0 */

			/* Subtract long-term prediction */
			LTP_res_ptr[i] =
			    (int16_t) silk_SAT16((int32_t) x_ptr[i] -
						    LTP_est);

			/* Scale residual */
			LTP_res_ptr[i] =
			    silk_SMULWB(invGains_Q16[k], LTP_res_ptr[i]);

			x_lag_ptr++;
		}

		/* Update pointers */
		LTP_res_ptr += subfr_length + pre_length;
		x_ptr += subfr_length;
	}
}

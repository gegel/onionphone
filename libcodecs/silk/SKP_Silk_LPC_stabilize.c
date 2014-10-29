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

#include "SKP_Silk_typedef.h"
#include "SKP_Silk_SigProc_FIX.h"

#define LPC_STABILIZE_LPC_MAX_ABS_VALUE_Q16     ( ( (int32_t)int16_t_MAX ) << 4 )

/* LPC stabilizer, for a single input data vector */
void SKP_Silk_LPC_stabilize(int16_t * a_Q12,	/* O    stabilized LPC vector [L]                       */
			    int32_t * a_Q16,	/* I    LPC vector [L]                                  */
			    const int32_t bwe_Q16,	/* I    Bandwidth expansion factor                      */
			    const int L	/* I    Number of LPC parameters in the input vector    */
    )
{
	int32_t maxabs, absval, sc_Q16;
	int i, idx = 0;
	int32_t invGain_Q30;

	SKP_Silk_bwexpander_32(a_Q16, L, bwe_Q16);

    /***************************/
	/* Limit range of the LPCs */
    /***************************/
	/* Limit the maximum absolute value of the prediction coefficients */
	while (SKP_TRUE) {
		/* Find maximum absolute value and its index */
		maxabs = int32_t_MIN;
		for (i = 0; i < L; i++) {
			absval = SKP_abs(a_Q16[i]);
			if (absval > maxabs) {
				maxabs = absval;
				idx = i;
			}
		}

		if (maxabs >= LPC_STABILIZE_LPC_MAX_ABS_VALUE_Q16) {
			/* Reduce magnitude of prediction coefficients */
			sc_Q16 = SKP_DIV32(int32_t_MAX, SKP_RSHIFT(maxabs, 4));
			sc_Q16 = 65536 - sc_Q16;
			sc_Q16 = SKP_DIV32(sc_Q16, idx + 1);
			sc_Q16 = 65536 - sc_Q16;
			sc_Q16 = SKP_LSHIFT(SKP_SMULWB(sc_Q16, 32604), 1);	// 0.995 in Q16
			SKP_Silk_bwexpander_32(a_Q16, L, sc_Q16);
		} else {
			break;
		}
	}

	/* Convert to 16 bit Q12 */
	for (i = 0; i < L; i++) {
		a_Q12[i] = (int16_t) SKP_RSHIFT_ROUND(a_Q16[i], 4);
	}

    /**********************/
	/* Ensure stable LPCs */
    /**********************/
	while (SKP_Silk_LPC_inverse_pred_gain(&invGain_Q30, a_Q12, L) == 1) {
		SKP_Silk_bwexpander(a_Q12, L, 65339);	// 0.997 in Q16
	}
}

void SKP_Silk_LPC_fit(int16_t * a_QQ,	/* O    Stabilized LPC vector, Q(24-rshift) [L]         */
		      int32_t * a_Q24,	/* I    LPC vector [L]                                  */
		      const int QQ,	/* I    Q domain of output LPC vector                   */
		      const int L	/* I    Number of LPC parameters in the input vector    */
    )
{
	int i, rshift, idx = 0;
	int32_t maxabs, absval, sc_Q16;

	rshift = 24 - QQ;

    /***************************/
	/* Limit range of the LPCs */
    /***************************/
	/* Limit the maximum absolute value of the prediction coefficients */
	while (SKP_TRUE) {
		/* Find maximum absolute value and its index */
		maxabs = int32_t_MIN;
		for (i = 0; i < L; i++) {
			absval = SKP_abs(a_Q24[i]);
			if (absval > maxabs) {
				maxabs = absval;
				idx = i;
			}
		}

		maxabs = SKP_RSHIFT(maxabs, rshift);
		if (maxabs >= int16_t_MAX) {
			/* Reduce magnitude of prediction coefficients */
			sc_Q16 =
			    65470 -
			    SKP_DIV32(SKP_MUL(65470 >> 2, maxabs - int16_t_MAX),
				      SKP_RSHIFT32(SKP_MUL(maxabs, idx + 1),
						   2));
			SKP_Silk_bwexpander_32(a_Q24, L, sc_Q16);
		} else {
			break;
		}
	}

	/* Convert to 16 bit Q(24-rshift) */
	SKP_assert(rshift > 0);
	SKP_assert(rshift < 31);
	for (i = 0; i < L; i++) {
		a_QQ[i] = (int16_t) SKP_RSHIFT_ROUND(a_Q24[i], rshift);
	}
}

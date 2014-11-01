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

#include "main.h"

/* Predictive dequantizer for NLSF residuals */
static inline void silk_NLSF_residual_dequant(	/* O    Returns RD value in Q30                     */
						     int16_t x_Q10[],	/* O    Output [ order ]                            */
						     const int8_t indices[],	/* I    Quantization indices [ order ]              */
						     const uint8_t pred_coef_Q8[],	/* I    Backward predictor coefs [ order ]          */
						     const int quant_step_size_Q16,	/* I    Quantization step size                      */
						     const int16_t order	/* I    Number of input values                      */
    )
{
	int i, out_Q10, pred_Q10;

	out_Q10 = 0;
	for (i = order - 1; i >= 0; i--) {
		pred_Q10 =
		    silk_RSHIFT(silk_SMULBB
				(out_Q10, (int16_t) pred_coef_Q8[i]), 8);
		out_Q10 = silk_LSHIFT(indices[i], 10);
		if (out_Q10 > 0) {
			out_Q10 =
			    silk_SUB16(out_Q10,
				       SILK_FIX_CONST(NLSF_QUANT_LEVEL_ADJ,
						      10));
		} else if (out_Q10 < 0) {
			out_Q10 =
			    silk_ADD16(out_Q10,
				       SILK_FIX_CONST(NLSF_QUANT_LEVEL_ADJ,
						      10));
		}
		out_Q10 =
		    silk_SMLAWB(pred_Q10, (int32_t) out_Q10,
				quant_step_size_Q16);
		x_Q10[i] = out_Q10;
	}
}

/***********************/
/* NLSF vector decoder */
/***********************/
void silk_NLSF_decode(int16_t * pNLSF_Q15,	/* O    Quantized NLSF vector [ LPC_ORDER ]         */
		      int8_t * NLSFIndices,	/* I    Codebook path vector [ LPC_ORDER + 1 ]      */
		      const silk_NLSF_CB_struct * psNLSF_CB	/* I    Codebook object                             */
    )
{
	int i;
	uint8_t pred_Q8[MAX_LPC_ORDER];
	int16_t ec_ix[MAX_LPC_ORDER];
	int16_t res_Q10[MAX_LPC_ORDER];
	int16_t W_tmp_QW[MAX_LPC_ORDER];
	int32_t W_tmp_Q9, NLSF_Q15_tmp;
	const uint8_t *pCB_element;

	/* Decode first stage */
	pCB_element =
	    &psNLSF_CB->CB1_NLSF_Q8[NLSFIndices[0] * psNLSF_CB->order];
	for (i = 0; i < psNLSF_CB->order; i++) {
		pNLSF_Q15[i] = silk_LSHIFT((int16_t) pCB_element[i], 7);
	}

	/* Unpack entropy table indices and predictor for current CB1 index */
	silk_NLSF_unpack(ec_ix, pred_Q8, psNLSF_CB, NLSFIndices[0]);

	/* Predictive residual dequantizer */
	silk_NLSF_residual_dequant(res_Q10, &NLSFIndices[1], pred_Q8,
				   psNLSF_CB->quantStepSize_Q16,
				   psNLSF_CB->order);

	/* Weights from codebook vector */
	silk_NLSF_VQ_weights_laroia(W_tmp_QW, pNLSF_Q15, psNLSF_CB->order);

	/* Apply inverse square-rooted weights and add to output */
	for (i = 0; i < psNLSF_CB->order; i++) {
		W_tmp_Q9 =
		    silk_SQRT_APPROX(silk_LSHIFT
				     ((int32_t) W_tmp_QW[i], 18 - NLSF_W_Q));
		NLSF_Q15_tmp =
		    silk_ADD32(pNLSF_Q15[i],
			       silk_DIV32_16(silk_LSHIFT
					     ((int32_t) res_Q10[i], 14),
					     W_tmp_Q9));
		pNLSF_Q15[i] = (int16_t) silk_LIMIT(NLSF_Q15_tmp, 0, 32767);
	}

	/* NLSF stabilization */
	silk_NLSF_stabilize(pNLSF_Q15, psNLSF_CB->deltaMin_Q15,
			    psNLSF_CB->order);
}

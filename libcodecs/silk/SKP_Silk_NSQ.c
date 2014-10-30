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

#include "SKP_Silk_main.h"

SKP_INLINE void SKP_Silk_nsq_scale_states(SKP_Silk_nsq_state * NSQ,	/* I/O NSQ state                        */
					  const int16_t x[],	/* I input in Q0                        */
					  int32_t x_sc_Q10[],	/* O input scaled with 1/Gain           */
					  int length,	/* I length of input                    */
					  int16_t sLTP[],	/* I re-whitened LTP state in Q0        */
					  int32_t sLTP_Q16[],	/* O LTP state matching scaled input    */
					  int subfr,	/* I subframe number                    */
					  const int LTP_scale_Q14,	/* I                                    */
					  const int32_t Gains_Q16[NB_SUBFR],	/* I                                 */
					  const int pitchL[NB_SUBFR]	/* I                                    */
    );

SKP_INLINE void SKP_Silk_noise_shape_quantizer(SKP_Silk_nsq_state * NSQ,	/* I/O  NSQ state                       */
					       int sigtype,	/* I    Signal type                     */
					       const int32_t x_sc_Q10[],	/* I                                    */
					       int q[],	/* O                                    */
					       int16_t xq[],	/* O                                    */
					       int32_t sLTP_Q16[],	/* I/O  LTP state                       */
					       const int16_t a_Q12[],	/* I    Short term prediction coefs     */
					       const int16_t b_Q14[],	/* I    Long term prediction coefs      */
					       const int16_t AR_shp_Q13[],	/* I    Noise shaping AR coefs          */
					       int lag,	/* I    Pitch lag                       */
					       int32_t HarmShapeFIRPacked_Q14,	/* I                                */
					       int Tilt_Q14,	/* I    Spectral tilt                   */
					       int32_t LF_shp_Q14,	/* I                                    */
					       int32_t Gain_Q16,	/* I                                    */
					       int Lambda_Q10,	/* I                                    */
					       int offset_Q10,	/* I                                    */
					       int length,	/* I    Input length                    */
					       int shapingLPCOrder,	/* I    Noise shaping AR filter order   */
					       int predictLPCOrder	/* I    Prediction filter order         */
    );

void SKP_Silk_NSQ(SKP_Silk_encoder_state * psEncC,	/* I/O  Encoder State                       */
		  SKP_Silk_encoder_control * psEncCtrlC,	/* I    Encoder Control                     */
		  SKP_Silk_nsq_state * NSQ,	/* I/O  NSQ state                           */
		  const int16_t x[],	/* I    prefiltered input signal            */
		  int q[],	/* O    quantized qulse signal              */
		  const int LSFInterpFactor_Q2,	/* I    LSF interpolation factor in Q2      */
		  const int16_t PredCoef_Q12[2 * MAX_LPC_ORDER],	/* I    Short term prediction coefficients  */
		  const int16_t LTPCoef_Q14[LTP_ORDER * NB_SUBFR],	/* I    Long term prediction coefficients   */
		  const int16_t AR2_Q13[NB_SUBFR * SHAPE_LPC_ORDER_MAX],	/* I                                        */
		  const int HarmShapeGain_Q14[NB_SUBFR],	/* I                                        */
		  const int Tilt_Q14[NB_SUBFR],	/* I    Spectral tilt                       */
		  const int32_t LF_shp_Q14[NB_SUBFR],	/* I                                        */
		  const int32_t Gains_Q16[NB_SUBFR],	/* I                                        */
		  const int Lambda_Q10,	/* I                                        */
		  const int LTP_scale_Q14	/* I    LTP state scaling                   */
    )
{
	int k, lag, start_idx, LSF_interpolation_flag;
	const int16_t *A_Q12, *B_Q14, *AR_shp_Q13;
	int16_t *pxq;
	int32_t sLTP_Q16[2 * MAX_FRAME_LENGTH];
	int16_t sLTP[2 * MAX_FRAME_LENGTH];
	int32_t HarmShapeFIRPacked_Q14;
	int offset_Q10;
	int32_t FiltState[MAX_LPC_ORDER];
	int32_t x_sc_Q10[MAX_FRAME_LENGTH / NB_SUBFR];

	NSQ->rand_seed = psEncCtrlC->Seed;
	/* Set unvoiced lag to the previous one, overwrite later for voiced */
	lag = NSQ->lagPrev;

	assert(NSQ->prev_inv_gain_Q16 != 0);

	offset_Q10 =
	    SKP_Silk_Quantization_Offsets_Q10[psEncCtrlC->sigtype][psEncCtrlC->
								   QuantOffsetType];

	if (LSFInterpFactor_Q2 == (1 << 2)) {
		LSF_interpolation_flag = 0;
	} else {
		LSF_interpolation_flag = 1;
	}

	/* Setup pointers to start of sub frame */
	NSQ->sLTP_shp_buf_idx = psEncC->frame_length;
	NSQ->sLTP_buf_idx = psEncC->frame_length;
	pxq = &NSQ->xq[psEncC->frame_length];
	for (k = 0; k < NB_SUBFR; k++) {
		A_Q12 =
		    &PredCoef_Q12[((k >> 1) | (1 - LSF_interpolation_flag)) *
				  MAX_LPC_ORDER];
		B_Q14 = &LTPCoef_Q14[k * LTP_ORDER];
		AR_shp_Q13 = &AR2_Q13[k * SHAPE_LPC_ORDER_MAX];

		/* Noise shape parameters */
		assert(HarmShapeGain_Q14[k] >= 0);
		HarmShapeFIRPacked_Q14 = SKP_RSHIFT(HarmShapeGain_Q14[k], 2);
		HarmShapeFIRPacked_Q14 |=
		    SKP_LSHIFT((int32_t) SKP_RSHIFT(HarmShapeGain_Q14[k], 1),
			       16);

		if (psEncCtrlC->sigtype == SIG_TYPE_VOICED) {
			/* Voiced */
			lag = psEncCtrlC->pitchL[k];

			NSQ->rewhite_flag = 0;
			/* Re-whitening */
			if ((k & (3 - SKP_LSHIFT(LSF_interpolation_flag, 1))) ==
			    0) {
				/* Rewhiten with new A coefs */

				start_idx =
				    psEncC->frame_length - lag -
				    psEncC->predictLPCOrder - LTP_ORDER / 2;
				start_idx = SKP_LIMIT(start_idx, 0, psEncC->frame_length - psEncC->predictLPCOrder);	/* Limit */

				SKP_memset(FiltState, 0,
					   psEncC->predictLPCOrder *
					   sizeof(int32_t));
				SKP_Silk_MA_Prediction(&NSQ->
						       xq[start_idx +
							  k *
							  (psEncC->
							   frame_length >> 2)],
						       A_Q12, FiltState,
						       sLTP + start_idx,
						       psEncC->frame_length -
						       start_idx,
						       psEncC->predictLPCOrder);

				NSQ->rewhite_flag = 1;
				NSQ->sLTP_buf_idx = psEncC->frame_length;
			}
		}

		SKP_Silk_nsq_scale_states(NSQ, x, x_sc_Q10,
					  psEncC->subfr_length, sLTP, sLTP_Q16,
					  k, LTP_scale_Q14, Gains_Q16,
					  psEncCtrlC->pitchL);

		SKP_Silk_noise_shape_quantizer(NSQ, psEncCtrlC->sigtype,
					       x_sc_Q10, q, pxq, sLTP_Q16,
					       A_Q12, B_Q14, AR_shp_Q13, lag,
					       HarmShapeFIRPacked_Q14,
					       Tilt_Q14[k], LF_shp_Q14[k],
					       Gains_Q16[k], Lambda_Q10,
					       offset_Q10, psEncC->subfr_length,
					       psEncC->shapingLPCOrder,
					       psEncC->predictLPCOrder);

		x += psEncC->subfr_length;
		q += psEncC->subfr_length;
		pxq += psEncC->subfr_length;
	}

	/* Save scalars for this layer */
	NSQ->lagPrev = psEncCtrlC->pitchL[NB_SUBFR - 1];
	/* Save quantized speech and noise shaping signals */
	SKP_memcpy(NSQ->xq, &NSQ->xq[psEncC->frame_length],
		   psEncC->frame_length * sizeof(int16_t));
	SKP_memcpy(NSQ->sLTP_shp_Q10, &NSQ->sLTP_shp_Q10[psEncC->frame_length],
		   psEncC->frame_length * sizeof(int32_t));

}

/***********************************/
/* SKP_Silk_noise_shape_quantizer  */
/***********************************/
SKP_INLINE void SKP_Silk_noise_shape_quantizer(SKP_Silk_nsq_state * NSQ,	/* I/O  NSQ state                       */
					       int sigtype,	/* I    Signal type                     */
					       const int32_t x_sc_Q10[],	/* I                                    */
					       int q[],	/* O                                    */
					       int16_t xq[],	/* O                                    */
					       int32_t sLTP_Q16[],	/* I/O  LTP state                       */
					       const int16_t a_Q12[],	/* I    Short term prediction coefs     */
					       const int16_t b_Q14[],	/* I    Long term prediction coefs      */
					       const int16_t AR_shp_Q13[],	/* I    Noise shaping AR coefs          */
					       int lag,	/* I    Pitch lag                       */
					       int32_t HarmShapeFIRPacked_Q14,	/* I                                */
					       int Tilt_Q14,	/* I    Spectral tilt                   */
					       int32_t LF_shp_Q14,	/* I                                    */
					       int32_t Gain_Q16,	/* I                                    */
					       int Lambda_Q10,	/* I                                    */
					       int offset_Q10,	/* I                                    */
					       int length,	/* I    Input length                    */
					       int shapingLPCOrder,	/* I    Noise shaping AR filter order   */
					       int predictLPCOrder	/* I    Prediction filter order         */
    )
{
	int i, j;
	int32_t LTP_pred_Q14, LPC_pred_Q10, n_AR_Q10, n_LTP_Q14;
	int32_t n_LF_Q10, r_Q10, q_Q0, q_Q10;
	int32_t thr1_Q10, thr2_Q10, thr3_Q10;
	int32_t Atmp, dither;
	int32_t exc_Q10, LPC_exc_Q10, xq_Q10;
	int32_t tmp, sLF_AR_shp_Q10;
	int32_t *psLPC_Q14;
	int32_t *shp_lag_ptr, *pred_lag_ptr;
	int32_t a_Q12_tmp[MAX_LPC_ORDER / 2], AR_shp_Q13_tmp[MAX_LPC_ORDER / 2];

	shp_lag_ptr =
	    &NSQ->sLTP_shp_Q10[NSQ->sLTP_shp_buf_idx - lag +
			       HARM_SHAPE_FIR_TAPS / 2];
	pred_lag_ptr = &sLTP_Q16[NSQ->sLTP_buf_idx - lag + LTP_ORDER / 2];

	/* Setup short term AR state */
	psLPC_Q14 = &NSQ->sLPC_Q14[MAX_LPC_ORDER - 1];

	/* Quantization thresholds */
	thr1_Q10 = SKP_SUB_RSHIFT32(-1536, Lambda_Q10, 1);
	thr2_Q10 = SKP_SUB_RSHIFT32(-512, Lambda_Q10, 1);
	thr2_Q10 =
	    SKP_ADD_RSHIFT32(thr2_Q10, SKP_SMULBB(offset_Q10, Lambda_Q10), 10);
	thr3_Q10 = SKP_ADD_RSHIFT32(512, Lambda_Q10, 1);

	/* Preload LPC coeficients to array on stack. Gives small performance gain */
	SKP_memcpy(a_Q12_tmp, a_Q12, predictLPCOrder * sizeof(int16_t));
	SKP_memcpy(AR_shp_Q13_tmp, AR_shp_Q13,
		   shapingLPCOrder * sizeof(int16_t));

	for (i = 0; i < length; i++) {
		/* Generate dither */
		NSQ->rand_seed = SKP_RAND(NSQ->rand_seed);

		/* dither = rand_seed < 0 ? 0xFFFFFFFF : 0; */
		dither = SKP_RSHIFT(NSQ->rand_seed, 31);

		/* Short-term prediction */
		assert((predictLPCOrder & 1) == 0);	/* check that order is even */
		assert(((int64_t) a_Q12 & 3) == 0);	/* check that array starts at 4-byte aligned address */
		assert(predictLPCOrder >= 10);	/* check that unrolling works */

		/* NOTE: the code below loads two int16 values in an int32, and multiplies each using the   */
		/* SMLAWB and SMLAWT instructions. On a big-endian CPU the two int16 variables would be     */
		/* loaded in reverse order and the code will give the wrong result. In that case swapping   */
		/* the SMLAWB and SMLAWT instructions should solve the problem.                             */
		/* Partially unrolled */
		Atmp = a_Q12_tmp[0];	/* read two coefficients at once */
		LPC_pred_Q10 = SKP_SMULWB(psLPC_Q14[0], Atmp);
		LPC_pred_Q10 = SKP_SMLAWT(LPC_pred_Q10, psLPC_Q14[-1], Atmp);
		Atmp = a_Q12_tmp[1];
		LPC_pred_Q10 = SKP_SMLAWB(LPC_pred_Q10, psLPC_Q14[-2], Atmp);
		LPC_pred_Q10 = SKP_SMLAWT(LPC_pred_Q10, psLPC_Q14[-3], Atmp);
		Atmp = a_Q12_tmp[2];
		LPC_pred_Q10 = SKP_SMLAWB(LPC_pred_Q10, psLPC_Q14[-4], Atmp);
		LPC_pred_Q10 = SKP_SMLAWT(LPC_pred_Q10, psLPC_Q14[-5], Atmp);
		Atmp = a_Q12_tmp[3];
		LPC_pred_Q10 = SKP_SMLAWB(LPC_pred_Q10, psLPC_Q14[-6], Atmp);
		LPC_pred_Q10 = SKP_SMLAWT(LPC_pred_Q10, psLPC_Q14[-7], Atmp);
		Atmp = a_Q12_tmp[4];
		LPC_pred_Q10 = SKP_SMLAWB(LPC_pred_Q10, psLPC_Q14[-8], Atmp);
		LPC_pred_Q10 = SKP_SMLAWT(LPC_pred_Q10, psLPC_Q14[-9], Atmp);
		for (j = 10; j < predictLPCOrder; j += 2) {
			Atmp = a_Q12_tmp[j >> 1];	/* read two coefficients at once */
			LPC_pred_Q10 =
			    SKP_SMLAWB(LPC_pred_Q10, psLPC_Q14[-j], Atmp);
			LPC_pred_Q10 =
			    SKP_SMLAWT(LPC_pred_Q10, psLPC_Q14[-j - 1], Atmp);
		}

		/* Long-term prediction */
		if (sigtype == SIG_TYPE_VOICED) {
			/* Unrolled loop */
			LTP_pred_Q14 = SKP_SMULWB(pred_lag_ptr[0], b_Q14[0]);
			LTP_pred_Q14 =
			    SKP_SMLAWB(LTP_pred_Q14, pred_lag_ptr[-1],
				       b_Q14[1]);
			LTP_pred_Q14 =
			    SKP_SMLAWB(LTP_pred_Q14, pred_lag_ptr[-2],
				       b_Q14[2]);
			LTP_pred_Q14 =
			    SKP_SMLAWB(LTP_pred_Q14, pred_lag_ptr[-3],
				       b_Q14[3]);
			LTP_pred_Q14 =
			    SKP_SMLAWB(LTP_pred_Q14, pred_lag_ptr[-4],
				       b_Q14[4]);
			pred_lag_ptr++;
		} else {
			LTP_pred_Q14 = 0;
		}

		/* Noise shape feedback */
		assert((shapingLPCOrder & 1) == 0);	/* check that order is even */
		assert(((int64_t) AR_shp_Q13 & 3) == 0);	/* check that array starts at 4-byte aligned address */
		assert(shapingLPCOrder >= 12);	/* check that unrolling works */

		/* Partially unrolled */
		Atmp = AR_shp_Q13_tmp[0];	/* read two coefficients at once */
		n_AR_Q10 = SKP_SMULWB(psLPC_Q14[0], Atmp);
		n_AR_Q10 = SKP_SMLAWT(n_AR_Q10, psLPC_Q14[-1], Atmp);
		Atmp = AR_shp_Q13_tmp[1];
		n_AR_Q10 = SKP_SMLAWB(n_AR_Q10, psLPC_Q14[-2], Atmp);
		n_AR_Q10 = SKP_SMLAWT(n_AR_Q10, psLPC_Q14[-3], Atmp);
		Atmp = AR_shp_Q13_tmp[2];
		n_AR_Q10 = SKP_SMLAWB(n_AR_Q10, psLPC_Q14[-4], Atmp);
		n_AR_Q10 = SKP_SMLAWT(n_AR_Q10, psLPC_Q14[-5], Atmp);
		Atmp = AR_shp_Q13_tmp[3];
		n_AR_Q10 = SKP_SMLAWB(n_AR_Q10, psLPC_Q14[-6], Atmp);
		n_AR_Q10 = SKP_SMLAWT(n_AR_Q10, psLPC_Q14[-7], Atmp);
		Atmp = AR_shp_Q13_tmp[4];
		n_AR_Q10 = SKP_SMLAWB(n_AR_Q10, psLPC_Q14[-8], Atmp);
		n_AR_Q10 = SKP_SMLAWT(n_AR_Q10, psLPC_Q14[-9], Atmp);
		Atmp = AR_shp_Q13_tmp[5];
		n_AR_Q10 = SKP_SMLAWB(n_AR_Q10, psLPC_Q14[-10], Atmp);
		n_AR_Q10 = SKP_SMLAWT(n_AR_Q10, psLPC_Q14[-11], Atmp);
		for (j = 12; j < shapingLPCOrder; j += 2) {
			Atmp = AR_shp_Q13_tmp[j >> 1];	/* read two coefficients at once */
			n_AR_Q10 = SKP_SMLAWB(n_AR_Q10, psLPC_Q14[-j], Atmp);
			n_AR_Q10 =
			    SKP_SMLAWT(n_AR_Q10, psLPC_Q14[-j - 1], Atmp);
		}
		n_AR_Q10 = SKP_RSHIFT(n_AR_Q10, 1);	/* Q11 -> Q10 */
		n_AR_Q10 = SKP_SMLAWB(n_AR_Q10, NSQ->sLF_AR_shp_Q12, Tilt_Q14);

		n_LF_Q10 =
		    SKP_LSHIFT(SKP_SMULWB
			       (NSQ->sLTP_shp_Q10[NSQ->sLTP_shp_buf_idx - 1],
				LF_shp_Q14), 2);
		n_LF_Q10 =
		    SKP_SMLAWT(n_LF_Q10, NSQ->sLF_AR_shp_Q12, LF_shp_Q14);

		assert(lag > 0 || sigtype == SIG_TYPE_UNVOICED);

		/* Long-term shaping */
		if (lag > 0) {
			/* Symmetric, packed FIR coefficients */
			n_LTP_Q14 =
			    SKP_SMULWB(SKP_ADD32
				       (shp_lag_ptr[0], shp_lag_ptr[-2]),
				       HarmShapeFIRPacked_Q14);
			n_LTP_Q14 =
			    SKP_SMLAWT(n_LTP_Q14, shp_lag_ptr[-1],
				       HarmShapeFIRPacked_Q14);
			shp_lag_ptr++;
			n_LTP_Q14 = SKP_LSHIFT(n_LTP_Q14, 6);
		} else {
			n_LTP_Q14 = 0;
		}

		/* Input minus prediction plus noise feedback  */
		//r = x[ i ] - LTP_pred - LPC_pred + n_AR + n_Tilt + n_LF + n_LTP;
		tmp = SKP_SUB32(LTP_pred_Q14, n_LTP_Q14);	/* Add Q14 stuff */
		tmp = SKP_RSHIFT_ROUND(tmp, 4);	/* round to Q10  */
		tmp = SKP_ADD32(tmp, LPC_pred_Q10);	/* add Q10 stuff */
		tmp = SKP_SUB32(tmp, n_AR_Q10);	/* subtract Q10 stuff */
		tmp = SKP_SUB32(tmp, n_LF_Q10);	/* subtract Q10 stuff */
		r_Q10 = SKP_SUB32(x_sc_Q10[i], tmp);

		/* Flip sign depending on dither */
		r_Q10 = (r_Q10 ^ dither) - dither;
		r_Q10 = SKP_SUB32(r_Q10, offset_Q10);
		r_Q10 = SKP_LIMIT(r_Q10, -64 << 10, 64 << 10);

		/* Quantize */
		if (r_Q10 < thr1_Q10) {
			q_Q0 =
			    SKP_RSHIFT_ROUND(SKP_ADD_RSHIFT32
					     (r_Q10, Lambda_Q10, 1), 10);
			q_Q10 = SKP_LSHIFT(q_Q0, 10);
		} else if (r_Q10 < thr2_Q10) {
			q_Q0 = -1;
			q_Q10 = -1024;
		} else if (r_Q10 > thr3_Q10) {
			q_Q0 =
			    SKP_RSHIFT_ROUND(SKP_SUB_RSHIFT32
					     (r_Q10, Lambda_Q10, 1), 10);
			q_Q10 = SKP_LSHIFT(q_Q0, 10);
		} else {
			q_Q0 = 0;
			q_Q10 = 0;
		}
		q[i] = q_Q0;

		/* Excitation */
		exc_Q10 = SKP_ADD32(q_Q10, offset_Q10);
		exc_Q10 = (exc_Q10 ^ dither) - dither;

		/* Add predictions */
		LPC_exc_Q10 =
		    SKP_ADD32(exc_Q10, SKP_RSHIFT_ROUND(LTP_pred_Q14, 4));
		xq_Q10 = SKP_ADD32(LPC_exc_Q10, LPC_pred_Q10);

		/* Scale XQ back to normal level before saving */
		xq[i] =
		    (int16_t)
		    SKP_SAT16(SKP_RSHIFT_ROUND
			      (SKP_SMULWW(xq_Q10, Gain_Q16), 10));

		/* Update states */
		psLPC_Q14++;
		*psLPC_Q14 = SKP_LSHIFT(xq_Q10, 4);
		sLF_AR_shp_Q10 = SKP_SUB32(xq_Q10, n_AR_Q10);
		NSQ->sLF_AR_shp_Q12 = SKP_LSHIFT(sLF_AR_shp_Q10, 2);

		NSQ->sLTP_shp_Q10[NSQ->sLTP_shp_buf_idx] =
		    SKP_SUB32(sLF_AR_shp_Q10, n_LF_Q10);
		sLTP_Q16[NSQ->sLTP_buf_idx] = SKP_LSHIFT(LPC_exc_Q10, 6);
		NSQ->sLTP_shp_buf_idx++;
		NSQ->sLTP_buf_idx++;

		/* Make dither dependent on quantized signal */
		NSQ->rand_seed += q[i];
	}
	/* Update LPC synth buffer */
	SKP_memcpy(NSQ->sLPC_Q14, &NSQ->sLPC_Q14[length],
		   MAX_LPC_ORDER * sizeof(int32_t));
}

SKP_INLINE void SKP_Silk_nsq_scale_states(SKP_Silk_nsq_state * NSQ,	/* I/O NSQ state                        */
					  const int16_t x[],	/* I input in Q0                        */
					  int32_t x_sc_Q10[],	/* O input scaled with 1/Gain           */
					  int length,	/* I length of input                    */
					  int16_t sLTP[],	/* I re-whitened LTP state in Q0        */
					  int32_t sLTP_Q16[],	/* O LTP state matching scaled input    */
					  int subfr,	/* I subframe number                    */
					  const int LTP_scale_Q14,	/* I                                    */
					  const int32_t Gains_Q16[NB_SUBFR],	/* I                                 */
					  const int pitchL[NB_SUBFR]	/* I                                    */
    )
{
	int i, scale_length, lag;
	int32_t inv_gain_Q16, gain_adj_Q16, inv_gain_Q32;

	inv_gain_Q16 = SKP_DIV32(int32_t_MAX, SKP_RSHIFT(Gains_Q16[subfr], 1));
	inv_gain_Q16 = SKP_min(inv_gain_Q16, int16_t_MAX);
	lag = pitchL[subfr];

	/* After rewhitening the LTP state is un-scaled */
	if (NSQ->rewhite_flag) {
		inv_gain_Q32 = SKP_LSHIFT(inv_gain_Q16, 16);
		if (subfr == 0) {
			/* Do LTP downscaling */
			inv_gain_Q32 =
			    SKP_LSHIFT(SKP_SMULWB(inv_gain_Q32, LTP_scale_Q14),
				       2);
		}
		for (i = NSQ->sLTP_buf_idx - lag - LTP_ORDER / 2;
		     i < NSQ->sLTP_buf_idx; i++) {
			sLTP_Q16[i] = SKP_SMULWB(inv_gain_Q32, sLTP[i]);
		}
	}

	/* Prepare for Worst case. Next frame starts with max lag voiced */
	scale_length = length * NB_SUBFR;	/* approx max lag */
	scale_length = scale_length - SKP_SMULBB(NB_SUBFR - (subfr + 1), length);	/* subtract samples that will be too old in next frame */
	scale_length = SKP_max_int(scale_length, lag + LTP_ORDER);	/* make sure to scale whole pitch period if voiced */

	/* Adjust for changing gain */
	if (inv_gain_Q16 != NSQ->prev_inv_gain_Q16) {
		gain_adj_Q16 =
		    SKP_DIV32_varQ(inv_gain_Q16, NSQ->prev_inv_gain_Q16, 16);

		for (i = NSQ->sLTP_shp_buf_idx - scale_length;
		     i < NSQ->sLTP_shp_buf_idx; i++) {
			NSQ->sLTP_shp_Q10[i] =
			    SKP_SMULWW(gain_adj_Q16, NSQ->sLTP_shp_Q10[i]);
		}

		/* Scale LTP predict state */
		if (NSQ->rewhite_flag == 0) {
			for (i = NSQ->sLTP_buf_idx - lag - LTP_ORDER / 2;
			     i < NSQ->sLTP_buf_idx; i++) {
				sLTP_Q16[i] =
				    SKP_SMULWW(gain_adj_Q16, sLTP_Q16[i]);
			}
		}
		NSQ->sLF_AR_shp_Q12 =
		    SKP_SMULWW(gain_adj_Q16, NSQ->sLF_AR_shp_Q12);

		/* scale short term state */
		for (i = 0; i < MAX_LPC_ORDER; i++) {
			NSQ->sLPC_Q14[i] =
			    SKP_SMULWW(gain_adj_Q16, NSQ->sLPC_Q14[i]);
		}
	}

	/* Scale input */
	for (i = 0; i < length; i++) {
		x_sc_Q10[i] =
		    SKP_RSHIFT(SKP_SMULBB(x[i], (int16_t) inv_gain_Q16), 6);
	}

	/* save inv_gain */
	assert(inv_gain_Q16 != 0);
	NSQ->prev_inv_gain_Q16 = inv_gain_Q16;
}

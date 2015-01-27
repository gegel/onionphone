/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *
 *  FUNCTION:   w_D_plsf_5()
 *
 *  PURPOSE: Decodes the 2 sets of LSP parameters in a frame using the
 *           received quantization indices.
 *
 *  DESCRIPTION:
 *           The two sets of LSFs are quantized using split by 5 matrix
 *           quantization (split-MQ) with 1st order MA w_prediction.
 *
 *   See "w_q_plsf_5.c" for more details about the quantization procedure
 *
 *************************************************************************/

#include <stdint.h>
#include "basic_op.h"

#include "sig_proc.h"

#include "q_plsf_5.tab"		/* Codebooks of LSF w_prediction residual */

#include "cnst.h"
#include "dtx.h"

/* M  ->order of linear w_prediction filter                      */
/* LSF_GAP  -> Minimum distance between LSF after quantization */
/*             50 Hz = 205                                     */
/* PRED_FAC -> Prediction factor = 0.65                        */
/* ALPHA    ->  0.9                                            */
/* ONE_ALPHA-> (1.0-ALPHA)                                     */

#define M         10
#define LSF_GAP   205
#define PRED_FAC  21299
#define ALPHA     31128
#define ONE_ALPHA 1639

/* Past quantized w_prediction w_error */

int16_t v_past_r2_q[M];

/* Past dequantized lsfs */

int16_t w_past_lsf_q[M];

/* Reference LSF parameter vector (comfort noise) */

int16_t w_lsf_p_CN[M];

/*  LSF memories for comfort noise interpolation */

int16_t w_lsf_old_CN[M], w_lsf_new_CN[M];

 /* LSF parameter buffer */

extern int16_t w_lsf_old_rx[DTX_HANGOVER][M];

void w_D_plsf_5(int16_t * indice,	/* input : quantization indices of 5 w_submatrices */
		int16_t * lsp1_q,	/* output: quantized 1st LSP vector              */
		int16_t * lsp2_q,	/* output: quantized 2nd LSP vector              */
		int16_t bfi,	/* input : bad frame indicator (set to 1 if a bad
				   frame is received)                    */
		int16_t w_rxdtx_ctrl,	/* input : RX DTX control word                   */
		int16_t w_w_rx_dtx_w_state	/* input : w_state of the comfort noise insertion
						   period                                */
    )
{
	int16_t i;
	const int16_t *p_dico;
	int16_t temp, sign;
	int16_t lsf1_r[M], lsf2_r[M];
	int16_t lsf1_q[M], lsf2_q[M];

	/* Update comfort noise LSF quantizer memory */

	if ((w_rxdtx_ctrl & RX_UPD_SID_QUANT_MEM) != 0) {
		update_w_lsf_p_CN(w_lsf_old_rx, w_lsf_p_CN);
	}
	/* Handle cases of comfort noise LSF decoding in which past
	   valid SID frames are repeated */

	if (((w_rxdtx_ctrl & RX_NO_TRANSMISSION) != 0)
	    || ((w_rxdtx_ctrl & RX_INVALID_SID_FRAME) != 0)
	    || ((w_rxdtx_ctrl & RX_LOST_SID_FRAME) != 0)) {

		if ((w_rxdtx_ctrl & RX_NO_TRANSMISSION) != 0) {
			/* DTX active: no transmission. Interpolate LSF values in memory */
			w_interpolate_CN_lsf(w_lsf_old_CN, w_lsf_new_CN, lsf2_q,
					     w_w_rx_dtx_w_state);
		} else {	/* Invalid or lost SID frame: use LSFs
				   from last good SID frame */
			for (i = 0; i < M; i++) {
				w_lsf_old_CN[i] = w_lsf_new_CN[i];
				lsf2_q[i] = w_lsf_new_CN[i];
				v_past_r2_q[i] = 0;
			}
		}

		for (i = 0; i < M; i++) {
			w_past_lsf_q[i] = lsf2_q[i];
		}

		/*  convert LSFs to the cosine domain */
		w_Lsf_lsp(lsf2_q, lsp2_q, M);

		return;
	}

	if (bfi != 0) {		/* if bad frame */
		/* use the past LSFs slightly shifted towards their mean */

		for (i = 0; i < M; i++) {
			/* lsfi_q[i] = ALPHA*w_past_lsf_q[i] + ONE_ALPHA*w_mean_lsf[i]; */

			lsf1_q[i] = w_add(w_mult(w_past_lsf_q[i], ALPHA),
					  w_mult(w_mean_lsf[i], ONE_ALPHA));

			lsf2_q[i] = lsf1_q[i];
		}

		/* estimate past quantized residual to be used in next frame */

		for (i = 0; i < M; i++) {
			/* temp  = w_mean_lsf[i] +  v_past_r2_q[i] * PRED_FAC; */

			temp =
			    w_add(w_mean_lsf[i],
				  w_mult(v_past_r2_q[i], PRED_FAC));

			v_past_r2_q[i] = w_sub(lsf2_q[i], temp);

		}
	} else
		/* if good LSFs received */
	{
		/* decode w_prediction residuals from 5 received indices */

		p_dico = &w_dico1_lsf[w_shl(indice[0], 2)];
		lsf1_r[0] = *p_dico++;
		lsf1_r[1] = *p_dico++;
		lsf2_r[0] = *p_dico++;
		lsf2_r[1] = *p_dico;

		p_dico = &w_dico2_lsf[w_shl(indice[1], 2)];
		lsf1_r[2] = *p_dico++;
		lsf1_r[3] = *p_dico++;
		lsf2_r[2] = *p_dico++;
		lsf2_r[3] = *p_dico;

		sign = indice[2] & 1;
		i = w_shr(indice[2], 1);
		p_dico = &w_dico3_lsf[w_shl(i, 2)];

		if (sign == 0) {
			lsf1_r[4] = *p_dico++;
			lsf1_r[5] = *p_dico++;
			lsf2_r[4] = *p_dico++;
			lsf2_r[5] = *p_dico++;
		} else {
			lsf1_r[4] = w_negate(*p_dico++);
			lsf1_r[5] = w_negate(*p_dico++);
			lsf2_r[4] = w_negate(*p_dico++);
			lsf2_r[5] = w_negate(*p_dico++);
		}

		p_dico = &w_dico4_lsf[w_shl(indice[3], 2)];
		lsf1_r[6] = *p_dico++;
		lsf1_r[7] = *p_dico++;
		lsf2_r[6] = *p_dico++;
		lsf2_r[7] = *p_dico;

		p_dico = &w_dico5_lsf[w_shl(indice[4], 2)];
		lsf1_r[8] = *p_dico++;
		lsf1_r[9] = *p_dico++;
		lsf2_r[8] = *p_dico++;
		lsf2_r[9] = *p_dico++;

		/* Compute quantized LSFs and update the past quantized residual */
		/* Use w_lsf_p_CN as w_predicted LSF vector in case of no w_speech
		   activity */

		if ((w_rxdtx_ctrl & RX_SP_FLAG) != 0) {
			for (i = 0; i < M; i++) {
				temp =
				    w_add(w_mean_lsf[i],
					  w_mult(v_past_r2_q[i], PRED_FAC));
				lsf1_q[i] = w_add(lsf1_r[i], temp);

				lsf2_q[i] = w_add(lsf2_r[i], temp);

				v_past_r2_q[i] = lsf2_r[i];
			}
		} else {	/* Valid SID frame */
			for (i = 0; i < M; i++) {
				lsf2_q[i] = w_add(lsf2_r[i], w_lsf_p_CN[i]);

				/* Use the dequantized values of lsf2 also for lsf1 */
				lsf1_q[i] = lsf2_q[i];

				v_past_r2_q[i] = 0;
			}
		}
	}

	/* verification that LSFs have minimum distance of LSF_GAP Hz */

	w_Reorder_lsf(lsf1_q, LSF_GAP, M);
	w_Reorder_lsf(lsf2_q, LSF_GAP, M);

	if ((w_rxdtx_ctrl & RX_FIRST_SID_UPDATE) != 0) {
		for (i = 0; i < M; i++) {
			w_lsf_new_CN[i] = lsf2_q[i];
		}
	}

	if ((w_rxdtx_ctrl & RX_CONT_SID_UPDATE) != 0) {
		for (i = 0; i < M; i++) {
			w_lsf_old_CN[i] = w_lsf_new_CN[i];
			w_lsf_new_CN[i] = lsf2_q[i];
		}
	}

	if ((w_rxdtx_ctrl & RX_SP_FLAG) != 0) {
		/* Update lsf history with quantized LSFs
		   when w_speech activity is present. If the current frame is
		   a bad one, update with most recent good comfort noise LSFs */

		if (bfi == 0) {
			w_update_lsf_history(lsf1_q, lsf2_q, w_lsf_old_rx);
		} else {
			w_update_lsf_history(w_lsf_new_CN, w_lsf_new_CN,
					     w_lsf_old_rx);
		}

		for (i = 0; i < M; i++) {
			w_lsf_old_CN[i] = lsf2_q[i];
		}
	} else {
		w_interpolate_CN_lsf(w_lsf_old_CN, w_lsf_new_CN, lsf2_q,
				     w_w_rx_dtx_w_state);
	}

	for (i = 0; i < M; i++) {
		w_past_lsf_q[i] = lsf2_q[i];
	}

	/*  convert LSFs to the cosine domain */

	w_Lsf_lsp(lsf1_q, lsp1_q, M);
	w_Lsf_lsp(lsf2_q, lsp2_q, M);

	return;
}

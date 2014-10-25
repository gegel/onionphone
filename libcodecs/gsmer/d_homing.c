/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
 *
 *   File Name:  d_homing.c
 *
 *   Purpose:
 *      This file contains the following functions:
 *
 *      w_decoder_homing_frame_w_test()  checks if a frame of input w_speech
 *                                   parameters matches the Decoder Homing
 *                                   Frame pattern.
 *
 *      w_decoder_reset()              called by w_reset_dec() to reset all of
 *                                   the w_state variables for the decoder
 *
 *      w_reset_dec()                  calls functions to reset the w_state
 *                                   variables for the decoder, and for
 *                                   the receive DTX and Comfort Noise.
 *
 **************************************************************************/

#include <stdint.h>
#include "cnst.h"
#include "dtx.h"
#include "codec.h"
#include "d_homing.h"
#include "q_plsf_5.tab"

#define PRM_NO    57

/***************************************************************************
 *
 *   FUNCTION NAME:  w_decoder_homing_frame_w_test
 *
 *   PURPOSE:
 *      Checks if a frame of input w_speech parameters matches the Decoder
 *      Homing Frame pattern, which is:
 *
 *      parameter    decimal value    hexidecimal value
 *      ---------    -------------    -----------------
 *      LPC 1        4                0x0004
 *      LPC 2        47               0x002F
 *      LPC 3        180              0x00B4
 *      LPC 4        144              0x0090
 *      LPC 5        62               0x003E
 *      LTP-LAG 1    342              0x0156
 *      LTP-GAIN 1   11               0x000B
 *      PULSE1_1     0                0x0000
 *      PULSE1_2     1                0x0001
 *      PULSE1_3     15               0x000F
 *      PULSE1_4     1                0x0001
 *      PULSE1_5     13               0x000D
 *      PULSE1_6     0                0x0000
 *      PULSE1_7     3                0x0003
 *      PULSE1_8     0                0x0000
 *      PULSE1_9     3                0x0003
 *      PULSE1_10    0                0x0000
 *      FCB-GAIN 1   3                0x0003
 *      LTP-LAG 2    54               0x0036
 *      LTP-GAIN 2   1                0x0001
 *      PULSE2_1     8                0x0008
 *      PULSE2_2     8                0x0008
 *      PULSE2_3     5                0x0005
 *      PULSE2_4     8                0x0008
 *      PULSE2_5     1                0x0001
 *      PULSE2_6     0                0x0000
 *      PULSE2_7     0                0x0000
 *      PULSE2_8     1                0x0001
 *      PULSE2_9     1                0x0001
 *      PULSE2_10    0                0x0000
 *      FCB-GAIN 2   0                0x0000
 *      LTP-LAG 3    342              0x0156
 *      LTP-GAIN 3   0                0x0000
 *      PULSE3_1     0                0x0000
 *      PULSE3_2     0                0x0000
 *      PULSE3_3     0                0x0000
 *      PULSE3_4     0                0x0000
 *      PULSE3_5     0                0x0000
 *      PULSE3_6     0                0x0000
 *      PULSE3_7     0                0x0000
 *      PULSE3_8     0                0x0000
 *      PULSE3_9     0                0x0000
 *      PULSE3_10    0                0x0000
 *      FCB-GAIN 3   0                0x0000
 *      LTP-LAG 4    54               0x0036
 *      LTP-GAIN 4   11               0x000B
 *      PULSE4_1     0                0x0000
 *      PULSE4_2     0                0x0000
 *      PULSE4_3     0                0x0000
 *      PULSE4_4     0                0x0000
 *      PULSE4_5     0                0x0000
 *      PULSE4_6     0                0x0000
 *      PULSE4_7     0                0x0000
 *      PULSE4_8     0                0x0000
 *      PULSE4_9     0                0x0000
 *      PULSE4_10    0                0x0000
 *      FCB-GAIN 4   0                0x0000
 *
 *   INPUT:
 *      w_parm[]  one frame of w_speech parameters in parallel format
 *
 *      nbr_of_params
 *              the number of consecutive parameters in w_parm[] to match
 *
 *   OUTPUT:
 *      None
 *
 *   RETURN:
 *      0       input frame does not match the Decoder Homing Frame pattern.
 *      1       input frame matches the Decoder Homing Frame pattern.
 *
 **************************************************************************/

int16_t w_decoder_homing_frame_w_test(int16_t w_parm[], int16_t nbr_of_params)
{
	static const int16_t dhf_mask[PRM_NO] = {
		0x0004,		/* LPC 1 */
		0x002f,		/* LPC 2 */
		0x00b4,		/* LPC 3 */
		0x0090,		/* LPC 4 */
		0x003e,		/* LPC 5 */

		0x0156,		/* LTP-LAG 1 */
		0x000b,		/* LTP-GAIN 1 */
		0x0000,		/* PULSE 1_1 */
		0x0001,		/* PULSE 1_2 */
		0x000f,		/* PULSE 1_3 */
		0x0001,		/* PULSE 1_4 */
		0x000d,		/* PULSE 1_5 */
		0x0000,		/* PULSE 1_6 */
		0x0003,		/* PULSE 1_7 */
		0x0000,		/* PULSE 1_8 */
		0x0003,		/* PULSE 1_9 */
		0x0000,		/* PULSE 1_10 */
		0x0003,		/* FCB-GAIN 1 */

		0x0036,		/* LTP-LAG 2 */
		0x0001,		/* LTP-GAIN 2 */
		0x0008,		/* PULSE 2_1 */
		0x0008,		/* PULSE 2_2 */
		0x0005,		/* PULSE 2_3 */
		0x0008,		/* PULSE 2_4 */
		0x0001,		/* PULSE 2_5 */
		0x0000,		/* PULSE 2_6 */
		0x0000,		/* PULSE 2_7 */
		0x0001,		/* PULSE 2_8 */
		0x0001,		/* PULSE 2_9 */
		0x0000,		/* PULSE 2_10 */
		0x0000,		/* FCB-GAIN 2 */

		0x0156,		/* LTP-LAG 3 */
		0x0000,		/* LTP-GAIN 3 */
		0x0000,		/* PULSE 3_1 */
		0x0000,		/* PULSE 3_2 */
		0x0000,		/* PULSE 3_3 */
		0x0000,		/* PULSE 3_4 */
		0x0000,		/* PULSE 3_5 */
		0x0000,		/* PULSE 3_6 */
		0x0000,		/* PULSE 3_7 */
		0x0000,		/* PULSE 3_8 */
		0x0000,		/* PULSE 3_9 */
		0x0000,		/* PULSE 3_10 */
		0x0000,		/* FCB-GAIN 3 */

		0x0036,		/* LTP-LAG 4 */
		0x000b,		/* LTP-GAIN 4 */
		0x0000,		/* PULSE 4_1 */
		0x0000,		/* PULSE 4_2 */
		0x0000,		/* PULSE 4_3 */
		0x0000,		/* PULSE 4_4 */
		0x0000,		/* PULSE 4_5 */
		0x0000,		/* PULSE 4_6 */
		0x0000,		/* PULSE 4_7 */
		0x0000,		/* PULSE 4_8 */
		0x0000,		/* PULSE 4_9 */
		0x0000,		/* PULSE 4_10 */
		0x0000 /* FCB-GAIN 4 */
	};

	int16_t i, j = 0;

	for (i = 0; i < nbr_of_params; i++) {
		j = w_parm[i] ^ dhf_mask[i];

		if (j)
			break;
	}

	return !j;
}

/***************************************************************************
 *
 *   FUNCTION NAME:  w_decoder_reset
 *
 *   PURPOSE:
 *      resets all of the w_state variables for the decoder
 *
 *   INPUT:
 *      None
 *
 *   OUTPUT:
 *      None
 *
 *   RETURN:
 *      None
 *
 **************************************************************************/

void w_decoder_reset(void)
{
	/* External declarations for decoder variables which need to be reset */

	/* variable defined in decoder.c */
	/* ----------------------------- */
	extern int16_t synth_buf[L_FRAME + M];

	/* variable defined in w_agc.c */
	/* -------------------------- */
	extern int16_t w_past_gain;

	/* variables defined in d_gains.c */
	/* ------------------------------ */
	/* Error concealment */
	extern int16_t w_pbuf[5], w_w_past_gain_pit, w_prev_gp, w_gbuf[5],
	    w_w_past_gain_code, w_prev_gc;

	/* CNI */
	extern int16_t w_gcode0_CN, w_gain_code_old_CN, w_gain_code_new_CN;
	extern int16_t w_gain_code_muting_CN;

	/* Memories of gain dequantization: */
	extern int16_t v_past_qua_en[4], v_pred[4];

	/* variables defined in d_plsf_5.c */
	/* ------------------------------ */
	/* Past quantized w_prediction w_error */
	extern int16_t v_past_r2_q[M];

	/* Past dequantized lsfs */
	extern int16_t w_past_lsf_q[M];

	/* CNI */
	extern int16_t w_lsf_p_CN[M], w_lsf_new_CN[M], w_lsf_old_CN[M];

	/* variables defined in dec_lag6.c */
	/* ------------------------------ */
	extern int16_t w_old_T0;

	/* variable defined in preemph.c */
	/* ------------------------------ */
	extern int16_t w_mem_pre;

	int16_t i;

	/* reset all the decoder w_state variables */
	/* ------------------------------------- */

	/* Variable in decoder.c: */
	for (i = 0; i < M; i++) {
		synth_buf[i] = 0;
	}

	/* Variables in dec_12k2.c: */
	Init_w_Decoder_12k2();

	/* Variable in w_agc.c: */
	w_past_gain = 4096;

	/* Variables in d_gains.c: */
	for (i = 0; i < 5; i++) {
		w_pbuf[i] = 410;	/* Error concealment */
		w_gbuf[i] = 1;	/* Error concealment */
	}

	w_w_past_gain_pit = 0;	/* Error concealment */
	w_prev_gp = 4096;	/* Error concealment */
	w_w_past_gain_code = 0;	/* Error concealment */
	w_prev_gc = 1;		/* Error concealment */
	w_gcode0_CN = 0;	/* CNI */
	w_gain_code_old_CN = 0;	/* CNI */
	w_gain_code_new_CN = 0;	/* CNI */
	w_gain_code_muting_CN = 0;	/* CNI */

	for (i = 0; i < 4; i++) {
		v_past_qua_en[i] = -2381;	/* past quantized energies */
	}

	v_pred[0] = 44;		/* MA w_prediction coeff */
	v_pred[1] = 37;		/* MA w_prediction coeff */
	v_pred[2] = 22;		/* MA w_prediction coeff */
	v_pred[3] = 12;		/* MA w_prediction coeff */

	/* Variables in d_plsf_5.c: */
	for (i = 0; i < M; i++) {
		v_past_r2_q[i] = 0;	/* Past quantized w_prediction w_error */
		w_past_lsf_q[i] = w_mean_lsf[i];	/* Past dequantized lsfs */
		w_lsf_p_CN[i] = w_mean_lsf[i];	/* CNI */
		w_lsf_new_CN[i] = w_mean_lsf[i];	/* CNI */
		w_lsf_old_CN[i] = w_mean_lsf[i];	/* CNI */
	}

	/* Variable in dec_lag6.c: */
	w_old_T0 = 40;		/* Old integer lag */

	/* Variable in preemph.c: */
	w_mem_pre = 0;		/* Filter memory */

	/* Variables in pstfilt2.c: */
	w_Init_w_Post_Filter();

	return;
}

/***************************************************************************
 *
 *   FUNCTION NAME:  w_reset_dec
 *
 *   PURPOSE:
 *      resets all of the w_state variables for the decoder, and for the
 *      receive DTX and Comfort Noise.
 *
 *   INPUT:
 *      None
 *
 *   OUTPUT:
 *      None
 *
 *   RETURN:
 *      None
 *
 **************************************************************************/

void w_reset_dec(void)
{
	w_decoder_reset();	/* reset all the w_state variables in the w_speech decoder */
	w_reset_w_rx_dtx();	/* reset all the receive DTX and CN w_state variables */

	return;
}

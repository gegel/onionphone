/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
 *
 *   File Name:  e_homing.c
 *
 *   Purpose:
 *      This file contains the following functions:
 *
 *      w_encoder_homing_frame_w_test()  checks if a frame of input samples
 *                                   matches the Encoder Homing Frame pattern.
 *
 *      w_encoder_reset()              called by w_reset_enc() to reset all
 *                                   the w_state variables for the encoder.
 *
 *      w_reset_enc()                  calls functions to reset the w_state
 *                                   variables for the encoder and VAD, and
 *                                   for the transmit DTX and Comfort Noise.
 *
 **************************************************************************/

#include <stdint.h>
#include "cnst.h"
#include "vad.h"
#include "dtx.h"
#include "codec.h"
#include "sig_proc.h"
#include "e_homing.h"

/***************************************************************************
 *
 *   FUNCTION NAME:  w_encoder_homing_frame_w_test
 *
 *   PURPOSE:
 *      Checks if a frame of input samples matches the Encoder Homing Frame
 *      pattern, which is 0x0008 for all 160 samples in the frame.
 *
 *   INPUT:
 *      input_frame[]    one frame of w_speech samples
 *
 *   OUTPUT:
 *      None
 *
 *   RETURN:
 *      0       input frame does not match the Encoder Homing Frame pattern.
 *      1       input frame matches the Encoder Homing Frame pattern.
 *
 **************************************************************************/

int16_t w_encoder_homing_frame_w_test(int16_t input_frame[])
{
	int16_t i, j;

	for (i = 0; i < L_FRAME; i++) {
		j = input_frame[i] ^ EHF_MASK;

		if (j)
			break;
	}

	return !j;
}

/***************************************************************************
 *
 *   FUNCTION NAME:  w_encoder_reset
 *
 *   PURPOSE:
 *      resets all of the w_state variables for the encoder
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

void w_encoder_reset(void)
{
	/* External declarations for encoder variables which need to be reset */

	/* Variables defined in levinson.c */
	/* ------------------------------- */
	extern int16_t w_old_A[M + 1];	/* Last A(z) for case of unsw_table filter */

	/* Variables defined in q_gains.c */
	/* ------------------------------- */
	/* Memories of gain quantization: */
	extern int16_t w_past_qua_en[4], w_pred[4];

	/* Variables defined in w_q_plsf_5.c */
	/* ------------------------------- */
	/* Past quantized w_prediction w_error */
	extern int16_t w_past_r2_q[M];

	int16_t i;

	/* reset all the encoder w_state variables */
	/* ------------------------------------- */

	/* Variables in cod_12k2.c: */
	Init_w_Coder_12k2();

	/* Variables in levinson.c: */
	w_old_A[0] = 4096;	/* Last A(z) for case of unsw_table filter */
	for (i = 1; i < M + 1; i++) {
		w_old_A[i] = 0;
	}

	/* Variables in pre_proc.c: */
	w_Init_w_Pre_Process();

	/* Variables in q_gains.c: */
	for (i = 0; i < 4; i++) {
		w_past_qua_en[i] = -2381;	/* past quantized energies */
	}

	w_pred[0] = 44;		/* MA w_prediction coeff */
	w_pred[1] = 37;		/* MA w_prediction coeff */
	w_pred[2] = 22;		/* MA w_prediction coeff */
	w_pred[3] = 12;		/* MA w_prediction coeff */

	/* Variables in w_q_plsf_5.c: */
	for (i = 0; i < M; i++) {
		w_past_r2_q[i] = 0;	/* Past quantized w_prediction w_error */
	}

	return;
}

/***************************************************************************
 *
 *   FUNCTION NAME:  w_reset_enc
 *
 *   PURPOSE:
 *      resets all of the w_state variables for the encoder and VAD, and for
 *      the transmit DTX and Comfort Noise.
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

void w_reset_enc(void)
{
	w_encoder_reset();	/* reset all the w_state variables in the w_speech encoder */
	w_er_vad_reset();	/* reset all the VAD w_state variables */
	w_reset_w_tx_dtx();	/* reset all the transmit DTX and CN variables */

	return;
}

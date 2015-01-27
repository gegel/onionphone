/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/***************************************************************************
 *
 *  FILE NAME:    dec_12k2.c
 *
 *  FUNCTIONS DEFINED IN THIS FILE:
 *                   Init_w_Decoder_12k2   and  w_Decoder_12k2
 *
 *
 *  Init_w_Decoder_12k2():
 *      Initialization of variables for the decoder section.
 *
 *  w_Decoder_12k2():
 *      Speech decoder routine operating on a frame basis.
 *
 ***************************************************************************/

#include <stdint.h>
#include "basic_op.h"
#include "sig_proc.h"

#include "codec.h"
#include "cnst.h"

#include "dtx.h"

extern int16_t w_dtx_mode;

/*---------------------------------------------------------------*
 *   Decoder constant parameters (defined in "cnst.h")           *
 *---------------------------------------------------------------*
 *   L_FRAME     : Frame size.                                   *
 *   L_FRAME_BY2 : Half the frame size.                          *
 *   L_SUBFR     : Sub-frame size.                               *
 *   M           : LPC order.                                    *
 *   MP1         : LPC order+1                                   *
 *   PIT_MIN     : Minimum pitch lag.                            *
 *   PIT_MAX     : Maximum pitch lag.                            *
 *   L_INTERPOL  : Length of filter for interpolation            *
 *   PRM_SIZE    : size of vector containing analysis parameters *
 *---------------------------------------------------------------*/

/*--------------------------------------------------------*
 *         Static memory allocation.                      *
 *--------------------------------------------------------*/

 /* Excitation vector */

static int16_t old_v_exc[L_FRAME + PIT_MAX + L_INTERPOL];
static int16_t *v_exc;

 /* Lsp (Line spectral pairs) */

static int16_t v_lsp_old[M];

 /* Filter's memory */

static int16_t v_mem_v_syn[M];

 /* Memories for bad frame handling */

static int16_t w_prev_bf;
static int16_t w_state;

/***************************************************************************
 *
 *   FUNCTION:  Init_w_Decoder_12k2
 *
 *   PURPOSE: Initialization of variables for the decoder section.
 *
 ***************************************************************************/

void Init_w_Decoder_12k2(void)
{
	/* Initialize static pointer */

	v_exc = old_v_exc + PIT_MAX + L_INTERPOL;

	/* Static vectors to w_zero */

	w_Set_w_zero(old_v_exc, PIT_MAX + L_INTERPOL);
	w_Set_w_zero(v_mem_v_syn, M);

	/* Initialize v_lsp_old [] */

	v_lsp_old[0] = 30000;
	v_lsp_old[1] = 26000;
	v_lsp_old[2] = 21000;
	v_lsp_old[3] = 15000;
	v_lsp_old[4] = 8000;
	v_lsp_old[5] = 0;
	v_lsp_old[6] = -8000;
	v_lsp_old[7] = -15000;
	v_lsp_old[8] = -21000;
	v_lsp_old[9] = -26000;

	/* Initialize memories of bad frame handling */

	w_prev_bf = 0;
	w_state = 0;

	return;
}

/***************************************************************************
 *
 *   FUNCTION:  w_Decoder_12k2
 *
 *   PURPOSE:   Speech decoder routine.
 *
 ***************************************************************************/

void w_Decoder_12k2(int16_t w_parm[],	/* input : vector of w_w_synthesis parameters
					   w_parm[0] = bad frame indicator (bfi)       */
		    int16_t w_w_synth[],	/* output: w_w_synthesis w_speech                  */
		    int16_t A_t[],	/* output: decoded LP filter in 4 w_subframes  */
		    int16_t TAF, int16_t SID_flag)
{

	/* LPC coefficients */

	int16_t *Az;		/* Pointer on A_t */

	/* LSPs */

	int16_t lsp_new[M];
	int16_t lsp_mid[M];

	/* Algebraic codevector */

	int16_t code[L_SUBFR];

	/* v_excitation */

	int16_t v_excp[L_SUBFR];

	/* Scalars */

	int16_t i, i_w_subfr;
	int16_t T0, T0_frac, index;
	int16_t gain_pit, gain_code, bfi, pit_sharp;
	int16_t temp;
	int32_t L_temp;

	extern int16_t w_rxdtx_ctrl, w_w_rx_dtx_w_state;
	extern int32_t w_L_pn_seed_rx;

	/* Test bad frame indicator (bfi) */

	bfi = *w_parm++;

	/* Set w_state machine */

	if (bfi != 0) {
		w_state = w_add(w_state, 1);
	} else if (w_sub(w_state, 6) == 0) {
		w_state = 5;
	} else {
		w_state = 0;
	}

	if (w_sub(w_state, 6) > 0) {
		w_state = 6;
	}
	w_rx_dtx(&w_rxdtx_ctrl, TAF, bfi, SID_flag);

	/* If this frame is the first w_speech frame after CNI period,     */
	/* set the BFH w_state machine to an appropriate w_state depending   */
	/* on whether there was DTX muting before start of w_speech or not */
	/* If there was DTX muting, the first w_speech frame is muted.     */
	/* If there was no DTX muting, the first w_speech frame is not     */
	/* muted. The BFH w_state machine starts from w_state 5, however, to */
	/* keep the audible noise resulting from a SID frame which is    */
	/* erroneously interpreted as a good w_speech frame as small as    */
	/* possible (the decoder output in this case is quickly muted)   */

	if ((w_rxdtx_ctrl & RX_FIRST_SP_FLAG) != 0) {

		if ((w_rxdtx_ctrl & RX_PREV_DTX_MUTING) != 0) {
			w_state = 5;
			w_prev_bf = 1;
		} else {
			w_state = 5;
			w_prev_bf = 0;
		}
	}
#if (WMOPS)
	/* function worst case */

	/* Note! The following w_test is performed only for determining
	   whether or not DTX mode is active, in order to switch off
	   worst worst case complexity printout when DTX mode is active
	 */
	if ((w_rxdtx_ctrl & RX_SP_FLAG) == 0) {
		w_dtx_mode = 1;
	}
#endif

	w_D_plsf_5(w_parm, lsp_mid, lsp_new, bfi, w_rxdtx_ctrl,
		   w_w_rx_dtx_w_state);

#if (WMOPS)
	/* function worst case */
#endif
	/* Advance w_w_synthesis parameters pointer */
	w_parm += 5;

	if ((w_rxdtx_ctrl & RX_SP_FLAG) != 0) {
		/* Interpolation of LPC for the 4 w_subframes */

		w_Int_lpc(v_lsp_old, lsp_mid, lsp_new, A_t);
	} else {
		/* Comfort noise: use the same parameters in each w_subframe */
		w_Lsp_Az(lsp_new, A_t);

		for (i = 0; i < MP1; i++) {
			A_t[i + MP1] = A_t[i];
			A_t[i + 2 * MP1] = A_t[i];
			A_t[i + 3 * MP1] = A_t[i];
		}
	}

	/* update the LSPs for the next frame */
	for (i = 0; i < M; i++) {
		v_lsp_old[i] = lsp_new[i];
	}
#if (WMOPS)
	/* function worst case */
#endif

    /*---------------------------------------------------------------------*
     *       Loop for every w_subframe in the analysis frame                 *
     *---------------------------------------------------------------------*
     * The w_subframe size is L_SUBFR and the loop is repeated               *
     * L_FRAME/L_SUBFR times                                               *
     *     - decode the pitch delay                                        *
     *     - decode algebraic code                                         *
     *     - decode pitch and codebook gains                               *
     *     - find the v_excitation and compute w_w_synthesis w_speech              *
     *---------------------------------------------------------------------*/

	/* pointer to interpolated LPC parameters */
	Az = A_t;

	for (i_w_subfr = 0; i_w_subfr < L_FRAME; i_w_subfr += L_SUBFR) {

		index = *w_parm++;	/* pitch index */

		if ((w_rxdtx_ctrl & RX_SP_FLAG) != 0) {
			T0 = w_Dec_lag6(index, PIT_MIN, PIT_MAX, i_w_subfr,
					L_FRAME_BY2, &T0_frac, bfi);
#if (WMOPS)
			/* function worst case */
#endif

	    /*-------------------------------------------------*
             * - Find the adaptive codebook vector.            *
             *-------------------------------------------------*/

			w_Pred_lt_6(&v_exc[i_w_subfr], T0, T0_frac, L_SUBFR);
#if (WMOPS)
			/* function worst case */
#endif
		} else {
			T0 = L_SUBFR;
		}

	/*-------------------------------------------------------*
         * - Decode pitch gain.                                  *
         *-------------------------------------------------------*/

		index = *w_parm++;

		gain_pit =
		    w_d_gain_pitch(index, bfi, w_state, w_prev_bf,
				   w_rxdtx_ctrl);

#if (WMOPS)
		/* function worst case */
#endif

	/*-------------------------------------------------------*
         * - Decode innovative codebook.                         *
         *-------------------------------------------------------*/

		if ((w_rxdtx_ctrl & RX_SP_FLAG) != 0) {
			w_dec_10i40_35bits(w_parm, code);
		} else {	/* Use pseudo noise for v_excitation when SP_flag == 0 */
			w_build_CN_code(code, &w_L_pn_seed_rx);
		}

		w_parm += 10;
#if (WMOPS)
		/* function worst case */
#endif

	/*-------------------------------------------------------*
         * - Add the pitch contribution to code[].               *
         *-------------------------------------------------------*/

		/* pit_sharp = gain_pit;                   */
		/* if (pit_sharp > 1.0) pit_sharp = 1.0;   */

		pit_sharp = w_shl(gain_pit, 3);

		/* This loop is not entered when SP_FLAG is 0 */
		for (i = T0; i < L_SUBFR; i++) {
			temp = w_mult(code[i - T0], pit_sharp);
			code[i] = w_add(code[i], temp);

		}
#if (WMOPS)
		/* function worst case */
#endif
		/* post processing of v_excitation elements */

		/* This w_test is not passed when SP_FLAG is 0 */
		if (w_sub(pit_sharp, 16384) > 0) {
			for (i = 0; i < L_SUBFR; i++) {
				temp = w_mult(v_exc[i + i_w_subfr], pit_sharp);
				L_temp = w_L_w_mult(temp, gain_pit);
				L_temp = w_L_w_shl(L_temp, 1);
				v_excp[i] = w_round(L_temp);

			}
		}
	/*-------------------------------------------------*
         * - Decode codebook gain.                         *
         *-------------------------------------------------*/

		index = *w_parm++;	/* index of energy VQ */

		w_d_gain_code(index, code, L_SUBFR, &gain_code, bfi, w_state,
			      w_prev_bf, w_rxdtx_ctrl, i_w_subfr,
			      w_w_rx_dtx_w_state);
#if (WMOPS)
		/* function worst case */
#endif

	/*-------------------------------------------------------*
         * - Find the total v_excitation.                          *
         * - Find w_w_synthesis w_speech corresponding to v_exc[].       *
         *-------------------------------------------------------*/
		for (i = 0; i < L_SUBFR; i++) {
			/* v_exc[i] = gain_pit*v_exc[i] + gain_code*code[i]; */

			L_temp = w_L_w_mult(v_exc[i + i_w_subfr], gain_pit);
			L_temp = w_L_mac(L_temp, code[i], gain_code);
			L_temp = w_L_w_shl(L_temp, 3);

			v_exc[i + i_w_subfr] = w_round(L_temp);

		}
#if (WMOPS)
		/* function worst case */
#endif

		if (w_sub(pit_sharp, 16384) > 0) {
			for (i = 0; i < L_SUBFR; i++) {
				v_excp[i] =
				    w_add(v_excp[i], v_exc[i + i_w_subfr]);

			}
			w_w_agc2(&v_exc[i_w_subfr], v_excp, L_SUBFR);
			w_Syn_filt(Az, v_excp, &w_w_synth[i_w_subfr], L_SUBFR,
				   v_mem_v_syn, 1);
		} else {
			w_Syn_filt(Az, &v_exc[i_w_subfr], &w_w_synth[i_w_subfr],
				   L_SUBFR, v_mem_v_syn, 1);
		}

#if (WMOPS)
		/* function worst case */
#endif
		/* interpolated LPC parameters for next w_subframe */
		Az += MP1;
	}

    /*--------------------------------------------------*
     * Update signal for next frame.                    *
     * -> shift to the left by L_FRAME  v_exc[]           *
     *--------------------------------------------------*/

	w_Copy(&old_v_exc[L_FRAME], &old_v_exc[0], PIT_MAX + L_INTERPOL);
#if (WMOPS)
	/* function worst case */
#endif
	w_prev_bf = bfi;

	return;
}

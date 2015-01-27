/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/***************************************************************************
 *
 *   File Name: dtx.c
 *
 *   Purpose:   Contains functions for performing DTX operation and comfort
 *              noise generation.
 *
 *     Below is a listing of all the functions appearing in the file.
 *     The functions are arranged according to their purpose.  Under
 *     each heading, the ordering is hierarchical.
 *
 *     Resetting of static variables of TX DTX:
 *       w_reset_w_tx_dtx()
 *     Resetting of static variables of RX DTX:
 *       w_reset_w_rx_dtx()
 *
 *     TX DTX handler (called by the w_speech encoder):
 *       w_tx_dtx()
 *     RX DTX handler (called by the w_speech decoder):
 *       w_rx_dtx()
 *     Encoding of comfort noise parameters into SID frame:
 *       w_CN_encoding()
 *     Encoding of SID codeword into SID frame:
 *       w_sid_codeword_encoding()
 *     Detecting of SID codeword from a frame:
 *       w_sid_frame_detection()
 *     Update the LSF parameter history:
 *       w_update_lsf_history()
 *     Update the reference LSF parameter vector:
 *       update_w_lsf_p_CN()
 *     Compute the averaged LSF parameter vector:
 *       w_aver_lsf_history()
 *     Update the fixed codebook gain parameter history of the encoder:
 *       w_update_gain_code_history_tx()
 *     Update the fixed codebook gain parameter history of the decoder:
 *       w_update_gain_code_history_rx()
 *     Compute the unquantized fixed codebook gain:
 *       compute_w_CN_w_excitation_gain()
 *     Update the reference fixed codebook gain:
 *       update_w_gcode0_CN()
 *     Compute the averaged fixed codebook gain:
 *       w_aver_gain_code_history()
 *     Compute the comfort noise fixed codebook w_excitation:
 *       w_build_CN_code()
 *       Generate a random integer value:
 *         w_pseudonoise()
 *     Interpolate a comfort noise parameter value over the comfort noise
 *       update period:
 *       w_interpolate_CN_param()
 *     Interpolate comfort noise LSF pparameter values over the comfort
 *       noise update period:
 *       w_interpolate_CN_lsf()
 *         w_interpolate_CN_param()
 *
 **************************************************************************/

#include <stdint.h>
#include "basic_op.h"
#include "cnst.h"
#include "sig_proc.h"

#include "dtx.h"

/* Inverse values of DTX hangover period and DTX hangover period + 1 */

#define INV_DTX_HANGOVER (0x7fff / DTX_HANGOVER)
#define INV_DTX_HANGOVER_P1 (0x7fff / (DTX_HANGOVER+1))

#define NB_PULSE 10		/* Number of pulses in fixed codebook w_excitation */

/* SID frame classification thresholds */

#define VALID_SID_THRESH 2
#define INVALID_SID_THRESH 16

/* Constant DTX_ELAPSED_THRESHOLD is used as threshold for allowing
   SID frame updating without hangover period in case when elapsed
   time measured from previous SID update is below 24 */

#define DTX_ELAPSED_THRESHOLD (24 + DTX_HANGOVER - 1)

/* Index map for encoding and detecting SID codeword */

static const int16_t w_SID_codeword_bit_idx[95] = {
	45, 46, 48, 49, 50, 51, 52, 53, 54, 55,
	56, 57, 58, 59, 60, 61, 62, 63, 64, 65,
	66, 67, 68, 94, 95, 96, 98, 99, 100, 101,
	102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 148, 149, 150,
	151, 152, 153, 154, 155, 156, 157, 158, 159, 160,
	161, 162, 163, 164, 165, 166, 167, 168, 169, 170,
	171, 196, 197, 198, 199, 200, 201, 202, 203, 204,
	205, 206, 207, 208, 209, 212, 213, 214, 215, 216,
	217, 218, 219, 220, 221
};

int16_t w_txdtx_ctrl;		/* Encoder DTX control word                */
int16_t w_rxdtx_ctrl;		/* Decoder DTX control word                */
int16_t w_CN_w_excitation_gain;	/* Unquantized fixed codebook gain         */
int32_t w_L_pn_seed_tx;		/* PN generator seed (encoder)             */
int32_t w_L_pn_seed_rx;		/* PN generator seed (decoder)             */
int16_t w_w_rx_dtx_w_state;	/* State of comfort noise insertion period */

static int16_t w_txdtx_hangover;	/* Length of hangover period (VAD=0, SP=1) */
static int16_t w_rxdtx_aver_period;	/* Length of hangover period (VAD=0, SP=1) */
static int16_t w_txdtx_N_elapsed;	/* Measured time from previous SID frame   */
static int16_t w_rxdtx_N_elapsed;	/* Measured time from previous SID frame   */
static int16_t w_old_CN_mem_tx[6];	/* The most recent CN parameters are stored */
static int16_t w_prev_SID_frames_lost;	/* Counter for lost SID frames         */
static int16_t w_buf_p_tx;	/* Circular buffer pointer for gain code 
				   history  update in tx                   */
static int16_t w_buf_p_rx;	/* Circular buffer pointer for gain code 
				   history update in rx                    */

int16_t w_lsf_old_tx[DTX_HANGOVER][M];	/* Comfort noise LSF averaging buffer  */
int16_t w_lsf_old_rx[DTX_HANGOVER][M];	/* Comfort noise LSF averaging buffer  */

int16_t w_gain_code_old_tx[4 * DTX_HANGOVER];	/* Comfort noise gain averaging 
						   buffer                       */
int16_t w_gain_code_old_rx[4 * DTX_HANGOVER];	/* Comfort noise gain averaging 
						   buffer                       */

/*************************************************************************
 *
 *   FUNCTION NAME: w_reset_w_tx_dtx
 *
 *   PURPOSE:  Resets the static variables of the TX DTX handler to their
 *             initial values
 *
 *************************************************************************/

void w_reset_w_tx_dtx()
{
	int16_t i;

	/* suppose infinitely long w_speech period before start */

	w_txdtx_hangover = DTX_HANGOVER;
	w_txdtx_N_elapsed = 0x7fff;
	w_txdtx_ctrl = TX_SP_FLAG | TX_VAD_FLAG;

	for (i = 0; i < 6; i++) {
		w_old_CN_mem_tx[i] = 0;
	}

	for (i = 0; i < DTX_HANGOVER; i++) {
		w_lsf_old_tx[i][0] = 1384;
		w_lsf_old_tx[i][1] = 2077;
		w_lsf_old_tx[i][2] = 3420;
		w_lsf_old_tx[i][3] = 5108;
		w_lsf_old_tx[i][4] = 6742;
		w_lsf_old_tx[i][5] = 8122;
		w_lsf_old_tx[i][6] = 9863;
		w_lsf_old_tx[i][7] = 11092;
		w_lsf_old_tx[i][8] = 12714;
		w_lsf_old_tx[i][9] = 13701;
	}

	for (i = 0; i < 4 * DTX_HANGOVER; i++) {
		w_gain_code_old_tx[i] = 0;
	}

	w_L_pn_seed_tx = PN_INITIAL_SEED;

	w_buf_p_tx = 0;
	return;
}

/*************************************************************************
 *
 *   FUNCTION NAME: w_reset_w_rx_dtx
 *
 *   PURPOSE:  Resets the static variables of the RX DTX handler to their
 *             initial values
 *
 *************************************************************************/

void w_reset_w_rx_dtx()
{
	int16_t i;

	/* suppose infinitely long w_speech period before start */

	w_rxdtx_aver_period = DTX_HANGOVER;
	w_rxdtx_N_elapsed = 0x7fff;
	w_rxdtx_ctrl = RX_SP_FLAG;

	for (i = 0; i < DTX_HANGOVER; i++) {
		w_lsf_old_rx[i][0] = 1384;
		w_lsf_old_rx[i][1] = 2077;
		w_lsf_old_rx[i][2] = 3420;
		w_lsf_old_rx[i][3] = 5108;
		w_lsf_old_rx[i][4] = 6742;
		w_lsf_old_rx[i][5] = 8122;
		w_lsf_old_rx[i][6] = 9863;
		w_lsf_old_rx[i][7] = 11092;
		w_lsf_old_rx[i][8] = 12714;
		w_lsf_old_rx[i][9] = 13701;
	}

	for (i = 0; i < 4 * DTX_HANGOVER; i++) {
		w_gain_code_old_rx[i] = 0;
	}

	w_L_pn_seed_rx = PN_INITIAL_SEED;
	w_w_rx_dtx_w_state = CN_INT_PERIOD - 1;

	w_prev_SID_frames_lost = 0;
	w_buf_p_rx = 0;

	return;
}

/*************************************************************************
 *
 *   FUNCTION NAME: w_tx_dtx
 *
 *   PURPOSE: DTX handler of the w_speech encoder. Determines when to w_add
 *            the hangover period to the end of the w_speech burst, and
 *            also determines when to use old SID parameters, and when
 *            to update the SID parameters. This function also initializes
 *            the pseudo noise generator shift register.
 *
 *            Operation of the TX DTX handler is based on the VAD flag
 *            given as input from the w_speech encoder.
 *
 *   INPUTS:      VAD_flag      Voice activity decision
 *                *w_txdtx_ctrl   Old encoder DTX control word
 *
 *   OUTPUTS:     *w_txdtx_ctrl   Updated encoder DTX control word
 *                w_L_pn_seed_tx  Initialized pseudo noise generator shift
 *                              register (global variable)
 *
 *   RETURN VALUE: none
 *
 *************************************************************************/

void w_tx_dtx(int16_t VAD_flag, int16_t * w_txdtx_ctrl)
{

	/* N_elapsed (frames since last SID update) is incremented. If SID
	   is updated N_elapsed is cleared later in this function */

	w_txdtx_N_elapsed = w_add(w_txdtx_N_elapsed, 1);

	/* If voice activity was detected, reset hangover w_counter */

	if (w_sub(VAD_flag, 1) == 0) {
		w_txdtx_hangover = DTX_HANGOVER;
		*w_txdtx_ctrl = TX_SP_FLAG | TX_VAD_FLAG;
	} else {

		if (w_txdtx_hangover == 0) {
			/* Hangover period is over, SID should be updated */

			w_txdtx_N_elapsed = 0;

			/* Check if this is the first frame after hangover period */

			if ((*w_txdtx_ctrl & TX_HANGOVER_ACTIVE) != 0) {
				*w_txdtx_ctrl = TX_PREV_HANGOVER_ACTIVE
				    | TX_SID_UPDATE;
				w_L_pn_seed_tx = PN_INITIAL_SEED;
			} else {
				*w_txdtx_ctrl = TX_SID_UPDATE;
			}
		} else {
			/* Hangover period is not over, update hangover w_counter */
			w_txdtx_hangover = w_sub(w_txdtx_hangover, 1);

			/* Check if elapsed time from last SID update is greater than
			   threshold. If not, set SP=0 (although hangover period is not
			   over) and use old SID parameters for new SID frame.
			   N_elapsed w_counter must be summed with hangover w_counter in order
			   to avoid erroneus SP=1 decision in case when N_elapsed is grown
			   bigger than threshold and hangover period is still active */

			if (w_sub(w_add(w_txdtx_N_elapsed, w_txdtx_hangover),
				  DTX_ELAPSED_THRESHOLD) < 0) {
				/* old SID frame should be used */
				*w_txdtx_ctrl = TX_USE_OLD_SID;
			} else {

				if ((*w_txdtx_ctrl & TX_HANGOVER_ACTIVE) != 0) {
					*w_txdtx_ctrl = TX_PREV_HANGOVER_ACTIVE
					    | TX_HANGOVER_ACTIVE | TX_SP_FLAG;
				} else {
					*w_txdtx_ctrl = TX_HANGOVER_ACTIVE
					    | TX_SP_FLAG;
				}
			}
		}
	}

	return;
}

/*************************************************************************
 *
 *   FUNCTION NAME: w_rx_dtx
 *
 *   PURPOSE: DTX handler of the w_speech decoder. Determines when to update
 *            the reference comfort noise parameters (LSF and gain) at the
 *            end of the w_speech burst. Also classifies the incoming frames
 *            according to SID flag and BFI flag
 *            and determines when the transmission is active during comfort
 *            noise insertion. This function also initializes the pseudo
 *            noise generator shift register.
 *
 *            Operation of the RX DTX handler is based on measuring the
 *            lengths of w_speech bursts and the lengths of the pauses between
 *            w_speech bursts to determine when there exists a hangover period
 *            at the end of a w_speech burst. The idea is to keep in w_sync with
 *            the TX DTX handler to be able to update the reference comfort
 *            noise parameters at the same time instances.
 *
 *   INPUTS:      *w_rxdtx_ctrl   Old decoder DTX control word
 *                TAF           Time alignment flag
 *                bfi           Bad frame indicator flag
 *                SID_flag      Silence descriptor flag
 *
 *   OUTPUTS:     *w_rxdtx_ctrl   Updated decoder DTX control word
 *                w_w_rx_dtx_w_state  Updated w_state of comfort noise interpolation
 *                              period (global variable)
 *                w_L_pn_seed_rx  Initialized pseudo noise generator shift
 *                              register (global variable)
 *
 *   RETURN VALUE: none
 *
 *************************************************************************/

void w_rx_dtx(int16_t * w_rxdtx_ctrl, int16_t TAF, int16_t bfi,
	      int16_t SID_flag)
{
	int16_t frame_type;

	/* Frame classification according to bfi-flag and ternary-valued
	   SID flag. The frames between SID updates (not actually trans-
	   mitted) are also classified here; they will be discarded later
	   and provided with "NO TRANSMISSION"-flag */

	if ((w_sub(SID_flag, 2) == 0) && (bfi == 0)) {
		frame_type = VALID_SID_FRAME;
	} else if ((SID_flag == 0) && (bfi == 0)) {
		frame_type = GOOD_SPEECH_FRAME;
	} else if ((SID_flag == 0) && (bfi != 0)) {
		frame_type = UNUSABLE_FRAME;
	} else {
		frame_type = INVALID_SID_FRAME;
	}

	/* Update of decoder w_state */
	/* Previous frame was classified as a w_speech frame */

	if ((*w_rxdtx_ctrl & RX_SP_FLAG) != 0) {

		if (w_sub(frame_type, VALID_SID_FRAME) == 0) {
			*w_rxdtx_ctrl = RX_FIRST_SID_UPDATE;
		} else if (w_sub(frame_type, INVALID_SID_FRAME) == 0) {
			*w_rxdtx_ctrl = RX_FIRST_SID_UPDATE
			    | RX_INVALID_SID_FRAME;
		} else if (w_sub(frame_type, UNUSABLE_FRAME) == 0) {
			*w_rxdtx_ctrl = RX_SP_FLAG;
		} else if (w_sub(frame_type, GOOD_SPEECH_FRAME) == 0) {
			*w_rxdtx_ctrl = RX_SP_FLAG;
		}
	} else {

		if (w_sub(frame_type, VALID_SID_FRAME) == 0) {
			*w_rxdtx_ctrl = RX_CONT_SID_UPDATE;
		} else if (w_sub(frame_type, INVALID_SID_FRAME) == 0) {
			*w_rxdtx_ctrl = RX_CONT_SID_UPDATE
			    | RX_INVALID_SID_FRAME;
		} else if (w_sub(frame_type, UNUSABLE_FRAME) == 0) {
			*w_rxdtx_ctrl = RX_CNI_BFI;
		} else if (w_sub(frame_type, GOOD_SPEECH_FRAME) == 0) {
			/* If the previous frame (during CNI period) was muted,
			   raise the RX_PREV_DTX_MUTING flag */

			if ((*w_rxdtx_ctrl & RX_DTX_MUTING) != 0) {
				*w_rxdtx_ctrl = RX_SP_FLAG | RX_FIRST_SP_FLAG
				    | RX_PREV_DTX_MUTING;

			} else {
				*w_rxdtx_ctrl = RX_SP_FLAG | RX_FIRST_SP_FLAG;

			}
		}
	}

	if ((*w_rxdtx_ctrl & RX_SP_FLAG) != 0) {
		w_prev_SID_frames_lost = 0;
		w_w_rx_dtx_w_state = CN_INT_PERIOD - 1;
	} else {
		/* First SID frame */

		if ((*w_rxdtx_ctrl & RX_FIRST_SID_UPDATE) != 0) {
			w_prev_SID_frames_lost = 0;
			w_w_rx_dtx_w_state = CN_INT_PERIOD - 1;
		}

		/* SID frame detected, but not the first SID */

		if ((*w_rxdtx_ctrl & RX_CONT_SID_UPDATE) != 0) {
			w_prev_SID_frames_lost = 0;

			if (w_sub(frame_type, VALID_SID_FRAME) == 0) {
				w_w_rx_dtx_w_state = 0;
			} else if (w_sub(frame_type, INVALID_SID_FRAME) == 0) {

				if (w_sub
				    (w_w_rx_dtx_w_state,
				     (CN_INT_PERIOD - 1)) < 0) {
					w_w_rx_dtx_w_state =
					    w_add(w_w_rx_dtx_w_state, 1);
				}
			}
		}

		/* Bad frame received in CNI mode */

		if ((*w_rxdtx_ctrl & RX_CNI_BFI) != 0) {

			if (w_sub(w_w_rx_dtx_w_state, (CN_INT_PERIOD - 1)) < 0) {
				w_w_rx_dtx_w_state =
				    w_add(w_w_rx_dtx_w_state, 1);
			}

			/* If an unusable frame is received during CNI period
			   when TAF == 1, the frame is classified as a lost
			   SID frame */

			if (w_sub(TAF, 1) == 0) {
				*w_rxdtx_ctrl =
				    *w_rxdtx_ctrl | RX_LOST_SID_FRAME;

				w_prev_SID_frames_lost =
				    w_add(w_prev_SID_frames_lost, 1);
			} else {	/* No transmission occurred */

				*w_rxdtx_ctrl =
				    *w_rxdtx_ctrl | RX_NO_TRANSMISSION;

			}

			if (w_sub(w_prev_SID_frames_lost, 1) > 0) {
				*w_rxdtx_ctrl = *w_rxdtx_ctrl | RX_DTX_MUTING;

			}
		}
	}

	/* N_elapsed (frames since last SID update) is incremented. If SID
	   is updated N_elapsed is cleared later in this function */

	w_rxdtx_N_elapsed = w_add(w_rxdtx_N_elapsed, 1);

	if ((*w_rxdtx_ctrl & RX_SP_FLAG) != 0) {
		w_rxdtx_aver_period = DTX_HANGOVER;
	} else {

		if (w_sub(w_rxdtx_N_elapsed, DTX_ELAPSED_THRESHOLD) > 0) {
			*w_rxdtx_ctrl |= RX_UPD_SID_QUANT_MEM;
			w_rxdtx_N_elapsed = 0;
			w_rxdtx_aver_period = 0;
			w_L_pn_seed_rx = PN_INITIAL_SEED;
		} else if (w_rxdtx_aver_period == 0) {
			w_rxdtx_N_elapsed = 0;
		} else {
			w_rxdtx_aver_period = w_sub(w_rxdtx_aver_period, 1);
		}
	}

	return;
}

/*************************************************************************
 *
 *   FUNCTION NAME: w_CN_encoding
 *
 *   PURPOSE:  Encoding of the comfort noise parameters into a SID frame.
 *             Use old SID parameters if necessary. Set the parameter
 *             indices not used by comfort noise parameters to w_zero.
 *
 *   INPUTS:      params[0..56]  Comfort noise parameter frame from the
 *                               w_speech encoder
 *                w_txdtx_ctrl     TX DTX handler control word
 *
 *   OUTPUTS:     params[0..56]  Comfort noise encoded parameter frame
 *
 *   RETURN VALUE: none
 *
 *************************************************************************/

void w_CN_encoding(int16_t params[], int16_t w_txdtx_ctrl)
{
	int16_t i;

	if ((w_txdtx_ctrl & TX_SID_UPDATE) != 0) {
		/* Store new CN parameters in memory to be used later as old
		   CN parameters */

		/* LPC parameter indices */
		for (i = 0; i < 5; i++) {
			w_old_CN_mem_tx[i] = params[i];
		}
		/* Codebook index computed in last w_subframe */
		w_old_CN_mem_tx[5] = params[56];
	}

	if ((w_txdtx_ctrl & TX_USE_OLD_SID) != 0) {
		/* Use old CN parameters previously stored in memory */
		for (i = 0; i < 5; i++) {
			params[i] = w_old_CN_mem_tx[i];
		}
		params[17] = w_old_CN_mem_tx[5];
		params[30] = w_old_CN_mem_tx[5];
		params[43] = w_old_CN_mem_tx[5];
		params[56] = w_old_CN_mem_tx[5];
	}
	/* Set all the rest of the parameters to w_zero (SID codeword will
	   be written later) */
	for (i = 0; i < 12; i++) {
		params[i + 5] = 0;
		params[i + 18] = 0;
		params[i + 31] = 0;
		params[i + 44] = 0;
	}

	return;
}

/*************************************************************************
 *
 *   FUNCTION NAME: w_sid_codeword_encoding
 *
 *   PURPOSE:  Encoding of the SID codeword into the SID frame. The SID
 *             codeword consists of 95 bits, all set to '1'.
 *
 *   INPUTS:      ser2[0..243]  Serial-mode w_speech parameter frame before
 *                              writing SID codeword into it
 *
 *   OUTPUTS:     ser2[0..243]  Serial-mode w_speech parameter frame with
 *                              SID codeword written into it
 *
 *   RETURN VALUE: none
 *
 *************************************************************************/

void w_sid_codeword_encoding(int16_t ser2[]
    )
{
	int16_t i;

	for (i = 0; i < 95; i++) {
		ser2[w_SID_codeword_bit_idx[i]] = 1;
	}

	return;
}

/*************************************************************************
 *
 *   FUNCTION NAME: w_sid_frame_detection
 *
 *   PURPOSE:  Detecting of SID codeword from a received frame. The frames
 *             are classified into three categories based on how many bit
 *             w_errors occur in the SID codeword:
 *                 - VALID SID FRAME
 *                 - INVALID SID FRAME
 *                 - SPEECH FRAME
 *
 *   INPUTS:      ser2[0..243]  Received w_serial-mode w_speech parameter frame
 *
 *   OUTPUTS:     none
 *
 *   RETURN VALUE: Ternary-valued SID classification flag
 *
 *************************************************************************/

int16_t w_sid_frame_detection(int16_t ser2[]
    )
{
	int16_t i, nbr_w_errors, sid;

	/* Search for bit w_errors in SID codeword */
	nbr_w_errors = 0;
	for (i = 0; i < 95; i++) {

		if (ser2[w_SID_codeword_bit_idx[i]] == 0) {
			nbr_w_errors = w_add(nbr_w_errors, 1);
		}
	}

	/* Frame classification */

	if (w_sub(nbr_w_errors, VALID_SID_THRESH) < 0) {	/* Valid SID frame */
		sid = 2;
	} else if (w_sub(nbr_w_errors, INVALID_SID_THRESH) < 0) {	/* Invalid SID frame */
		sid = 1;
	} else {		/* Speech frame */
		sid = 0;
	}

	return sid;
}

/*************************************************************************
 *
 *   FUNCTION NAME: w_update_lsf_history
 *
 *   PURPOSE: Update the LSF parameter history. The LSF parameters kept
 *            in the buffer are used later for computing the reference
 *            LSF parameter vector and the averaged LSF parameter vector.
 *
 *   INPUTS:      lsf1[0..9]    LSF vector of the 1st half of the frame
 *                lsf2[0..9]    LSF vector of the 2nd half of the frame
 *                lsf_old[0..DTX_HANGOVER-1][0..M-1]
 *                              Old LSF history
 *
 *   OUTPUTS:     lsf_old[0..DTX_HANGOVER-1][0..M-1]
 *                              Updated LSF history
 *
 *   RETURN VALUE: none
 *
 *************************************************************************/

void w_update_lsf_history(int16_t lsf1[M],
			  int16_t lsf2[M], int16_t lsf_old[DTX_HANGOVER][M]
    )
{
	int16_t i, j, temp;

	/* shift LSF data to make room for LSFs from current frame */
	/* This can also be implemented by using circular buffering */

	for (i = DTX_HANGOVER - 1; i > 0; i--) {
		for (j = 0; j < M; j++) {
			lsf_old[i][j] = lsf_old[i - 1][j];
		}
	}

	/* Store new LSF data to lsf_old buffer */

	for (i = 0; i < M; i++) {
		temp = w_add(w_shr(lsf1[i], 1), w_shr(lsf2[i], 1));
		lsf_old[0][i] = temp;
	}

	return;
}

/*************************************************************************
 *
 *   FUNCTION NAME: update_w_lsf_p_CN
 *
 *   PURPOSE: Update the reference LSF parameter vector. The reference
 *            vector is computed by averaging the quantized LSF parameter
 *            vectors which exist in the LSF parameter history.
 *
 *   INPUTS:      lsf_old[0..DTX_HANGOVER-1][0..M-1]
 *                                 LSF parameter history
 *
 *   OUTPUTS:     w_lsf_p_CN[0..9]   Computed reference LSF parameter vector
 *
 *   RETURN VALUE: none
 *
 *************************************************************************/

void update_w_lsf_p_CN(int16_t lsf_old[DTX_HANGOVER][M], int16_t w_lsf_p_CN[M]
    )
{
	int16_t i, j;
	int32_t L_temp;

	for (j = 0; j < M; j++) {
		L_temp = w_L_w_mult(INV_DTX_HANGOVER, lsf_old[0][j]);
		for (i = 1; i < DTX_HANGOVER; i++) {
			L_temp =
			    w_L_mac(L_temp, INV_DTX_HANGOVER, lsf_old[i][j]);
		}
		w_lsf_p_CN[j] = w_round(L_temp);
	}

	return;
}

/*************************************************************************
 *
 *   FUNCTION NAME: w_aver_lsf_history
 *
 *   PURPOSE: Compute the averaged LSF parameter vector. Computation is
 *            performed by averaging the LSF parameter vectors which exist
 *            in the LSF parameter history, together with the LSF
 *            parameter vectors of the current frame.
 *
 *   INPUTS:      lsf_old[0..DTX_HANGOVER-1][0..M-1]
 *                                   LSF parameter history
 *                lsf1[0..M-1]       LSF vector of the 1st half of the frame
 *                lsf2[0..M-1]       LSF vector of the 2nd half of the frame
 *
 *   OUTPUTS:     lsf_aver[0..M-1]   Averaged LSF parameter vector
 *
 *   RETURN VALUE: none
 *
 *************************************************************************/

void w_aver_lsf_history(int16_t lsf_old[DTX_HANGOVER][M],
			int16_t lsf1[M], int16_t lsf2[M], int16_t lsf_aver[M]
    )
{
	int16_t i, j;
	int32_t L_temp;

	for (j = 0; j < M; j++) {
		L_temp = w_L_w_mult(0x3fff, lsf1[j]);
		L_temp = w_L_mac(L_temp, 0x3fff, lsf2[j]);
		L_temp = w_L_w_mult(INV_DTX_HANGOVER_P1, w_extract_h(L_temp));

		for (i = 0; i < DTX_HANGOVER; i++) {
			L_temp =
			    w_L_mac(L_temp, INV_DTX_HANGOVER_P1, lsf_old[i][j]);
		}

		lsf_aver[j] = w_extract_h(L_temp);
	}

	return;
}

/*************************************************************************
 *
 *   FUNCTION NAME: w_update_gain_code_history_tx
 *
 *   PURPOSE: Update the fixed codebook gain parameter history of the
 *            encoder. The fixed codebook gain parameters kept in the buffer
 *            are used later for computing the reference fixed codebook
 *            gain parameter value and the averaged fixed codebook gain
 *            parameter value.
 *
 *   INPUTS:      new_gain_code   New fixed codebook gain value
 *
 *                w_gain_code_old_tx[0..4*DTX_HANGOVER-1]
 *                                Old fixed codebook gain history of encoder
 *
 *   OUTPUTS:     w_gain_code_old_tx[0..4*DTX_HANGOVER-1]
 *                                Updated fixed codebook gain history of encoder
 *
 *   RETURN VALUE: none
 *
 *************************************************************************/

void w_update_gain_code_history_tx(int16_t new_gain_code,
				   int16_t w_gain_code_old_tx[4 * DTX_HANGOVER]
    )
{

	/* Circular buffer */
	w_gain_code_old_tx[w_buf_p_tx] = new_gain_code;

	if (w_sub(w_buf_p_tx, (4 * DTX_HANGOVER - 1)) == 0) {
		w_buf_p_tx = 0;
	} else {
		w_buf_p_tx = w_add(w_buf_p_tx, 1);
	}

	return;
}

/*************************************************************************
 *
 *   FUNCTION NAME: w_update_gain_code_history_rx
 *
 *   PURPOSE: Update the fixed codebook gain parameter history of the
 *            decoder. The fixed codebook gain parameters kept in the buffer
 *            are used later for computing the reference fixed codebook
 *            gain parameter value.
 *
 *   INPUTS:      new_gain_code   New fixed codebook gain value
 *
 *                w_gain_code_old_tx[0..4*DTX_HANGOVER-1]
 *                                Old fixed codebook gain history of decoder
 *
 *   OUTPUTS:     w_gain_code_old_tx[0..4*DTX_HANGOVER-1]
 *                                Updated fixed codebk gain history of decoder
 *
 *   RETURN VALUE: none
 *
 *************************************************************************/

void w_update_gain_code_history_rx(int16_t new_gain_code,
				   int16_t w_gain_code_old_rx[4 * DTX_HANGOVER]
    )
{

	/* Circular buffer */
	w_gain_code_old_rx[w_buf_p_rx] = new_gain_code;

	if (w_sub(w_buf_p_rx, (4 * DTX_HANGOVER - 1)) == 0) {
		w_buf_p_rx = 0;
	} else {
		w_buf_p_rx = w_add(w_buf_p_rx, 1);
	}

	return;
}

/*************************************************************************
 *
 *   FUNCTION NAME: compute_w_CN_w_excitation_gain
 *
 *   PURPOSE: Compute the unquantized fixed codebook gain. Computation is
 *            based on the energy of the Linear Prediction residual signal.
 *
 *   INPUTS:      w_res2[0..39]   Linear Prediction residual signal
 *
 *   OUTPUTS:     none
 *
 *   RETURN VALUE: Unquantized fixed codebook gain
 *
 *************************************************************************/

int16_t compute_w_CN_w_excitation_gain(int16_t w_res2[L_SUBFR]
    )
{
	int16_t i, norm, norm1, temp, overfl;
	int32_t L_temp;

	/* Compute the energy of the LP residual signal */

	norm = 0;
	do {
		overfl = 0;

		L_temp = 0L;
		for (i = 0; i < L_SUBFR; i++) {
			temp = w_shr(w_res2[i], norm);
			L_temp = w_L_mac(L_temp, temp, temp);
		}

		if (w_L_w_sub(L_temp, MAX_32) == 0) {
			norm = w_add(norm, 1);
			overfl = 1;	/* Set the overflow flag */
		}

	}
	while (overfl != 0);

	L_temp = L_w_add(L_temp, 1L);	/* Avoid the case of all w_zeros */

	/* Take the square root of the obtained energy value (sqroot is a 2nd
	   order Taylor series approximation) */

	norm1 = w_norm_l(L_temp);
	temp = w_extract_h(w_L_w_shl(L_temp, norm1));
	L_temp = w_L_w_mult(temp, temp);
	L_temp = w_L_w_sub(805306368L, w_L_w_shr(L_temp, 3));
	L_temp = L_w_add(L_temp, w_L_w_mult(24576, temp));

	temp = w_extract_h(L_temp);

	if ((norm1 & 0x0001) != 0) {
		temp = w_w_mult_r(temp, 23170);
		norm1 = w_sub(norm1, 1);
	}
	/* Divide the result of sqroot operation by sqroot(10) */

	temp = w_w_mult_r(temp, 10362);

	/* Re-scale to get the final value */

	norm1 = w_shr(norm1, 1);
	norm1 = w_sub(norm1, norm);

	if (norm1 >= 0) {
		temp = w_shr(temp, norm1);
	} else {
		temp = w_shl(temp, w_abs_s(norm1));
	}

	return temp;
}

/*************************************************************************
 *
 *   FUNCTION NAME: update_w_gcode0_CN
 *
 *   PURPOSE: Update the reference fixed codebook gain parameter value.
 *            The reference value is computed by averaging the quantized
 *            fixed codebook gain parameter values which exist in the
 *            fixed codebook gain parameter history.
 *
 *   INPUTS:      gain_code_old[0..4*DTX_HANGOVER-1]
 *                              fixed codebook gain parameter history
 *
 *   OUTPUTS:     none
 *
 *   RETURN VALUE: Computed reference fixed codebook gain
 *
 *************************************************************************/

int16_t update_w_gcode0_CN(int16_t gain_code_old[4 * DTX_HANGOVER]
    )
{
	int16_t i, j;
	int32_t L_temp, L_ret;

	L_ret = 0L;
	for (i = 0; i < DTX_HANGOVER; i++) {
		L_temp = w_L_w_mult(0x1fff, gain_code_old[4 * i]);
		for (j = 1; j < 4; j++) {
			L_temp =
			    w_L_mac(L_temp, 0x1fff, gain_code_old[4 * i + j]);
		}
		L_ret = w_L_mac(L_ret, INV_DTX_HANGOVER, w_extract_h(L_temp));
	}

	return w_extract_h(L_ret);
}

/*************************************************************************
 *
 *   FUNCTION NAME: w_aver_gain_code_history
 *
 *   PURPOSE: Compute the averaged fixed codebook gain parameter value.
 *            Computation is performed by averaging the fixed codebook
 *            gain parameter values which exist in the fixed codebook
 *            gain parameter history, together with the fixed codebook
 *            gain parameter value of the current w_subframe.
 *
 *   INPUTS:      w_CN_w_excitation_gain
 *                              Unquantized fixed codebook gain value
 *                              of the current w_subframe
 *                gain_code_old[0..4*DTX_HANGOVER-1]
 *                              fixed codebook gain parameter history
 *
 *   OUTPUTS:     none
 *
 *   RETURN VALUE: Averaged fixed codebook gain value
 *
 *************************************************************************/

int16_t w_aver_gain_code_history(int16_t w_CN_w_excitation_gain,
				int16_t gain_code_old[4 * DTX_HANGOVER]
    )
{
	int16_t i;
	int32_t L_ret;

	L_ret = w_L_w_mult(0x470, w_CN_w_excitation_gain);

	for (i = 0; i < (4 * DTX_HANGOVER); i++) {
		L_ret = w_L_mac(L_ret, 0x470, gain_code_old[i]);
	}
	return w_extract_h(L_ret);
}

/*************************************************************************
 *
 *   FUNCTION NAME: w_build_CN_code
 *
 *   PURPOSE: Compute the comfort noise fixed codebook w_excitation. The
 *            gains of the pulses are always +/-1.
 *
 *   INPUTS:      *seed         Old CN generator shift register w_state
 *
 *   OUTPUTS:     cod[0..39]    Generated comfort noise fixed codebook vector
 *                *seed         Updated CN generator shift register w_state
 *
 *   RETURN VALUE: none
 *
 *************************************************************************/

void w_build_CN_code(int16_t cod[], int32_t * seed)
{
	int16_t i, j, k;

	for (i = 0; i < L_SUBFR; i++) {
		cod[i] = 0;
	}

	for (k = 0; k < NB_PULSE; k++) {
		i = w_pseudonoise(seed, 2);	/* generate pulse position */
		i = w_shr(w_extract_l(w_L_w_mult(i, 10)), 1);
		i = w_add(i, k);

		j = w_pseudonoise(seed, 1);	/* generate sign           */

		if (j > 0) {
			cod[i] = 4096;
		} else {
			cod[i] = -4096;
		}
	}

	return;
}

/*************************************************************************
 *
 *   FUNCTION NAME: w_pseudonoise
 *
 *   PURPOSE: Generate a random integer value to use in comfort noise
 *            generation. The algorithm uses polynomial x^31 + x^3 + 1
 *            (length of PN sequence is 2^31 - 1).
 *
 *   INPUTS:      *shift_reg    Old CN generator shift register w_state
 *
 *
 *   OUTPUTS:     *shift_reg    Updated CN generator shift register w_state
 *
 *   RETURN VALUE: Generated random integer value
 *
 *************************************************************************/

int16_t w_pseudonoise(int32_t * shift_reg, int16_t no_bits)
{
	int16_t noise_bits, Sn, i;

	noise_bits = 0;
	for (i = 0; i < no_bits; i++) {
		/* State n == 31 */

		if ((*shift_reg & 0x00000001L) != 0) {
			Sn = 1;
		} else {
			Sn = 0;
		}

		/* State n == 3 */

		if ((*shift_reg & 0x10000000L) != 0) {
			Sn = Sn ^ 1;
		} else {
			Sn = Sn ^ 0;
		}

		noise_bits = w_shl(noise_bits, 1);
		noise_bits = noise_bits | (w_extract_l(*shift_reg) & 1);

		*shift_reg = w_L_w_shr(*shift_reg, 1);

		if (Sn & 1) {
			*shift_reg = *shift_reg | 0x40000000L;
		}
	}

	return noise_bits;
}

/*************************************************************************
 *
 *   FUNCTION NAME: w_interpolate_CN_param
 *
 *   PURPOSE: Interpolate a comfort noise parameter value over the comfort
 *            noise update period.
 *
 *   INPUTS:      old_param     The older parameter of the interpolation
 *                              (the endpoint the interpolation is started
 *                              from)
 *                new_param     The newer parameter of the interpolation
 *                              (the endpoint the interpolation is ended to)
 *                w_w_rx_dtx_w_state  State of the comfort noise insertion period
 *
 *   OUTPUTS:     none
 *
 *   RETURN VALUE: Interpolated CN parameter value
 *
 *************************************************************************/

int16_t w_interpolate_CN_param(int16_t old_param,
			      int16_t new_param, int16_t w_w_rx_dtx_w_state)
{
	static const int16_t interp_factor[CN_INT_PERIOD] = {
		0x0555, 0x0aaa, 0x1000, 0x1555, 0x1aaa, 0x2000,
		0x2555, 0x2aaa, 0x3000, 0x3555, 0x3aaa, 0x4000,
		0x4555, 0x4aaa, 0x5000, 0x5555, 0x5aaa, 0x6000,
		0x6555, 0x6aaa, 0x7000, 0x7555, 0x7aaa, 0x7fff
	};
	int16_t temp;
	int32_t L_temp;

	L_temp = w_L_w_mult(interp_factor[w_w_rx_dtx_w_state], new_param);
	temp = w_sub(0x7fff, interp_factor[w_w_rx_dtx_w_state]);
	temp = w_add(temp, 1);
	L_temp = w_L_mac(L_temp, temp, old_param);
	temp = w_round(L_temp);

	return temp;
}

/*************************************************************************
 *
 *   FUNCTION NAME:  w_interpolate_CN_lsf
 *
 *   PURPOSE: Interpolate comfort noise LSF parameter vector over the comfort
 *            noise update period.
 *
 *   INPUTS:      w_lsf_old_CN[0..9]
 *                              The older LSF parameter vector of the
 *                              interpolation (the endpoint the interpolation
 *                              is started from)
 *                w_lsf_new_CN[0..9]
 *                              The newer LSF parameter vector of the
 *                              interpolation (the endpoint the interpolation
 *                              is ended to)
 *                w_w_rx_dtx_w_state  State of the comfort noise insertion period
 *
 *   OUTPUTS:     lsf_interp_CN[0..9]
 *                              Interpolated LSF parameter vector
 *
 *   RETURN VALUE: none
 *
 *************************************************************************/

void w_interpolate_CN_lsf(int16_t w_lsf_old_CN[M],
			  int16_t w_lsf_new_CN[M],
			  int16_t lsf_interp_CN[M],
			  int16_t w_w_rx_dtx_w_state)
{
	int16_t i;

	for (i = 0; i < M; i++) {
		lsf_interp_CN[i] = w_interpolate_CN_param(w_lsf_old_CN[i],
							  w_lsf_new_CN[i],
							  w_w_rx_dtx_w_state);
	}

	return;
}

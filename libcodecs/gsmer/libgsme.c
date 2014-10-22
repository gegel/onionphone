/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* GSM_EFR (GSM 06.53) codec wrapper */
/***************************************************************************
 *    Format for w_speech:
 *      Speech is 16 bits data.
 * 
 *    Format for bitstream:
 *      244  words (2-byte) containing 244 bits.
 *          Bit 0 = 0x0000 and Bit 1 = 0x0001
 *      One word (2-byte) for voice activity decision (VAD) flag bit
 *          0x0000 -> inactive (no detected w_speech activity);
 *          0x0001 -> active
 *      One word (2-byte) for w_speech (SP) flag bit
 *          0x0000 -> inactive (no transmission of w_speech frames);
 *          0x0001 -> active
 *		One word (2-byte) for no w_error/transmitted (Not BFI) flag bit
 *          0x0000 -> dummy (not transmitted) frame;
 *          0x0001 -> received farme
 *      One word (2-byte) for TAF flag
 *          0x0000 -> regullag frame;
 *          0x0001 -> time aligned frame
 *
 ***************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ophtools.h>

#include "ophint.h"
#include "basic_op.h"
#include "sig_proc.h"
#include "count.h"
#include "codec.h"
#include "cnst.h"
#include "n_stack.h"
#include "e_homing.h"
#include "d_homing.h"
#include "dtx.h"

static Word16 w_TAF_count = { 1 };

Word16 w_dtx_mode;
extern Word16 w_txdtx_ctrl;
extern Word16 *w_new_w_speech;	/* Pointer to new w_speech data            */

/* L_FRAME, M, PRM_SIZE, AZ_SIZE, SERIAL_SIZE: defined in "cnst.h" */

static Word16 w_reset_flag;
static Word16 w_prm[PRM_SIZE];	/* Analysis parameters.                  */
static Word16 w_serial[SERIAL_SIZE + 4];	/* Output bitstream buffer               */
static Word16 w_syn[L_FRAME];	/* Buffer for w_w_synthesis w_speech           */
static Word16 LastFrameType;

Word16 synth_buf[L_FRAME + M];

static Word16 *w_w_synth;	/* Synthesis                  */
static Word16 w_parm[PRM_SIZE + 1];	/* Synthesis parameters    58   */
static Word16 w_dw_reset_flag;
static Word16 w_w_reset_flag_old = 1;
static Word16 w_Az_dec[AZ_SIZE];	/* Decoded Az for post-filter */
				/* in 4 w_subframes, length= 44 */

void gsmer_init(int dtx)
{
	if (dtx)
		w_dtx_mode = 1;
	else
		w_dtx_mode = 0;
	w_reset_enc();		/* Bring the encoder, VAD and DTX to the initial w_state */
	LastFrameType = 0;

	w_w_synth = synth_buf + M;
	w_reset_dec();		/* Bring the decoder and receive DTX to the initial w_state */
}

//encode 160 shorts samples to 31 uchars stream
//returns SP + 2*VAD flags
int gsmer_encode(unsigned char *rb, const short *pcm)
{
	Word16 i;

	/* Check whether this frame is an encoder homing frame */
	w_reset_flag = w_encoder_homing_frame_w_test((Word16 *) pcm);

	for (i = 0; i < L_FRAME; i++) {	/* Delete the 3 LSBs (13-bit input) */
		w_new_w_speech[i] = pcm[i] & 0xfff8;	//     
	}

	w_Pre_Process(w_new_w_speech, L_FRAME);	/* filter + downscaling */

	w_Coder_12k2(w_prm, w_syn);	/* Find w_speech parameters   */

	if ((w_txdtx_ctrl & TX_SP_FLAG) == 0) {
		/* Write comfort noise parameters into the parameter frame.
		   Use old parameters in case SID frame is not to be updated */
		w_CN_encoding(w_prm, w_txdtx_ctrl);
	}

	w_Prm2bits_12k2(w_prm, &w_serial[0]);	/* Parameters to w_serial bits */

	if ((w_txdtx_ctrl & TX_SP_FLAG) == 0) {
		/* Insert SID codeword into the w_serial parameter frame */
		w_sid_codeword_encoding(&w_serial[0]);
	}

	/* Write the VAD- and SP-flags to file after the w_speech
	   parameter bit stream */

	if ((w_txdtx_ctrl & TX_VAD_FLAG) != 0)
		w_serial[244] = 1;
	else
		w_serial[244] = 0;
	if ((w_txdtx_ctrl & TX_SP_FLAG) != 0)
		w_serial[245] = 1;
	else
		w_serial[245] = 0;

	/* Set BFI flag */
	w_serial[246] = 1;

	/* Set TAF flag every 24 frames */
	w_serial[247] = 0;
	if ((w_TAF_count == 0) && (w_serial[245])) {
		w_serial[247] = 1;
	}

	w_TAF_count = (w_TAF_count + 1) % 24;

/* Pack to bitStream */
	memzero(rb, 31);
	for (i = 0; i < 248; i++) {
		if (w_serial[i])
			rb[i >> 3] |= ((unsigned char)1 << (i & 7));
	}

	if (w_reset_flag != 0) {
		w_reset_enc();	/*Bring the encoder, VAD and DTX to the home w_state */
	}
	//return 
	i = w_serial[245] + 2 * w_serial[244];

	if ((i == 0) && (LastFrameType != 0)) {
		i = 2;
		LastFrameType = 0;
	} else
		LastFrameType = i;

	return i;

}

//decode 31 uchars stream to 160 shorts samples 
//returns SP + 2*VAD flags
int gsmer_decode(short *pcm, const unsigned char *rb)
{
	/* These constants define the number of consecutive parameters
	   that function w_decoder_homing_frame_w_test() checks */
#define WHOLE_FRAME 57
#define TO_FIRST_SUBFRAME 18
	Word16 i, temp;
	Word16 TAF, SID_flag, VAD, SP, BFI;

	/* Unpack bitStream */
	for (i = 0; i < 248; i++) {
		if (rb[i >> 3] & ((unsigned char)1 << (i & 7)))
			w_serial[i + 1] = 1;
		else
			w_serial[i + 1] = 0;
	}

	VAD = w_serial[245];	//Voice activ flag from sender
	SP = w_serial[246];	//Speach flag from sender

	/* Set channel status (BFI) flag */
	/* -------------------------------------- */
	BFI = (Word16) (!w_serial[247]);	/* BFI flag */
	w_serial[0] = BFI;

	/* Evaluate SID flag                                  */
	/* Function w_sid_frame_detection() is defined in dtx.c */
	/* -------------------------------------------------- */
	SID_flag = w_sid_frame_detection(&w_serial[1]);
	w_serial[245] = SID_flag;

	/* Evaluate TAF flag */
	TAF = 0;
	w_serial[246] = TAF;

	w_Bits2w_prm_12k2(w_serial, w_parm);	/* w_serial to parameters   */

	if (w_parm[0] == 0) {	/* BFI == 0, perform DHF check */
		if (w_w_reset_flag_old == 1) {	/* Check for second and further
						   successive DHF (to first w_subfr.) */
			w_dw_reset_flag =
			    w_decoder_homing_frame_w_test(&w_parm[1],
							  TO_FIRST_SUBFRAME);
		} else {
			w_dw_reset_flag = 0;
		}
	} else {		/* BFI==1, bypass DHF check (frame
				   is taken as not being a DHF) */
		w_dw_reset_flag = 0;
	}

	//================================================
	if ((w_dw_reset_flag != 0) && (w_w_reset_flag_old != 0)) {
		/* Force the output to be the encoder homing frame pattern */

		for (i = 0; i < L_FRAME; i++) {
			w_w_synth[i] = EHF_MASK;
		}
	} else {

		w_Decoder_12k2(w_parm, w_w_synth, w_Az_dec, TAF, SID_flag);	/* Synthesis */

		w_Post_Filter(w_w_synth, w_Az_dec);	/* Post-filter */

		for (i = 0; i < L_FRAME; i++)
			/* Upscale the 15 bit linear PCM to 16 bits,
			   then truncate to 13 bits */
		{
			temp = w_shl(w_w_synth[i], 1);
			w_w_synth[i] = temp & 0xfff8;
		}
	}			/* else */

	memcpy(pcm, w_w_synth, 320);

	/* BFI == 0, perform check for first DHF (whole frame) */
	if ((w_parm[0] == 0) && (w_w_reset_flag_old == 0)) {
		w_dw_reset_flag =
		    w_decoder_homing_frame_w_test(&w_parm[1], WHOLE_FRAME);
	}

	if (w_dw_reset_flag != 0) {
		/* Bring the decoder and receive DTX to the home w_state */
		w_reset_dec();
	}
	w_w_reset_flag_old = w_dw_reset_flag;

	return SP + 2 * VAD;
}

/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
 *
 *   File Name:  d_homing.h
 *
 *   Purpose:   Contains the prototypes for all the functions of
 *              decoder homing.
 *
 **************************************************************************/

#define EHF_MASK 0x0008		/* Encoder Homing Frame pattern */

#define D_HOMING

/* Function Prototypes */

Word16 w_decoder_homing_frame_w_test(Word16 w_parm[], Word16 nbr_of_params);

void w_decoder_reset(void);

void w_reset_dec(void);

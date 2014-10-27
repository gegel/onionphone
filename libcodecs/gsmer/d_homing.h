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

int16_t w_decoder_homing_frame_w_test(int16_t w_parm[], int16_t nbr_of_params);

void w_decoder_reset(void);

void w_reset_dec(void);

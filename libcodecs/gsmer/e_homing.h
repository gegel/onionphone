/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
 *
 *   File Name:  e_homing.h
 *
 *   Purpose:   Contains the prototypes for all the functions of
 *              encoder homing.
 *
 **************************************************************************/

#define EHF_MASK 0x0008		/* Encoder Homing Frame pattern */

/* Function Prototypes */

int16_t w_encoder_homing_frame_w_test(int16_t input_frame[]);

void w_encoder_reset(void);

void w_reset_enc(void);

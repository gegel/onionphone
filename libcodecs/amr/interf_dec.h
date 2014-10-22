/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*
 * ===================================================================
 *  TS 26.104
 *  REL-5 V5.4.0 2004-03
 *  REL-6 V6.1.0 2004-03
 *  3GPP AMR Floating-point Speech Codec
 * ===================================================================
 *
 */

/*
 * interf_dec.h
 *
 *
 * Project:
 *    AMR Floating-Point Codec
 *
 * Contains:
 *    Defines interface to AMR decoder
 *
 */

#pragma once

#ifndef _interf_dec_h_
#define _interf_dec_h_

#include <ophint.h>

/*
 * Function prototypes
 */
/*
 * Conversion from packed bitstream to endoded parameters
 * Decoding parameters to speech
 */
void Decoder_Interface_Decode(void *st,
#ifndef ETSI
			      unsigned char *bits,
#else
			      short *bits,
#endif
			      short *synth, int bfi);

/*
 * Reserve and init. memory
 */
void *Decoder_Interface_init(void);

/*
 * Exit and free memory
 */
void Decoder_Interface_exit(void *state);

void AMR475_decode(void *st, UWord8 * serial, Word16 * synth, int bfi);
void AMR_decode(void *st, UWord8 mode, UWord8 * serial, Word16 * synth,
		int bfi);
#endif /* _interf_dec_h_ */

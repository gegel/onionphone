/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* ================================================================== */
/*                                                                    */
/*    Microsoft Speech coder     ANSI-C Source Code                   */
/*    SC1200 1200 bps speech coder                                    */
/*    Fixed Point Implementation      Version 7.0                     */
/*    Copyright (C) 2000, Microsoft Corp.                             */
/*    All rights reserved.                                            */
/*                                                                    */
/* ================================================================== */

/* =============================================== */
/* global.h: global variables for the sc1200 coder */
/* =============================================== */

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "sc1200.h"

/* ====== Data I/O for high level language implementation ====== */
extern long frame_count;
extern short rate;

/* ====== Global variables for fixed-point library ====== */
extern int32_t saturation;
extern int32_t temp_saturation;

/* ====== General parameters ====== */
extern struct melp_param melp_par[];	/* melp analysis parameters */
extern unsigned char chbuf[];	/* channel bit data buffer */
extern int16_t frameSize, frameSize12, frameSize24;
						/* frame size 2.4 = 180 1.2 = 540 */
extern int16_t bitNum, bitNum12, bitNum24;	/* number of bits */

/* ====== Quantization ====== */
extern const int16_t msvq_bits[];
extern const int16_t msvq_levels[];
extern struct quant_param quant_par;

/* ====== Buffers ====== */
extern int16_t hpspeech[];	/* input speech buffer dc removed */
extern int16_t dcdel[];
extern int16_t dcdelin[];
extern int16_t dcdelout_hi[];
extern int16_t dcdelout_lo[];

/* ====== Classifier ====== */
extern int16_t voicedEn, silenceEn;
extern int32_t voicedCnt;

/* ====== Fourier Harmonics Weights ====== */
extern int16_t w_fs[];
extern int16_t w_fs_inv[];
extern BOOLEAN w_fs_init;

// ====== Output bitstream word size ====== */
extern int16_t chwordsize;
#endif

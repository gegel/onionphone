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
/* fft_lib.h: FFT function include file */
/* =============================================== */

#ifndef _FFT_LIB_H_
#define _FFT_LIB_H_

/* Radix-2, DIT, 2N-point Real FFT */
void rfft(int16_t datam1[], int16_t n);
/* Radix-2, DIT, N-point Complex FFT */
int16_t cfft(int16_t datam1[], int16_t nn);
/* Radix-2, DIT, 256-point Complex FFT */
int16_t fft_npp(int16_t data[], int16_t dir);

void fs_init();

#endif

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

/* ========================================= */
/* melp.c: Mixed Excitation LPC speech coder */
/* ========================================= */

//STANAG-4991

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sc1200.h"
#include "mat_lib.h"
#include "global.h"
#include "macro.h"
#include "mathhalf.h"
#include "dsp_sub.h"
#include "melp_sub.h"
#include "constant.h"
#include "math_lib.h"
#include "math.h"

#if NPP
#include "npp.h"
#endif

#define X05_Q7				64	/* 0.5 * (1 << 7) */
#define THREE_Q7			384	/* 3 * (1 << 7) */

/* ====== External memory ====== */

int16_t mode;
int16_t chwordsize;
int16_t bitBufSize, bitBufSize12, bitBufSize24;
/* ========== Static definations ========== */

#define PROGRAM_NAME			"SC1200 1200 bps speech coder"
#define PROGRAM_VERSION			"Version 7 / 42 Bits"
#define PROGRAM_DATE			"10/25/2000"

/* ========== Public Prototypes ========== */
void melpe_n(short *sp);
void melpe_i(void);
void melpe_a(unsigned char *buf, short *sp);
void melpe_s(short *sp, unsigned char *buf);
void melpe_al(unsigned char *buf, short *sp);
void melpe_i2(void);

//------------------------------NPP------------------------

//denoise 180 samples sp->sp
void melpe_n(short *sp)
{
	//noise preprocessor for other codecs (frame=180)
	npp(sp, sp);
}

//-------------------------1200----------------------------------

//init melpe codec at 1200 bps
void melpe_i(void)
{
	//====== Run MELPE codec ====== 
	mode = ANA_SYN;
	rate = RATE1200;
	frameSize = (int16_t) BLOCK;

	chwordsize = 8;
	bitNum12 = 81;
	bitNum24 = 54;
	bitBufSize12 = 11;
	bitBufSize24 = 7;
	bitBufSize = bitBufSize12;

	melp_ana_init();
	melp_syn_init();
}

//compress 540 samples sp (67.5 mS) -> 81 bits buf (11 bytes)
void melpe_a(unsigned char *buf, short *sp)
{
	//analysys
	npp(sp, sp);
	npp(&(sp[FRAME]), &(sp[FRAME]));
	npp(&(sp[2 * FRAME]), &(sp[2 * FRAME]));
	analysis(sp, melp_par);
	memcpy(buf, chbuf, 11);
}

//decompress 81 bits buf (11 bytes) -> 540 samples sp (67.5 mS)
void melpe_s(short *sp, unsigned char *buf)
{
	//syntesis
	memcpy(chbuf, buf, 11);
	synthesis(melp_par, sp);
}


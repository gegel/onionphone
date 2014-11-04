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
#include "transcode.h"

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
void melpe_sl(short *sp, unsigned char *buf);
void melpe_i2(void);

void melpe_s2(short *sp, unsigned char *buf);
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

/*Extremally low bitrate but latency 542 ms but equal jitter compensation) modes:*/

//Compress 4320 samples sp (524 mS) -> 648 bits buf (81 bytes)
void melpe_al(unsigned char *buf, short *sp)
{
	int i;
	char c = 0;		//container for last bits of compressed frames
	for (i = 0; i < 8; i++)	//process 8 frames
	{
		melpe_a(buf, sp);	//compress frame (result is 81 bit)
		c |= (buf[10] << i);	//save last bit
		sp += 540;	//pointer to next inputeed frame of samples
		buf += 10;	//pointer to next outputeed compressed frame
	}
	(*buf) = c;		//store last bits of all compressed frames
}

//Decompress 648 bits buf (81 bytes) -> 4320 samples sp (524 mS)
void melpe_sl(short *sp, unsigned char *buf)
{
	int i;
	char cc;
	char c = buf[80];	//load last bits of all comressed frames;
	for (i = 0; i < 8; i++)	//process 8 frames
	{
		cc = buf[10];	//preserve first byte of next frame
		buf[10] = (c >> i) & 1;	//use last bit of current frame (total is 81 bit)
		melpe_s(sp, buf);	//decompress frame
		buf[10] = cc;	//restore first byte of next frame
		buf += 10;	//pointer to next inputted compressed frame
		sp += 540;	//pointer to next outputted frame of samples
	}
}

//-------------------2400------------------------

//init melpe codec at 2400 bps
void melpe_i2(void)
{
	//====== Run MELPE codec at 2400 bps====== 
	mode = ANA_SYN;
	rate = RATE2400;
	frameSize = (int16_t) FRAME;
	chwordsize = 8;
	bitNum12 = 81;
	bitNum24 = 54;
	bitBufSize12 = 11;
	bitBufSize24 = 7;
	bitBufSize = bitBufSize24;

	melp_ana_init();
	melp_syn_init();
}

//compress 180 samples sp (22.5 mS) -> 54 bits buf (7 bytes)
void melpe_a2(unsigned char *buf, short *sp)
{
	//analysys
	npp(sp, sp);
	analysis(sp, melp_par);
	memcpy(buf, chbuf, 7);
}

void melpe_s2(short *sp, unsigned char *buf)
{
	//syntesis
	memcpy(chbuf, buf, 7);
	synthesis(melp_par, sp);
}

/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*

2.4 kbps MELP Proposed Federal Standard mf_speech coder

version 1.2

Copyright (c) 1996, Texas Instruments, Inc.  

Texas Instruments has intellectual property rights on the MELP
algorithm.  The Texas Instruments contact for licensing issues for
commercial and non-government use is William Gordon, Director,
Government Contracts, Texas Instruments Incorporated, Semiconductor
Group (phone 972 480 7442).


*/

/*                                                                  */
/*  melp.c: Mixed Excitation LPC mf_speech coder                       */
/*                                                                  */

/*  compiler include files  */
#include	<stdio.h>
#include "melp.h"
#include "spbstd.h"
#include "mat.h"

/*  compiler constants */
#define ANA_SYN 0
#define ANALYSIS 1
#define SYNTHESIS 2

/* note: CHSIZE is shortest integer number of words in channel packet */
#define CHSIZE 9
#define NUM_CH_BITS 54

/*  external memory */

static struct mf_melp_param mf_melp_par;	/* melp parameters */
static float mf_fmf_speech[mf_FRAME];
static unsigned int mf_ichbuf[CHSIZE];

void melp_ini(void)
{
	mf_mf_melp_ana_init();
	mf_mf_melp_syn_init();
}

//MELP encode
void melp_enc(unsigned char *bits, short *mf_speech)
{
	int i;
	mf_melp_par.chptr = mf_ichbuf;
	mf_melp_par.chbit = 0;

	for (i = 0; i < mf_FRAME; i++)
		mf_fmf_speech[i] = mf_speech[i];
	mf_melp_ana(mf_fmf_speech, &mf_melp_par);
	bits[0] = (0x3F & mf_ichbuf[0]) | (0xC0 & (mf_ichbuf[7] << 6));
	bits[1] = (0x3F & mf_ichbuf[1]) | (0xC0 & (mf_ichbuf[7] << 4));
	bits[2] = (0x3F & mf_ichbuf[2]) | (0xC0 & (mf_ichbuf[7] << 2));
	bits[3] = (0x3F & mf_ichbuf[3]) | (0xC0 & (mf_ichbuf[8] << 6));
	bits[4] = (0x3F & mf_ichbuf[4]) | (0xC0 & (mf_ichbuf[8] << 4));
	bits[5] = (0x3F & mf_ichbuf[5]) | (0xC0 & (mf_ichbuf[8] << 2));
	bits[6] = (0x3F & mf_ichbuf[6]);
}

//MELP decode
void melp_dec(short *mf_speech, unsigned char *bits)
{
	int i;

	mf_melp_par.chptr = mf_ichbuf;
	mf_melp_par.chbit = 0;

	mf_ichbuf[0] = 0x3F & bits[0];
	mf_ichbuf[1] = 0x3F & bits[1];
	mf_ichbuf[2] = 0x3F & bits[2];
	mf_ichbuf[3] = 0x3F & bits[3];
	mf_ichbuf[4] = 0x3F & bits[4];
	mf_ichbuf[5] = 0x3F & bits[5];
	mf_ichbuf[6] = 0x3F & bits[6];
	mf_ichbuf[7] = (bits[0] >> 6) | (bits[1] >> 4) | (bits[2] >> 2);
	mf_ichbuf[8] = (bits[3] >> 6) | (bits[4] >> 4) | (bits[5] >> 2);

	mf_melp_syn(&mf_melp_par, mf_fmf_speech);
	for (i = 0; i < mf_FRAME; i++)
		mf_speech[i] = (int)mf_fmf_speech[i];
}

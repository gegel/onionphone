/***********************************************************************
*
*     NSA LPC-10 Voice Coder
*
*     Unix C Version 
*
*     5 November 1990
*
*************************************************************************/

#include <stdio.h>
#include "lpcdefs.h"
#include "config.ch"
#include "common.h"

FILE *fdi, *fdo;

#define BUFSIZE 600

int nframe, nfb, nbits, nunsfm, iclip, maxosp, NFBLK;
FILE *fbi, *fbo, *fopen();

/* analys */
float *inbuf, *pebuf, *lpbuf, *ivbuf;
float lparray[LBUFH-LBUFL+1], ivarray[PWINH-PWINL+1];
float pearray[SBUFH-SBUFL+1], inarray[SBUFH-SBUFL+1];
int vwin[2][AF], awin[2][AF], voibuf[2][AF+1];
float rmsbuf[AF], psi[MAXORD], rcbuf[MAXORD][AF];
float amdf[LTAU];
float phi[MAXORD][MAXORD];

/* bsynz.c */
float exc[MAXPIT+MAXORD], exc2[MAXPIT+MAXORD];
float noise[MAXPIT+MAXORD];

/* decode.c */
int drc[3][MAXORD], dpit[3], drms[3];

/* dyptrk */
float s[60];
int p[60][2];

/* onset */
float l2buf[16];

/* synths.c */
int ipiti[11], ivuv[11];
float rci[MAXORD][11], rmsi[11], pc[MAXORD];

/*----------------------------------------------------------------------*/
buf_man(inbuffer, outbuffer, len)
float outbuffer[], inbuffer[];
int len;
{
	static float big_buffer[BUFSIZE];
	static int sptr=0, eptr=360, first=1;
	int i;
	
	/* Initialize pseudo-circular buffer */
	if(first)	{
		first = 0;
		for(i=0;i<360;i++)
		  big_buffer[i] = 0.0;
	}
	
	/* write new data to end of buffer */
	for(i=0;i<len;i++)	{
		big_buffer[eptr++] = inbuffer[i];
		if (eptr == BUFSIZE) eptr = 0;
		/*eptr = (eptr+1) % BUFSIZE;*/
	}
	
	/* send next 180 samples */
	for(i=0;i<LFRAME;i++)	{
		outbuffer[i] = big_buffer[sptr++];
		if(sptr == BUFSIZE) sptr = 0;
		/*sptr = (sptr+1)%BUFSIZE;*/
	}
}

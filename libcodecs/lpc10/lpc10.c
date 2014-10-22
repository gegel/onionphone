/***********************************************************************
*
*     NSA LPC-10 Voice Coder
*
*     Unix C Version 
*
*     5 November 1990
*
*     Modified for use within Speak Freely by John Walker
*
*************************************************************************/

#include "lpc10.h"
#include "lpcdefs.h"
#include "common.h"
#include <string.h>


#define BUFSIZE 600

int count=0;

int tau[LTAU] = {
	20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,
	35,36,37,38,39,40,42,44,46,48,50,52,54,56,58,60,62,64,66,
	68,70,72,74,76,78,80,84,88,92,96,100,104,108,112,116,120,
	124,128,132,136,140,144,148,152,156
};

int nframe, nfb, nbits, nunsfm, iclip, maxosp, NFBLK;

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

static void buf_man(float inbuffer[], float outbuffer[], int len)
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

/*  LPC10INIT  --  Initialise for LPC decoding.  */

void lpc10init(void)
{
  initialize1();
  initialize2();
}


/*  LPC10ENCODE  --  Encode a set of samples with LPC-10.  */

int lpc10encode(short *in, unsigned char *out, int inlen)
{
    int i, n, o = 0;
    static int pitch;
    static float rms, rc[MAXORD];
    static int voice[2];
    static int j, ipitv, irms, irc[MAXORD], ibits[MAXNB];
    static float speech[MAXFRM + MAXPIT];

    i = inlen % LFRAME;

    if (i != 0) {
	i = LFRAME - i;
	while (i-- > 0) {
	    in[inlen++] = 0;
	}
    }

    for (n = 0; n < inlen / LFRAME; n++) {
	for (i = 0; i < LFRAME; i++) {
	    speech[i] = ((float)in[i]) / 32768.0f;
	}
	hp100(speech);
	analys(speech, voice, &pitch, &rms, rc-1);

	memset(ibits, 0, MAXNB * sizeof(int));
	memset(out, 0, 7);

	encode(voice - 1, &pitch, &rms, rc-1, &ipitv, &irms, irc-1);
	channel(0, &ipitv, &irms, irc-1, ibits-1);
	for (j = 0; j < MAXNB; j++) {
	    out[j >> 3] |= (!!ibits[j]) << (j & 7);
	}
	o += 7;
	out += 7;
	in += LFRAME;
    }
    return o;
}

/*  LPC10DECODE  --  Decode a set of samples with LPC-10.  */

int lpc10decode(unsigned char *in, short *out, int inlen)
{
    int i, len, n, o = 0;
    static int pitch;
    static float rms, rc[MAXORD];
    static int voice[2];
    static int j, ipitv, irms, irc[MAXORD], ibits[MAXNB];
    static float speech[MAXFRM + MAXPIT];

    for (n = 0; n < inlen / 7; n++) {
	for (j = 0; j < MAXNB; j++) {
	    ibits[j] = !!(in[j >> 3] & (1 << (j & 7)));
	}
	in += 7;
	channel(1, &ipitv, &irms, irc-1, ibits-1);
	decode(ipitv, &irms, irc-1, voice - 1, &pitch, &rms, rc-1);
	synths(voice-1, &pitch, &rms, rc-1, speech-1, &len);
	buf_man(speech, speech, len);
	for (i = 0; i < LFRAME; i++) {
	    *out++ = ((short) mmax(-32768.0, mmin(32768.0 * speech[i], 32767.0)));
	}
	o += LFRAME;
    }
    return o;
}

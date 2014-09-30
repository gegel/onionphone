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

int count=0;

int tau[LTAU] = {
  20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,
	35,36,37,38,39,40,42,44,46,48,50,52,54,56,58,60,62,64,66,
	68,70,72,74,76,78,80,84,88,92,96,100,104,108,112,116,120,
	124,128,132,136,140,144,148,152,156
};

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

main(argc,argv)
int argc;
char *argv[];
{
int	     pitch;
float			rms, rc[MAXORD];
int			len;
int voice[2];

short		eof;
static float	speech[MAXFRM+MAXPIT];

    static long auhdr[] = {
	0x2E736E64,
	6 * 4,
	~0,
	1,
	8000,
	1
    };

int i; /* this is only for some debugging purposes */

  setup(argc,argv);

  fwrite(auhdr, sizeof auhdr, 1, fdo);

  initialize1();
  initialize2();

  while (1) {

    if (fdi != NULL) 
        diskio(0,fdi,speech,LFRAME,&eof,"",0);  
    if(eof==END) break;

    hp100(speech);
    analys(speech, voice, &pitch, &rms, rc-1);
    trans(voice-1, &pitch, &rms, rc);

    /* Synthesize speech from received parameters */
    synths(voice-1, &pitch, &rms, rc-1, speech-1, &len );

    /* Equalize to LFRAME samples for output */
    buf_man(speech, speech, len);
    diskio(1,fdo,speech,LFRAME,&eof,"",0);
  }
  return 0;
}

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

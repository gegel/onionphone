/*  This routine initializes all the routine's arrays for all the routines
so that the "first" variable won't have to be checked every time the
routine is entered
*/

#include "lpcdefs.h"
#include <string.h>
#include <ophtools.h>

/* analys */
extern float lparray[LBUFH-LBUFL+1], ivarray[PWINH-PWINL+1];
extern float pearray[SBUFH-SBUFL+1], inarray[SBUFH-SBUFL+1];
extern float *inbuf, *pebuf, *lpbuf, *ivbuf;
extern int vwin[2][AF], awin[2][AF], voibuf[2][AF+1];
extern float rmsbuf[AF], amdf[LTAU], psi[MAXORD], rcbuf[MAXORD][AF];

/* bsynz */
extern float exc[MAXPIT+MAXORD],  exc2[MAXPIT+MAXORD];

/* decode */
extern int drc[3][MAXORD], dpit[3], drms[3];

/* dyptrk */
extern float s[60];
extern int p[60][2];

/* onset */
extern float l2buf[16];

/* synths */
extern int ipiti[11], ivuv[11];
extern float rci[MAXORD][11], rmsi[11], pc[MAXORD];


void initialize1(void)
{
/* ---------------------- bsynz -------------------------- */
memzero(exc, (MAXPIT + MAXORD) * sizeof(float));
memzero(exc2, (MAXPIT + MAXORD) * sizeof(float));

/* ---------------------- dyptrk -------------------------- */
memzero(p, 60 * 2 * sizeof(int));
memzero(s, 60 * sizeof(float));
	
/* ----------------------- decode ----------------------- */
memzero(dpit, 3 * sizeof(int));
memzero(drms, 3 * sizeof(int));
memzero(drc, MAXORD * 3 * sizeof(int));

/* ----------------------- synths ----------------------- */
memzero(ipiti, 11 * sizeof(int));
memzero(ivuv, 11 * sizeof(int));
memzero(rmsi, 11 * sizeof(float));
memzero(pc, MAXORD * sizeof(float));
memzero(rci, MAXORD * 11 * sizeof(float));
}


void initialize2(void)
{
/* ----------------------- analys.c ----------------------- */
/* allocate space for the arrays */
inbuf = inarray;
lpbuf = lparray;
ivbuf = ivarray;
pebuf = pearray;
/* Initialize space created */
memzero(inarray, (SBUFH - SBUFL + 1) * sizeof(float));
memzero(pearray, (SBUFH - SBUFL + 1) * sizeof(float));
memzero(lparray, (LBUFH - LBUFL + 1) * sizeof(float));
memzero(ivarray, (PWINH - PWINL + 1) * sizeof(float));

/* align C arrays to FORTRAN indexing */
inbuf -= SBUFL;
pebuf -= SBUFL;
lpbuf -= LBUFL;
ivbuf -= PWINL;

/* assign initial values to arrays - with C indexing */
vwin[0][AF-1] = DVWINL;
vwin[1][AF-1] = DVWINH;
vwin[0][1] = 0;
vwin[1][1] = 0;
awin[0][AF-1] = DVWINL;
awin[1][AF-1] = DVWINH;
awin[0][1] = 0;
awin[1][1] = 0;

memzero(rcbuf, MAXORD * AF * sizeof(float));
memzero(rmsbuf, AF * sizeof(float));
memzero(amdf, LTAU * sizeof(float));

memzero(voibuf, (AF + 1) * 2 * sizeof(int));

/* onset */
memzero(l2buf, 16 * sizeof(float));
}

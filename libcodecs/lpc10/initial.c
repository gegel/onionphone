/*  This routine initializes all the routine's arrays for all the routines
so that the "first" variable won't have to be checked every time the
routine is entered
*/

#include "lpcdefs.h"
#include <string.h>

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
int i, j;
/* ---------------------- bsynz -------------------------- */
for(i=0;i<MAXPIT+MAXORD;i++)	{
  exc[i] = 0.0;
  exc2[i] = 0.0;
}

/* ---------------------- dyptrk -------------------------- */
memset(p, 0, 60*2);
for(i=0;i<60;i++)
  s[i] = 0.0;
	
/* ----------------------- decode ----------------------- */
memset(dpit, 0, 3);
memset(drms, 0, 3);
memset(drc, 0, MAXORD*3);

/* ----------------------- synths ----------------------- */
memset(ipiti, 0, 11);
memset(ivuv, 0, 11);
for(i=0;i<10;i++)	{
	rmsi[i] = 0.0;
	pc[i] = 0.0;
}
for(i=0;i<MAXORD;i++)
  for(j=0;j<11;j++)
    rci[i][j] = 0.0;

}


void initialize2(void)
{
	int i;
/* ----------------------- analys.c ----------------------- */
/* allocate space for the arrays */
inbuf = inarray;
lpbuf = lparray;
ivbuf = ivarray;
pebuf = pearray;
/* Initialize space created */
for(i=0;i<SBUFH-SBUFL+1;i++)	{
	inarray[i] = 0.0;
	pearray[i] = 0.0;
}
for(i=0;i<LBUFH-LBUFL+1;i++)
  lparray[i] = 0.0;
for(i=0;i<PWINH-PWINL+1;i++)
  ivarray[i] = 0.0;

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

for(i=0;i<MAXORD;i++)
  rcbuf[0][i] = rcbuf[1][i] = rcbuf[2][i] = 0.0;

for(i=0;i<AF;i++)
  rmsbuf[i] = 0.0;

for(i=0;i<LTAU;i++)
  amdf[i] = 0.0;

memset(voibuf, 0, (AF+1)*2);

/* onset */
for(i=0;i<16;i++)
  l2buf[i] = 0.0;

}

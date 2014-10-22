
/*******************************************************************
*
*	ANALYS Version 50
*
*******************************************************************/
#include <stdlib.h>
#include "lpcdefs.h"

/*  Constants
*    NF =     Number of frames
*    AF =     Frame in which analysis is done
*    OSLEN =  Length of the onset buffer
*    LTAU =   Number of pitch lags
*    SBUFL, SBUFH =   Start and end index of speech buffers
*    LBUFL, LBUFH =   Start and end index of LPF speech buffer
*    MINWIN, MAXWIN = Min and Max length of voicing (and analysis) windows
*    PWLEN, PWINH, PWINL = Length, upper and lower limits of pitch window
*    DVWINL, DVWINH = Default lower and upper limits of voicing window
*/


#define LPFILT_DELAY 15
#define NUM_VF 9

extern int tau[LTAU];
extern float lparray[LBUFH-LBUFL+1], ivarray[PWINH-PWINL+1];
extern float pearray[SBUFH-SBUFL+1], inarray[SBUFH-SBUFL+1];
extern float *inbuf, *pebuf, *lpbuf, *ivbuf;
extern int vwin[2][AF], awin[2][AF], voibuf[2][AF+1];
extern float rmsbuf[AF], amdf[LTAU], psi[MAXORD], rcbuf[MAXORD][AF];
extern float phi[MAXORD][MAXORD];

static float zpre[2]={0.0, 0.0};

void analys(float speech[], int voice[2], int *pitch, float *rms, float rc[])
{
/*  Data Buffers
*    INBUF	Raw speech (with DC bias removed each frame)
*    PEBUF	Preemphasized speech
*    LPBUF	Low pass speech buffer
*    IVBUF	Inverse filtered speech
*    OSBUF	Indexes of onsets in speech buffers
*    VWIN	Voicing window indices
*    AWIN	Analysis window indices
*    EWIN	Energy window indices
*    VOIBUF	Voicing decisions on windows in VWIN
*    RMSBUF	RMS energy
*    RCBUF	Reflection Coefficients
*
*  Pitch is handled separately from the above parameters.
*  The following variables deal with pitch:
*    MIDX	Encoded initial pitch estimate for analysis frame
*    IPITCH	Initial pitch computed for frame AF (decoded from MIDX)
*    PITCH	The encoded pitch value (index into TAU) for the present
*		frame (delayed and smoothed by Dyptrack)
*/

/*	REAL INBUF(SBUFL:SBUFH), PEBUF(SBUFL:SBUFH)
	REAL LPBUF(LBUFL:LBUFH), IVBUF(PWINL:PWINH)	*/


float abuf[MAXWIN], temp;
static float bias=0.;
static int osptr=1;
static int obound[AF];
static int ewin[2][AF];

int i, j, lanal;
int ipitch, minptr, maxptr, mintau, midx;
float ivrc[2];

static int osbuf[OSLEN], first=1;

#ifdef NN_VOICE
int vstart;
#else
int half;
#endif

if (first)	{
	first = 0;
	for(i=0;i<OSLEN;i++)
	  osbuf[i] = 0;
	for(i=0;i<AF;i++);
	  obound[i] = 0;
}

speech--;

/*   Calculations are done on future frame due to requirements
*   of the pitch tracker.  Delay RMS and RC's 2 frames to give
*   current frame parameters on return.
*   Update all buffers
*/

for (i = SBUFL;i<=SBUFH-LFRAME;i++) {
	inbuf[i] = inbuf[LFRAME+i];
	pebuf[i] = pebuf[LFRAME+i];
}

for( i = PWINL; i<=PWINH-LFRAME; i++) {
	   ivbuf[i] = ivbuf[LFRAME+i];
}
for( i = LBUFL;i<=LBUFH-LFRAME;i++) {
	   lpbuf[i] = lpbuf[LFRAME+i];
}


/* loop below adjusted for C indexing */
j=1;
for( i = 1;i<=osptr-1;i++) {
	if (osbuf[i-1] > LFRAME) {
		osbuf[j-1]=osbuf[i-1]-LFRAME;
		j++;
  }
}
osptr=j;

voibuf[0][0] = voibuf[0][1];
voibuf[1][0] = voibuf[1][1];
for( i = 0;i<AF-1;i++) {
	vwin[0][i] = vwin[0][i+1] - LFRAME;
	vwin[1][i] = vwin[1][i+1] - LFRAME;
	awin[0][i] = awin[0][i+1] - LFRAME;
	awin[1][i] = awin[1][i+1] - LFRAME;
	ewin[0][i] = ewin[0][i+1] - LFRAME;
	ewin[1][i] = ewin[1][i+1] - LFRAME;
	obound[i] = obound[i+1];
	voibuf[0][i+1] = voibuf[0][i+2];
	voibuf[1][i+1] = voibuf[1][i+2];
	rmsbuf[i] = rmsbuf[i+1];
	for( j = 0;j<ORDER;j++) {
		rcbuf[j][i] = rcbuf[j][i+1];
	}
}

/*   Copy input speech, scale to sign+12 bit integers
*   Remove long term DC bias.  (This code can be removed
*   if adequate high pass filtering and reliable A/D
*   conversion is available)
*/


temp = 0;
for( i = 1;i<=LFRAME;i++){
	inbuf[SBUFH-LFRAME+i] = (float) (*(speech+i)*4096. - bias);
	temp += inbuf[SBUFH-LFRAME+i];
}
if( temp > LFRAME ) bias++;
if( temp < -LFRAME ) bias--;

/*   Place Voicing Window  */

i = SBUFH + 1 - LFRAME;
preemp( &inbuf[i-1], &pebuf[i-1], LFRAME, 0.4f, zpre );

onset( pebuf, osbuf-1, &osptr);

placev(osbuf, osptr, &obound[AF-1],vwin);

/*	  The Pitch Extraction algorithm estimates the pitch for a frame
*   of speech by locating the minimum of the average magnitude difference
*   function (AMDF).  The AMDF operates on low-pass, inverse filtered
*   speech.  (The low-pass filter is an 800 Hz, 19 tap, equiripple, FIR
*   filter and the inverse filter is a 2nd-order LPC filter.)  The pitch
*   estimate is later refined by dynamic programming (DYPTRK).	However,
*   since some of DYPTRK's parameters are a function of the voicing
*   decisions, a voicing decision must precede the final pitch estimation.
*   See subroutines LPFILT, IVFILT, and TBDM. 
*/

lpfilt31(&inbuf[LBUFH-1-PWLEN-1], &lpbuf[LBUFH+1-PWLEN-1]);

ivfilt( &lpbuf[PWINL-1], &ivbuf[PWINL-1], ivrc-1 );  /*C-shifted*/

tbdm( &ivbuf[PWINL-1], tau-1, amdf-1, &minptr, &maxptr, &mintau ); /*C-shifted*/

/*	  Voicing decisions are made for each half frame of input speech.
*   An initial voicing classification is made for each half of the
*   analysis frame, and the voicing decisions for the present frame
*   are finalized.  See subroutine VOICIN.
*	 The voicing detector (VOICIN) classifies the input signal as
*   unvoiced (including silence) or voiced using the AMDF windowed
*   maximum-to-minimum ratio, the zero crossing rate, energy measures,
*   reflection coefficients, and prediction gains. 
*	 The pitch and voicing rules apply smoothing and isolated
*   corrections to the pitch and voicing estimates and, in the process,
*   introduce two frames of delay into the corrected pitch estimates and 
*   voicing decisions.
*/

#ifndef NN_VOICE
for (half = 1;half<=2;half++) {
  voicin( vwin, inbuf, lpbuf, half, amdf[minptr-1], amdf[maxptr-1], mintau, ivrc-1, obound-1, voibuf);
}

#else

vstart = PWINL + (PWLEN>>1) - (LFRAME>>1);
voicing(inbuf, lpbuf+LPFILT_DELAY, vstart, vstart+LFRAME-1, mintau, &voibuf[0][3], &voibuf[1][3]);
#endif
/*   Find the minimum cost pitch decision over several frames
*   given the current voicing decision and the AMDF array
*/

dyptrk(amdf-1, minptr, voibuf[1][AF], pitch, &midx );
ipitch = tau[midx-1];

/*   Place spectrum analysis and energy windows */
placea( ipitch, voibuf/*-(AF+1)*/, obound[AF-1], vwin, awin, ewin);

/*   Remove short term DC bias over the analysis window, Put result in ABUF */

lanal = awin[1][AF-1] + 1 - awin[0][AF-1];

dcbias(lanal, &pebuf[awin[0][AF-1]]-1, abuf-1 );

/*   Compute RMS over integer number of pitch periods within the
*   analysis window.
*   Note that in a hardware implementation this computation may be
*   simplified by using diagonal elements of PHI computed by MLOAD.
*/
energy( ewin[1][AF-1]-ewin[0][AF-1]+1, &abuf[ewin[0][AF-1]-awin[0][AF-1]]-1, &rmsbuf[AF-1] );

/*   Matrix load and invert, check RC's for stability  */
mload(lanal, abuf-1, phi, psi-1 );
invert(phi, psi, rcbuf);
rcchk(rcbuf);

/*   Set return parameters  */

voice[0] = voibuf[0][AF-2];
voice[1] = voibuf[1][AF-2];
*rms = rmsbuf[AF-3];
for(i = 1;i<=ORDER;i++)
	rc[i] = rcbuf[i-1][AF-3];
}

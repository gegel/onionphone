/***********************************************************************
*
*	TBDM Version 49
*
**********************************************************************
*
*  TURBO DIFMAG: Compute High Resolution Average Magnitude Difference Function
*
* Inputs:
*  SPEECH - Low pass filtered speech
*  LPITA  - Length of speech buffer
*  TAU	  - Table of lags
*  LTAU   - Number of lag values to compute
* Outputs:
*  AMDF   - Average Magnitude Difference for each lag in TAU
*  MINPTR - Index of minimum AMDF value
*  MAXPTR - Index of maximum AMDF value within +/- 1/2 octave of min
*  MINTAU - Lag corresponding to minimum AMDF value
*/

#include "lpcdefs.h"

void tbdm(float speech[], int tau[], float amdf[],
    	  int *minptr, int *maxptr, int *mintau)
{
int minamd;
int i, ptr, ltau2, minp2, maxp2;
float amdf2[6];
static int tau2[6];
/* static int count=0; */


/*	REAL SPEECH(LPITA+TAU(LTAU)), AMDF(LTAU), AMDF2(6)  	*/

/*   Compute full AMDF using log spaced lags, find coarse minimum	*/

difmag(speech, tau, LTAU, tau[LTAU], amdf, minptr, maxptr );


*mintau = tau[*minptr];
minamd = (int) amdf[*minptr];

/*   Build table containing all lags within +/- 3 of the AMDF minimum
*    excluding all that have already been computed	*/

ltau2 = 0;
ptr = *minptr - 2;
for(i=mmax(*mintau-3,41); i<=mmin(*mintau+3,tau[LTAU]); i++)	{
	while( tau[ptr] < i )	{
		ptr++;
	}
	if( tau[ptr] !=i) {
		ltau2++;
		tau2[ltau2-1] = i;
	}
}

/*   Compute AMDF of the new lags, if there are any, and choose one
*    if it is better than the coarse minimum	*/

if( ltau2 > 0 ) {
    difmag(speech, &tau2[0]-1, ltau2, tau[LTAU], &amdf2[0]-1, &minp2, &maxp2);
	
	if( amdf2[minp2-1] < minamd ) {
		*mintau = tau2[minp2-1];
		minamd = (int) amdf2[minp2-1];
	}
}

/*   Check one octave up, if there are any lags not yet computed	*/

if( *mintau >= 80 ) {
	i = (int) (*mintau*0.5);
	if( (i & 1) == 0 ) {
		ltau2 = 2;
		tau2[0] = i-1;
		tau2[1] = i+1;
	}
	else	{
		ltau2 = 1;
		tau2[0] = i;
	}
    difmag(speech, &tau2[0]-1, ltau2, tau[LTAU], &amdf2[0]-1, &minp2, &maxp2 );
	
	if( amdf2[minp2-1] < minamd ) {
		*mintau = tau2[minp2-1];
		minamd = (int) amdf2[minp2-1];
		*(minptr) -= 20;
	}
}

/*   Force minimum of the AMDF array to the high resolution minimum	*/

	amdf[*minptr] = (float) minamd;
	
/*   Find maximum of AMDF within 1/2 octave of minimum	*/

*maxptr = mmax(*minptr-5,1);
for(i=*maxptr+1; i<= mmin(*minptr+5,LTAU); i++) {
	if( amdf[i] > amdf[*maxptr]) *maxptr = i;
}

}

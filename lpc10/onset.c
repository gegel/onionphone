/*******************************************************************
*
*	ONSET Version 49
*
*******************************************************************
*
*	Floating point version
*/

#include "lpcdefs.h"

void onset(float pebuf[], int osbuf[], int *osptr)
{
/*   Detection of onsets in (or slightly preceding) the futuremost frame
*   of speech.

*   Arguments
*    PEBUF	Preemphasized speech
*    OSBUF	Buffer which holds sorted indexes of onsets (Modified)
*    OSPTR	Free pointer into OSBUF (Modified)

*   Parameters for onset detection algorithm:
*    L2 	Threshold for filtered slope of FPC (function of L2WID!)
*    L2LAG	Lag due to both filters which compute filtered slope of FPC
*    L2WID	Width of the filter which computes the slope of FPC
*    OSHYST	The number of samples which of slope(FPC) which must be below
*		the threshold before a new onset may be declared.
*   Variables
*    N, D	Numerator and denominator of prediction filters
*    FPC	Current prediction coefs
*    L2BUF, L2SUM1, L2SUM2    State of slope filter
*/

int l2lag=9, l2wid=16, oshyst=10;
float l2=1.7f;

static float n=0., d=1., fpc;
static float l2buf[16], l2sum1=0., l2sum2=0.;
static int l2ptr1=1, l2ptr2=9, lasti;
int i;
static short hyst=0;
/* static short first=1; */


if (hyst) lasti -= 180;


for(i=SBUFH-LFRAME+1; i<=SBUFH;i++)  {

/*   Compute FPC; Use old FPC on divide by zero; Clamp FPC to +/- 1.	*/

   n= (float) ((pebuf[i]*pebuf[i-1]+63.*n) * 0.015625);
   d= (float) ((pebuf[i-1]*pebuf[i-1]+63.*d) * 0.015625);

   if (d != 0.) {
      if ((float)fabs((double)n) > d) {
	 /*fpc = sign(1., n);*/
	fpc = (n<0)?-1.0f:1.0f;
      }
      else
	 fpc=n/d;
   }

/*   Filter FPC */
   l2sum2 = l2buf[l2ptr1-1];
   l2sum1 = l2sum1 - l2buf[l2ptr2-1] + fpc;
   l2buf[l2ptr2-1] = l2sum1;
   l2buf[l2ptr1-1] = fpc;
   l2ptr1 = (l2ptr1%l2wid)+1;
   l2ptr2 = (l2ptr2%l2wid)+1;

   if ((float)fabs((double)(l2sum1-l2sum2)) > l2) {  
      if (!hyst) { 
/*   Ignore if buffer full	*/
	 if (*osptr <= OSLEN) { 
	    osbuf[*osptr] = i - l2lag;
	    *osptr = *osptr + 1;
	 } 
	 hyst = 1;
      } 
      lasti = i;
   }
   else  if (hyst && i - lasti >= oshyst) { 
      hyst = 0;
   } 

} /* end while */


}

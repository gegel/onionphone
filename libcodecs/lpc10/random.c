/***********************************************************************
*
*	RANDOM Version 49
*
***********************************************************************
*
*  Pseudo random number generator based on Knuth, Vol 2, p. 27.
*
* Function Return:
*  RANDOM - Integer variable, uniformly distributed over -32768 to 32767
*
* Warning:  This routine contains statements which are specific to VAX Fortran.
*               I'll try it anyway - Tiner
*/

#include "lpcdefs.h"

#define MIDTAP 1
#define MAXTAP 4

int Rrandom(void)
{
int the_random;
static short y[MAXTAP+1]={-21161, -8478, 30892,-10216, 16950};
static int j=MIDTAP, k=MAXTAP;

/*   The following is a 16 bit 2's complement addition,
*   with overflow checking disabled	*/

y[k] += y[j];

#ifdef WRONG
if(y[k] > 32767) /* to make 16 bit rollover to -/+ */
#else
if(((unsigned short) y[k]) > 32767) /* to make 16 bit rollover to -/+ */
#endif
  y[k] = -(32768 - (y[k] & 32767));
if(y[k] < -32768)
  y[k] = y[k] & 32767;
	
the_random = y[k];
k--;
if (k < 0) k = MAXTAP;
j--;
if (j < 0) j = MAXTAP;

return(the_random);

}


/********************************************************************
*
*	PREEMP Version 48
*
********************************************************************
*
*   Preemphasize speech with	  ( 1 - .9375z**-1 )  [old preemphasis]
*   cascaded with ( 1 + .2z**-1 ) / ( 1 + .5z**-1 )   [6 db/oct ramp]
*
* Inputs:
*  NSAMP  - Number of samples to filter
*  INBUF  - Input speech buffer
*  COEF   - Preemphasis coeficient
* Output:
*  PEBUF  - Preemphasized speech buffer (can be equal to INBUF)
*  Z	  - Filter state
*/

#include "lpcdefs.h"

void preemp(float *inbuf, float *pebuf, int nsamp, float coef, float *z)
{
int i;
float temp=0.0;


for(i=1; i<=nsamp; i++) {
	temp = inbuf[i] - *z + coef**(z+1);
	*(z+1) = *z;
	*z = inbuf[i];
	pebuf[i] = temp;
}


}

/***********************************************************************
*
*	DIFMAG Version 49
*
**********************************************************************
*
*  Compute Average Magnitude Difference Function
*
* Inputs:
*  SPEECH - Low pass filtered speech
*  LPITA  - Length of speech buffer
*  TAU	  - Table of lags
*  LTAU   - Number of lag values to compute
*  MAXLAG - Maximum possible lag value
* Outputs:
*  AMDF   - Average Magnitude Difference for each lag in TAU
*  MINPTR - Index of minimum AMDF value
*  MAXPTR - Index of maximum AMDF value
*/

#include <math.h>
#include "lpcdefs.h"

void difmag(float speech[], int tau[], int ltau, int maxlag,
    	    float amdf[], int *minptr, int *maxptr)
{
int i, j, n1, n2;
float sum;

*minptr = 1;
*maxptr = 1;
for(i=1;i<=ltau;i++)	{
	n1 = (int) ((maxlag-tau[i]) *0.5 + 1);
	n2 = n1 + MAXWIN - 1;
	sum = 0.;
	for(j=n1;j<=n2;j+=4)	{
		sum += (float)fabs((double) (speech[j] - speech[j+tau[i]]) );
	}
	amdf[i] = sum;
	if( amdf[i] < amdf[*minptr]) 
		*minptr = i;
	if( amdf[i] > amdf[*maxptr])
		*maxptr = i;
}

}

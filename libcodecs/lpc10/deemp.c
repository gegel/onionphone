/******************************************************************
*
*	DEEMP Version 48
*
******************************************************************
*
*  De-Emphasize output speech with   1 / ( 1 - .75z**-1 )
*    cascaded with 200 Hz high pass filter
*    ( 1 - 1.9998z**-1 + z**-2 ) / ( 1 - 1.75z**-1 + .78z**-2 )
*
* Input:
*  N  - Number of samples
* In/Output:
*  X  - Speech
*/

#include "lpcdefs.h"

void deemp0(float x[], int n)
{
static float dei1=0.0, dei2=0.0, deo1=0.0, deo2=0.0, deo3=0.0;
static float deo4=0.0;
float dei0;
int k;

for(k=1;k<=n;k++)   {
	dei0 = x[k];
	x[k] = (float) (x[k] - 1.9998*dei1 + dei2 + 2.75*deo1 - 2.93*deo2 + 1.48*deo3 - 0.312*deo4);
	dei2 = dei1;
	dei1 = dei0;
	deo4 = deo3;
	deo3 = deo2;
	deo2 = deo1;
	deo1 = x[k];
}
}

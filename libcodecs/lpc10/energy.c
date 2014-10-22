/**********************************************************************
*
*	ENERGY Version 50
*
**********************************************************************
*
* Compute RMS energy
*
* Inputs:
*  LEN	  - Length of speech buffer
*  SPEECH - Speech buffer
* Output:
*  RMS	  - Root Mean Square energy
*/

#include "lpcdefs.h"
#include <math.h>

void energy(int len, float speech[], float *rms)
{
int  i;


*rms = 0;
for(i=1;i<=len;i++)
	*rms += speech[i]*speech[i];

*rms = (float) sqrt( *rms / len );

}

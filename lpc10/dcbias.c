/**********************************************************************
*
*	DCBIAS Version 50
*
**********************************************************************
*
*    Calculate and remove DC bias from buffer
*
* Inputs:
*  LEN	  - Length of speech buffers
*  SPEECH - Input speech buffer
* Output:
*  SIGOUT - Output speech buffer
*/
#include "lpcdefs.h"

void dcbias(int len, float *speech, float *sigout)
{
int i;
float bias;
register float *ptr1, *ptr2;

bias = 0;
ptr1 = speech+1;
for(i=1;i<=len;i++)	
	bias = bias + *ptr1++;

bias = bias/len;
ptr1 = sigout+1;
ptr2 = speech+1;
for(i=1;i<=len;i++)
	*ptr1++ = *ptr2++ - bias;


}

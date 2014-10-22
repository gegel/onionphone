/**********************************************************************
*
*	IVFILT Version 48
*
**********************************************************************
*
*   2nd order inverse filter, speech is decimated 4:1
*
* Inputs:
*  LEN	  - Length of speech buffers
*  NSAMP  - Number of samples to filter
*  LPBUF  - Low pass filtered speech buffer
* Output:
*  IVBUF  - Inverse filtered speech buffer
*  IVRC   - Inverse filter reflection coefficients (for voicing)
*/

#include "lpcdefs.h"

void ivfilt(float lpbuf[], float ivbuf[], float ivrc[])
{
int i, j, k;
float r[3], pc1, pc2;

/*  Calculate Autocorrelations	*/
for(i=0;i<=2;i++)	{
	r[i] = 0.;
	k = 4*i;
	for( j = (i+1)*4+PWLEN-LFRAME;j<=PWLEN;j+=2)	{
		r[i] += lpbuf[j]*lpbuf[j-k];
	}
}

/*  Calculate predictor coefficients	*/

pc1 = 0.;
pc2 = 0.;
ivrc[1] = 0.;
ivrc[2] = 0.;
if(r[0]>0.000001) {
	ivrc[1] = r[1]/r[0];
	ivrc[2] = (r[2]-ivrc[1]*r[1]) / (r[0]-ivrc[1]*r[1]);
	pc1 = ivrc[1] - ivrc[1]*ivrc[2];
	pc2 = ivrc[2];
}

/*  Inverse filter LPBUF into IVBUF	*/

for(i=PWLEN+1-LFRAME;i<=PWLEN;i++) {
	ivbuf[i] = lpbuf[i] - pc1*lpbuf[i-4] - pc2*lpbuf[i-8];
}


}

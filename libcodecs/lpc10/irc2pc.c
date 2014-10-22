/******************************************************************
*
*	IRC2PC Version 48
*
******************************************************************
*
*   Convert Reflection Coefficients to Predictor Coeficients
*
* Inputs:
*  RC	  - Reflection coefficients
*  ORDER  - Number of RC's
*  GPRIME - Excitation modification gain
* Outputs:
*  PC	  - Predictor coefficients
*  *G2PASS - Excitation modification sharpening factor
*/

#include "lpcdefs.h"
#include <math.h>

void irc2pc(float rc[MAXORD][11], float pc[], float gprime,
    	    float *g2pass, int where)
{
int i,j;
float temp[MAXORD];

*g2pass = 1.;

for(i=0;i<ORDER;i++)	
	*g2pass = (float) (*g2pass*( 1. - rc[i][where]*rc[i][where] ));

*g2pass = (float) (gprime*sqrt(*g2pass));
pc[0] = rc[0][where];

for(i=1;i<ORDER;i++)	{
	for(j=0;j<i;j++)
		temp[j] = pc[j] - rc[i][where]*pc[i-j-1];
	
	for(j=0;j<i;j++)
		pc[j] = temp[j];
	
	pc[i] = rc[i][where];
}

}

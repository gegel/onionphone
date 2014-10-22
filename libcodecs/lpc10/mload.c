/******************************************************************
*
*	MLOAD Version 48
*
******************************************************************
*
*  Load a covariance matrix
*
*  Inputs:
*    ORDER	      - Analysis order
*    AWINS	      - Analysis window start
*    AWINF	      - Analysis window finish
*    SPEECH(AWINF)    - Speech buffer
*  Outputs:
*    PHI(ORDER,ORDER) - Covariance matrix
*    PSI(ORDER)       - Prediction vector
*/

#include "lpcdefs.h"

void mload(int awinf, float speech[],
    	   float phi[ORDER][ORDER], float psi[])  
{
int r, c, i, start;


/*   Load first column of triangular covariance matrix PHI	*/

start = 1 + ORDER;
for(r=1;r<=ORDER;r++)	{
	phi[r-1][0] = 0.;
	for(i=start;i<=awinf;i++)  {
		phi[r-1][0] += speech[i-1]*speech[i-r];
	}
}

/*   Load last element of vector PSI	*/

psi[ORDER] = 0.;
for(i=start;i<=awinf;i++)
	psi[ORDER] += speech[i]*speech[i-ORDER];
	

/*   End correct to get additional columns of PHI	*/

for(r=2;r<=ORDER;r++)
	for(c=2;c<=r;c++)
		phi[r-1][c-1] = phi[r-2][c-2]
		       - speech[awinf+1-r]*speech[awinf+1-c]
			 + speech[start-r]*speech[start-c];
								
/*   End correct to get additional elements of PSI	*/

for(c=1;c<ORDER;c++)
	psi[c] = phi[c][0] - speech[start-1]*speech[start-1-c]
			       + speech[awinf]*speech[awinf-c];
										

}

/*****************************************************************
*
*	INVERT Version 45G
*
*****************************************************************
*
*  Invert a covariance matrix using Choleski decomposition method
*
*  Inputs:
*    ORDER	      - Analysis ORDER
*    PHI(ORDER,ORDER) - Covariance matrix
*    PSI(ORDER)       - Column vector to be predicted
*  Outputs:
*    RC(ORDER)	      - Pseudo reflection coefficients
*  Internal:
*    V(ORDER,ORDER)   - Temporary matrix
*    X(ORDER)	      - Column scaling factors
*
*  NOTE: Temporary matrix V is not needed and may be replaced
*    by PHI if the original PHI values do not need to be preserved.
*/

#include "lpcdefs.h"
#include <math.h>

void invert(float phi[MAXORD][MAXORD], float psi[], float rc[MAXORD][AF])
{
int i, j, k;
static float save, eps=1.0f-10;

/*  Decompose PHI into V * D * V' where V is a triangular matrix whose
*   main diagonal elements are all 1, V' is the transpose of V, and
*   D is a vector.  Here D(n) is stored in location V(n,n).		*/


for(j=0;j<ORDER;j++)	{
/**  for(i=j;i<ORDER;i++) {
		v[i][j] = phi[i][j];
	}**/
	for(k=0;k<j;k++)	{
		save = phi[j][k]*phi[k][k];
		for(i=j;i<ORDER;i++)	{
	phi[i][j] -= phi[i][k]*save;
		}
	}
	
/*  Compute intermediate results, which are similar to RC's     */

	if((float) fabs((double)phi[j][j]) < eps ) break;
  rc[j][AF-1] = psi[j];
	for(k=0;k<j;k++)
	{
		rc[j][AF-1] -= rc[k][AF-1]*phi[j][k];
	}
	phi[j][j] = 1.f/phi[j][j];
	rc[j][AF-1] = rc[j][AF-1]*phi[j][j];
	rc[j][AF-1] = mmax(mmin(rc[j][AF-1],.999f),-.999f);
	
	
}
if((float) fabs((double)phi[j][j]) < eps )	{

/*  Zero out higher order RC's if algorithm terminated early    */

	for(i=j;i<ORDER;i++)
	   rc[i][AF-1] = 0.;

}



}

/**********************************************************************
*
*	MEDIAN Version 45G
*
**********************************************************************
*
*  Find median of three values
*
* Input:
*  D1,D2,D3 - Three input values
* Output:
*  MEDIAN - Median value
*/

#include "lpcdefs.h"

int median(int d1, int d2, int d3)
{
int the_median;

the_median = d2;
if    ( d2 > d1 && d2 > d3 ) {
	the_median = d1;
	if ( d3 > d1 ) the_median = d3;
}
else if ( d2 < d1 && d2 < d3 ) {
	   the_median = d1;
	   if ( d3 < d1 ) the_median = d3;
}
return(the_median);
}

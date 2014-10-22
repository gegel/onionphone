/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		bwexp
*
* FUNCTION
*		Bandwidth expansion of LPC predictor coefficients
*
* SYNOPSIS
*		subroutine bwexp(alpha, pc, pcexp, n)
*
*   formal 
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	alpha		float	i	Bandwidth expansion factor
*	pc		float	i	predictor coefficients
*	pcexp		float	o	expanded predictor coefficients
*	n		int	i	predictor order
***************************************************************************
*	
* DESCRIPTION
*
*	Subroutine to perform bandwidth modification by moving the poles
*	(or zeros) radially in the z plane.  If the bandwidth expansion
*	factor (alpha) is less than unity, the bandwidths are expanded by
*	shifting the poles (or zeros) toward the origin of the z plane.
*	The predictor coefficients are scaled directly according to:
*
*			      i-1
*               a' = a  alpha           where i = 1, . . . , order+1
*		 i    i
*
*	Resulting in a bandwidth expansion of:
*
*		-(fs/pi)ln(alpha) Hz
*
*	(e.g., fs = 8 kHz, alpha = 0.994127 -> 15 Hz bandwidth expansion)
*
*       CELP's LPC predictor coefficient convention is:
*	       p+1	   -(i-1)
*	A(z) = SUM   a	 z	    where a  = +1.0
*	       i=1    i 		   1
*
***************************************************************************
*
* CALLED BY
*
*	autohf	confg	impls	postfilter
*
* CALLS
*
*	
*
**************************************************************************/

static void bwexp(float alpha, float pc[], float pcexp[], int n)
{
	int i;
	for (i = 0; i <= n; i++)
		pcexp[i] = pc[i] * pow(alpha, (double)(i));
}

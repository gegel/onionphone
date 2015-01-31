/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* NAME
*	pctorc 
*
* FUNCTION
*
*	Convert from lp-polynomial to reflection coefficients.
*
*	BEWARE: This code does not use memory efficiently.
*
* SYNOPSIS
*
*	subroutine pctorc(lpc, rc, n)
*
*   formal 
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	lpc(n+1)	float	i	Array of n+1 coefficients
*					a(0)+a(1)z**(-1) + a(2)Z**(-2) +
*					.... + a(n)z**(-n)
*	rc(n)		float	i/o	  reflection coefficients (voiced-> +rc1)
*	n		int	i	Order of polynomial
*     
***************************************************************************
*	
* DESCRIPTION
*
*	This routine uses the Levinson recursion to compute reflection
*	coefficients from the LPC coefficients.  The first LPC
*	coefficient is assumed to be 1, and although it is passed
*	to the routine, it is not used in the calculations.
*	Note:  the dimension of the internal array t limits the value
*	of the maximum order.
*
*       CELP's LPC predictor coefficient convention is:
*	       p+1	   -(i-1)
*	A(z) = SUM   a	 z	    where a  = +1.0
*	       i=1    i 		   1
*
*	The sign convention used defines the first reflection coefficient
*	as the normalized first autocorrelation coefficient, which results
*	in positive values of rc(1) for voiced speech.
*
***************************************************************************
*
* CALLED BY
*
*	autohf	postfilter  specdist celp  intsynth
*
* CALLS
*
*
*
**************************************************************************/

static void pctorc(const float lpc[], float rc[], int n)
{
	float t[MAXNO + 1], a[MAXNO + 1];
	int i, j;

	for (i = 0; i <= n; i++)
		a[i] = lpc[i];
	for (i = n; i > 1; i--) {
		rc[i - 1] = -a[i];
		for (j = 1; j < i; j++)
			t[i - j] =
			    (a[i - j] + rc[i - 1] * a[j]) / (1.0 -
							     rc[i - 1] * rc[i -
									    1]);
		for (j = 1; j < i; j++)
			a[j] = t[j];
	}
	rc[0] = -a[1];
}

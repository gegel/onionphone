/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		zerofilt
*
* FUNCTION
*		Direct form all-zero filter
*
* SYNOPSIS
*		subroutine zerofilt(b, n, z, xy, len)
*
*   formal 
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	b		real	i	N+1 filter coefficients
*	n		int	i	Filter order 
*	z		real	i/o	N+1 filter delay elements 
*					(maintained by the user)
*	xy		real	i/o	Input/Output data array 
*	len		int	i	Number of samples to filter
*
***************************************************************************
*	
* DESCRIPTION
*
*	Nonrecursive all-zero in-place time-domain filter.
*	The filter is implemented in a direct form realisation.
*
*		N	-i
*	H(z) = SUM b(i)z
*	       i=0
*
*	x(t) ->---(z0)----- b0 >------+-----> y(t)
*		   |		      |
*		   z1------ b1 >------+
*		   |		      |
*		   z2------ b2 >------+
*		   |		      |
*		   :		      :
*		   |		      |
*		   zN------ bN >------+
*
***************************************************************************
*
* CALLED BY
*
*	celp	confg	postfilt
*
* CALLS
*
*
*
***************************************************************************
*
* REFERENCES
*
*	Oppenheim & Schafer, Digital Signal Processing, PH, 1975, p. 149.
*
**************************************************************************/

static void zerofilt(const float b[], int n, float z[], float xy[], int len)
{
	float ar;
	int t, j;

	for (t = 0; t < len; t++) {
		z[0] = xy[t];
		ar = 0.0;
		for (j = n; j > 0; j--) {
			ar += z[j] * b[j];
			z[j] = z[j - 1];
		}
		xy[t] = ar + z[0] * b[0];
	}
}

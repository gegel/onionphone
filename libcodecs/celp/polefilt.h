/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		polefilt
*
* FUNCTION
*		Direct form all-pole filter
*
* SYNOPSIS
*		subroutine polefilt(a, n, z, xy, len)
*
*   formal 
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	a		float	i	N+1 filter coefficients
*	n		int	i	Filter order 
*	z		float	i/o	N+1 filter delay elements
*					(maintained by the user)
*					(z[0] is a dummy delay)
*	xy		float	i/o	Input/Output data array 
*	len		int	i	Number of samples to filter
*
***************************************************************************
*	
* DESCRIPTION
*
*	Recursive all-pole in-place time-domain filter.
*	The transfer function 1/A(z) is implemented
*	in a direct form realisation.
*
*		    N	    -i
*	H(z) = 1 / SUM a(i)z	     with a(0) = +1.0
*		   i=0
*
*	NOTE:  a(0) is not used for the actual computing,
*	as can easily be seen in the following flow graph.
*
*	x(t) ->---+------->--------(z0)-----> y(t)
*		  |		     |
*		  +-------< -a1 ----z1
*		  |		     |
*		  +-------< -a2 ----z2
*		  |		     |
*		  :		     :
*		  |		     |
*		  +-------< -aN ----zN
*
***************************************************************************
*
* CALLED BY
*
*	celp	confg	impulse   postfilt
*
* CALLS
*
*
***************************************************************************
*
* REFERENCES
*
*	Oppenheim & Schafer, Digital Signal Processing, PH, 1975, p. 149.
*
**************************************************************************/

static void polefilt(const float a[], int n, float z[], float xy[], int len)
{
	int t, j;

	if (a[0] != 1.0) {
#ifdef CELPDIAG
		fprintf(stderr, "polefilt:  bad coefficients");
#endif
		CELP_ERROR(CELP_ERR_POLEFILT);
		return;
	}

	for (t = 0; t < len; t++) {
		z[0] = xy[t];
		for (j = n; j > 0; j--) {
			z[0] -= a[j] * z[j];
			z[j] = z[j - 1];
		}
		xy[t] = z[0];
	}
}

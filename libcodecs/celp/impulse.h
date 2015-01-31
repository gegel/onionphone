/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		impulse
*
* FUNCTION
*		compute impulse response with direct form filter
*		exclusive of adaptive code book contribution
*
* SYNOPSIS
*		subroutine impulse(l)
*
*   formal
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	l		int	i	impulse response length
*
*   external
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	no		int	i
*	fc[]		float	i
*	h[]		float	i/o
*	gamma2		float	i
*
***************************************************************************
*
* Global Variables
*
*
*	SPECTRUM VARIABLE:
*	d5	real	auxiliary array
*
****************************************************************************
* CALLED BY
*
*	csub
*
* CALLS
*
*	bwexp	setr	polefilt
*
**************************************************************************/

static void impulse(int l)
{
	float d5[MAXNO + 1], fctemp[MAXNO + 1];

	setr(l, 0.0, h);
	h[0] = 1.0;
	setr(no + 1, 0.0, d5);
	bwexp(gamma2, fc, fctemp, no);
	polefilt(fctemp, no, d5, h, l);

}

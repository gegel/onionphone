/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		ham
*
* FUNCTION
*		creates hamming window
*
* SYNOPSIS
*		subroutine ham(win, n)
*   formal 
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	win(n)		float	i/o	hamming window
*	n		int	i	dim of win
*
***************************************************************************
*
* DESCRIPTION
*
*
*
***************************************************************************
*
* CALLED BY
*
*	celp  delay
*
* CALLS
*
*
*
**************************************************************************/

static void ham(float win[], int n)
{
	int i;

	for (i = 0; i < n; i++)
		win[i] = 0.54 - 0.46 * cos((2.0 * CELP_PI * (i)) / (n - 1));
}

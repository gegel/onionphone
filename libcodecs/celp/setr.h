/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		setr
*
* FUNCTION
*		initialize a real array to a real value
*
* SYNOPSIS
*		subroutine setr(n, v, a)
*
*   formal 
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	n		int	i	dimension of array
*	v		float	i	initialization value
*	a[n]		float	i/o	array
*
***************************************************************************
*
* CALLED BY
*
*	csub	impulse  
*
* CALLS
*
*	
*
**************************************************************************/

static void setr(int n, float v, float a[])
{
	int i;

	for (i = 0; i < n; i++)
		a[i] = v;
}

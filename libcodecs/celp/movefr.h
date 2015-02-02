/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		movefr
*
* FUNCTION
*		copy real array to another array
*
* SYNOPSIS
*		subroutine movefr(n, a, b)
*
*   formal 
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	n		int	i	number of elements to copy
*	a		float	i	source
*	b		float	o	destination
***************************************************************************
*
* CALLED BY
*
*	csub	psearch
*
* CALLS
*
*
**************************************************************************/

#ifdef MOVEFR_SUB
static void movefr(int n, float *a, float *b)
{
	int i;

	for (i = 0; i < n; i++)
		*b++ = *a++;
}
#endif

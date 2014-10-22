/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* NAME
*	cor
*
* FUNCTION
*
*	compute auto-correlation coefficients by direct multiplication
*
* SYNOPSIS
*
*	subroutine cor(rar, idim, n, c0, c)
*
*   formal 
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	rar		float	i	Input data frame
*	idim		int	i	frame size
*	n		int	i	Number of correlation terms, 
*					 exclusive C0
*	c0		float	o	C(0)
*	c		float	o	Auto correlation terms C(i), i=1,n
*	
***************************************************************************
*	
* DESCRIPTION
*
*	COR computes the autocorrelation coefficients of the data
*	sequence rar according to the following formula:
*
*	       idim
*	C(i) = SUM   rar(k) * rar(k-i)	 , where i = 0, ..., n lags
*	       k=i+1
*
*	c0 = C(0)
*
*	NOTE:  rar(k-i) is truncated, so C(i) are true autocorrelations.
*
***************************************************************************
*
* CALLED BY
*
*	autohf	distortion
*
* CALLS
*
*
*
**************************************************************************/

static void cor(float *rar, int idim, int n, float *c0, float *c)
{
	int i, k;

	for (*c0 = 0.0, i = 0; i < idim; i++)
		*c0 += *(rar + i) * *(rar + i);

	for (i = 0; i < n; i++) {
		for (*(c + i) = 0.0, k = i + 1; k < idim; k++)
			*(c + i) += *(rar + k) * *(rar + k - i - 1);
	}
}

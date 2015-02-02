/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*****************************************************************************
*
* NAME
*	durbin
*
* FUNCTION
*
*	Durbin recursion to do autocorrelation analysis.
*	Converts autocorrelation sequence to reflection coefficients
*	and prediciton coefficients.
*
* SYNOPSIS
*
*	subroutine durbin (c0, c, a, n)
*
*   formal 
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	c0		real	i	c(0)
*	c[n]		real	i	auto-correlation coefficients
*	a[n]		real	o	prediction coefficients (voiced-> +a1)
*	n		int	i	number of reflection coefficients
*	
****************************************************************************
*	
* DESCRIPTION
*
*	This performs the classical Durbin recursion
*	on the correlation sequence c0, c[0], c[1] . . .
*	to obtain n reflection coefficients (rc).
*
*	The sign convention used defines the first reflection coefficient
*	as the normalized first autocorrelation coefficient, which results
*	in positive values of rc[0] for voiced speech.
*
****************************************************************************
*
* CALLED BY
*
*	autohf
*
* CALLS
*
****************************************************************************
*	
* REFERENCES
*
*	Parsons, Voice and Speech Processing, McGraw-Hill, 1987, p.160&378.
*
****************************************************************************/

static void durbin(float c0, float *c, float *a, int n)
{
	int i, j;
	float alpha, beta, rc[MAXNO], tmp[MAXNO];

	for (i = 0; i < MAXNO; i++)
		tmp[i] = 0;

	/* If zero energy, set rc's to zero & return  */

	if (c0 <= 0.0) {
		for (i = 0; i < n; i++)
			rc[i] = 0.0;
		return;
	}

	/* Intialization   */

	alpha = c0;
	*a = rc[0] = -*c / c0;
	beta = *c;

	/* Recursion   */

	for (i = 1; i < n; i++) {
		alpha = alpha + beta * rc[i - 1];
		beta = *(c + i);

		for (j = 0; j <= i - 1; j++)
			beta = beta + *(c + j) * *(a + i - j - 1);
		rc[i] = -beta / alpha;

		for (j = 0; j <= i - 1; j++)
			tmp[j] = rc[i] * *(a + i - j - 1);

		for (j = 0; j <= i - 1; j++)
			*(a + j) = *(a + j) + tmp[j];
		*(a + i) = rc[i];
	}
}

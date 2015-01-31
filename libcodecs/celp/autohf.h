/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		autohf
*
* FUNCTION
*		LPC autocorrelation analysis with high frequency compensation
*
*
* SYNOPSIS
*		subroutine autohf(si, w, n, p, omega, a, rc)
*
*   formal 
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	si(n)		float	i	signal input
*	w(n)		float	i	window (i.e., Hamming)
*	n		int	i	length of input sequence
*	p		int	i	order of LPC polynomial
*	omega		float	i	bandwidth expansion factor
*	a		float	o	LPC coefficients (1 to m+1)
*	rc		float	o	reflection coefficients (1 to m)
*					(voiced-> +rc1)
*
*   external 
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	frame		int	i	
*
***************************************************************************
*	
* DESCRIPTION
*
*	Subroutine to perform HF corrected autocorrelation LPC analysis.
*	First, autocorrelation coefficients are calculated and high
*	frequency corrected to partially compensate for the analog
*	antialiasing filter*.  (Traditionally, this technique has only been
*	applied to covariance analysis, but it applies to autocorrelation
*	analysis as well).  Next, the autocorrelation function is converted
*	to reflection coefficients by the Schur recursion (aka LeRoux &
*	Guegen).  Then, the reflection coefficients are converted to LPC
*	predictor coefficients.  Finally, the predictors are bandwidth
*	expanded by omega.
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
*	celp
*
* CALLS
*
*	actorc	bwexp	cor	pctorc	rctopc
*
***************************************************************************
*	
* REFERENCES
*
*	*Atal & Schroeder, Predictive Coding of Speech Signals
*	 and Subjective Error Criteria, IEEE TASSP, June 1979.
*
**************************************************************************/

static void autohf(float si[], float w[], int n, int p, float omega, float a[],
		   float rc[])
{
	int i, unstable;
	float c0, c[MAXNO], atemp[MAXNO + 1], s[MAXLL];

	setr(MAXNO + 1, 0.0, atemp);
	for (i = 0; i < n; i++)
		s[i] = si[i] * w[i];	/* apply window                 */
	unstable = FALSE;
	cor(s, n, p, &c0, c);	/* calculate autocorrelations   */
	if (c0 < 0.0)
		unstable = TRUE;
	atemp[0] = 1.0;		/* convert autocorrelations to pc's  */
	durbin(c0, c, &atemp[1], p);

	bwexp(omega, atemp, a, p);	/* expand corrected pc's        */
	pctorc(a, rc, p);	/* match rc's to expanded pc's  */
	/* and test for stability       */
	for (i = 0; i < p; i++) {
		if (fabs(rc[i]) > 1.0)
			unstable = TRUE;
	}
	if (unstable) {
#ifdef CELPDIAG
		fprintf(stderr, "autohf:  unstable lpc analysis at frame %d\n",
			frame);
#endif
		for (i = 0; i < p; i++) {
			a[i + 1] = 0.0;
			rc[i] = 0.0;
		}
	}
}

/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* NAME
*	lsptopc 
*
* FUNCTION
*
*	convert lsp frequencies to predictor coefficients
*
* SYNOPSIS
*
*	subroutine lsptopc(f, pc)
*
*   formal
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	f		real	i	lsp frequencies
*	pc		real	o	LPC predictor coefficients
*
*   external
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	no		int	i
*	frame		int	i
*
***************************************************************************
*
* DESCRIPTION
*
*	LSPTOPC converts line spectral frequencies to LPC predictor
*	coefficients.
*
*	The analysis filter may be reconstructed:
*
*		A(z) = 1/2 [ P(z) + Q(z) ]
*
*       CELP's LPC predictor coefficient convention is:
*	       p+1	   -(i-1)
*	A(z) = SUM   a	 z	    where a  = +1.0
*	       i=1    i 		   1
*
***************************************************************************
*
* CALLED BY
*
*	celp	spectdist   intsynth
*
* CALLS
*
*
*
**************************************************************************/

static void lsptopc(float f[], float pc[])
{
	int i, j, k, noh;
#ifdef CELPDIAG
	int lspflag;
#endif
	float freq[MAXNO], p[MAXNO / 2], q[MAXNO / 2];
	float a[MAXNO / 2 + 1], a1[MAXNO / 2 + 1], a2[MAXNO / 2 + 1];
	float b[MAXNO / 2 + 1], b1[MAXNO / 2 + 1], b2[MAXNO / 2 + 1];
	float xx, xf;

	/* *check input for ill-conditioned cases                      */

#ifdef CELPDIAG
	if (f[0] <= 0.0 || f[0] >= 0.5) {
		fprintf(stderr,
			"lsptopc: LSPs out of bounds; f(0) = %f at frame %d\n",
			f[0], frame);
	}
#endif
	lspflag = FALSE;
	for (i = 1; i < no; i++) {
		if (f[i] <= f[i - 1])
			lspflag = TRUE;
#ifdef CELPDIAG
		if (f[i] <= 0.0 || f[i] >= 0.5) {
			fprintf(stderr,
				"lsptopc: LSPs out of bounds; f(%d) = %f at frame %d\n",
				i, f[i], frame);
		}
#endif
	}
#ifdef CELPDIAG
	if (lspflag) {
		fprintf(stderr, "lsptopc: nonmonotonic LSPs at frame %d\n",
			frame);
	}
#endif

	/* *initialization                                             */

	noh = no / 2;
	for (j = 0; j < no; j++)
		freq[j] = f[j];
	for (i = 0; i < noh + 1; i++) {
		a[i] = 0.;
		a1[i] = 0.;
		a2[i] = 0.;
		b[i] = 0.;
		b1[i] = 0.;
		b2[i] = 0.;
	}

	/* *lsp filter parameters                                      */

	for (i = 0; i < noh; i++) {
		p[i] = -2. * cos(2. * CELP_PI * freq[2 * i]);
		q[i] = -2. * cos(2. * CELP_PI * freq[2 * i + 1]);
	}

	/* *impulse response of analysis filter                        */

	xf = 0.0;
	for (k = 0; k < no + 1; k++) {
		xx = 0.0;
		if (k == 0)
			xx = 1.0;
		a[0] = xx + xf;
		b[0] = xx - xf;
		xf = xx;
		for (i = 0; i < noh; i++) {
			a[i + 1] = a[i] + p[i] * a1[i] + a2[i];
			b[i + 1] = b[i] + q[i] * b1[i] + b2[i];
			a2[i] = a1[i];
			a1[i] = a[i];
			b2[i] = b1[i];
			b1[i] = b[i];
		}
		if (k != 0)
			pc[k - 1] = -.5 * (a[noh] + b[noh]);
	}

	/* *convert to CELP's predictor coefficient array configuration */

	for (i = no - 1; i >= 0; i--)
		pc[i + 1] = -pc[i];
	pc[0] = 1.0;
}

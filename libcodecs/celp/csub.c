/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		csub
*
* FUNCTION
*		control routine to find optimal excitation 
*		(adaptive and stochastic code book searches)
*
* SYNOPSIS
*		subroutine csub(s, v, l, lp)
*
*   formal
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	s[l]		float	i	speech or residual segment
*	v[l]		float	o	optimum excitation vector
*	l		int	i	stochastic analysis frame size
*	lp		int	i	adaptive (pitch) analysis frame size
*
*   external
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	idb		int	i
*	no		int	i
*	nseg		int	i
*	sg		int	i
*	e0[]		float	i/o
*	fndex_e0_vid	int	i
*	fndpp_e0_vid	int	i
*	mxsw		int	i	modified excitation switch
*	
***************************************************************************
*
*  Global
*
*	SPECTRUM VARIABLES:
*	d2a	real	memory 1/A(z)
*	d2b	real	memory 1/A(z)
*	d3a	real	memory A(z)
*	d3b	real	memory A(z)
*	d4a	real	memory 1/A(z/gamma)
*	d4b	real	memory 1/A(z/gamma)
*
*	PITCH VARIABLES:
*	d1a	real	memory 1/P(z)
*	d1b	real	memory 1/P(z)
***************************************************************************
*
* CALLED BY
*
*	celp
*
* CALLS
*
*	confg	cbsearch   psearch   movefr   save_sg	setr
*
*	mescite1   mexcite2
*
**************************************************************************/

static float d1a[MAXPA], d1b[MAXPA], d2a[MAXNO + 1], d2b[MAXNO + 1],
    d3a[MAXNO + 1], d3b[MAXNO + 1];
static float d4a[MAXNO + 1], d4b[MAXNO + 1];

static void csub(float s[], float v[], int l, int lp)
{

	/* *find the intial error without pitch VQ                     */

	setr(l, 0.0, e0);
	confg(s, l, d1a, d2a, d3a, d4a, 0, 1, 1, 1);
	movefr(no + 1, d2b, d2a);
	movefr(no + 1, d3b, d3a);
	movefr(no + 1, d4b, d4a);

	/* *find impulse response (h) of perceptual weighting filter   */

	impulse(l);

	/* *norm of the first error signal for const. exc.             */

	if (mxsw)
		mexcite1(l);

	/* *pitch (adaptive code book) search                                          */

	/* Get pp parameters every segment if lp = l. If lp <> l then  */
	/* get pp parameters on odd segments.                          */

	if (lp == l)
		psearch(l);
	else if (nseg % 2 == 1)
		psearch(lp);

	/* *find initial error with pitch VQ                           */

	setr(l, 0.0, e0);
	confg(s, l, d1a, d2a, d3a, d4a, 1, 1, 1, 1);

	/* *norm of second error signal for const. exc.                */

	if (mxsw)
		mexcite2(l);

	/* *stochastic code book search                                */

	cbsearch(l, v);

	/* *update filter states                                       */

	movefr(l, v, e0);
	confg(s, l, d1b, d2b, d3b, d4b, 1, 1, 1, 1);
	movefr(idb, d1b, d1a);
	movefr(no + 1, d2b, d2a);
	movefr(no + 1, d3b, d3a);
	movefr(no + 1, d4b, d4a);
}

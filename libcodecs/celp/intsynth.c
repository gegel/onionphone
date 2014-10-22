/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		intsynth
*
* FUNCTION
*		Linearly interpolate between transmitted LSPs
*		to generate nn (=4) intermediate sets of LSP
*		frequencies for subframe synthesis. 
*		
*
* SYNOPSIS
*		subroutine intsynth(lspnew, nn, lsp, twoerror, syndavg)
*
*   formal 
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	lspnew		float	i/o	new frequency array
*	nn		int	i	number of segments/frame
*	lsp		float	o	interpolated frequency matrix
*	twoerror	int	i	flag for occurrence of two errors
*					  in Hamming protected bits.
*	syndavg 	float	i	bit error estimation parameter
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
*               This routine interpolates lsp's for subframe synthesis.
*	This version is only for use with absolute scalar LSP coding!
*	The interpolated LSPs are identical to the interpolated set in the
*	transmitter provided there are no transmission errors.	If the 
*       LSP's are nonmonotonic, then LSP errors have occured and an 
*       attempt is made to "fix" them by repeating previous LSP values. 
*	If this correction fails (the vector is still nonomonotonic),
*	then the entire previous LSP vector is repeated.  (This version
*	ignores twoerror and syndavg.)
*
*
***************************************************************************
*
* CALLED BY
*
*	celp
*
* CALLS
*
*	lsptopc  pctorc
*
**************************************************************************/

static void intsynth(float lspnew[], int nn, float lsp[][MAXNO],
		     int twoerror, float syndavg)
{
#ifndef CELPDIAG
	(void)twoerror;
	(void)syndavg;
#endif
	int i, j, nonmono;
	float temp[MAXNO + 1], rc[MAXNO];
	static const float w[2][4] = {
		{0.875, 0.625, 0.375, 0.125},
		{0.125, 0.375, 0.625, 0.875}
	};
	static float lspold[MAXNO] = {
		.03, .05, .09, .13, .19,
		.23, .29, .33, .39, .44
	};

	/* *try to fix any nonmonotonic LSPs by repeating pair         */

	for (i = 1; i < no; i++) {
		if (lspnew[i] <= lspnew[i - 1]) {
#ifdef CELPDIAG
			fprintf(stderr,
				"intsynth: try to fix any nonmonotonic LSPs\n");
#endif
			lspnew[i] = lspold[i];
			lspnew[i - 1] = lspold[i - 1];
		}
	}

	/* *check fixed LSPs (also check for pairs too close?)         */

	nonmono = FALSE;
	for (i = 1; i < no; i++) {
		if (lspnew[i] <= lspnew[i - 1])
			nonmono = TRUE;
	}

	/* *if fix fails, repeat entire LSP vector                     */

	if (nonmono) {
#ifdef CELPDIAG
		fprintf(stderr, "intsynth: repeat entire LSP vector\n");
		fprintf(stderr,
			"intsynth: syndavg = %f  twoerror = %d  frame = %d\n",
			syndavg, twoerror, frame);
		fprintf(stderr, "lspold          lspnew\n");
#endif
		for (i = 0; i < no; i++) {
#ifdef CELPDIAG
			fprintf(stderr, "%-14f %-14f \n", lspold[i], lspnew[i]);
#endif
			lspnew[i] = lspold[i];
		}
	}

	/* *OPTIONAL (and not finished):                               */
	/* *if large prediction gain then repeat close LSP pair        */

	lsptopc(lspnew, temp);
	pctorc(temp, rc, no);

	/* *interpolate lsp's                                          */
	for (i = 0; i < nn; i++) {
		for (j = 0; j < no; j++)
			lsp[i][j] = w[0][i] * lspold[j] + w[1][i] * lspnew[j];

		/* *OPTIONAL bug checker                                     */
		/* *check for monotonically increasing lsp's                 */
#ifdef CELPDIAG
		nonmono = FALSE;
		for (j = 1; j < no; j++) {
			if (lsp[i][j] <= lsp[i][j - 1])
				nonmono = TRUE;
		}

		if (nonmono) {
			fprintf(stderr,
				"intsynth: nonmono LSPs @ frame %d CAN'T HAPPEN\n",
				frame);
			fprintf(stderr, "intsynth: LSPs=");
			for (j = 0; j < no; j++)
				fprintf(stderr, "  %f", lsp[i][j]);
			fprintf(stderr, "\n");
		}
#endif
	}

	/*            *update lsp history                                      */

	for (i = 0; i < no; i++)
		lspold[i] = lspnew[i];
}

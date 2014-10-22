/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		intanaly
*
* FUNCTION
*		Linearly interpolate between transmitted LSPs
*		to generate nn (=4) intermediate sets of LSP
*		frequencies for subframe analysis. 
*		
*
* SYNOPSIS
*		subroutine intanaly(lspnew, nn, lsp)
*
*   formal 
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	lspnew		float	i	new frequency array
*	nn		int	i	number of segments/frame
*	lsp		float	o	interpolated frequency matrix
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
*               This routine linearly interpolates lsp's for analysis in
*	nn (=4) subframes.  This is a combination of inter- and 
*	intra-frame interpolation.  There are two routines, one for the 
*	analyzer and one for the synthesizer.
*
*		The lsps are interpolated from two transmitted frames,
*	 old and new.  The lsp interpolation is calculated as follows:
*
*	superframe:	  old		       new
*
*		|		      | 		    |
*		|---------------------|---------------------|
*		|		      | 		    |
*
*                \                                         /
*                 \                                       /
*
*	subframe:	  1	  2	   3	    4
*		     |					 |
*	       ...---|--------|--------|--------|--------|
*		     |					 |
*			  v	  v	   v	    v
*
*	weighting:
*		old:	 7/8	  5/8	   3/8	    1/8
*		new:	 1/8	  3/8	   5/8	    7/8
*
*	Note: This is dependent on nn = ll/l = 4!
*	
***************************************************************************
*
* CALLED BY
*
*	celp
*
* CALLS
*
*
*
**************************************************************************/

static float tempfreq;
static const float w[2][4] = { {0.875, 0.625, 0.375, 0.125},
{0.125, 0.375, 0.625, 0.875}
};

static void intanaly(float lspnew[], int nn, float lsp[][MAXNO])
{
	int i, j, nonmono;
	static float lspold[MAXNO] =
	    { .03, .05, .09, .13, .19, .23, .29, .33, .39, .44 };
	static float oldlsp[MAXNO];

	for (i = 0; i < nn; i++)

		/* *interpolate lsp's                                          */

	{
		for (j = 0; j < no; j++)
			lsp[i][j] = w[0][i] * lspold[j] + w[1][i] * lspnew[j];

		/* *OPTIONAL bug checker
		 *check for monotonically increasing lsp's
		 *swap crossed LSPs                                 */

		for (j = 1; j < no; j++) {
			if (lsp[i][j] < lsp[i][j - 1]) {
#ifdef CELPDIAG
				fprintf(stderr,
					"intanaly: Swapping nonmono lsps @ frame %d\n",
					frame);
#endif
				tempfreq = lsp[i][j];
				lsp[i][j] = lsp[i][j - 1];
				lsp[i][j - 1] = tempfreq;
			}
		}

		/* *recheck for monotonically increasing lsp's
		 *substitute old LSPs (they must be really messed up!)      */

		nonmono = FALSE;
		for (j = 1; j < no; j++) {
			if (lsp[i][j] < lsp[i][j - 1])
				nonmono = TRUE;
		}
		if (nonmono) {
#ifdef CELPDIAG
			fprintf(stderr,
				"intanaly: Resetting interp LSP at frame %d\n",
				frame);
#endif
			for (j = 0; j < no; j++) {
				if (i == 0)
					lsp[i][j] = oldlsp[j];
				else
					lsp[i][j] = lsp[i - 1][j];
			}
		}
	}

	/*            *save lsp's for next pass                               */

	for (j = 0; j < no; j++) {
		lspold[j] = lspnew[j];
		oldlsp[j] = lsp[nn][j];
	}
}

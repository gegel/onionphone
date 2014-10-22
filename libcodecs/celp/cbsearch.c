/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		cbsearch
*
* FUNCTION
*		find optimal MSPE excitation code word
*
* SYNOPSIS
*		subroutine cbsearch(l, v)
*
*   formal 
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	l		int	i	length of vectors
*	v(l)		float	o	optimum excitation segment found
*
*   external 
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	cbgbits 	int	i	code book gain bit allocation
*	cbindex 	int	i/o	code book index
*	gindex		int	i/o	gain index
*	ncsize		int	i	code book size
*	no		int	i	filter order  predictor
*	h[]		float	i	impulse response
*	x[];		float	i	code book
*	cbgtype[];	char	i	code book gain quantizer type
*	cb_gain_vid	int	i
*	cb_qgain_vid	int	i
*	cb_match_vid	int	i
*	cb_exc_vid	int	i
*	cb_ir_vid	int	i
*	dbcon_vid	int	i
*	mxsw		int	i	modified excitation switch
*
***************************************************************************
*	
* DESCRIPTION
*
*  Code book search is performed by closed-loop analysis using conventional
*  minimum squared prediction error (MSPE) criteria of the perceptually
*  weighted error signal.  The code book is overlaped to allow recursive
*  computational savings in routine cgain:
*
*	index	code book
*		+-------------------------+
*	1	| 2(M-1)       2(M-1)+L-1 |
*	2	| 2(M-2)       2(M-2)+L-1 |
*	:	| :	       :	  |
*	N-1	| .	       .	  |
*	N	| .	       .	  |
*	:	| 2	       61	  |
*	:	| 0	       59	  |
*		+-------------------------+
*
*	where: M = maximum code book size
*	       N = actual code book search size (any value between 1 & M)
*	       L = code word length
*
*	each code word is:  2(M-index) -> 2(M-index)+L-1
*	
***************************************************************************
*
*   global 
*
*
*	CODE BOOK VARIABLES:
*	err	real	error for each code book entry
*	gain	real	gain for each code book entry
*
***************************************************************************
* CALLED BY
*
*	csub
*
* CALLS
*
*	cgain	[gainencode]	 mexcite3
*
***************************************************************************
*
*
* REFERENCES
*
*	Tremain, Thomas E., Joseph P. Campbell, Jr and Vanoy C. Welch,
*       "A 4.8 kbps Code Excited Linear Predictive Coder," Proceedings
*	of the Mobile Satellite Conference, 3-5 May 1988, pp. 491-496.
*
*	Campbell, Joseph P. Jr., Vanoy C. Welch and Thomas E. Tremain,
*       "An Expandable Error-Protected 4800 bps CELP Coder (U.S. Federal
*       Standard 4800 bps Voice Coder)," Proceedings of ICASSP, 1989.
*	(and Proceedings of Speech Tech, 1989.)
*
*************************************************************************/

#define LEN		30	/* length of truncated impulse response */

static void cbsearch(int l, float v[])
{
	int i, codeword;
	float emax, gain[MAXNCSIZE], err[MAXNCSIZE];

	/*            *find gain and -error term for each code word           */
	/*            *and search for best code word (max -error term)        */
	/*            *(codewords are overlapped by shifts of -2              */
	/*            * along the code vector x)                              */
	/*            *NOTE: gain(i) & err(i) can be replaced by scalars      */
	codeword = 2 * MAXNCSIZE - 2;
	cbindex = 1;
	gain[0] = cgain(&x[codeword], l, TRUE, LEN, &err[0]);
	emax = *err;
	codeword -= 2;
	for (i = 1; i < ncsize; i++) {
		gain[i] = cgain(&x[codeword], l, FALSE, LEN, &err[i]);
		codeword -= 2;
		if (err[i] >= emax) {
			emax = err[i];
			cbindex = i + 1;
		}
	}

	/*   if (err(cbindex).lt.0.0)print *,' CB match<0',frame,err(cbindex) */

	/*            *pointer to best code word                              */

	codeword = 2 * (MAXNCSIZE - cbindex);

	/*            *OPTIONAL (may be useful for integer DSPs)              */
	/*            *given best code word, recompute its gain to            */
	/*            *correct any accumulated errors in recursions           */
	gain[cbindex - 1] = cgain(&x[codeword], l, TRUE, l, &err[cbindex - 1]);

	/* *constrained excitation                                            */
	if (mxsw)
		mexcite3(&gain[cbindex - 1]);

	/*            *gain quantization, UNNECESSARY for closed-loop quant   */

	if (strncmp(cbgtype, "none", 4) != 0) {
		if (cbgbits == 5) {
			gain[cbindex - 1] =
			    gainencode(gain[cbindex - 1], &gindex);
		} else {
#ifdef CELPDIAG
			fprintf(stderr, "cbsearch: not quantizing cbgain\n");
#endif
		}
	}

	/*            *scale selected code word vector -> excitation array    */
	/*            *call VDECODE?                                          */

	for (i = 0; i < l; i++)
		v[i] = gain[cbindex - 1] * x[i + codeword];
}

#undef LEN

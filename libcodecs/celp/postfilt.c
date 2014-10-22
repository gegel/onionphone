/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		postfilt & postfilt2
*
* FUNCTION
*		 
*		reduce coder noise (sample wise AGC version)
*
* SYNOPSIS
*
*		subroutine postfilt(s, l, alpha, beta, powerin
*				powerout, dp1, dp2, dp3)
*
*   formal 
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	s		float	i/o	speech input/postfiltered output
*	l		int	i	subframe size
*	alpha		float	i	filter parameter
*	beta		float	i	filter parameter
*	powerin 	float	i/o	input power estimate
*	powerout	float	i/o	output power estimate
*	dp1		float	i/o	filter memory
*	dp2		float	i/o	filter memory
*	dp3		float	i/o	filter memory
*
*   external 
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	no		int	i
*	fci[]		float	i
*
***************************************************************************
*
* DESCRIPTION
*
*	Adaptive postfilter routine to reduce perceptual coder noise.
*	The postfilter emphasis the spectral regions predicted by the
*	short-term LPC analysis.  This tends to mask coder noise by
*	concentrating it under the formant peaks.  Unfortunately, acoustic
*	background noise may also be enhanced because LPC analysis often
*	models acoustic noise instead of speech formants.  In addtion,
*	postfiltering can be detrimental to tandem coding if not taken
*	into consideration.  (To overcome these problems, we hope to
*       eventually incorporate the postfilter's enhancement properties
*	into the analysis process.)
*
*	Adaptive spectral tilt compensation is applied to flatten the 
*	overall tilt of the postfilter.  [[Slight high frequency boost is 
*	applied for output shaping.  A pitch postfilter is used to reduce 
*	pitch buzz.]]  Finally, AGC compensates for the filter gains using 
*	a time constant set by parameter tc that should be dependent on
*	frame length.
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
*	bwexp	zerofilt   polefilt   pctorc
*
***************************************************************************
*
* REFERENCES
*
*	Chen & Gersho, Real-Time Vector APC Speech Coding at 4800 bps
*       with Adaptive Postfiltering, ICASSP '87
*
*       Juin-Hwey (Raymond) Chen, "Low-Bit-Rate Predictive Coding of
*       Speech Waveforms Based on Vector Quantization," PhD Dissertation,
*	UCSB ECE Dept., March 1987.
*
*       Ramamoorthy, Jayant, Cox, & Sondhi, "Enhancement of ADPCM Speech
*	Coding with Backward-Adaptive Algorithms for Postfiltering and
*       Noise Feedback," IEEE JOSAIC, Feb. 1988, pp. 364-382.
*
**************************************************************************/
#define TC  0.01

static void postfilt(float s[], int l, float alpha, float beta,
		     float *powerin, float *powerout, float dp1[],
		     float dp2[], float dp3[])
{
	int n;
	float ast[2];
	float pcexp1[MAXNO + 1], pcexp2[MAXNO + 1], rcexp2[MAXNO];

#ifdef POSTFIL2
	float scale;
#else
	float newpowerin[MAXL + 1], newpowerout[MAXL + 1];
#endif

	/*                    *estimate input power                           */

#ifdef POSTFIL2
	for (n = 0; n < l; n++)
		*powerin = *powerin * (1.0 - TC) + TC * s[n] * s[n];
#else
	newpowerin[0] = *powerin;
	for (n = 0; n < l; n++)
		newpowerin[n + 1] =
		    (1.0 - TC) * newpowerin[n] + TC * s[n] * s[n];
	*powerin = newpowerin[l];
#endif

	/* *BW expansion                                                      */
	bwexp(beta, fci, pcexp1, no);
	bwexp(alpha, fci, pcexp2, no);

	/* *pole-zero postfilter                                              */
	zerofilt(pcexp1, no, dp1, s, l);
	polefilt(pcexp2, no, dp2, s, l);

	/* *find spectral tilt (1st order fit) of postfilter
	 *(denominator dominates the tilt)                                    */
	pctorc(pcexp2, rcexp2, no);

	/* *tilt compensation by a scaled zero
	 *(don't allow hF roll-off)                                         */
	ast[0] = 1.0;
	ast[1] = (rcexp2[0] > 0.) ? -0.5 * rcexp2[0] : 0;
	zerofilt(ast, 1, dp3, s, l);

	/* *estimate output power                                             */

#ifdef POSTFIL2
	for (n = 0; n < l; n++)
		*powerout = *powerout * (1.0 - TC) + TC * s[n] * s[n];

	/* *block wise automatic gain control                                 */

	if (*powerout > 0.0)
		for (scale = sqrt(*powerin / *powerout), n = 0; n < l; n++)
			s[n] *= scale;
#else
	newpowerout[0] = *powerout;
	for (n = 0; n < l; n++)
		newpowerout[n + 1] =
		    (1.0 - TC) * newpowerout[n] + TC * s[n] * s[n];
	*powerout = newpowerout[l];

	/* *sample wise automatic gain control                                */

	for (n = 0; n < l; n++) {
		if (newpowerout[n + 1] > 0.0)
			s[n] *= sqrt(newpowerin[n + 1] / newpowerout[n + 1]);
	}
#endif
}

#undef TC

/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* NAME
*		prefilt
*
* FUNCTION
*	
*		pitch prefilter
*
* SYNOPSIS
*		subroutine prefilt(s, l, dpp)
*
*   formal 
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	s		float	i/o	speech input/postfiltered output
*	l		int	i	subframe size
*	dpp		float	i/o	filter memory	
*
*
*   external
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	bb[]		float	i/o
*	prewt		real	i
*	idb		int	i
*
***************************************************************************
*
* DESCRIPTION
*	Note:  The pitch prefilter using a weighting factor 0.4 does not
*	alter the output speech quality (as determinted in blind listening 
*	tests) and therefore we do not use the prefilter.  However we are 
*	providing this code for research purposes.  Perhaps with other
*	enhancements or other conditions other than what we have tested,
*	the prefilter will enhance speech quality sufficiently to warrant
*	its extra computational burden.*
*
***************************************************************************
*
* REFERENCES
*       Gerson, Ira A. and Mark A. Jasuik, "Vector Sum Excited Linear
*       Prediction (VSELP) Speech Coding at 8 kbps", Proceedings of ICASSP
*       '90, p. 461.
*
***************************************************************************
*
* CALLED BY
*
*	celp
*
* CALLS
*
*	pitchvq
*
***************************************************************************/

#define TC 0.01

static void prefilt(float s[], int l, float dpp[])
{
	float scale2, powerin, powerout;
	int i;

	/* estimate input power                                       */
	powerin = 0.0;
	for (i = 0; i < l; i++)
		powerin += s[i] * s[i];
	/* powerin = (1.0 - TC) * powerin + TC * (s[i] * s[i]);     */
	bb[2] = prewt * bb[2];
	pitchvq(s, l, dpp, idb, bb, "short");

	/* estimate output power                                      */

	powerout = 0.0;
	for (i = 0; i < l; i++)
		powerout += s[i] * s[i];
	/* powerout = (1.0 - TC) * powerout + TC * (s[i] * s[i]);   */

	if (powerout > 0.0) {
		scale2 = sqrt(powerin / powerout);
		for (i = 0; i < l; i++)
			s[i] = scale2 * s[i];
	}
}

#undef TC

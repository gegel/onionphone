/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		clip
*
* FUNCTION
*		determine if speech is clipped
*
* SYnoPSIS
*		function clip(s, l)
*
*   formal 
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	s		float	i	input speech
*	l		int	i	length of input speech
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

static int clip(float s[], int l)
{
	int i, count;
	float sum;

	/*    Count number of clippings and sum their magnitudes              */

	count = 0;
	sum = 0.0;
	for (i = 0; i < l; i++) {
		if (fabs(s[i]) > 32768.0) {
			count++;
			sum += fabs(s[i]);
		}
	}

	/*    Clipping heuristics (could also use energy, delta energy, etc.) */

	return (((count >= 10) || (count >= 5 && sum > 1.e6)) ? TRUE : FALSE);
}

/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		vdecode
*
* FUNCTION
*
*		create excitation vector from code book index and decoded gain
*
* SYNOPSIS
*		subroutine vdecode(decodedgain, l, vdecoded)
*
*   formal
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	decodedgain	r	i	decoded gain value
*	l		i	i	pitch&code frame length
*	vdecoded	r	o	decoded excitation array
*
*   external
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	x[]		float	i
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

static void vdecode(float decodedgain, int l, float vdecoded[])
{
	int i, codeword;

	/* *copy selected vector to excitation array                           */

	codeword = 2 * (MAXNCSIZE - cbindex);
	if (codeword < 0) {
#ifdef CELPDIAG
		fprintf(stderr, "vdecode: cbindex > MAXNCSIZE at frame %d\n",
			frame);
#endif
		codeword = 0;
	}
	for (i = 0; i < l; i++)
		vdecoded[i] = x[i + codeword] * decodedgain;
}

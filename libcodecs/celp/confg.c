/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		confg
*
* FUNCTION
*		computes initial states (direct form filters)
*
* SYNOPSIS
*
*	subroutine confg(s,l,d1,d2,d3,d4,isw1,isw2,isw3,isw4)
*
*   formal 
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	s(l)		float	i	speech or residual segment
*	l		int	i	segment size
*	d1		float	i/o	memory 1/P(z)
*	d2		float	i/o	memory 1/A(z)
*	d3		float	i/o	memory A(z)
*	d4		float	i/o	memory 1/A(z/gamma)
*	isw1		int	i	= 1 enable 1/P(z)
*	isw2		int	i	= 1 enable 1/A(z)
*	isw3		int	i	= 1 enable A(z)
*	isw4		int	i	= 1 enable 1/A(z/gamma)
*
*   external 
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	idb			i
*	no			i
*	bb[]			i
*	e0[]			i/o
*	fc[]			i
*	gamma2		i
*
***************************************************************************
*
* CALLED BY
*
*	csub	fndpp
*
* CALLS
*
*	bwexp	zerofilt	polefilt	pitchvq
*
**************************************************************************/

static void confg(float s[], int l, float d1[], float d2[],
		  float d3[], float d4[], int isw1, int isw2, int isw3,
		  int isw4)
{
	float fctemp[MAXNO + 1];
	int i;

	setr(MAXNO + 1, 0.0, fctemp);

	if (isw1 != 0)
		pitchvq(e0, l, d1, idb, bb, "long");
	if (isw2 != 0)
		polefilt(fc, no, d2, e0, l);

	for (i = 0; i < l; i++)
		e0[i] = s[i] - e0[i];

	if (isw3 != 0)
		zerofilt(fc, no, d3, e0, l);
	if (isw4 != 0) {
		bwexp(gamma2, fc, fctemp, no);
		polefilt(fctemp, no, d4, e0, l);
	}
}

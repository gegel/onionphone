/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		pitchvq
*
* FUNCTION
*		Pitch VQ (n-taps, where n = 0, ..., 3)
*               "self-excited" or "vq" or "adaptive code book"
*
* SYNOPSIS
*		subroutine pitchvq(rar, idim, buf, idimb, b, type)
*
*   formal
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	rar		float	i/o	Data segement/Filtered data segment
*	idim		int	i	Dimension of data segement RAR
*	buf		float	i/o	Data buffer, dim IDIMB
*	idimb		int	i	Dimension of data buffer is
*						max(M) + IDIM+1
*	b[0]		float	i	Pitch delay (M)
*	b[1-3]		float	i	pitch predictor coefficients.
*					BETA1=B(2),BETA2=B(3), BETA3=B(4)
*					For a 1-tap predictor B(2)=B(4)=0.0
*       type            char    i       Type "long" calls ldelay.c, the 
*					delay routine using long interpola-
*                                       tion windows.  Type "short" calls
*					delay.c, the short delay routine.
*
***************************************************************************
*
* CALLED BY
*
*	celp   confg
*
* CALLS
*
*	delay
*
*
***************************************************************************
*
* USAGE
*
*    Adaptive code book (pitch) synthesis routine:
*
*    1) For lags < frame size:	gain-shape adaptive code book VQ
*
*    2) For lags => frame size this is equivalent to a pitch synthesis "filter"
*
*			       -[b(1)-1]	-b(1)	     -[b(1)+1]
*	H(z) =	1 /  1 + b(2) z 	+ b(3) z     + b(4) z
*
*	NOTE: largest delay must not exceed the value IDIMB-IDIM
*
*
***************************************************************************
*
* REFERENCES
*
*	Singhal & Atal, Improving Performance of Multi-Pulse LPC Coders at
*	Low Bit Rates, ICASSP, 1984.
*
*	Rose & Barnwell.  The Self Excited Vocoder-An Alternate Approcch 
*	to Toll Quality at 4800 bps, ICASSP, 1986, pp. 453-456.
*
*	Kleijn, Krasinski and Ketchum, Improved Speech Quality and 
*	Efficient Vector Quantization in SELP, ICASSP, 1988.
*
**************************************************************************/

static void pitchvq(float rar[], int idim, float buf[],
		    int idimb, float b[], char type[])
{
	int k, m, i, start;
	float buf2[MAXLP], frac;

	k = idimb - idim;
	start = k + 1;
	m = (int)b[0];
	frac = b[0] - m;

	/* *update memory                                                      */

	for (i = 0; i < k; i++)
		buf[i] = buf[i + idim];

	/* *update memory with selected pitch memory from selected delay (m)    */

	if (fabs(frac) < 1.e-4) {
		for (i = k; i < idimb; i++)
			buf[i] = buf[i - m];
	}

	/* *fractional update if fractional part isn't "zero"                  */

	if (fabs(frac) > 1.e-4) {
		if (strcmp(type, "long") == 0)
			ldelay(buf, start, idim, frac, m, buf2);
		else
			delay(buf, start, idim, frac, m, buf2);
		for (i = 0; i < idim; i++)
			buf[i + k] = buf2[i];
	}

	/* *return "rar" with scaled memory added to stochastic contribution   */

	for (i = 0; i < idim; i++)
		buf[i + k] = rar[i] += b[2] * buf[i + k];

}

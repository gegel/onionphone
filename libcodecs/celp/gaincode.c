/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		gainencode
*
* FUNCTION
*
*		encode and quantize code book gain
*
* SYNOPSIS
*		subroutine gainencode(input, index)
*
*   formal
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	input		int	i	code book gain input (true value)
*	index		float	o	encoded code book gain ZERO BASED index
*	gainencode	float	func	encoded code book gain
*
***************************************************************************
*
* DESCRIPTION
*
*
*      Fast code book gain quantizer to allow practical quantization
*      inside the code book search loop. A binary tree search quantization 
*      is implemented below.
*
*
***************************************************************************
*
* CALLED BY
*
*	cbsearch cgain
*
* CALLS
*
*
***************************************************************************
*
* REFERENCES
*
*
**************************************************************************/
 /* *Log quantization                               */

static const float gainlog5[32] = {
	-1330., -870., -660., -520., -418., -340., -278., -224.,
	-178., -136., -98., -64., -35., -13., -3., -1.,
	1., 3., 13., 35., 64., 98., 136., 178.,
	224., 278., 340., 418., 520., 660., 870., 1330.
};

static float gainencode(float input, int *index)
{
	int i;
	static const float midpoints[31] = {
		-1100., -765., -590., -469., -379., -309., -251., -201.,
		-157., -117., -81., -49.5, -24., -8., -2., 0.,
		2., 8., 24., 49.5, 81., 117., 157., 201.,
		251., 309., 379., 469., 590., 765., 1100.
	};

	/* *Binary tree search for closest gain                                */

	for (*index = 15, i = 8; i >= 1; i = i >> 1) {
		if (input > midpoints[*index])
			*index += i;
		else
			*index -= i;
	}
	if (input > midpoints[*index])
		(*index)++;

	/* *Return quantized gain and ZERO based index                         */

	return (gainlog5[*index]);
}

/**************************************************************************
*
* ROUTINE
*		gaindecode
*
* FUNCTION
*
*		decode code book gain from the gain index (gindex)
*		and bit index (bits).
*
* SYNOPSIS
*		subroutine gaindecode(gindex, bits, gain)
*
*   formal
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	gindex		int	i	gain index value
*	bits		int	i	# bits for encode
*	gain		float	o	decoded code book gain value
*
***************************************************************************
*
* CALLED BY
*
*	dcodcbg 
*
* CALLS
*
*
***************************************************************************
*
* REFERENCES
*
*	Quantizing for Minimum Distorion
*	J. Max
*	IRE Trans. Inform. Theory, vol. IT-6, pp.7-12, Mar. 1960
*
***************************************************************************
*
*	The data used in the table generation is from 3m3f.spd.
*
**************************************************************************/

void gaindecode(int gindex, int bits, float *gain)
{
	/* Choose appropriate gain                                         */

	if (bits == 5)
		*gain = gainlog5[gindex];
	else {
#ifdef CELPDIAG
		fprintf(stderr, "gaindecode: unquantized cbgain\n");
#endif
		CELP_ERROR(CELP_ERR_GAINDECODE);
	}
}

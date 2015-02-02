/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		dcodcbg.c
*
* FUNCTION
*		 Decodes all code book gains for a frame
*
*
* SYNOPSIS
*		subroutine dcodcbg(cbgbits,bitsum1,bitsum2,bitpointer,
*				   nn,stream,cbg)
*
*   formal
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	cbgbits 	int	i	number of bits for quantization
*	bitsum1 	int	i	number of bits for odd subframes
*	bitsum2 	int	i	number of bits for even subframes
*	bitpointer	int	i/o	number of bits used
*	nn		int	i	number of subframes/frame
*	stream		short	i	bit stream
*	cbg		float	o	vector of code book indicies
*
***************************************************************************
*
* DESCRIPTION
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
*	gaindecode	unpack
*
***************************************************************************
*
* REFERENCES
*
*
**************************************************************************/

static void dcodcbg(int cbgbits, int bitsum1, int bitsum2,
		    int *bitpointer, int nn, short stream[], float cbg[])
{
	int i, pointer, index;

	pointer = *bitpointer;
	for (i = 0; i < nn; i++) {
		unpack(stream, cbgbits, &index, &pointer);
		gaindecode(index, cbgbits, &cbg[i]);
		if (i == 0 || i == 2 || i == 4)
			pointer += bitsum2 - cbgbits;
		else if (i == 1 || i == 3 || i == 5)
			pointer += bitsum1 - cbgbits;
		else {
#ifdef CELPDIAG
			fprintf(stderr,
				"dcodcbg: ***Error in decoding cbgain\n");
#endif
			CELP_ERROR(CELP_ERR_DCODCBG);
			return;
		}
	}
	*bitpointer += cbgbits;
}

/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		dcodcbi
*
* FUNCTION
*		 Decodes all code book indicies for a frame
*
*
* SYNOPSIS
*		subroutine dcodcbi(cbbits,bitsum1,bitsum2,nn,cbi)
*
*   formal
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	cbbits		int	i	number of bits for quantization
*	bitsum1 	int	i	number of bits for odd subframes
*	bitsum2 	int	i	number of bits for even subframes
*	bitpointer	int	i/o	number of bits used
*	nn		int	i	number of subframes/frame
*	stream		short	i	bit stream
*	cbi		int	o	vector of code book indicies
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
*	unpack
*
***************************************************************************
*
* REFERENCES
*
*
**************************************************************************/

static void dcodcbi(int cbbits, int bitsum1, int bitsum2,
		    int *bitpointer, int nn, short stream[], int cbi[])
{
	int i, pointer;

	pointer = *bitpointer;
	for (i = 0; i < nn; i++) {
		unpack(stream, cbbits, &cbi[i], &pointer);
		cbi[i]++;
		if (i == 0 || i == 2 || i == 4)
			pointer += bitsum2 - cbbits;
		else if (i == 1 || i == 3 || i == 5)
			pointer += bitsum1 - cbbits;
		else {
#ifdef CELPDIAG
			fprintf(stderr,
				"dcodcbi: ***Error in decoding code book index\n");
#endif
			CELP_ERROR(CELP_ERR_DCODCBI);
			return;
		}
	}
	*bitpointer += cbbits;
}

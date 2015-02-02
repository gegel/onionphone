/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		dcodpg.c
*
* FUNCTION
*		 Decodes all pitch gain values for a frame
*
*
* SYNOPSIS
*		subroutine dcodpg(pgbits,bitsum1,bitsum2,bitpointer,
*				  nn,stream,pgs)
*
*   formal
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	pgbits		int	i	number of bits for quantization
*	bitsum1 	int	i	number of bits for odd subframes
*	bitsum2 	int	i	number of bits for even subframes
*	bitpointer	int	i	number of bits used
*	nn		int	i	number of subframes/frame
*	stream		short	i	bit stream
*	pgs		float	o	vector of pitch gains
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
*	pitchdecode	 unpack
*
***************************************************************************
*
* REFERENCES
*
*
**************************************************************************/

static void dcodpg(int pgbits, int bitsum1, int bitsum2,
		   int *bitpointer, int nn, short stream[], float pgs[])
{
	int i, pointer, index;

	pointer = *bitpointer;
	for (i = 0; i < nn; i++) {
		unpack(stream, pgbits, &index, &pointer);
		pitchdecode(index, &pgs[i]);
		if (i == 0 || i == 2 || i == 4)
			pointer += bitsum2 - pgbits;
		else if (i == 1 || i == 3 || i == 5)
			pointer += bitsum1 - pgbits;
		else {
#ifdef CELPDIAG
			fprintf(stderr,
				"dcodpg:  Error in decoding pitch gain\n");
			CELP_ERROR(CELP_ERR_DCODPG);
			return;
#endif
		}
	}
	*bitpointer += pgbits;
}

/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		packtau (pack tau into binaru bit stream)
*
* FUNCTION
*		Input decimal value and number of binary bits, 
*		program returns binary value packed in array.
*
* SYNOPSIS
*		subroutine packtau(value, bits, pdencode, array, pointer)
*
*   formal 
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	value		int	I	decimal value
*	bits		int	I	number of bits to convert to
*					(ie. 01001 for 5 bits, 1st 0 incl.)
*	pdencode	int	I	pitch delay indexing table
*	array		short	O	array to which one bit is assigned
*					for binary representation
*	pointer 	int *	I/O	points to appropriate element in
*					array
*
***************************************************************************
*
* DESCRIPTION
*
*	This program packs a decimal value into a binary value
*	to be decoded by unpacktau.c in CELP synthesizer sections.
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

static void packtau(int value, int bits, const int pdencode[], short array[],
		    int *pointer)
{
	int i;

	/* *change index to permuted index                                     */

	value = pdencode[value];

	/* insert in bitstream                                                 */

	for (i = 0; i < bits; (*pointer)++, i++)
		array[*pointer] = (value & 1 << i) >> i;
}

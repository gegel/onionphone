/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		pack (pack tau into binaru bit stream)
*
* FUNCTION
*		Input decimal value and number of binary bits, 
*		program returns binary value packed in array.
*
* SYNOPSIS
*		subroutine pack(value, bits, array, pointer)
*
*   formal 
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	value		int	i	decimal value
*	bits		int	i	number of bits to convert to
*					(ie. 01001 for 5 bits, 1st 0 incl.)
*	array		short	o	array to which one bit is assigned
*					for binary representation
*	pointer 	int *	i/o	points to appropriate element in
*					array
*
***************************************************************************
*
* DESCRIPTION
*
*	This program packs a decimal value into a binary value
*	to be decoded by unpack.c in CELP synthesizer sections.
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

static void pack(int value, int bits, short array[], int *pointer)
{
	int i;

	for (i = 0; i < bits; (*pointer)++, i++)
		array[*pointer] = (value & 1 << i) >> i;
}

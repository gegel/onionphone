/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* ROUTINE
*		unpack (unpack from binary to decimal)
*
* FUNCTION
*		Input binary array and number of binary bits,
*		program returns unpacked decimal value.
*
* SYNOPSIS
*		subroutine unpack(array, bits, value, pointer)
*
*   formal
*
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	array		short	i	array to which one bit is assigned
*					for binary representation
*	bits		int	i	number of bits to convert
*					(ie. 01001 for 5 bits, 1st 0 incl.)
*	value		int	o	decimal value to convert to
*	pointer 	int	i/o	points to appropriate element in
*					array
*
***************************************************************************
*
* DESCRIPTION
*
*	This program unpacks binary values packed by pack.c
*	into decimal values.
*
***************************************************************************
*
* CALLED BY
*
*	celp  dcodcbg  dcodcbi	dcodpg	dcodtau
*
* CALLS
*
*
*
***************************************************************************
*
* REFERENCES
*
*
**************************************************************************/

static void unpack(const short array[], int bits, int *value, int *pointer)
{
	int i;

	for (i = 0, *value = 0; i < bits; i++, (*pointer)++)
		*value |= array[*pointer + 1] << i;
}

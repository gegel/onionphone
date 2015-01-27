/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *
 *  FUNCTION:  w_Bits2w_prm_12k2
 *
 *  PURPOSE: Retrieves the vector of encoder parameters from the received
 *           w_serial bits in a frame.
 *
 *  DESCRIPTION: The encoder parameters are:
 *
 *     BFI      bad frame indicator      1 bit
 *
 *     LPC:
 *              1st codebook             7 bit
 *              2nd codebook             8 bit
 *              3rd codebook             8+1 bit
 *              4th codebook             8 bit
 *              5th codebook             6 bit
 *
 *     1st and 3rd w_subframes:
 *           pitch period                9 bit
 *           pitch gain                  4 bit
 *           codebook index              35 bit
 *           codebook gain               5 bit
 *
 *     2nd and 4th w_subframes:
 *           pitch period                6 bit
 *           pitch gain                  4 bit
 *           codebook index              35 bit
 *           codebook gain               5 bit
 *
 *************************************************************************/

#include <stdint.h>
#include "basic_op.h"

/* Local function */

int16_t w_Bin2int(int16_t no_of_bits,	/* input : number of bits associated with value */
			int16_t * bitstream	/* output: w_address where bits are written       */
    );

#define BIT_0     0
#define BIT_1     1
#define PRM_NO    57

void w_Bits2w_prm_12k2(int16_t bits[],	/* input : w_serial bits (244 + bfi)                */
		       int16_t w_prm[]	/* output: analysis parameters  (57+1 parameters) */
    )
{
	int16_t i;

	static const int16_t bitno[PRM_NO] = {
		7, 8, 9, 8, 6,	/* LSP VQ          */
		9, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 5,	/* first w_subframe  */
		6, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 5,	/* second w_subframe */
		9, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 5,	/* third w_subframe  */
		6, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 5
	};			/* fourth w_subframe */

	*w_prm++ = *bits++;	/* read BFI */

	for (i = 0; i < PRM_NO; i++) {
		w_prm[i] = w_Bin2int(bitno[i], bits);
		bits += bitno[i];
	}
	return;
}

/*************************************************************************
 *
 *  FUNCTION:  w_Bin2int                   
 *
 *  PURPOSE: Read "no_of_bits" bits from the array bitstream[] and convert
 *           to integer.
 *
 *************************************************************************/

int16_t w_Bin2int(int16_t no_of_bits,	/* input : number of bits associated with value */
			int16_t * bitstream	/* output: w_address where bits are written       */
    )
{
	int16_t value, i, bit;

	value = 0;
	for (i = 0; i < no_of_bits; i++) {
		value = w_shl(value, 1);
		bit = *bitstream++;

		if (w_sub(bit, BIT_1) == 0)
			value = w_add(value, 1);
	}
	return (value);
}

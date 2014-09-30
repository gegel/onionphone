/*************************************************************************
 *
 *  FUNCTION:  w_Prm2bits_12k2
 *
 *  PURPOSE:  converts the encoder parameter vector into a vector of w_serial
 *                      bits.
 *
 *  DESCRIPTION: The encoder parameters are:
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

#include "typedef.h"
#include "basic_op.h"
#include "count.h"

/* Local function */

void w_Int2bin (
    Word16 value,       /* input : value to be converted to binary      */
    Word16 no_of_bits,  /* input : number of bits associated with value */
    Word16 *bitstream   /* output: w_address where bits are written       */
);

#define BIT_0     0
#define BIT_1     1
#define MASK      0x0001
#define PRM_NO    57

void w_Prm2bits_12k2 (
    Word16 w_prm[],       /* input : analysis parameters  (57 parameters)   */
    Word16 bits[]       /* output: 244 w_serial bits                        */
)
{
    Word16 i;

    static const Word16 bitno[PRM_NO] =
    {
        7, 8, 9, 8, 6,                          /* LSP VQ          */
        9, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 5,  /* first w_subframe  */
        6, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 5,  /* second w_subframe */
        9, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 5,  /* third w_subframe  */
        6, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 5   /* fourth w_subframe */
    };
    for (i = 0; i < PRM_NO; i++)
    {
        w_Int2bin (w_prm[i], bitno[i], bits);
        bits += bitno[i];
    }

    return;
}

/*************************************************************************
 *
 *  FUNCTION:  w_Int2bin
 *
 *  PURPOSE:  convert integer to binary and write the bits to the array
 *            bitstream[]. The most significant bits are written first.
 *
 *************************************************************************/

void w_Int2bin (
    Word16 value,       /* input : value to be converted to binary      */
    Word16 no_of_bits,  /* input : number of bits associated with value */
    Word16 *bitstream   /* output: w_address where bits are written       */
)
{
    Word16 *pt_bitstream, i, bit;

    pt_bitstream = &bitstream[no_of_bits];        

    for (i = 0; i < no_of_bits; i++)
    {
        bit = value & MASK;                       
          
        if (bit == 0)
        {
            *--pt_bitstream = BIT_0;              
        }
        else
        {
            *--pt_bitstream = BIT_1;              
        }
        value = w_shr (value, 1);
    }
}

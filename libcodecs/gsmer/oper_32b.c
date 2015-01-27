/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*****************************************************************************
 *                                                                           *
 *  This file contains operations in double precision.                       *
 *  These operations are not standard double precision operations.           *
 *  They are used where single precision is not enough but the full 32 bits  *
 *  precision is not necessary. For example, the function w_Div_32() has a     *
 *  24 bits precision which is enough for our purposes.                      *
 *                                                                           *
 *  The double precision numbers use a special representation:               *
 *                                                                           *
 *     L_32 = hi<<16 + lo<<1                                                 *
 *                                                                           *
 *  L_32 is a 32 bit integer.                                                *
 *  hi and lo are 16 bit signed integers.                                    *
 *  As the low part also contains the sign, this allows fast w_multiplication. *
 *                                                                           *
 *      0x8000 0000 <= L_32 <= 0x7fff fffe.                                  *
 *                                                                           *
 *  We will use DPF (Double Precision Format )in this file to specify        *
 *  this special format.                                                     *
 *****************************************************************************
*/

#include <stdint.h>
#include "basic_op.h"
#include "oper_32b.h"


/*****************************************************************************
 *                                                                           *
 *  Function w_L_Extract()                                                     *
 *                                                                           *
 *  Extract from a 32 bit integer two 16 bit DPF.                            *
 *                                                                           *
 *  Arguments:                                                               *
 *                                                                           *
 *   L_32      : 32 bit integer.                                             *
 *               0x8000 0000 <= L_32 <= 0x7fff ffff.                         *
 *   hi        : b16 to b31 of L_32                                          *
 *   lo        : (L_32 - hi<<16)>>1                                          *
 *****************************************************************************
*/

void w_L_Extract(int32_t L_32, int16_t * hi, int16_t * lo)
{
	*hi = w_extract_h(L_32);
	*lo = w_extract_l(w_L_msu(w_L_w_shr(L_32, 1), *hi, 16384));
	return;
}

/*****************************************************************************
 *                                                                           *
 *  Function w_L_Comp()                                                        *
 *                                                                           *
 *  Compose from two 16 bit DPF a 32 bit integer.                            *
 *                                                                           *
 *     L_32 = hi<<16 + lo<<1                                                 *
 *                                                                           *
 *  Arguments:                                                               *
 *                                                                           *
 *   hi        msb                                                           *
 *   lo        lsf (with sign)                                               *
 *                                                                           *
 *   Return Value :                                                          *
 *                                                                           *
 *             32 bit long signed integer (int32_t) whose value falls in the  *
 *             range : 0x8000 0000 <= L_32 <= 0x7fff fff0.                   *
 *                                                                           *
 *****************************************************************************
*/

int32_t w_L_Comp(int16_t hi, int16_t lo)
{
	int32_t L_32;

	L_32 = w_L_deposit_h(hi);
	return (w_L_mac(L_32, lo, 1));	/* = hi<<16 + lo<<1 */
}

/*****************************************************************************
 * Function w_Mpy_32()                                                         *
 *                                                                           *
 *   Multiply two 32 bit integers (DPF). The result is divided by 2**31      *
 *                                                                           *
 *   L_32 = (hi1*hi2)<<1 + ( (hi1*lo2)>>15 + (lo1*hi2)>>15 )<<1              *
 *                                                                           *
 *   This operation can also be viewed as the w_multiplication of two Q31      *
 *   number and the result is also in Q31.                                   *
 *                                                                           *
 * Arguments:                                                                *
 *                                                                           *
 *  hi1         hi part of first number                                      *
 *  lo1         lo part of first number                                      *
 *  hi2         hi part of second number                                     *
 *  lo2         lo part of second number                                     *
 *                                                                           *
 *****************************************************************************
*/

int32_t w_Mpy_32(int16_t hi1, int16_t lo1, int16_t hi2, int16_t lo2)
{
	int32_t L_32;

	L_32 = w_L_w_mult(hi1, hi2);
	L_32 = w_L_mac(L_32, w_mult(hi1, lo2), 1);
	L_32 = w_L_mac(L_32, w_mult(lo1, hi2), 1);

	return (L_32);
}

/*****************************************************************************
 * Function w_w_Mpy_32_16()                                                      *
 *                                                                           *
 *   Multiply a 16 bit integer by a 32 bit (DPF). The result is divided      *
 *   by 2**15                                                                *
 *                                                                           *
 *                                                                           *
 *   L_32 = (hi1*lo2)<<1 + ((lo1*lo2)>>15)<<1                                *
 *                                                                           *
 * Arguments:                                                                *
 *                                                                           *
 *  hi          hi part of 32 bit number.                                    *
 *  lo          lo part of 32 bit number.                                    *
 *  n           16 bit number.                                               *
 *                                                                           *
 *****************************************************************************
*/

int32_t w_w_Mpy_32_16(int16_t hi, int16_t lo, int16_t n)
{
	int32_t L_32;

	L_32 = w_L_w_mult(hi, n);
	L_32 = w_L_mac(L_32, w_mult(lo, n), 1);

	return (L_32);
}

/*****************************************************************************
 *                                                                           *
 *   Function Name : w_Div_32                                                  *
 *                                                                           *
 *   Purpose :                                                               *
 *             Fractional integer division of two 32 bit numbers.            *
 *             L_num / L_denom.                                              *
 *             L_num and L_denom must be positive and L_num < L_denom.       *
 *             L_denom = denom_hi<<16 + denom_lo<<1                          *
 *             denom_hi is a normalize number.                               *
 *                                                                           *
 *   Inputs :                                                                *
 *                                                                           *
 *    L_num                                                                  *
 *             32 bit long signed integer (int32_t) whose value falls in the  *
 *             range : 0x0000 0000 < L_num < L_denom                         *
 *                                                                           *
 *    L_denom = denom_hi<<16 + denom_lo<<1      (DPF)                        *
 *                                                                           *
 *       denom_hi                                                            *
 *             16 bit positive normalized integer whose value falls in the   *
 *             range : 0x4000 < hi < 0x7fff                                  *
 *       denom_lo                                                            *
 *             16 bit positive integer whose value falls in the              *
 *             range : 0 < lo < 0x7fff                                       *
 *                                                                           *
 *   Return Value :                                                          *
 *                                                                           *
 *    L_div                                                                  *
 *             32 bit long signed integer (int32_t) whose value falls in the  *
 *             range : 0x0000 0000 <= L_div <= 0x7fff ffff.                  *
 *                                                                           *
 *  Algorithm:                                                               *
 *                                                                           *
 *  - find = 1/L_denom.                                                      *
 *      First approximation: approx = 1 / denom_hi                           *
 *      1/L_denom = approx * (2.0 - L_denom * approx )                       *
 *                                                                           *
 *  -  result = L_num * (1/L_denom)                                          *
 *****************************************************************************
*/

int32_t w_Div_32(int32_t L_num, int16_t denom_hi, int16_t denom_lo)
{
	int16_t approx, hi, lo, n_hi, n_lo;
	int32_t L_32;

	/* First approximation: 1 / L_denom = 1/denom_hi */

	approx = w_div_s((int16_t) 0x3fff, denom_hi);

	/* 1/L_denom = approx * (2.0 - L_denom * approx) */

	L_32 = w_w_Mpy_32_16(denom_hi, denom_lo, approx);

	L_32 = w_L_w_sub((int32_t) 0x7fffffffL, L_32);

	w_L_Extract(L_32, &hi, &lo);

	L_32 = w_w_Mpy_32_16(hi, lo, approx);

	/* L_num * (1/L_denom) */

	w_L_Extract(L_32, &hi, &lo);
	w_L_Extract(L_num, &n_hi, &n_lo);
	L_32 = w_Mpy_32(n_hi, n_lo, hi, lo);
	L_32 = w_L_w_shl(L_32, 2);

	return (L_32);
}

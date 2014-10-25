/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *
 *  FUNCTION:   w_Inv_sqrt
 *
 *  PURPOSE:   Computes 1/sqrt(L_x),  where  L_x is positive.
 *             If L_x is negative or w_zero, the result is 1 (3fff ffff).
 *
 *  DESCRIPTION:
 *       The function 1/sqrt(L_x) is approximated by a w_table and linear
 *       interpolation. The inverse square root is computed using the
 *       following steps:
 *          1- Normalization of L_x.
 *          2- If (30-exponent) is even then shift right once.
 *          3- exponent = (30-exponent)/2  +1
 *          4- i = bit25-b31 of L_x;  16<=i<=63  because of normalization.
 *          5- a = bit10-b24
 *          6- i -=16
 *          7- L_y = w_table[i]<<16 - (w_table[i] - w_table[i+1]) * a * 2
 *          8- L_y >>= exponent
 *
 *************************************************************************/

#include <stdint.h>
#include "basic_op.h"
#include "count.h"

#include "inv_sqrt.tab"		/* Table for inv_sqrt() */

int32_t w_Inv_sqrt(int32_t L_x	/* (i) : input value    */
    )
{
	int16_t exp, i, a, tmp;
	int32_t L_y;

	if (L_x <= (int32_t) 0)
		return ((int32_t) 0x3fffffffL);

	exp = w_norm_l(L_x);
	L_x = w_L_w_shl(L_x, exp);	/* L_x is normalize */

	exp = w_sub(30, exp);

	if ((exp & 1) == 0) {	/* If exponent even -> shift right */
		L_x = w_L_w_shr(L_x, 1);
	}
	exp = w_shr(exp, 1);
	exp = w_add(exp, 1);

	L_x = w_L_w_shr(L_x, 9);
	i = w_extract_h(L_x);	/* Extract b25-b31 */
	L_x = w_L_w_shr(L_x, 1);
	a = w_extract_l(L_x);	/* Extract b10-b24 */
	a = a & (int16_t) 0x7fff;

	i = w_sub(i, 16);

	L_y = w_L_deposit_h(w_table[i]);	/* w_table[i] << 16          */
	tmp = w_sub(w_table[i], w_table[i + 1]);	/* w_table[i] - w_table[i+1])  */
	L_y = w_L_msu(L_y, tmp, a);	/* L_y -=  tmp*a*2         */

	L_y = w_L_w_shr(L_y, exp);	/* denormalization */

	return (L_y);
}

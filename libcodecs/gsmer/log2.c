/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *
 *   FUNCTION:   w_Log2()
 *
 *   PURPOSE:   Computes log2(L_x),  where   L_x is positive.
 *              If L_x is negative or w_zero, the result is 0.
 *
 *   DESCRIPTION:
 *        The function w_Log2(L_x) is approximated by a w_table and linear
 *        interpolation. The following steps are used to compute w_Log2(L_x)
 *
 *           1- Normalization of L_x.
 *           2- exponent = 30-exponent
 *           3- i = bit25-b31 of L_x;  32<=i<=63  (because of normalization).
 *           4- a = bit10-b24
 *           5- i -=32
 *           6- fraction = w_table[i]<<16 - (w_table[i] - w_table[i+1]) * a * 2
 *
 *************************************************************************/

#include <stdint.h>
#include "basic_op.h"


#include "log2.tab"		/* Table for w_Log2() */

void w_Log2(int32_t L_x,		/* (i) : input value                                 */
	    int16_t * exponent,	/* (o) : Integer part of w_Log2.   (range: 0<=val<=30) */
	    int16_t * fraction	/* (o) : Fractional part of w_Log2. (range: 0<=val<1) */
    )
{
	int16_t exp, i, a, tmp;
	int32_t L_y;

	if (L_x <= (int32_t) 0) {
		*exponent = 0;
		*fraction = 0;
		return;
	}
	exp = w_norm_l(L_x);
	L_x = w_L_w_shl(L_x, exp);	/* L_x is normalized */

	*exponent = w_sub(30, exp);

	L_x = w_L_w_shr(L_x, 9);
	i = w_extract_h(L_x);	/* Extract b25-b31 */
	L_x = w_L_w_shr(L_x, 1);
	a = w_extract_l(L_x);	/* Extract b10-b24 of fraction */
	a = a & (int16_t) 0x7fff;

	i = w_sub(i, 32);

	L_y = w_L_deposit_h(w_table[i]);	/* w_table[i] << 16        */
	tmp = w_sub(w_table[i], w_table[i + 1]);	/* w_table[i] - w_table[i+1] */
	L_y = w_L_msu(L_y, tmp, a);	/* L_y -= tmp*a*2        */

	*fraction = w_extract_h(L_y);

	return;
}

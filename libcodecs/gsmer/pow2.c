/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *
 *   FUNCTION:  w_Pow2()
 *
 *   PURPOSE: computes  L_x = pow(2.0, exponent.fraction)
 *
 *   DESCRIPTION:
 *       The function w_Pow2(L_x) is approximated by a w_table and linear
 *       interpolation.
 *          1- i = bit10-b15 of fraction,   0 <= i <= 31
 *          2- a = bit0-b9   of fraction   
 *          3- L_x = w_table[i]<<16 - (w_table[i] - w_table[i+1]) * a * 2
 *          4- L_x = L_x >> (30-exponent)     (with w_rounding)
 *
 *************************************************************************/

#include <stdint.h>
#include "basic_op.h"
#include "count.h"

#include "pow2.tab"		/* Table for w_Pow2() */

int32_t w_Pow2(int16_t exponent,	/* (i)  : Integer part.      (range: 0<=val<=30)   */
		     int16_t fraction	/* (i)  : Fractional part.  (range: 0.0<=val<1.0) */
    )
{
	int16_t exp, i, a, tmp;
	int32_t L_x;

	L_x = w_L_w_mult(fraction, 32);	/* L_x = fraction<<6           */
	i = w_extract_h(L_x);	/* Extract b10-b16 of fraction */
	L_x = w_L_w_shr(L_x, 1);
	a = w_extract_l(L_x);	/* Extract b0-b9   of fraction */
	a = a & (int16_t) 0x7fff;

	L_x = w_L_deposit_h(w_table[i]);	/* w_table[i] << 16        */
	tmp = w_sub(w_table[i], w_table[i + 1]);	/* w_table[i] - w_table[i+1] */
	L_x = w_L_msu(L_x, tmp, a);	/* L_x -= tmp*a*2        */

	exp = w_sub(30, exponent);
	L_x = w_w_L_w_w_shr_r(L_x, exp);

	return (L_x);
}

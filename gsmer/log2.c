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

#include "typedef.h"
#include "basic_op.h"
#include "count.h"

#include "log2.tab"     /* Table for w_Log2() */

void w_Log2 (
    Word32 L_x,         /* (i) : input value                                 */
    Word16 *exponent,   /* (o) : Integer part of w_Log2.   (range: 0<=val<=30) */
    Word16 *fraction    /* (o) : Fractional part of w_Log2. (range: 0<=val<1) */
)
{
    Word16 exp, i, a, tmp;
    Word32 L_y;

      
    if (L_x <= (Word32) 0)
    {
        *exponent = 0;            
        *fraction = 0;            
        return;
    }
    exp = w_norm_l (L_x);
    L_x = w_L_w_shl (L_x, exp);     /* L_x is normalized */

    *exponent = w_sub (30, exp);    

    L_x = w_L_w_shr (L_x, 9);
    i = w_extract_h (L_x);        /* Extract b25-b31 */
    L_x = w_L_w_shr (L_x, 1);
    a = w_extract_l (L_x);        /* Extract b10-b24 of fraction */
    a = a & (Word16) 0x7fff;      

    i = w_sub (i, 32);

    L_y = w_L_deposit_h (w_table[i]);       /* w_table[i] << 16        */
    tmp = w_sub (w_table[i], w_table[i + 1]); /* w_table[i] - w_table[i+1] */
    L_y = w_L_msu (L_y, tmp, a);  /* L_y -= tmp*a*2        */

    *fraction = w_extract_h (L_y);  

    return;
}

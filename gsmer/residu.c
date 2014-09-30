/*************************************************************************
 *
 *  FUNCTION:  w_Residu
 *
 *  PURPOSE:  Computes the LP residual.
 *
 *  DESCRIPTION:
 *     The LP residual is computed by filtering the input w_speech through
 *     the LP inverse filter A(z).
 *
 *************************************************************************/

#include "typedef.h"
#include "basic_op.h"
#include "count.h"

/* m = LPC order == 10 */
#define m 10

void w_Residu (
    Word16 a[], /* (i)     : w_prediction coefficients                      */
    Word16 x[], /* (i)     : w_speech signal                                */
    Word16 y[], /* (o)     : residual signal                              */
    Word16 lg   /* (i)     : size of filtering                            */
)
{
    Word16 i, j;
    Word32 s;

    for (i = 0; i < lg; i++)
    {
        s = w_L_w_mult (x[i], a[0]);
        for (j = 1; j <= m; j++)
        {
            s = w_L_mac (s, a[j], x[i - j]);
        }
        s = w_L_w_shl (s, 3);
        y[i] = w_round (s);         
    }
    return;
}

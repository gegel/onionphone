/*************************************************************************
 *
 *  FUNCTION:  w_Weight_Ai
 *
 *  PURPOSE: Spectral expansion of LP coefficients.  (order==10)
 *
 *  DESCRIPTION:
 *      a_exp[i] = a[i] * fac[i-1]    ,i=1,10
 *
 *************************************************************************/

#include "typedef.h"
#include "basic_op.h"
#include "count.h"

/* m = LPC order == 10 */
#define m 10

void w_Weight_Ai (
    Word16 a[],         /* (i)     : a[m+1]  LPC coefficients   (m=10)    */
    Word16 fac[],       /* (i)     : Spectral expansion factors.          */
    Word16 a_exp[]      /* (o)     : Spectral expanded LPC coefficients   */
)
{
    Word16 i;

    a_exp[0] = a[0];                                      
    for (i = 1; i <= m; i++)
    {
        a_exp[i] = w_round (w_L_w_mult (a[i], fac[i - 1]));     
    }

    return;
}

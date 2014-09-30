/*************************************************************************
 *
 *  FUNCTION:  w_Lag_window()
 *
 *  PURPOSE:  Lag windowing of autocorrelations.
 *
 *  DESCRIPTION:
 *         r[i] = r[i]*lag_wind[i],   i=1,...,10
 *
 *     r[i] and lag_wind[i] are in special double precision format.
 *     See "oper_32b.c" for the format.
 *
 *************************************************************************/

#include "typedef.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "count.h"

#include "lag_wind.tab"

void w_Lag_window (
    Word16 m,           /* (i)     : LPC order                        */
    Word16 r_h[],       /* (i/o)   : w_Autocorrelations  (msb)          */
    Word16 r_l[]        /* (i/o)   : w_Autocorrelations  (lsb)          */
)
{
    Word16 i;
    Word32 x;

    for (i = 1; i <= m; i++)
    {
        x = w_Mpy_32 (r_h[i], r_l[i], w_lag_h[i - 1], w_lag_l[i - 1]);
        w_L_Extract (x, &r_h[i], &r_l[i]);
    }
    return;
}

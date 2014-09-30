/*************************************************************************
 *
 *   FUNCTION:   w_Copy
 *
 *   PURPOSE:   w_Copy vector x[] to y[]
 *
 *
 *************************************************************************/

#include "typedef.h"
#include "basic_op.h"
#include "count.h"

void w_Copy (
    Word16 x[],         /* (i)   : input vector   */
    Word16 y[],         /* (o)   : output vector  */
    Word16 L            /* (i)   : vector length  */
)
{
    Word16 i;

    for (i = 0; i < L; i++)
    {
        y[i] = x[i];              
    }

    return;
}

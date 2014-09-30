/*************************************************************************
 *
 *  FUNCTION:   Set w_zero()
 *
 *  PURPOSE:  Set vector x[] to w_zero
 *
 *************************************************************************/

#include "typedef.h"
#include "basic_op.h"
#include "count.h"

void w_Set_w_zero (
    Word16 x[],         /* (o)    : vector to clear     */
    Word16 L            /* (i)    : length of vector    */
)
{
    Word16 i;

    for (i = 0; i < L; i++)
    {
        x[i] = 0;                 
    }

    return;
}

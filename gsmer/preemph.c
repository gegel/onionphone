/*---------------------------------------------------------------------*
 * routine w_preemphasis()                                               *
 * ~~~~~~~~~~~~~~~~~~~~~                                               *
 * Preemphasis: filtering through 1 - g z^-1                           *
 *---------------------------------------------------------------------*/

#include "typedef.h"
#include "basic_op.h"
#include "count.h"

Word16 w_mem_pre;

void w_preemphasis (
    Word16 *signal, /* (i/o)   : input signal overwritten by the output */
    Word16 g,       /* (i)     : w_preemphasis coefficient                */
    Word16 L        /* (i)     : size of filtering                      */
)
{
    Word16 *p1, *p2, temp, i;

    p1 = signal + L - 1;                      
    p2 = p1 - 1;                              
    temp = *p1;                               

    for (i = 0; i <= L - 2; i++)
    {
        *p1 = w_sub (*p1, w_mult (g, *p2--));     
        p1--;
    }

    *p1 = w_sub (*p1, w_mult (g, w_mem_pre));       

    w_mem_pre = temp;                           

    return;
}

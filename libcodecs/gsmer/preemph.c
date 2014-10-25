/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*---------------------------------------------------------------------*
 * routine w_preemphasis()                                               *
 * ~~~~~~~~~~~~~~~~~~~~~                                               *
 * Preemphasis: filtering through 1 - g z^-1                           *
 *---------------------------------------------------------------------*/

#include <stdint.h>
#include "basic_op.h"
#include "count.h"

int16_t w_mem_pre;

void w_preemphasis(int16_t * signal,	/* (i/o)   : input signal overwritten by the output */
		   int16_t g,	/* (i)     : w_preemphasis coefficient                */
		   int16_t L	/* (i)     : size of filtering                      */
    )
{
	int16_t *p1, *p2, temp, i;

	p1 = signal + L - 1;
	p2 = p1 - 1;
	temp = *p1;

	for (i = 0; i <= L - 2; i++) {
		*p1 = w_sub(*p1, w_mult(g, *p2--));
		p1--;
	}

	*p1 = w_sub(*p1, w_mult(g, w_mem_pre));

	w_mem_pre = temp;

	return;
}

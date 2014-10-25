/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *
 *   FUNCTION:   w_Copy
 *
 *   PURPOSE:   w_Copy vector x[] to y[]
 *
 *
 *************************************************************************/

#include <stdint.h>
#include "basic_op.h"
#include "count.h"

void w_Copy(int16_t x[],		/* (i)   : input vector   */
	    int16_t y[],		/* (o)   : output vector  */
	    int16_t L		/* (i)   : vector length  */
    )
{
	int16_t i;

	for (i = 0; i < L; i++) {
		y[i] = x[i];
	}

	return;
}

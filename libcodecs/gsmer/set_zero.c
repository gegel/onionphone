/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *
 *  FUNCTION:   Set w_zero()
 *
 *  PURPOSE:  Set vector x[] to w_zero
 *
 *************************************************************************/

#include <stdint.h>
#include "basic_op.h"
#include "count.h"

void w_Set_w_zero(int16_t x[],	/* (o)    : vector to clear     */
		  int16_t L	/* (i)    : length of vector    */
    )
{
	int16_t i;

	for (i = 0; i < L; i++) {
		x[i] = 0;
	}

	return;
}

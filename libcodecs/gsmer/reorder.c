/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *
 *  FUNCTION:  w_Reorder_lsf()
 *
 *  PURPOSE: To make sure that the LSFs are properly ordered and to keep a
 *           certain minimum distance between adjacent LSFs.                               *
 *           The LSFs are in the frequency range 0-0.5 and represented in Q15
 *
 *************************************************************************/

#include <stdint.h>
#include "basic_op.h"


void w_Reorder_lsf(int16_t * lsf,	/* (i/o)     : vector of LSFs   (range: 0<=val<=0.5) */
		   int16_t min_dist,	/* (i)       : minimum required distance             */
		   int16_t n	/* (i)       : LPC order                             */
    )
{
	int16_t i;
	int16_t lsf_min;

	lsf_min = min_dist;
	for (i = 0; i < n; i++) {

		if (w_sub(lsf[i], lsf_min) < 0) {
			lsf[i] = lsf_min;
		}
		lsf_min = w_add(lsf[i], min_dist);
	}
}

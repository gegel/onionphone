/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *
 *  FUNCTION:  w_Syn_filt:
 *
 *  PURPOSE:  Perform w_w_synthesis filtering through 1/A(z).
 *
 *************************************************************************/

#include <stdint.h>
#include "basic_op.h"
#include "count.h"

/* m = LPC order == 10 */
#define m 10

void w_Syn_filt(int16_t a[],	/* (i)     : a[m+1] w_prediction coefficients   (m=10)  */
		int16_t x[],	/* (i)     : input signal                             */
		int16_t y[],	/* (o)     : output signal                            */
		int16_t lg,	/* (i)     : size of filtering                        */
		int16_t mem[],	/* (i/o)   : memory associated with this filtering.   */
		int16_t update	/* (i)     : 0=no update, 1=update of memory.         */
    )
{
	int16_t i, j;
	int32_t s;
	int16_t tmp[80];		/* This is usually done by memory allocation (lg+m) */
	int16_t *yy;

	/* w_Copy mem[] to yy[] */

	yy = tmp;

	for (i = 0; i < m; i++) {
		*yy++ = mem[i];
	}

	/* Do the filtering. */

	for (i = 0; i < lg; i++) {
		s = w_L_w_mult(x[i], a[0]);
		for (j = 1; j <= m; j++) {
			s = w_L_msu(s, a[j], yy[-j]);
		}
		s = w_L_w_shl(s, 3);
		*yy++ = w_round(s);
	}

	for (i = 0; i < lg; i++) {
		y[i] = tmp[i + m];
	}

	/* Update of memory if update==1 */

	if (update != 0) {
		for (i = 0; i < m; i++) {
			mem[i] = y[lg - m + i];
		}
	}
	return;
}

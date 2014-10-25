/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *
 *  FUNCTION:  w_Residu
 *
 *  PURPOSE:  Computes the LP residual.
 *
 *  DESCRIPTION:
 *     The LP residual is computed by filtering the input w_speech through
 *     the LP inverse filter A(z).
 *
 *************************************************************************/

#include <stdint.h>
#include "basic_op.h"
#include "count.h"

/* m = LPC order == 10 */
#define m 10

void w_Residu(int16_t a[],	/* (i)     : w_prediction coefficients                      */
	      int16_t x[],	/* (i)     : w_speech signal                                */
	      int16_t y[],	/* (o)     : residual signal                              */
	      int16_t lg		/* (i)     : size of filtering                            */
    )
{
	int16_t i, j;
	int32_t s;

	for (i = 0; i < lg; i++) {
		s = w_L_w_mult(x[i], a[0]);
		for (j = 1; j <= m; j++) {
			s = w_L_mac(s, a[j], x[i - j]);
		}
		s = w_L_w_shl(s, 3);
		y[i] = w_round(s);
	}
	return;
}

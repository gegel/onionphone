/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *
 *  FUNCTION:  w_Weight_Ai
 *
 *  PURPOSE: Spectral expansion of LP coefficients.  (order==10)
 *
 *  DESCRIPTION:
 *      a_exp[i] = a[i] * fac[i-1]    ,i=1,10
 *
 *************************************************************************/

#include <stdint.h>
#include "basic_op.h"


/* m = LPC order == 10 */
#define m 10

void w_Weight_Ai(int16_t a[],	/* (i)     : a[m+1]  LPC coefficients   (m=10)    */
		 int16_t fac[],	/* (i)     : Spectral expansion factors.          */
		 int16_t a_exp[]	/* (o)     : Spectral expanded LPC coefficients   */
    )
{
	int16_t i;

	a_exp[0] = a[0];
	for (i = 1; i <= m; i++) {
		a_exp[i] = w_round(w_L_w_mult(a[i], fac[i - 1]));
	}

	return;
}

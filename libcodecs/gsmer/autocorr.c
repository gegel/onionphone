/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *
 *  FUNCTION:  autocorr
 *
 *  PURPOSE:   Compute autocorrelations of signal with windowing
 *
 *  DESCRIPTION:
 *       - Windowing of input w_speech:   s'[n] = s[n] * w[n]
 *       - w_Autocorrelations of input w_speech:
 *             r[k] = sum_{i=k}^{239} s'[i]*s'[i-k]    k=0,...,10
 *         The autocorrelations are expressed in normalized double precision
 *         format.
 *
 *************************************************************************/

#include "ophint.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "count.h"
#include "cnst.h"

Word16 w_Autocorr(Word16 x[],	/* (i)    : Input signal                    */
		  Word16 m,	/* (i)    : LPC order                       */
		  Word16 r_h[],	/* (o)    : w_Autocorrelations  (msb)         */
		  Word16 r_l[],	/* (o)    : w_Autocorrelations  (lsb)         */
		  Word16 wind[]	/* (i)    : window for LPC analysis         */
    )
{
	Word16 i, j, norm;
	Word16 y[L_WINDOW];
	Word32 sum;
	Word16 overfl, overfl_shft;

	/* Windowing of signal */

	for (i = 0; i < L_WINDOW; i++) {
		y[i] = w_w_mult_r(x[i], wind[i]);
	}

	/* Compute r[0] and w_test for overflow */

	overfl_shft = 0;

	do {
		overfl = 0;
		sum = 0L;

		for (i = 0; i < L_WINDOW; i++) {
			sum = w_L_mac(sum, y[i], y[i]);
		}

		/* If overflow divide y[] by 4 */

		if (w_L_w_sub(sum, MAX_32) == 0L) {
			overfl_shft = w_add(overfl_shft, 4);
			overfl = 1;	/* Set the overflow flag */

			for (i = 0; i < L_WINDOW; i++) {
				y[i] = w_shr(y[i], 2);
			}
		}

	}
	while (overfl != 0);

	sum = L_w_add(sum, 1L);	/* Avoid the case of all w_zeros */

	/* Normalization of r[0] */

	norm = w_norm_l(sum);
	sum = w_L_w_shl(sum, norm);
	w_L_Extract(sum, &r_h[0], &r_l[0]);	/* Put in DPF format (see oper_32b) */

	/* r[1] to r[m] */

	for (i = 1; i <= m; i++) {
		sum = 0;

		for (j = 0; j < L_WINDOW - i; j++) {
			sum = w_L_mac(sum, y[j], y[j + i]);
		}

		sum = w_L_w_shl(sum, norm);
		w_L_Extract(sum, &r_h[i], &r_l[i]);
	}

	norm = w_sub(norm, overfl_shft);

	return norm;
}

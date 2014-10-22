/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *
 *  FUNCTION:   w_dec_10i40_35bits()
 *
 *  PURPOSE:  Builds the innovative codevector from the received
 *            index of algebraic codebook.
 *
 *   See  c1035pf.c  for more details about the algebraic codebook structure.
 *
 *************************************************************************/

#include "ophint.h"
#include "basic_op.h"
#include "count.h"

#define L_CODE    40		/* codevector length */
#define NB_PULSE  10		/* number of pulses  */
#define NB_TRACK  5		/* number of track */

void w_dec_10i40_35bits(Word16 index[],	/* (i)     : index of 10 pulses (sign+position)       */
			Word16 cod[]	/* (o)     : algebraic (fixed) codebook w_excitation    */
    )
{
	static const Word16 dgray[8] = { 0, 1, 3, 2, 5, 6, 4, 7 };
	Word16 i, j, pos1, pos2, sign, tmp;

	for (i = 0; i < L_CODE; i++) {
		cod[i] = 0;
	}

	/* decode the positions and signs of pulses and build the codeword */

	for (j = 0; j < NB_TRACK; j++) {
		/* compute index i */

		tmp = index[j];
		i = tmp & 7;
		i = dgray[i];

		i = w_extract_l(w_L_w_shr(w_L_w_mult(i, 5), 1));
		pos1 = w_add(i, j);	/* position of pulse "j" */

		i = w_shr(tmp, 3) & 1;

		if (i == 0) {
			sign = 4096;	/* +1.0 */
		} else {
			sign = -4096;	/* -1.0 */
		}

		cod[pos1] = sign;

		/* compute index i */

		i = index[w_add(j, 5)] & 7;
		i = dgray[i];
		i = w_extract_l(w_L_w_shr(w_L_w_mult(i, 5), 1));

		pos2 = w_add(i, j);	/* position of pulse "j+5" */

		if (w_sub(pos2, pos1) < 0) {
			sign = w_negate(sign);
		}
		cod[pos2] = w_add(cod[pos2], sign);
	}

	return;
}

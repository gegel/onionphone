/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *
 *   FUNCTIONS:  w_Lsp_lsf and w_Lsf_lsp
 *
 *   PURPOSE:
 *      w_Lsp_lsf:   Transformation lsp to lsf
 *      w_Lsf_lsp:   Transformation lsf to lsp
 *
 *   DESCRIPTION:
 *         lsp[i] = cos(2*pi*lsf[i]) and lsf[i] = arccos(lsp[i])/(2*pi)
 *
 *   The transformation from lsp[i] to lsf[i] and lsf[i] to lsp[i] are
 *   approximated by a look-up w_table and interpolation.
 *
 *************************************************************************/

#include <stdint.h>
#include "basic_op.h"
#include "count.h"

#include "lsp_lsf.tab"		/* Look-up w_table for transformations */

void w_Lsf_lsp(int16_t lsf[],	/* (i) : lsf[m] normalized (range: 0.0<=val<=0.5) */
	       int16_t lsp[],	/* (o) : lsp[m] (range: -1<=val<1)                */
	       int16_t m		/* (i) : LPC order                                */
    )
{
	int16_t i, ind, offset;
	int32_t L_tmp;

	for (i = 0; i < m; i++) {
		ind = w_shr(lsf[i], 8);	/* ind    = b8-b15 of lsf[i] */
		offset = lsf[i] & 0x00ff;	/* offset = b0-b7  of lsf[i] */

		/* lsp[i] = w_table[ind]+ ((w_table[ind+1]-w_table[ind])*offset) / 256 */

		L_tmp =
		    w_L_w_mult(w_sub(w_table[ind + 1], w_table[ind]), offset);
		lsp[i] = w_add(w_table[ind], w_extract_l(w_L_w_shr(L_tmp, 9)));

	}
	return;
}

void w_Lsp_lsf(int16_t lsp[],	/* (i)  : lsp[m] (range: -1<=val<1)                */
	       int16_t lsf[],	/* (o)  : lsf[m] normalized (range: 0.0<=val<=0.5) */
	       int16_t m		/* (i)  : LPC order                                */
    )
{
	int16_t i, ind;
	int32_t L_tmp;

	ind = 63;		/* begin at end of w_table -1 */

	for (i = m - 1; i >= 0; i--) {
		/* find value in w_table that is just greater than lsp[i] */

		while (w_sub(w_table[ind], lsp[i]) < 0) {
			ind--;

		}

		/* acos(lsp[i])= ind*256 + ( ( lsp[i]-w_table[ind] ) *
		   w_slope[ind] )/4096 */

		L_tmp = w_L_w_mult(w_sub(lsp[i], w_table[ind]), w_slope[ind]);
		/*(lsp[i]-w_table[ind])*w_slope[ind])>>12 */
		lsf[i] = w_round(w_L_w_shl(L_tmp, 3));
		lsf[i] = w_add(lsf[i], w_shl(ind, 8));
	}
	return;
}

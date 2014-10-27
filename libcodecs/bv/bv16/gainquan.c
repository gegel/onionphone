/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*****************************************************************************/
/* BroadVoice(R)16 (BV16) Fixed-Point ANSI-C Source Code                     */
/* Revision Date: November 13, 2009                                          */
/* Version 1.1                                                               */
/*****************************************************************************/

/*****************************************************************************/
/* Copyright 2000-2009 Broadcom Corporation                                  */
/*                                                                           */
/* This software is provided under the GNU Lesser General Public License,    */
/* version 2.1, as published by the Free Software Foundation ("LGPL").       */
/* This program is distributed in the hope that it will be useful, but       */
/* WITHOUT ANY SUPPORT OR WARRANTY; without even the implied warranty of     */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LGPL for     */
/* more details.  A copy of the LGPL is available at                         */
/* http://www.broadcom.com/licenses/LGPLv2.1.php,                            */
/* or by writing to the Free Software Foundation, Inc.,                      */
/* 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                 */
/*****************************************************************************/

/*****************************************************************************
  gainquan.c : gain quantization based on inter-bv_subframe 
           moving-average prediction of logarithmic gain.

  $Log$
******************************************************************************/

#include <stdint.h>
#include "bvcommon.h"
#include "bv16cnst.h"
#include "bv16strct.h"
#include "bv16externs.h"
#include "basop32.h"
#include "mathutil.h"
#include "../itug729ilib/oper_32b.h"

int16_t gainquan(int32_t * gainq,	/* Q18 */
		int32_t * ee,	/* Q1 */
		int16_t * lgpm,	/* Q11 */
		int32_t * prevlg, int32_t level)
{
	int32_t lg, elg, lgq, limit;
	int16_t lg_exp, lg_frac, lgpe, d, dmin;
	int16_t i, n, gidx = 0, *p_gidx;

	/* Divide ee by hfrsz = 8*5 */
	if (*ee < 2 * FRSZ)
		lg = 0;
	else {
		L_Extract(*ee, &lg_exp, &lg_frac);	/* Q1 -> Q4 for divided by 8 */
		lg = Mpy_32_16(lg_exp, lg_frac, 6554);	/* bv_multiplied by 0.2 in Q15 */
		Log2(lg, &lg_exp, &lg_frac);	/* Q4 treated as Q0 */
		lg_exp = bv_sub(lg_exp, 4);	/* compensated Q4 */
		lg = L_bv_add(L_bv_shl(bv_L_deposit_h(lg_exp), 9), L_bv_shr(bv_L_deposit_h(lg_frac), 6));	/* Q25 */
	}

	/* Calculate estimated log-gain */
	elg = L_bv_shr(bv_L_deposit_h(lgmean), 1);	/* Q26 */
	for (i = 0; i < LGPORDER; i++) {
		elg = bv_L_mac0(elg, lgp[i], lgpm[i]);	/* Q26 */
	}
	elg = L_bv_shr(elg, 1);	/* Q25 */

	/* Subtract log-gain mean & estimated log-gain to get prediction error */
	lgpe = intround(L_bv_shl(L_bv_sub(lg, elg), 2));	/* Q11 */

	/* Scalar quantization of log-gain prediction error */
	dmin = MAX_16;
	p_gidx = idxord;
	for (i = 0; i < LGPECBSZ; i++) {
		d = bv_abs_s(bv_sub(lgpe, lgpecb[*p_gidx++]));
		if (d < dmin) {
			dmin = d;
			gidx = i;
		}
	}

	/* Calculate quantized log-gain */
	lgq = L_bv_add(L_bv_shr(bv_L_deposit_h(lgpecb[idxord[gidx]]), 2), elg);
	/* Q25 */

	/* Look up from lgclimit() table the maximum log gain change allowed */
	i = bv_shr(bv_sub(bv_shr(bv_extract_h(L_bv_sub(prevlg[0], level)), 9), LGLB), 1);	/* get column index */
	if (i >= NGB) {
		i = NGB - 1;
	} else if (i < 0) {
		i = 0;
	}
	n = bv_shr(bv_sub
		   (bv_shr(bv_extract_h(L_bv_sub(prevlg[0], prevlg[1])), 9),
		    LGCLB), 1);
	/* get row index */
	if (n >= NGCB) {
		n = NGCB - 1;
	} else if (n < 0) {
		n = 0;
	}
	i = i * NGCB + n;

	/* Check whether quantized log-gain cause a gain change > lgclimit */
	limit = L_bv_add(prevlg[0], bv_L_deposit_h(lgclimit[i]));	/* limit log-gain */
	while ((lgq > limit) && (gidx > 0)) {	/* if q log-gain exceeds limit */
		gidx -= 1;	/* decrement gain quantizer index by 1 */
		lgq =
		    L_bv_add(L_bv_shr(bv_L_deposit_h(lgpecb[idxord[gidx]]), 2),
			     elg);
	}

	/* get true codebook index */
	gidx = idxord[gidx];

	/* Update log-gain predictor memory */
	prevlg[1] = prevlg[0];
	prevlg[0] = lgq;
	for (i = LGPORDER - 1; i > 0; i--) {
		lgpm[i] = lgpm[i - 1];
	}
	lgpm[0] = lgpecb[gidx];

	/* Convert quantized log-gain to linear domain */
	elg = L_bv_shr(lgq, 10);	/* Q25 -> Q26 (0.5F) --> Q16 */
	L_Extract(elg, &lg_exp, &lg_frac);
	lg_exp = bv_add(lg_exp, 18);	/* output to be Q2 */
	*gainq = Pow2(lg_exp, lg_frac);

	return gidx;
}

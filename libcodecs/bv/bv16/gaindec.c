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
  gaindec.c : gain decoding functions

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

int32_t gaindec(int32_t * lgq,	/* Q25 */
		      int16_t gidx, int16_t * lgpm,	/* Q11 */
		      int32_t * prevlg,	/* Q25 */
		      int32_t level,	/* Q25 */
		      int16_t * nggalgc, int32_t * lg_el)
{
	int32_t elg;
	int16_t lg_exp, lg_frac, lgc;
	int16_t i, n, k;

	/* Calculate estimated log-gain */
	elg = L_bv_shr(bv_L_deposit_h(lgmean), 1);	/* Q26 */
	for (i = 0; i < LGPORDER; i++) {
		elg = bv_L_mac0(elg, lgp[i], lgpm[i]);	/* Q26 */
	}
	elg = L_bv_shr(elg, 1);

	/* Calculate decoded log-gain */
	*lgq = L_bv_add(L_bv_shr(bv_L_deposit_h(lgpecb[gidx]), 2), elg);	/* Q25 */

	/* Look up from lgclimit() table the maximum log gain change allowed */
	i = bv_shr(bv_sub
		   (bv_shr(bv_extract_h(L_bv_sub(prevlg[0], level)), 9), LGLB),
		   1);
	/* get column index */
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

	/* Update log-gain predictor memory,
	   check whether decoded log-gain exceeds lgclimit */
	for (k = LGPORDER - 1; k > 0; k--) {
		lgpm[k] = lgpm[k - 1];
	}
	lgc = bv_extract_h(L_bv_sub(*lgq, prevlg[0]));	/* Q9 */
	if ((lgc > lgclimit[i]) && (gidx > 0)) {	/* if log-gain exceeds limit */
		*lgq = prevlg[0];	/* use the log-gain of previous frame */
		lgpm[0] = bv_extract_h(L_bv_shl(L_bv_sub(*lgq, elg), 2));
		*nggalgc = 0;
		*lg_el = L_bv_add(bv_L_deposit_h(lgclimit[i]), prevlg[0]);
	} else {
		lgpm[0] = lgpecb[gidx];
		*nggalgc = bv_add(*nggalgc, 1);
		if (*nggalgc > Nfdm)
			*nggalgc = Nfdm + 1;
		*lg_el = *lgq;
	}

	/* Update log-gain predictor memory */
	prevlg[1] = prevlg[0];
	prevlg[0] = *lgq;

	/* Convert quantized log-gain to linear domain */
	elg = L_bv_shr(*lgq, 10);	/* Q25 -> Q26 (0.5F) --> Q16 */
	L_Extract(elg, &lg_exp, &lg_frac);
	lg_exp = bv_add(lg_exp, 18);	/* output to be Q2 */
	return Pow2(lg_exp, lg_frac);
}


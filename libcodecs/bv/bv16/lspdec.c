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
  lspdec.c :  LSP decoding function

  $Log$
******************************************************************************/

#include <stdint.h>
#include "bvcommon.h"
#include "bv16cnst.h"
#include "bv16strct.h"
#include "bv16externs.h"
#include "basop32.h"

void lspdec(int16_t * lspq,	/* Q15 */
	    int16_t * lspidx, int16_t * lsppm,	/* Q15 */
	    int16_t * lspq_last)
{
	int32_t a0;
	int16_t elsp[LPCO], lspe[LPCO];
	int16_t lspeq1[LPCO], lspeq2[LPCO];
	int16_t *fp1, *fp2;
	int16_t i, k, sign, idx, stbl;

	/* Calculate estimated (ma-predicted) lsp vector */
	fp1 = lspp;		/* Q14 */
	fp2 = lsppm;		/* Q15 */
	for (i = 0; i < LPCO; i++) {
		a0 = 0;
		for (k = 0; k < LSPPORDER; k++) {
			a0 = bv_L_mac(a0, *fp1++, *fp2++);
		}
		elsp[i] = intround(L_bv_shl(a0, 1));	/* Q15 */
	}

	/* Perform first-stage vq codebook decode */
	vqdec(lspeq1, lspidx[0], lspecb1, LPCO);

	/* Perform second-stage vq codebook decode */
	idx = lspidx[1];
	if (lspidx[1] >= LSPECBSZ2) {
		sign = -1;
		idx = bv_sub((int16_t) (2 * LSPECBSZ2 - 1), idx);
	} else
		sign = 1;

	vqdec(lspeq2, idx, lspecb2, LPCO);

	/* Get overall quantizer output vector of the two-stage vq */
	if (sign == 1)
		for (i = 0; i < LPCO; i++)
			lspe[i] = bv_shr(bv_add(lspeq1[i], lspeq2[i]), 2);
	else
		for (i = 0; i < LPCO; i++)
			lspe[i] = bv_shr(bv_sub(lspeq1[i], lspeq2[i]), 2);

	/* Calculate quantized lsp for stability check */
	for (i = 0; i < STBLDIM; i++) {
		lspq[i] = bv_add(bv_add(lspe[i], elsp[i]), lspmean[i]);
	}

	/* detect bit-errors based on ordering property of lsp */
	stbl = stblchck(lspq, STBLDIM);

	/* replace LSP if bit-errors are detected */
	if (!stbl) {
		for (i = 0; i < LPCO; i++) {
			lspq[i] = lspq_last[i];
			lspe[i] = bv_sub(bv_sub(lspq[i], elsp[i]), lspmean[i]);
		}
	}
	/* calculate remaining quantized LSP for error free case */
	else {
		for (i = STBLDIM; i < LPCO; i++)
			lspq[i] = bv_add(bv_add(lspe[i], elsp[i]), lspmean[i]);
	}

	/* Update lsp ma predictor memory */
	i = LPCO * LSPPORDER - 1;
	fp1 = &lsppm[i];
	fp2 = &lsppm[i - 1];
	for (i = LPCO - 1; i >= 0; i--) {
		for (k = LSPPORDER; k > 1; k--) {
			*fp1-- = *fp2--;
		}
		*fp1-- = lspe[i];
		fp2--;
	}

	/* Ensure correct ordering of lsp to guarantee lpc filter stability */
	stblz_lsp(lspq, LPCO);

}


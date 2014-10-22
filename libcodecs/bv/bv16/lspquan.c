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
  lspquan.c : Lsp quantization based on inter-frame moving-average
               prediction and two-stage VQ.

  $Log$
******************************************************************************/

#include "ophint.h"
#include "bvcommon.h"
#include "bv16cnst.h"
#include "bv16strct.h"
#include "bv16externs.h"
#include "basop32.h"

void vqmse(Word16 * xq,
	   Word16 * idx, Word16 * x, Word16 * cb, int vdim, int cbsz);

void svqwmse(Word16 * xq,
	     Word16 * idx,
	     Word16 * x,
	     Word16 * xa, Word16 * w, Word16 * cb, int vdim, int cbsz);

void lspquan(Word16 * lspq,	/* Q15 */
	     Word16 * lspidx, Word16 * lsp,	/* Q15 */
	     Word16 * lsppm)
{				/* Q15 */
	Word32 a0;
	Word16 d[LPCO], w[LPCO];
	Word16 elsp[LPCO], lspe[LPCO], lspa[LPCO];
	Word16 lspeq1[LPCO], lspeq2[LPCO];
	Word16 *fp1, *fp2, min_d;
	int i, k;

	/* Calculate the weights for weighted mean-square error distortion */
	min_d = MAX_16;
	for (i = 0; i < LPCO - 1; i++) {
		d[i] = bv_sub(lsp[i + 1], lsp[i]);	/* LSP difference vector */
		if (d[i] < min_d)
			min_d = d[i];
	}

	w[0] = bv_shr(bv_div_s(min_d, d[0]), 1);
	for (i = 1; i < LPCO - 1; i++) {
		if (d[i] < d[i - 1])
			w[i] = bv_shr(bv_div_s(min_d, d[i]), 1);
		else
			w[i] = bv_shr(bv_div_s(min_d, d[i - 1]), 1);
	}
	w[LPCO - 1] = bv_shr(bv_div_s(min_d, d[LPCO - 2]), 1);

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

	/* Subtract lsp mean value & estimated lsp to get prediction error */
	for (i = 0; i < LPCO; i++) {
		lspe[i] = bv_shl(bv_sub(bv_sub(lsp[i], lspmean[i]), elsp[i]), 2);	/* Q17 */
	}

	/* Perform first-stage vq codebook search */
	vqmse(lspeq1, &lspidx[0], lspe, lspecb1, LPCO, LSPECBSZ1);

	/* Calculate quantization error vector of first-stage vq */
	for (i = 0; i < LPCO; i++) {
		d[i] = bv_sub(lspe[i], lspeq1[i]);	/* Q17 */
	}

	/* Perform second-stage vq codebook search */
	for (i = 0; i < LPCO; i++)
		lspa[i] = bv_add(bv_add(lspmean[i], elsp[i]), bv_shr(lspeq1[i], 2));	/* Q15 */

	svqwmse(lspeq2, &lspidx[1], d, lspa, w, lspecb2, LPCO, LSPECBSZ2);

	/* Get overall quantizer output vector of the two-stage vq */
	for (i = 0; i < LPCO; i++) {
		lspe[i] = bv_shr(bv_add(lspeq1[i], lspeq2[i]), 2);
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

	/* Calculate quantized lsp */
	for (i = 0; i < LPCO; i++) {
		lspq[i] = bv_add(bv_add(lspmean[i], elsp[i]), lspe[i]);
	}

	/* Ensure correct ordering of lsp to guarantee lpc filter stability */
	stblz_lsp(lspq, LPCO);
}

/*==========================================================================*/

void vqmse(Word16 * xq,		/* Q17 VQ output vector (quantized version of input vector) */
	   Word16 * idx,	/* VQ codebook index for the nearest neighbor */
	   Word16 * x,		/* Q17 input vector */
	   Word16 * cb,		/* VQ codebook */
	   int vdim,		/* vector dimension */
	   int cbsz)
{				/* codebook size (number of codevectors) */
	Word32 dmin, d;
	Word16 *fp1;
	Word16 j, k;

	Word16 e;

	fp1 = cb;
	dmin = MAX_32;
	for (j = 0; j < cbsz; j++) {
		d = 0;
		for (k = 0; k < vdim; k++) {
			e = bv_sub(x[k], *fp1++);	// Q17
			d = bv_L_mac0(d, e, e);	// Q34
		}
		if (L_bv_sub(d, dmin) < 0) {
			dmin = d;
			*idx = j;
		}
	}

	j = *idx * vdim;
	for (k = 0; k < vdim; k++) {
		xq[k] = cb[j + k];
	}
}

/* Signed WMSE VQ */
void svqwmse(Word16 * xq,	/* Q17 */
	     Word16 * idx, Word16 * x,	/* Q17 */
	     Word16 * xa,	/* Q15 */
	     Word16 * w,	/* Q15 */
	     Word16 * cb,	/* Q17 */
	     int vdim, int cbsz)
{
	Word32 dmin, d;
	Word16 *fp1, *fp2;
	Word16 xqc[STBLDIM];
	Word16 j, k, stbl, sign = 1;
	Word16 e, we;

	fp1 = cb;
	dmin = MAX_32;
	*idx = -1;

	for (j = 0; j < cbsz; j++) {

		/* Try negative sign */
		d = 0;
		fp2 = fp1;

		for (k = 0; k < vdim; k++) {
			e = bv_add(x[k], *fp1++);	// Q17
			we = bv_mult(w[k], e);	// Q17
			d = bv_L_mac0(d, we, e);	// Q34
		}

		/* check candidate - negative sign */
		if (L_bv_sub(d, dmin) < 0) {

			for (k = 0; k < STBLDIM; k++)
				xqc[k] = bv_sub(xa[k], bv_shr(*fp2++, 2));	// Q15

			/* check stability - negative sign */
			stbl = stblchck(xqc, STBLDIM);

			if (stbl > 0) {
				dmin = d;
				*idx = j;
				sign = -1;
			}
		}

		/* Try positive sign */
		fp1 -= vdim;
		d = 0;
		fp2 = fp1;

		for (k = 0; k < vdim; k++) {
			e = bv_sub(x[k], *fp1++);	// Q17
			we = bv_mult(w[k], e);	// Q17
			d = bv_L_mac0(d, we, e);	// Q34
		}

		/* check candidate - positive sign */
		if (L_bv_sub(d, dmin) < 0) {

			for (k = 0; k < STBLDIM; k++)
				xqc[k] = bv_add(xa[k], bv_shr(*fp2++, 2));	// Q15

			/* check stability - positive sign */
			stbl = stblchck(xqc, STBLDIM);

			if (stbl > 0) {
				dmin = d;
				*idx = j;
				sign = +1;
			}
		}
	}

	if (*idx == -1) {
		*idx = 0;
		sign = 1;
	}

	fp1 = cb + (*idx) * vdim;

	for (k = 0; k < vdim; k++)
		xq[k] = *fp1++;

	if (sign == -1)
		for (k = 0; k < vdim; k++)
			xq[k] = bv_negate(xq[k]);

	if (sign < 0)
		*idx = bv_sub((Word16) (2 * cbsz - 1), (*idx));
}

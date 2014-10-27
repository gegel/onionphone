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
  excdec.c :excitation signal decoding with integrated long-term and 
            short-term synthesis.

  $Log$
******************************************************************************/

#include <stdint.h>
#include "bvcommon.h"
#include "bv16cnst.h"
#include "basop32.h"
#include "utility.h"

void excdec_w_synth(int16_t * xq,	/* (o) Q0 quantized signal vector               */
		    int16_t * ltsym,	/* (i/o) Q0 quantized excitation signal vector  */
		    int16_t * stsym,	/* (i/o) Q0 short-term predictor memory         */
		    int16_t * idx,	/* (o) quantizer codebook index for uq[] vector */
		    int16_t * b,	/* (i) Q15 coefficient of 3-tap pitch predictor */
		    int16_t * cb,	/* (i) Q0 codebook                              */
		    int16_t pp,	/* pitch period (# of 8 kHz samples)            */
		    int16_t * aq,	/* (i) Q12 short-term predictor coefficients    */
		    int16_t gain_exp,	/* gain_exp of current bv_sub-frame                */
		    int32_t * EE)
{
	int16_t i, n, m, *ip, id;
	int16_t *fp1, *fp2, *fp3;
	int32_t a0;
	int16_t tt;
	int32_t E;
	int32_t a1;
	int16_t buf1[LPCO + FRSZ];	/* buffer for filter memory & signal     */
	int16_t uq[VDIM];	/* selected codebook vector (incl. sign) */

	W16copy(buf1, stsym, LPCO);	/* buffer is used to avoid memory shifts */

	ip = idx;
	E = 0;

	/* Loop through every vector of the current bv_subframe */
	for (m = 0; m < FRSZ; m += VDIM) {

      /********************************************************************************/
		/*                               Excitation vector                              */
      /********************************************************************************/

		id = *ip++;	/* get codebook index of current vector */
		fp1 = uq;
		if (id < CBSZ) {
			fp2 = &cb[id * VDIM];
			for (n = 0; n < VDIM; n++) {
				*fp1++ = *fp2++;	// Q0
			}
		} else {
			id -= CBSZ;
			fp2 = &cb[id * VDIM];
			for (n = 0; n < VDIM; n++) {
				*fp1++ = bv_negate(*fp2++);	// Q0
			}
		}

      /********************************************************************************/
		/*                      Long-term and short-term synthesis                      */
      /********************************************************************************/

		fp2 = uq;
		fp3 = ltsym + m;
		for (n = m; n < m + VDIM; n++) {

			/* Un-normalized excitation */
			a0 = L_bv_shr(bv_L_deposit_h(*fp2++), gain_exp);	// Q16

			/* Excitation energy for PLC */
			tt = intround(a0);	// Q0
			E = bv_L_mac0(E, tt, tt);

			/* Long-term predicion */
			fp1 = &ltsym[n - pp + 1];	// Q0
			a1 = L_bv_mult(*fp1--, b[0]);	// Q16
			a1 = bv_L_mac(a1, *fp1--, b[1]);
			a1 = bv_L_mac(a1, *fp1, b[2]);

			/* Update long-term filter synthesis memory */
			a0 = L_bv_add(a0, a1);
			*fp3++ = intround(a0);	// Q0

			/* Short-term prediction */
			fp1 = &buf1[n];	// Q0
			a1 = 0;	// Q13
			for (i = LPCO; i > 0; i--)
				a1 = bv_L_msu(a1, *fp1++, aq[i]);	// Q13
			a1 = L_bv_shl(a1, 3);	// Q16
			a1 = L_bv_add(a0, a1);
			*fp1++ = intround(a1);	// Q0

		}
	}

	/* Update noise feedback filter memory after filtering current bv_subframe */
	W16copy(stsym, buf1 + FRSZ, LPCO);

	/* copy to speech buffer */
	W16copy(xq, buf1 + LPCO, FRSZ);
	*EE = E;

	return;
}

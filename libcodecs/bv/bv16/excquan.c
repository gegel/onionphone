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
  excquan.c : Vector Quantizer for 2-Stage Noise Feedback Coding 
            with long-term predictive noise feedback coding embedded 
            inside the short-term predictive noise feedback coding loop.

  Note that the Noise Feedback Coding of the excitation signal is implemented
  using the Zero-State Responsse and Zero-input Response decomposition as 
  described in: J.-H. Chen, "Novel Codec Structures for Noise Feedback 
  Coding of Speech," Proc. ICASSP, 2006.  The principle of the Noise Feedback
  Coding of the excitation signal is described in: "BV16 Speech Codec 
  Specification for Voice over IP Applications in Cable Telephony," American 
  National Standard, ANSI/SCTE 24-21 2006.

  Note that indicated Q-values may be relative to the over-all normalization
  by gain_exp.

  $Log$
******************************************************************************/

#include <stdint.h>
#include "bvcommon.h"
#include "bv16cnst.h"
#include "bv16strct.h"
#include "basop32.h"
#include "utility.h"

void vq2snfc_zsr_codebook(int16_t * qzsr,	// normalized by gain_exp-1
			  int16_t * cb,	// normalized by gain_exp
			  int16_t * aq,	// Q12
			  int16_t * fsz,	// Q12
			  int16_t * fsp)	// Q12
{
	int32_t a0, a1;
	int16_t buf1[VDIM], buf2[VDIM], buf3[VDIM];
	int16_t *fp1, *fp2, *fp3, *fpa, *fpb;
	int16_t j, i, n;

	/* Q-values of signals are relative to the normalization by gain_exp */

	/* Calculate bv_negated Zero State Response */
	fp2 = cb;		/* fp2 points to start of first codevector */
	fp3 = qzsr;		/* fp3 points to start of first zero-state response vector */

	/* For each codevector */
	for (j = 0; j < CBSZ; j++) {

		/* Calculate the elements of the bv_negated ZSR */
		for (i = 0; i < VDIM; i++) {
			/* Short-term prediction */
			a0 = 0;
			fp1 = buf1;	// Q0
			for (n = i; n > 0; n--)
				a0 = bv_L_msu(a0, *fp1++, aq[n]);	// Q13
			a0 = L_bv_shl(a0, 3);	// Q16

			/* Update memory of short-term prediction filter */
			a0 = L_bv_add(a0, bv_L_deposit_h(*fp2++));
			*fp1++ = intround(a0);

			/* noise feedback filter */
			a1 = 0;
			fpa = buf2;	// Q0
			fpb = buf3;	// Q0
			for (n = i; n > 0; n--) {
				a1 = bv_L_mac(a1, *fpa++, fsz[n]);	// Q13
				a1 = bv_L_msu(a1, *fpb++, fsp[n]);	// Q13
			}
			a1 = L_bv_shl(a1, 3);	// Q16

			/* Update memory of pole section of noise feedback filter */
			*fpb++ = intround(a1);	// Q0

			/* ZSR */
			a0 = L_bv_add(a0, a1);	// Q16

			/* Update memory of zero section of noise feedback filter */
			*fpa++ = bv_negate(intround(a0));

			/* Get ZSR at correct normalization - gain_exp-1 */
			*fp3++ = intround(L_bv_shr(a0, 1));
		}
	}

	return;
}

/* COMPUTE PITCH-PREDICTED VECTOR, WHICH SHOULD BE INDEPENDENT OF THE
RESIDUAL VQ CODEVECTORS BEING TRIED IF vdim < MIN. PITCH PERIOD */

void vq2snfc_ppv(int32_t * ltfv,	// Q16
		 int32_t * ppv,	// Q16
		 int16_t * ltsym,	// Q0
		 int16_t * ltnfm,	// Q0
		 int16_t * b,	// Q15
		 int16_t beta)	// Q13
{
	int32_t a0, a1;
	int16_t n, *sp1;
	for (n = 0; n < VDIM; n++) {
		sp1 = &ltsym[n];	// Q0
		a0 = L_bv_mult(*sp1--, b[0]);	// Q16
		a0 = bv_L_mac(a0, *sp1--, b[1]);	// Q16
		a0 = bv_L_mac(a0, *sp1--, b[2]);	// Q16
		*ppv++ = a0;	// Q16
		a1 = L_bv_mult(ltnfm[n], beta);	// Q14
		a1 = L_bv_shl(a1, 2);	// Q16
		*ltfv++ = L_bv_add(a0, a1);	// Q16
	}

	return;
}

void vq2snfc_zir(int16_t * qzir,
		 int32_t * ppv,
		 int32_t * ltfv,
		 int16_t * aq,
		 int16_t * buf1,
		 int16_t * buf2,
		 int16_t * buf3,
		 int16_t * fsz, int16_t * fsp, int16_t * s, int16_t gexpm3)
{
	int32_t a0, a1, a2;
	int16_t i, n;
	int16_t *sp1, *spa, *spb;

	for (n = 0; n < VDIM; n++) {

		/* Perform bv_multiply-bv_adds along the delay line of filter */
		sp1 = &buf1[n];	/* Q16 */
		a0 = 0;		/* Q13 */
		for (i = LPCO; i > 0; i--)
			a0 = bv_L_msu(a0, *sp1++, aq[i]);
		a0 = L_bv_shl(a0, 3);	/* Q16 */
		/* Perform bv_multiply-bv_adds along the noise feedback filter */
		spa = &buf2[n];
		spb = &buf3[n];
		a1 = 0;
		for (i = NSTORDER; i > 0; i--) {
			a1 = bv_L_mac(a1, *spa++, fsz[i]);
			a1 = bv_L_msu(a1, *spb++, fsp[i]);
		}
		a1 = L_bv_shl(a1, 3);	/* Q16 */
		*spb = intround(a1);	/* update output of the noise feedback filter */
		a2 = bv_L_deposit_h(s[n]);
		a2 = L_bv_sub(a2, a0);
		a2 = L_bv_sub(a2, a1);	/* Q16 */
		*qzir++ = intround(L_bv_shl(L_bv_sub(a2, *ltfv++), gexpm3));
		/* Update short-term noise feedback filter memory */
		a0 = L_bv_add(a0, *ppv);	/* a0 now conatins the qs[n] */
		*sp1 = intround(a0);
		a2 = L_bv_sub(a2, *ppv++);	/* a2 now contains qszi[n] */
		*spa = intround(a2);	/* update short-term noise feedback filter memory */

	}

	return;
}

/* loop through every codevector of the residual vq codebook */
/* and find the one that minimizes the energy of q[n] */

int16_t vq2snfc_vq(int16_t * qzsr,	// normalized by gain_exp - 1
		  int16_t * qzir,	// normalized by gain_exp - 3
		  int16_t * rsign)
{
	int32_t Emin, E;
	int16_t j, n, jmin, sign = 1, e;
	int16_t *fp4, *fp2;

	Emin = MAX_32;
	jmin = 0;
	fp4 = qzsr;
	for (j = 0; j < CBSZ; j++) {
		/* Try positive sign */
		fp2 = qzir;
		E = 0;
		for (n = 0; n < VDIM; n++) {
			e = bv_sub(bv_shl(*fp2++, 2), *fp4++);
			E = bv_L_mac0(E, e, e);
		}
		if (L_bv_sub(E, Emin) < 0) {
			jmin = j;
			Emin = E;
			sign = 1;
		}
		/* Try negative sign */
		fp4 -= VDIM;
		fp2 = qzir;
		E = 0;
		for (n = 0; n < VDIM; n++) {
			e = bv_add(bv_shl(*fp2++, 2), *fp4++);
			E = bv_L_mac0(E, e, e);
		}
		if (L_bv_sub(E, Emin) < 0) {
			jmin = j;
			Emin = E;
			sign = -1;
		}
	}

	*rsign = sign;

	return jmin;
}

void vq2snfc_update_mem(int16_t * s,	// Q0
			int16_t * stsym,	// Q0
			int16_t * stnfz,	// Q0
			int16_t * stnfp,	// Q0
			int16_t * ltsym,	// Q0
			int16_t * ltnfm,	// Q0
			int16_t * aq,	// Q12
			int16_t * fsz,	// Q12
			int16_t * fsp,	// Q12
			int16_t * uq,	// normalized by gain_exp
			int32_t * ppv,	// Q16
			int32_t * ltfv,	// Q16
			int16_t gain_exp)
{
	int16_t *fp3, *fp4;
	int16_t *fp1, *fpa, *fpb;
	int16_t *p_uq, *p_s;
	int16_t n, i;
	int32_t *p_ppv, *p_ltfv;
	int32_t a0, a1, v, vq, qs, uq32;

	fp3 = ltsym;
	fp4 = ltnfm;
	p_ppv = ppv;
	p_ltfv = ltfv;
	p_uq = uq;
	p_s = s;
	for (n = 0; n < VDIM; n++) {

		uq32 = L_bv_shr(bv_L_deposit_h(*p_uq++), gain_exp);
		// Q16

		/* Short-term excitation */
		vq = L_bv_add(*p_ppv++, uq32);	// Q16
		/* Update memory of long-term synthesis filter */
		*fp3++ = intround(vq);	// Q0

		/* Short-term prediction */
		a0 = 0;
		fp1 = stsym + n;	// Q0
		for (i = LPCO; i > 0; i--)
			a0 = bv_L_msu(a0, *fp1++, aq[i]);	// Q13
		a0 = L_bv_shl(a0, 3);	// Q16

		/* Update memory of short-term synthesis filter */
		*fp1++ = intround(L_bv_add(a0, vq));	// Q0

		/* Short-term pole-zero noise feedback filter */
		fpa = stnfz + n;	// Q0
		fpb = stnfp + n;	// Q0
		a1 = 0;
		for (i = NSTORDER; i > 0; i--) {
			a1 = bv_L_mac(a1, *fpa++, fsz[i]);	// Q13
			a1 = bv_L_msu(a1, *fpb++, fsp[i]);	// Q13
		}
		a1 = L_bv_shl(a1, 3);	// Q16

		/* Update memory of pole section of noise feedback filter */
		*fpb++ = intround(a1);	// Q0

		v = L_bv_sub(L_bv_sub(bv_L_deposit_h(*p_s++), a0), a1);
		// Q16
		qs = L_bv_sub(v, vq);	// Q16

		/* Update memory of zero section of noise feedback filter */
		*fpa++ = intround(qs);	// Q0

		/* Update memory of long-term noise feedback filter */
		*fp4++ = intround(L_bv_sub(L_bv_sub(v, *p_ltfv++), uq32));
		// Q0
	}

	return;
}

void excquan(int16_t * idx,	/* quantizer codebook index for uq[] vector */
	     int16_t * s,	/* (i) Q0 input signal vector */
	     int16_t * aq,	/* (i) Q12 noise feedback filter coefficient array */
	     int16_t * fsz,	/* (i) Q12 short-term noise feedback filter - numerator */
	     int16_t * fsp,	/* (i) Q12 short-term noise feedback filter - denominator */
	     int16_t * b,	/* (i) Q15 coefficient of 3-tap pitch predictor */
	     int16_t beta,	/* (i) Q13 coefficient of pitch feedback filter */
	     int16_t * stsym,	/* (i/o) Q0 filter memory */
	     int16_t * ltsym,	/* (i/0) Q0 long-term synthesis filter memory */
	     int16_t * ltnfm,	/* (i/o) Q0 long-term noise feedback filter memory */
	     int16_t * stnfz,	/* (i/o) Q0 filter memory */
	     int16_t * stnfp,	/* (i/o) Q0 filter memory */
	     int16_t * cb,	/* (i) scalar quantizer codebook - normalized by gain_exp */
	     int16_t pp,		/* pitch period (# of 8 kHz samples) */
	     int16_t gain_exp)
{
	int16_t qzir[VDIM];	// normalized by gain_exp-3
	int16_t uq[VDIM];	// normalized by gain_exp
	int16_t buf1[LPCO + FRSZ];	// Q0
	int16_t buf2[NSTORDER + FRSZ];	// Q0
	int16_t buf3[NSTORDER + FRSZ];	// Q0
	int32_t ltfv[VDIM], ppv[VDIM];	// Q16
	int16_t qzsr[VDIM * CBSZ];	// normalized by gain_exp-1
	int16_t *sp3;
	int16_t sign = 1;
	int16_t m, n, jmin, iv;
	int16_t gexpm3;

	gexpm3 = bv_sub(gain_exp, 3);

	/* COPY FILTER MEMORY TO BEGINNING PART OF TEMPORARY BUFFER */
	W16copy(buf1, stsym, LPCO);	/* buffer is used to avoid memory shifts */

	/* COPY NOISE FEEDBACK FILTER MEMORY */
	W16copy(buf2, stnfz, NSTORDER);
	W16copy(buf3, stnfp, NSTORDER);

	/* -------------------------------------- */
	/*  Z e r o - S t a t e  R e s p o n s e  */
	/* -------------------------------------- */

	vq2snfc_zsr_codebook(qzsr, cb, aq, fsz, fsp);

	/* --------------------------------------------------- */
	/*  LOOP THROUGH EVERY VECTOR OF THE CURRENT SUBFRAME  */
	/* --------------------------------------------------- */

	iv = 0;			/* iv = index of the current vector */
	for (m = 0; m < FRSZ; m += VDIM) {

		/* --------------------------------------- */
		/*  Z e r o - I n p u t   R e s p o n s e  */
		/* --------------------------------------- */

		/* Compute pitch-predicted vector, which should be independent of the
		   residual vq codevectors being tried if vdim < min. pitch period */
		vq2snfc_ppv(ltfv, ppv, &ltsym[MAXPP1 + m - pp + 1],
			    &ltnfm[MAXPP1 + m - pp], b, beta);

		/* Compute zero-input response */
		vq2snfc_zir(qzir, ppv, ltfv, aq, &buf1[m], &buf2[m], &buf3[m],
			    fsz, fsp, &s[m], gexpm3);

		/* --------------------------------------- */
		/*  C O D E B O O K   S E A R C H          */
		/* --------------------------------------- */

		jmin = vq2snfc_vq(qzsr, qzir, &sign);

		/* The best codevector has been found; assign vq codebook index */
		idx[iv++] = (sign == -1) ? (jmin + CBSZ) : jmin;

		sp3 = &cb[jmin * VDIM];	/* sp3 points to start of best codevector */

		for (n = 0; n < VDIM; n++) {
			uq[n] = sign * (*sp3++);	/* Q0 */
		}

		/* ----------------------------------------- */
		/*  U p d a t e   F i l t e r   M e m o r y  */
		/* ----------------------------------------- */
		vq2snfc_update_mem(s + m, buf1 + m, buf2 + m, buf3 + m,
				   ltsym + MAXPP1 + m, ltnfm + MAXPP1 + m, aq,
				   fsz, fsp, uq, ppv, ltfv, gain_exp);

	}

	/* Update noise feedback filter memory after filtering current bv_subframe */
	W16copy(stsym, buf1 + FRSZ, LPCO);
	W16copy(stnfz, buf2 + FRSZ, NSTORDER);
	W16copy(stnfp, buf3 + FRSZ, NSTORDER);

	return;
}

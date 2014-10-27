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
  bv16strct.h : BV16 data structures

  $Log$
******************************************************************************/

#ifndef  BV16STRCT_H
#define  BV16STRCT_H

struct BV16_Decoder_State {
	int16_t stsym[LPCO];
	int16_t ltsym[LTMOFF];
	int16_t xq[XQOFF];
	int16_t lsppm[LPCO * LSPPORDER];
	int16_t lgpm[LGPORDER];
	int16_t lsplast[LPCO];
	int32_t prevlg[2];
	int32_t lmax;
	int32_t lmin;
	int32_t lmean;
	int32_t x1;
	int32_t level;
	int16_t pp_last;
	int16_t cfecount;
	int16_t ngfae;
	int16_t bq_last[3];
	int16_t nggalgc;
	int16_t estl_alpha_min;
	uint32_t idum;
	int16_t per;		/* Q15 */
	int32_t E;
	int16_t atplc[LPCO + 1];
	int16_t ma_a;
	int16_t b_prv[2];
	int16_t pp_prv;
};

struct BV16_Encoder_State {
	int32_t prevlg[2];
	int32_t lmax;
	int32_t lmin;
	int32_t lmean;
	int32_t x1;
	int32_t level;
	int16_t x[XOFF];		/* Signal memory */
	int16_t xwd[XDOFF];	/* Memory of DECF:1 decimated version of xw() */
	int16_t xwd_exp;		/* or block floating-point in coarptch.c */
	int16_t dq[XOFF];	/* Q0 - Quantized short-term pred error */
	int16_t dfm_h[DFO + FRSZ];	/* Decimated xwd() filter memory */
	int16_t dfm_l[DFO + FRSZ];
	int16_t stwpm[LPCO];	/* Q0 - Short-term weighting all-pole filter memory */
	int16_t stsym[LPCO];	/* Q0 - Short-term synthesis filter memory */
	int16_t stnfz[NSTORDER];	/* Q0 - Short-term noise feedback filter memory - zero section */
	int16_t stnfp[NSTORDER];	/* Q0 - Short-term noise feedback filter memory - pole section */
	int16_t ltnfm[MAXPP1];	/* Q0 - Long-term noise feedback filter memory */
	int16_t lsplast[LPCO];
	int16_t lsppm[LPCO * LSPPORDER];	/* Q15 - LSP Predictor Memory */
	int16_t lgpm[LGPORDER];	/* Q11 - Log-Gain Predictor Memory */
	int16_t cpplast;		/* Pitch period pf the previous frame */
	int16_t hpfzm[HPO];
	int16_t hpfpm[2 * HPO];
	int16_t old_A[1 + LPCO];	/* Q12 - LPC of previous frame */
};

struct BV16_Bit_Stream {
	int16_t lspidx[2];
	int16_t ppidx;
	int16_t bqidx;
	int16_t gidx;
	int16_t qvidx[FRSZ / VDIM];
};

#endif				/* BV16STRCT_H */

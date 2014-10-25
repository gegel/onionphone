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
  bvcommon.h : Common Fixed-Point Library: common #defines and prototypes

  $Log$
******************************************************************************/

#ifndef  BVCOMMON_H
#define  BVCOMMON_H

/* ----- Basic Codec Parameters ----- */

#define  LPCO  8		/* LPC Order for 8 kHz sampled lowband signal     */
#define  Ngrd     60

#define  LSPMIN  49		/* 0.00150 minimum lsp frequency */
#define  LSPMAX  32694		/* 0.99775 maximum lsp frequency */
#define  DLSPMIN 410		/* 0.01250 minimum lsp spacing */
#define  STBLDIM 3		/* dimension of stability enforcement */

/* LPC bandwidth expansion */
extern int16_t bwel[];

/* LPC to lsp Conversion */
extern int16_t bv_grid[];

/* LPC WEIGHTING FILTER */
extern int16_t STWAL[];

/* Coarse Pitch Search */
extern int16_t invk[];

/* Pitch tap codebook - actually content different for BV16 and BV32 */
extern int16_t pp9cb[];

/* Function prototypes */

void azfilter(int16_t a[],	/* (i) Q12 : prediction coefficients          */
	      int16_t m,		/* (i)     : LPC order                        */
	      int16_t x[],	/* (i) Q0  : input signal samples, incl. past */
	      int16_t y[],	/* (o) Q0  : filtered output signal           */
	      int16_t lg		/* (i)     : size of filtering                */
    );

void apfilter(int16_t a[],	/* (i) Q12 : prediction coefficients  */
	      int16_t m,		/* (i)     : LPC order                */
	      int16_t x[],	/* (i) Q0  : input signal             */
	      int16_t y[],	/* (o) Q0  : output signal            */
	      int16_t lg,	/* (i)     : size of filtering        */
	      int16_t mem[],	/* (i/o) Q0: filter memory            */
	      int16_t update	/* (i)     : memory update flag       */
    );

void lsp2a(int16_t lsp[],	/* (i) Q15 : line spectral frequencies            */
	   int16_t a[]);		/* (o) Q12 : predictor coefficients (order = 10)  */

void stblz_lsp(int16_t * lsp,	/* Q15 */
	       int16_t order);

int16_t stblchck(int16_t * x, int16_t vdim);

void a2lsp(int16_t a[],		/* (i) Q12 : predictor coefficients              */
	   int16_t lsp[],	/* (o) Q15 : line spectral frequencies           */
	   int16_t old_lsp[]);	/* (i)     : old lsp[] (in case not found 10 roots) */

void Autocorr(int32_t r[],	/* (o) : Autocorrelations      */
	      int16_t x[],	/* (i) : Input signal          */
	      int16_t window[],	/* (i) : LPC Analysis window   */
	      int16_t l_window,	/* (i) : window length         */
	      int16_t m);	/* (i) : LPC order             */

void Spectral_Smoothing(int16_t m,	/* (i)     : LPC order                    */
			int32_t rl[],	/* (i/o)   : Autocorrelations  lags       */
			int16_t lag_h[],	/* (i)     : SST coefficients  (msb)      */
			int16_t lag_l[]);	/* (i)     : SST coefficients  (lsb)   */

void Levinson(int32_t r32[],	/* (i)  : r32[] double precision vector of autocorrelation coefficients   */
	      int16_t a[],	/* (o)  : a[] in Q12 - LPC coefficients                                   */
	      int16_t old_a[],	/* (i/o): old_a[] in Q12 - previous LPC coefficients                      */
	      int16_t m);	/* (i)  : LPC order                                                       */

void pp3dec(int16_t idx, int16_t * b);

void vqdec(int16_t * xq, int16_t idx, int16_t * cb, int16_t vdim);

#endif				/* BVCOMMON_H */

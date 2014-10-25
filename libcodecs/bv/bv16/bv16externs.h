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
  bv16externs.c : BV16 Fixed-Point externs

  $Log$
******************************************************************************/

/* POINTERS */
extern int16_t winl[WINSZ];
extern int16_t sstwinl_h[];
extern int16_t sstwinl_l[];
extern int16_t gfsz[];
extern int16_t gfsp[];
extern int16_t idxord[];
extern int16_t hpfa[];
extern int16_t hpfb[];
extern int16_t adf_h[];
extern int16_t adf_l[];
extern int16_t bdf[];
extern int16_t x[];
extern int16_t x2[];
extern int16_t MPTH[];

/* LSP Quantization */
extern int16_t lspecb1[];
extern int16_t lspecb2[];
extern int16_t lspmean[];
extern int16_t lspp[];

/* Log-Gain Quantization */
extern int16_t lgpecb[];
extern int16_t lgp[];
extern int16_t lgmean;

/* Log-Gain Limitation */
extern int16_t lgclimit[];

/* Excitation Codebook */
extern int16_t cccb[];

/* Function Prototypes */
int32_t estlevel(int32_t lg, int32_t * level, int32_t * lmax,
		 int32_t * lmin, int32_t * lmean, int32_t * x1,
		int16_t ngfae, int16_t nggalgc, int16_t * estl_alpha_min);	/* Q15 */

void excdec_w_synth(int16_t * xq,	/* (o) Q0 quantized signal vector               */
		    int16_t * ltsym,	/* (i/o) Q16 quantized excitation signal vector */
		    int16_t * stsym,	/* (i/o) Q0 short-term predictor memory         */
		    int16_t * idx,	/* (o) quantizer codebook index for uq[] vector */
		    int16_t * b,	/* (i) Q15 coefficient of 3-tap pitch predictor */
		    int16_t * cb,	/* (i) Q0 codebook                              */
		    int16_t pp,	/* pitch period (# of 8 kHz samples)            */
		    int16_t * aq,	/* (i) Q12 short-term predictor coefficients    */
		    int16_t gain_exp,	/* gain_exp of current bv_sub-frame                */
		    int32_t * EE);

int32_t gaindec(int32_t * lgq,	/* Q25 */
	       int16_t gidx, int16_t * lgpm,	/* Q11 */
	       int32_t * prevlg,	/* Q25 */
	       int32_t level,	/* Q25 */
	       int16_t * nggalgc, int32_t * lg_el);

void gainplc(int32_t E, int16_t * lgeqm, int32_t * lgqm);

void lspdec(int16_t * lspq,	/* Q15 */
	    int16_t * lspidx, int16_t * lsppm,	/* Q15 */
	    int16_t * lspqlast);

void lspdecplc(int16_t * lspq,	/* Q15 */
	       int16_t * lsppm);	/* Q15 */

int16_t coarsepitch(int16_t * xw,	/* (i) Q1 weighted low-band signal frame */
		   struct BV16_Encoder_State *c);	/* (i/o) coder state */

int16_t refinepitch(int16_t * x, int16_t cpp, int16_t * ppt);

int16_t pitchtapquan(int16_t * x, int16_t pp, int16_t * b, int32_t * re);

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
	     int16_t gain_exp);

int16_t gainquan(int32_t * gainq,	/* Q18 */
		int32_t * ee,	/* Q3 */
		int16_t * lgpm,	/* Q11 */
		int32_t * prevlg,	/* Q25 */
		int32_t level);	/* Q25 */

void lspquan(int16_t * lspq, int16_t * lspidx, int16_t * lsp,
	     int16_t * lsppm);

void preprocess(struct BV16_Encoder_State *cs, int16_t * output,	/* (o) Q0 output signal, less factor 1.5  */
		int16_t * input,	/* (i) Q0 input signal                    */
		int16_t N);	/* length of signal                       */

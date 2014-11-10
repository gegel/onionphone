/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/* $Header: /cvsroot/speak-freely-u/speak_freely/gsm/src/code.c,v 1.1.1.1 2002/11/09 12:41:01 johnwalker Exp $ */

#include	"config.h"

#ifdef	HAS_STRING_H
#include	<string.h>
#else
#	include "proto.h"
extern char *memcpy P((char *, char *, int));
#endif

#include	"private.h"
#include	"gsm.h"
#include	"proto.h"

/* 
 *  4.2 FIXED POINT IMPLEMENTATION OF THE RPE-LTP CODER 
 */

void Gsm_Coder P8((S, s, LARc, Nc, bc, Mc, xmaxc, xMc), struct gsm_state *S, int16_t * s,	/* [0..159] samples                     IN      */
/*
 * The RPE-LTD coder works on a frame by frame basis.  The length of
 * the frame is equal to 160 samples.  Some computations are done
 * once per frame to produce at the output of the coder the
 * LARc[1..8] parameters which are the coded LAR coefficients and 
 * also to realize the inverse filtering operation for the entire
 * frame (160 samples of signal d[0..159]).  These parts produce at
 * the output of the coder:
 */
		  int16_t * LARc,	/* [0..7] LAR coefficients              OUT     */
/*
 * Procedure 4.2.11 to 4.2.18 are to be executed four times per
 * frame.  That means once for each sub-segment RPE-LTP analysis of
 * 40 samples.  These parts produce at the output of the coder:
 */
		  int16_t * Nc,	/* [0..3] LTP lag                       OUT     */
		  int16_t * bc,	/* [0..3] coded LTP gain                OUT     */
		  int16_t * Mc,	/* [0..3] RPE grid selection            OUT     */
		  int16_t * xmaxc,	/* [0..3] Coded maximum amplitude       OUT     */
		  int16_t * xMc	/* [13*4] normalized RPE samples        OUT     */
    )
{
	int k;
	int16_t *dp = S->dp0 + 120;	/* [ -120...-1 ] */
	int16_t *dpp = dp;		/* [ 0...39 ]    */

	static int16_t e[50] = { 0 };

	int16_t so[160];

	Gsm_Preprocess(S, s, so);
	Gsm_LPC_Analysis(S, so, LARc);
	Gsm_Short_Term_Analysis_Filter(S, LARc, so);

	for (k = 0; k <= 3; k++, xMc += 13) {

		Gsm_Long_Term_Predictor(S, so + k * 40,	/* d      [0..39] IN  */
					dp,	/* dp  [-120..-1] IN  */
					e + 5,	/* e      [0..39] OUT */
					dpp,	/* dpp    [0..39] OUT */
					Nc++, bc++);

		Gsm_RPE_Encoding(S, e + 5,	/* e      ][0..39][ IN/OUT */
				 xmaxc++, Mc++, xMc);
		/*
		 * Gsm_Update_of_reconstructed_short_time_residual_signal
		 *                      ( dpp, e + 5, dp );
		 */

		{
			register int i;
			register volatile int32_t ltmp;
			for (i = 0; i <= 39; i++)
				dp[i] = GSM_ADD(e[5 + i], dpp[i]);
		}
		dp += 40;
		dpp += 40;

	}
	(void)memcpy((char *)S->dp0, (char *)(S->dp0 + 160),
		     120 * sizeof(*S->dp0));
}

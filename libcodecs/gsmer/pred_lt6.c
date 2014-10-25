/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *
 *  FUNCTION:   w_Pred_lt_6()
 *
 *  PURPOSE:  Compute the result of long term w_prediction with fractional
 *            interpolation of resolution 1/6. (Interpolated past w_excitation).
 *
 *  DESCRIPTION:
 *       The past w_excitation signal at the given delay is interpolated at
 *       the given fraction to build the adaptive codebook w_excitation.
 *       On return w_exc[0..w_L_w_subfr-1] contains the interpolated signal
 *       (adaptive codebook w_excitation).
 *
 *************************************************************************/

#include <stdint.h>
#include "basic_op.h"
#include "count.h"

#define UP_SAMP      6
#define L_INTERPOL   10
#define FIR_SIZE     (UP_SAMP*L_INTERPOL+1)

/* 1/6 resolution interpolation filter  (-3 dB at 3600 Hz) */

static const int16_t w_inter_6[FIR_SIZE] = {
	29443,
	28346, 25207, 20449, 14701, 8693, 3143,
	-1352, -4402, -5865, -5850, -4673, -2783,
	-672, 1211, 2536, 3130, 2991, 2259,
	1170, 0, -1001, -1652, -1868, -1666,
	-1147, -464, 218, 756, 1060, 1099,
	904, 550, 135, -245, -514, -634,
	-602, -451, -231, 0, 191, 308,
	340, 296, 198, 78, -36, -120,
	-163, -165, -132, -79, -19, 34,
	73, 91, 89, 70, 38, 0
};

void w_Pred_lt_6(int16_t w_exc[],	/* in/out: w_excitation buffer */
		 int16_t T0,	/* input : integer pitch lag */
		 int16_t frac,	/* input : fraction of lag   */
		 int16_t w_L_w_subfr	/* input : w_subframe size     */
    )
{
	int16_t i, j, k;
	int16_t *x0, *x1, *x2;
	const int16_t *c1, *c2;
	int32_t s;

	x0 = &w_exc[-T0];

	frac = w_negate(frac);

	if (frac < 0) {
		frac = w_add(frac, UP_SAMP);
		x0--;
	}
	for (j = 0; j < w_L_w_subfr; j++) {
		x1 = x0++;
		x2 = x0;
		c1 = &w_inter_6[frac];
		c2 = &w_inter_6[w_sub(UP_SAMP, frac)];

		s = 0;
		for (i = 0, k = 0; i < L_INTERPOL; i++, k += UP_SAMP) {
			s = w_L_mac(s, x1[-i], c1[k]);
			s = w_L_mac(s, x2[i], c2[k]);
		}

		w_exc[j] = w_round(s);
	}

	return;
}

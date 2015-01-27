/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *
 *  FUNCTION:  w_Interpol_6()
 *
 *  PURPOSE:  Interpolating the normalized correlation with 1/6 resolution.
 *
 *************************************************************************/

#include <stdint.h>
#include "basic_op.h"


#define UP_SAMP      6
#define L_INTERPOL   4
#define FIR_SIZE     (UP_SAMP*L_INTERPOL+1)

/* 1/6 resolution interpolation filter  (-3 dB at 3600 Hz) */

static const int16_t w_inter_6[FIR_SIZE] = {
	29519,
	28316, 24906, 19838, 13896, 7945, 2755,
	-1127, -3459, -4304, -3969, -2899, -1561,
	-336, 534, 970, 1023, 823, 516,
	220, 0, -131, -194, -215, 0
};

int16_t w_Interpol_6(int16_t * x,	/* (i)  : input vector        */
			   int16_t frac	/* (i)  : fraction            */
    )
{
	int16_t i, k;
	int16_t *x1, *x2;
	const int16_t *c1, *c2;
	int32_t s;

	if (frac < 0) {
		frac = w_add(frac, UP_SAMP);
		x--;
	}
	x1 = &x[0];
	x2 = &x[1];
	c1 = &w_inter_6[frac];
	c2 = &w_inter_6[w_sub(UP_SAMP, frac)];

	s = 0;
	for (i = 0, k = 0; i < L_INTERPOL; i++, k += UP_SAMP) {
		s = w_L_mac(s, x1[-i], c1[k]);
		s = w_L_mac(s, x2[i], c2[k]);
	}

	return w_round(s);
}

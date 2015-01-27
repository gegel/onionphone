/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *
 *  FUNCTION:   w_Pitch_fr6()
 *
 *  PURPOSE: Find the pitch period with 1/6 w_subsample resolution (closed loop).
 *
 *  DESCRIPTION:
 *        - find the normalized correlation between the target and filtered
 *          past w_excitation in the search range.
 *        - select the delay with maximum normalized correlation.
 *        - interpolate the normalized correlation at fractions -3/6 to 3/6
 *          with step 1/6 aw_round the chosen delay.
 *        - The fraction which gives the maximum interpolated value is chosen.
 *
 *************************************************************************/

#include <stdint.h>
#include "basic_op.h"
#include "oper_32b.h"

#include "sig_proc.h"
#include "codec.h"

 /* L_inter = Length for fractional interpolation = nb.coeff/2 */

#define L_inter 4

 /* Local functions */

void w_Norm_Corr(int16_t w_exc[], int16_t xn[], int16_t h[],
		 int16_t w_L_w_subfr,
		 int16_t t_min, int16_t t_max, int16_t corr_norm[]);

int16_t w_Pitch_fr6(int16_t w_exc[],	/* (i)     : w_excitation buffer                      */
			  int16_t xn[],	/* (i)     : target vector                          */
			  int16_t h[],	/* (i)     : impulse response of w_w_synthesis and
					   weighting filters                     */
			  int16_t w_L_w_subfr,	/* (i)     : Length of w_subframe                     */
			  int16_t t0_min,	/* (i)     : minimum value in the searched range.   */
			  int16_t t0_max,	/* (i)     : maximum value in the searched range.   */
			  int16_t i_w_subfr,	/* (i)     : indicator for first w_subframe.          */
			  int16_t * pit_frac	/* (o)     : chosen fraction.                       */
    )
{
	int16_t i;
	int16_t t_min, t_max;
	int16_t max, lag, frac;
	int16_t *corr;
	int16_t corr_int;
	int16_t corr_v[40];	/* Total length = t0_max-t0_min+1+2*L_inter */

	/* Find interval to compute normalized correlation */

	t_min = w_sub(t0_min, L_inter);
	t_max = w_add(t0_max, L_inter);

	corr = &corr_v[-t_min];

	/* Compute normalized correlation between target and filtered w_excitation */

	w_Norm_Corr(w_exc, xn, h, w_L_w_subfr, t_min, t_max, corr);

	/* Find integer pitch */

	max = corr[t0_min];
	lag = t0_min;

	for (i = t0_min + 1; i <= t0_max; i++) {

		if (w_sub(corr[i], max) >= 0) {
			max = corr[i];
			lag = i;
		}
	}

	/* If first w_subframe and lag > 94 do not search fractional pitch */

	if ((i_w_subfr == 0) && (w_sub(lag, 94) > 0)) {
		*pit_frac = 0;
		return (lag);
	}
	/* Test the fractions aw_round T0 and choose the one which maximizes   */
	/* the interpolated normalized correlation.                          */

	max = w_Interpol_6(&corr[lag], -3);
	frac = -3;

	for (i = -2; i <= 3; i++) {
		corr_int = w_Interpol_6(&corr[lag], i);

		if (w_sub(corr_int, max) > 0) {
			max = corr_int;
			frac = i;
		}
	}

	/* Limit the fraction value in the interval [-2,-1,0,1,2,3] */

	if (w_sub(frac, -3) == 0) {
		frac = 3;
		lag = w_sub(lag, 1);
	}
	*pit_frac = frac;

	return (lag);
}

/*************************************************************************
 *
 *  FUNCTION:   w_Norm_Corr()
 *
 *  PURPOSE: Find the normalized correlation between the target vector
 *           and the filtered past w_excitation.
 *
 *  DESCRIPTION:
 *     The normalized correlation is given by the correlation between the
 *     target and filtered past w_excitation divided by the square root of
 *     the energy of filtered w_excitation.
 *                   corr[k] = <x[], y_k[]>/sqrt(y_k[],y_k[])
 *     where x[] is the target vector and y_k[] is the filtered past
 *     w_excitation at delay k.
 *
 *************************************************************************/

void
w_Norm_Corr(int16_t w_exc[], int16_t xn[], int16_t h[], int16_t w_L_w_subfr,
	    int16_t t_min, int16_t t_max, int16_t corr_norm[])
{
	int16_t i, j, k;
	int16_t corr_h, corr_l, norm_h, w_norm_l;
	int32_t s;

	/* Usally dynamic allocation of (w_L_w_subfr) */
	int16_t w_excf[80];
	int16_t scaling, h_fac, *s_w_excf, scaled_w_excf[80];

	k = -t_min;

	/* compute the filtered w_excitation for the first delay t_min */

	w_Convolve(&w_exc[k], h, w_excf, w_L_w_subfr);

	/* scale "w_excf[]" to avoid overflow */

	for (j = 0; j < w_L_w_subfr; j++) {
		scaled_w_excf[j] = w_shr(w_excf[j], 2);
	}

	/* Compute 1/sqrt(energy of w_excf[]) */

	s = 0;
	for (j = 0; j < w_L_w_subfr; j++) {
		s = w_L_mac(s, w_excf[j], w_excf[j]);
	}

	if (w_L_w_sub(s, 67108864L) <= 0) {	/* if (s <= 2^26) */
		s_w_excf = w_excf;
		h_fac = 15 - 12;
		scaling = 0;
	} else {
		/* "w_excf[]" is divided by 2 */
		s_w_excf = scaled_w_excf;
		h_fac = 15 - 12 - 2;
		scaling = 2;
	}

	/* loop for every possible period */

	for (i = t_min; i <= t_max; i++) {
		/* Compute 1/sqrt(energy of w_excf[]) */

		s = 0;
		for (j = 0; j < w_L_w_subfr; j++) {
			s = w_L_mac(s, s_w_excf[j], s_w_excf[j]);
		}

		s = w_Inv_sqrt(s);
		w_L_Extract(s, &norm_h, &w_norm_l);

		/* Compute correlation between xn[] and w_excf[] */

		s = 0;
		for (j = 0; j < w_L_w_subfr; j++) {
			s = w_L_mac(s, xn[j], s_w_excf[j]);
		}
		w_L_Extract(s, &corr_h, &corr_l);

		/* Normalize correlation = correlation * (1/sqrt(energy)) */

		s = w_Mpy_32(corr_h, corr_l, norm_h, w_norm_l);

		corr_norm[i] = w_extract_h(w_L_w_shl(s, 16));

		/* modify the filtered w_excitation w_excf[] for the next iteration */

		if (w_sub(i, t_max) != 0) {
			k--;
			for (j = w_L_w_subfr - 1; j > 0; j--) {
				s = w_L_w_mult(w_exc[k], h[j]);
				s = w_L_w_shl(s, h_fac);
				s_w_excf[j] =
				    w_add(w_extract_h(s), s_w_excf[j - 1]);
			}
			s_w_excf[0] = w_shr(w_exc[k], scaling);
		}
	}
	return;
}

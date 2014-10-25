/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *
 *  FUNCTION:  w_G_pitch
 *
 *  PURPOSE:  Compute the pitch (adaptive codebook) gain. Result in Q12
 *
 *  DESCRIPTION:
 *      The adaptive codebook gain is given by
 *
 *              g = <x[], y[]> / <y[], y[]>
 *
 *      where x[] is the target vector, y[] is the filtered adaptive
 *      codevector, and <> denotes dot product.
 *      The gain is limited to the range [0,1.2]
 *
 *************************************************************************/

#include <stdint.h>
#include "basic_op.h"
#include "oper_32b.h"
#include "count.h"
#include "sig_proc.h"

int16_t w_G_pitch(int16_t xn[],	/* (i)   : Pitch target.                           */
			int16_t y1[],	/* (i)   : Filtered adaptive codebook.             */
			int16_t w_L_w_subfr	/*       : Length of w_subframe.                     */
    )
{
	int16_t i;
	int16_t xy, yy, exp_xy, exp_yy, gain;
	int32_t s;

	int16_t scaled_y1[80];	/* Usually dynamic allocation of (w_L_w_subfr) */

	/* divide by 2 "y1[]" to avoid overflow */

	for (i = 0; i < w_L_w_subfr; i++) {
		scaled_y1[i] = w_shr(y1[i], 2);
	}

	/* Compute scalar product <y1[],y1[]> */

	s = 0L;			/* Avoid case of all w_zeros */
	for (i = 0; i < w_L_w_subfr; i++) {
		s = w_L_mac(s, y1[i], y1[i]);
	}

	if (w_L_w_sub(s, MAX_32) != 0L) {	/* Test for overflow */
		s = L_w_add(s, 1L);	/* Avoid case of all w_zeros */
		exp_yy = w_norm_l(s);
		yy = w_round(w_L_w_shl(s, exp_yy));
	} else {
		s = 1L;		/* Avoid case of all w_zeros */
		for (i = 0; i < w_L_w_subfr; i++) {
			s = w_L_mac(s, scaled_y1[i], scaled_y1[i]);
		}
		exp_yy = w_norm_l(s);
		yy = w_round(w_L_w_shl(s, exp_yy));
		exp_yy = w_sub(exp_yy, 4);
	}

	/* Compute scalar product <xn[],y1[]> */

	w_Overflow = 0;
	s = 1L;			/* Avoid case of all w_zeros */
	for (i = 0; i < w_L_w_subfr; i++) {
		w_Carry = 0;
		s = w_w_L_macNs(s, xn[i], y1[i]);

		if (w_Overflow != 0) {
			break;
		}
	}

	if (w_Overflow == 0) {
		exp_xy = w_norm_l(s);
		xy = w_round(w_L_w_shl(s, exp_xy));
	} else {
		s = 1L;		/* Avoid case of all w_zeros */
		for (i = 0; i < w_L_w_subfr; i++) {
			s = w_L_mac(s, xn[i], scaled_y1[i]);
		}
		exp_xy = w_norm_l(s);
		xy = w_round(w_L_w_shl(s, exp_xy));
		exp_xy = w_sub(exp_xy, 2);
	}

	/* If (xy < 4) gain = 0 */

	i = w_sub(xy, 4);

	if (i < 0)
		return ((int16_t) 0);

	/* compute gain = xy/yy */

	xy = w_shr(xy, 1);	/* Be sure xy < yy */
	gain = w_div_s(xy, yy);

	i = w_add(exp_xy, 3 - 1);	/* Denormalization of division */
	i = w_sub(i, exp_yy);

	gain = w_shr(gain, i);

	/* if(gain >1.2) gain = 1.2 */

	if (w_sub(gain, 4915) > 0) {
		gain = 4915;
	}
	return (gain);
}

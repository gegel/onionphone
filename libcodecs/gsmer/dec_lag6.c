/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *   FUNCTION:   w_Dec_lag6
 *
 *   PURPOSE:  Decoding of fractional pitch lag with 1/6 resolution.
 *             Extract the integer and fraction parts of the pitch lag from
 *             the received adaptive codebook index.
 *
 *    See "w_Enc_lag6.c" for more details about the encoding procedure.
 *
 *    The fractional lag in 1st and 3rd w_subframes is encoded with 9 bits
 *    while that in 2nd and 4th w_subframes is relatively encoded with 6 bits.
 *    Note that in relative encoding only 61 values are used. If the
 *    decoder receives 61, 62, or 63 as the relative pitch index, it means
 *    that a transmission w_error occurred. In this case, the pitch lag from
 *    previous w_subframe is used.
 *
 *************************************************************************/

#include "ophint.h"
#include "basic_op.h"
#include "count.h"

/* Old integer lag */

Word16 w_old_T0;

Word16 w_Dec_lag6(		/* output: return integer pitch lag       */
			 Word16 index,	/* input : received pitch index           */
			 Word16 pit_min,	/* input : minimum pitch lag              */
			 Word16 pit_max,	/* input : maximum pitch lag              */
			 Word16 i_w_subfr,	/* input : w_subframe flag                  */
			 Word16 L_frame_by2,	/* input : w_speech frame size divided by 2 */
			 Word16 * T0_frac,	/* output: fractional part of pitch lag   */
			 Word16 bfi	/* input : bad frame indicator            */
    )
{
	Word16 pit_flag;
	Word16 T0, i;
	static Word16 T0_min, T0_max;

	pit_flag = i_w_subfr;	/* flag for 1st or 3rd w_subframe */

	if (w_sub(i_w_subfr, L_frame_by2) == 0) {
		pit_flag = 0;
	}

	if (pit_flag == 0) {	/* if 1st or 3rd w_subframe */

		if (bfi == 0) {	/* if bfi == 0 decode pitch */

			if (w_sub(index, 463) < 0) {
				/* T0 = (index+5)/6 + 17 */
				T0 = w_add(w_mult(w_add(index, 5), 5462), 17);
				i = w_add(w_add(T0, T0), T0);
				/* *T0_frac = index - T0*6 + 105 */
				*T0_frac =
				    w_add(w_sub(index, w_add(i, i)), 105);

			} else {
				T0 = w_sub(index, 368);
				*T0_frac = 0;
			}
		} else
			/* bfi == 1 */
		{
			T0 = w_old_T0;
			*T0_frac = 0;
		}

		/* find T0_min and T0_max for 2nd (or 4th) w_subframe */

		T0_min = w_sub(T0, 5);

		if (w_sub(T0_min, pit_min) < 0) {
			T0_min = pit_min;
		}
		T0_max = w_add(T0_min, 9);

		if (w_sub(T0_max, pit_max) > 0) {
			T0_max = pit_max;
			T0_min = w_sub(T0_max, 9);
		}
	} else
		/* second or fourth w_subframe */
	{

		/* if bfi == 0 decode pitch */
		if ((bfi == 0) && (w_sub(index, 61) < 0)) {
			/* i = (index+5)/6 - 1 */
			i = w_sub(w_mult(w_add(index, 5), 5462), 1);
			T0 = w_add(i, T0_min);
			i = w_add(w_add(i, i), i);
			*T0_frac = w_sub(w_sub(index, 3), w_add(i, i));

		} else
			/* bfi == 1  OR index >= 61 */
		{
			T0 = w_old_T0;
			*T0_frac = 0;
		}
	}

	w_old_T0 = T0;

	return T0;
}

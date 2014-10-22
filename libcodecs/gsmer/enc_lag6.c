/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *
 *   FUNCTION:  w_Enc_lag6
 *
 *   PURPOSE:  Encoding of fractional pitch lag with 1/6 resolution.
 *
 *   DESCRIPTION:
 *                    First and third w_subframes:
 *                    --------------------------
 *   The pitch range is divided as follows:
 *           17 3/6  to   94 3/6   resolution 1/6
 *           95      to   143      resolution 1
 *
 *   The period is encoded with 9 bits.
 *   For the range with fractions:
 *     index = (T-17)*6 + frac - 3;
 *                         where T=[17..94] and frac=[-2,-1,0,1,2,3]
 *   and for the integer only range
 *     index = (T - 95) + 463;        where T=[95..143]
 *
 *                    Second and fourth w_subframes:
 *                    ----------------------------
 *   For the 2nd and 4th w_subframes a resolution of 1/6 is always used,
 *   and the search range is relative to the lag in previous w_subframe.
 *   If t0 is the lag in the previous w_subframe then
 *   t_min=t0-5   and  t_max=t0+4   and  the range is given by
 *       (t_min-1) 3/6   to  (t_max) 3/6
 *
 *   The period in the 2nd (and 4th) w_subframe is encoded with 6 bits:
 *     index = (T-(t_min-1))*6 + frac - 3;
 *                 where T=[t_min-1..t_max] and frac=[-2,-1,0,1,2,3]
 *
 *   Note that only 61 values are used. If the decoder receives 61, 62,
 *   or 63 as the relative pitch index, it means that a transmission
 *   w_error occurred and the pitch from previous w_subframe should be used.
 *
 *************************************************************************/

#include "ophint.h"
#include "basic_op.h"
#include "count.h"

Word16 w_Enc_lag6(		/* output: Return index of encoding     */
			 Word16 T0,	/* input : Pitch delay                  */
			 Word16 * T0_frac,	/* in/out: Fractional pitch delay       */
			 Word16 * T0_min,	/* in/out: Minimum search delay         */
			 Word16 * T0_max,	/* in/out: Maximum search delay         */
			 Word16 pit_min,	/* input : Minimum pitch delay          */
			 Word16 pit_max,	/* input : Maximum pitch delay          */
			 Word16 pit_flag	/* input : Flag for 1st or 3rd w_subframe */
    )
{
	Word16 index, i;

	if (pit_flag == 0) {	/* if 1st or 3rd w_subframe */
		/* encode pitch delay (with fraction) */

		if (w_sub(T0, 94) <= 0) {
			/* index = T0*6 - 105 + *T0_frac */
			i = w_add(w_add(T0, T0), T0);
			index = w_add(w_sub(w_add(i, i), 105), *T0_frac);
		} else {	/* set fraction to 0 for delays > 94 */
			*T0_frac = 0;
			index = w_add(T0, 368);
		}

		/* find T0_min and T0_max for second (or fourth) w_subframe */

		*T0_min = w_sub(T0, 5);

		if (w_sub(*T0_min, pit_min) < 0) {
			*T0_min = pit_min;
		}
		*T0_max = w_add(*T0_min, 9);

		if (w_sub(*T0_max, pit_max) > 0) {
			*T0_max = pit_max;
			*T0_min = w_sub(*T0_max, 9);
		}
	} else
		/* if second or fourth w_subframe */
	{
		/* index = 6*(T0-*T0_min) + 3 + *T0_frac  */
		i = w_sub(T0, *T0_min);
		i = w_add(w_add(i, i), i);
		index = w_add(w_add(w_add(i, i), 3), *T0_frac);
	}

	return index;
}

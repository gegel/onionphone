/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *
 *  FUNCTION:  w_Pitch_ol
 *
 *  PURPOSE: Compute the open loop pitch lag.
 *
 *  DESCRIPTION:
 *      The open-loop pitch lag is determined based on the perceptually
 *      weighted w_speech signal. This is done in the following steps:
 *        - find three maxima of the correlation <sw[n],sw[n-T]> in the
 *          follwing three ranges of T : [18,35], [36,71], and [72, 143]
 *        - divide each maximum by <sw[n-t], sw[n-t]> where t is the delay at
 *          that maximum correlation.
 *        - select the delay of maximum normalized correlation (among the
 *          three candidates) while favoring the lower delay ranges.
 *
 *************************************************************************/

#include <stdint.h>
#include "basic_op.h"
#include "oper_32b.h"

#include "sig_proc.h"

#define THRESHOLD 27853

/* local function */

static int16_t w_Lag_max(int16_t scal_sig[],	/* input : scaled signal                          */
			       int16_t scal_fac,	/* input : scaled signal factor                   */
			       int16_t L_frame,	/* input : length of frame to compute pitch       */
			       int16_t lag_max,	/* input : maximum lag                            */
			       int16_t lag_min,	/* input : minimum lag                            */
			       int16_t * cor_max);	/* output: normalized correlation of selected lag */

int16_t w_Pitch_ol(int16_t signal[],	/* input : signal used to compute the open loop pitch */
			 /*     signal[-pit_max] to signal[-1] should be known */
			 int16_t pit_min,	/* input : minimum pitch lag                          */
			 int16_t pit_max,	/* input : maximum pitch lag                          */
			 int16_t L_frame	/* input : length of frame to compute pitch           */
    )
{
	int16_t i, j;
	int16_t max1, max2, max3;
	int16_t p_max1, p_max2, p_max3;
	int32_t t0;

	/* Scaled signal                                                */
	/* Can be allocated with memory allocation of(pit_max+L_frame)  */

	int16_t scaled_signal[512];
	int16_t *scal_sig, scal_fac;

	scal_sig = &scaled_signal[pit_max];

	t0 = 0L;
	for (i = -pit_max; i < L_frame; i++) {
		t0 = w_L_mac(t0, signal[i], signal[i]);
	}
    /*--------------------------------------------------------*
     * Scaling of input signal.                               *
     *                                                        *
     *   if w_Overflow        -> scal_sig[i] = signal[i]>>2     *
     *   else if t0 < 1^22  -> scal_sig[i] = signal[i]<<2     *
     *   else               -> scal_sig[i] = signal[i]        *
     *--------------------------------------------------------*/

    /*--------------------------------------------------------*
     *  Verification for risk of overflow.                    *
     *--------------------------------------------------------*/

	if (w_L_w_sub(t0, MAX_32) == 0L) {	/* Test for overflow */
		for (i = -pit_max; i < L_frame; i++) {
			scal_sig[i] = w_shr(signal[i], 3);
		}
		scal_fac = 3;
	} else if (w_L_w_sub(t0, (int32_t) 1048576L) < (int32_t) 0)
		/* if (t0 < 2^20) */
	{
		for (i = -pit_max; i < L_frame; i++) {
			scal_sig[i] = w_shl(signal[i], 3);
		}
		scal_fac = -3;
	} else {
		for (i = -pit_max; i < L_frame; i++) {
			scal_sig[i] = signal[i];
		}
		scal_fac = 0;
	}

    /*--------------------------------------------------------------------*
     *  The pitch lag search is divided in three sections.                *
     *  Each section cannot have a pitch w_multiple.                        *
     *  We find a maximum for each section.                               *
     *  We compare the maximum of each section by favoring small lags.    *
     *                                                                    *
     *  First section:  lag delay = pit_max     downto 4*pit_min          *
     *  Second section: lag delay = 4*pit_min-1 downto 2*pit_min          *
     *  Third section:  lag delay = 2*pit_min-1 downto pit_min            *
     *-------------------------------------------------------------------*/

	j = w_shl(pit_min, 2);
	p_max1 = w_Lag_max(scal_sig, scal_fac, L_frame, pit_max, j, &max1);

	i = w_sub(j, 1);
	j = w_shl(pit_min, 1);
	p_max2 = w_Lag_max(scal_sig, scal_fac, L_frame, i, j, &max2);

	i = w_sub(j, 1);
	p_max3 = w_Lag_max(scal_sig, scal_fac, L_frame, i, pit_min, &max3);

    /*--------------------------------------------------------------------*
     * Compare the 3 sections maximum, and favor small lag.               *
     *-------------------------------------------------------------------*/

	if (w_sub(w_mult(max1, THRESHOLD), max2) < 0) {
		max1 = max2;
		p_max1 = p_max2;
	}

	if (w_sub(w_mult(max1, THRESHOLD), max3) < 0) {
		p_max1 = p_max3;
	}
	return (p_max1);
}

/*************************************************************************
 *
 *  FUNCTION:  w_Lag_max
 *
 *  PURPOSE: Find the lag that has maximum correlation of scal_sig[] in a
 *           given delay range.
 *
 *  DESCRIPTION:
 *      The correlation is given by
 *           cor[t] = <scal_sig[n],scal_sig[n-t]>,  t=lag_min,...,lag_max
 *      The functions outputs the maximum correlation after normalization
 *      and the corresponding lag.
 *
 *************************************************************************/

static int16_t w_Lag_max(int16_t scal_sig[],	/* input : scaled signal.                          */
			       int16_t scal_fac,	/* input : scaled signal factor.                   */
			       int16_t L_frame,	/* input : length of frame to compute pitch        */
			       int16_t lag_max,	/* input : maximum lag                             */
			       int16_t lag_min,	/* input : minimum lag                             */
			       int16_t * cor_max)
{				/* output: normalized correlation of selected lag  */
	int16_t i, j;
	int16_t *p, *p1;
	int32_t max, t0;
	int16_t max_h, max_l, ener_h, ener_l;
	int16_t p_max = 0;

	max = MIN_32;

	for (i = lag_max; i >= lag_min; i--) {
		p = scal_sig;
		p1 = &scal_sig[-i];
		t0 = 0;

		for (j = 0; j < L_frame; j++, p++, p1++) {
			t0 = w_L_mac(t0, *p, *p1);
		}

		if (w_L_w_sub(t0, max) >= 0) {
			max = t0;
			p_max = i;
		}
	}

	/* compute energy */

	t0 = 0;
	p = &scal_sig[-p_max];
	for (i = 0; i < L_frame; i++, p++) {
		t0 = w_L_mac(t0, *p, *p);
	}
	/* 1/sqrt(energy) */

	t0 = w_Inv_sqrt(t0);
	t0 = w_L_w_shl(t0, 1);

	/* max = max/sqrt(energy)  */

	w_L_Extract(max, &max_h, &max_l);
	w_L_Extract(t0, &ener_h, &ener_l);

	t0 = w_Mpy_32(max_h, max_l, ener_h, ener_l);
	t0 = w_L_w_shr(t0, scal_fac);

	*cor_max = w_extract_h(w_L_w_shl(t0, 15));	/* divide by 2 */

	return (p_max);
}

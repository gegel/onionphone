/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/***************************************************************************
 *
 *   File Name: vad.c
 *
 *   Purpose:   Contains all functions for voice activity detection, as
 *              described in the high level specification of VAD.
 *
 *     Below is a listing of all the functions appearing in the file.
 *     The functions are arranged according to their purpose.  Under
 *     each heading, the ordering is hierarchical.
 *
 *     Resetting of static variables of VAD:
 *       reset_vad()
 *
 *     Main routine of VAD (called by the w_speech encoder):
 *       w_vad_computation()
 *       Adaptive filtering and energy computation:
 *         w_er_energy_computation()
 *       Averaging of autocorrelation function values:
 *         w_acf_averaging()
 *       Computation of w_predictor values:
 *         w_er_w_predictor_values()
 *           w_er_schur_recursion()
 *           w_er_step_up()
 *           w_er_compute_rav1()
 *       Spectral comparison:
 *         w_er_spectral_comparison()
 *       Information tone detection:
 *         w_er_tone_detection()
 *           w_er_step_up()
 *       Threshold adaptation:
 *         w_er_threshold_adaptation()
 *       VAD decision:
 *         w_er_vad_decision()
 *       VAD hangover w_addition:
 *         w_er_vad_hangover()
 *
 *     Periodicity detection routine (called by the w_speech encoder):
 *       periodicity_detection()
 *
 **************************************************************************/

#include "ophint.h"
#include "cnst.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "count.h"
#include "vad.h"

/* Constants of VAD hangover w_addition */

#define HANGCONST 10
#define BURSTCONST 3

/* Constant of spectral comparison */

#define STAT_THRESH 3670L	/* 0.056 */

/* Constants of periodicity detection */

#define LTHRESH 2
#define NTHRESH 4

/* Pseudo floating point representations of constants
   for threshold adaptation */

#define M_PTH    32500		/*** 130000.0 ***/
#define E_PTH    17
#define M_PLEV   21667		/*** 346666.7 ***/
#define E_PLEV   19
#define M_MARGIN 16927		/*** 69333340 ***/
#define E_MARGIN 27

#define FAC 17203		/* 2.1 */

/* Constants of tone detection */

#define FREQTH 3189
#define PREDTH 1464

/* Static variables of VAD */

static Word16 w_rvad[9], scal_w_rvad;
static Pfloat w_thvad;
static Word32 w_L_sacf[27];
static Word32 w_L_sav0[36];
static Word16 w_pt_sacf, w_pt_sav0;
static Word32 w_L_lastdm;
static Word16 w_adaptcount;
static Word16 w_burstcount, w_hangcount;
static Word16 oldlw_agcount, veryoldlw_agcount, w_oldlag;

Word16 w_ptch;

/*************************************************************************
 *
 *   FUNCTION NAME: w_er_vad_reset
 *
 *   PURPOSE:  Resets the static variables of the VAD to their
 *             initial values
 *
 *************************************************************************/

void w_er_vad_reset()
{
	Word16 i;

	/* Initialize w_rvad variables */
	w_rvad[0] = 0x6000;
	for (i = 1; i < 9; i++) {
		w_rvad[i] = 0;
	}
	scal_w_rvad = 7;

	/* Initialize threshold level */
	w_thvad.e = 20;		  /*** exponent ***/
	w_thvad.m = 27083;	  /*** mantissa ***/

	/* Initialize ACF averaging variables */
	for (i = 0; i < 27; i++) {
		w_L_sacf[i] = 0L;
	}
	for (i = 0; i < 36; i++) {
		w_L_sav0[i] = 0L;
	}
	w_pt_sacf = 0;
	w_pt_sav0 = 0;

	/* Initialize spectral comparison variable */
	w_L_lastdm = 0L;

	/* Initialize threshold adaptation variable */
	w_adaptcount = 0;

	/* Initialize VAD hangover w_addition variables */
	w_burstcount = 0;
	w_hangcount = -1;

	/* Initialize periodicity detection variables */
	oldlw_agcount = 0;
	veryoldlw_agcount = 0;
	w_oldlag = 18;

	w_ptch = 1;

	return;
}

/****************************************************************************
 *
 *     FUNCTION:  w_vad_computation
 *
 *     PURPOSE:   Returns a decision as to whether the current frame being
 *                processed by the w_speech encoder contains w_speech or not.
 *
 *     INPUTS:    r_h[0..8]     autocorrelation of input signal frame (msb)
 *                r_l[0..8]     autocorrelation of input signal frame (lsb)
 *                scal_acf      scaling factor for the autocorrelations
 *                rc[0..3]      w_speech encoder reflection coefficients
 *                w_ptch          flag to indicate a periodic signal component
 *
 *     OUTPUTS:   none
 *
 *     RETURN VALUE: vad decision
 *
 ***************************************************************************/

Word16 w_vad_computation(Word16 r_h[],
			 Word16 r_l[],
			 Word16 scal_acf, Word16 rc[], Word16 w_ptch)
{
	Word32 L_av0[9], L_av1[9];
	Word16 vad, vvad, rav1[9], scal_rav1, stat, tone;
	Pfloat acf0, pvad;

	w_er_energy_computation(r_h, scal_acf, w_rvad, scal_w_rvad, &acf0,
				&pvad);
	w_acf_averaging(r_h, r_l, scal_acf, L_av0, L_av1);
	w_er_w_predictor_values(L_av1, rav1, &scal_rav1);
	stat = w_er_spectral_comparison(rav1, scal_rav1, L_av0);
	w_er_tone_detection(rc, &tone);
	w_er_threshold_adaptation(stat, w_ptch, tone, rav1, scal_rav1, pvad,
				  acf0, w_rvad, &scal_w_rvad, &w_thvad);
	vvad = w_er_vad_decision(pvad, w_thvad);
	vad = w_er_vad_hangover(vvad);

	return vad;
}

/****************************************************************************
 *
 *     FUNCTION:  w_er_energy_computation
 *
 *     PURPOSE:   Computes the input and residual energies of the adaptive
 *                filter in a floating point representation.
 *
 *     INPUTS:    r_h[0..8]      autocorrelation of input signal frame (msb)
 *                scal_acf       scaling factor for the autocorrelations
 *                w_rvad[0..8]     autocorrelated adaptive filter coefficients
 *                scal_w_rvad      scaling factor for w_rvad[]
 *
 *     OUTPUTS:   *acf0          signal frame energy (mantissa+exponent)
 *                *pvad          filtered signal energy (mantissa+exponent)
 *
 *     RETURN VALUE: none
 *
 ***************************************************************************/

void w_er_energy_computation(Word16 r_h[],
			     Word16 scal_acf,
			     Word16 w_rvad[],
			     Word16 scal_w_rvad, Pfloat * acf0, Pfloat * pvad)
{
	Word16 i, temp, norm_prod;
	Word32 L_temp;

	/* r[0] is always greater than w_zero (no need to w_test for r[0] == 0) */

	/* Computation of acf0 (exponent and mantissa) */

	acf0->e = w_sub(32, scal_acf);
	acf0->m = r_h[0] & 0x7ff8;

	/* Computation of pvad (exponent and mantissa) */

	pvad->e = w_add(acf0->e, 14);
	pvad->e = w_sub(pvad->e, scal_w_rvad);

	L_temp = 0L;

	for (i = 1; i <= 8; i++) {
		temp = w_shr(r_h[i], 3);
		L_temp = w_L_mac(L_temp, temp, w_rvad[i]);
	}

	temp = w_shr(r_h[0], 3);
	L_temp = L_w_add(L_temp, w_L_w_shr(w_L_w_mult(temp, w_rvad[0]), 1));

	if (L_temp <= 0L) {
		L_temp = 1L;
	}
	norm_prod = w_norm_l(L_temp);
	pvad->e = w_sub(pvad->e, norm_prod);
	pvad->m = w_extract_h(w_L_w_shl(L_temp, norm_prod));

	return;
}

/****************************************************************************
 *
 *     FUNCTION:  w_acf_averaging
 *
 *     PURPOSE:   Computes the arrays L_av0[0..8] and L_av1[0..8].
 *
 *     INPUTS:    r_h[0..8]     autocorrelation of input signal frame (msb)
 *                r_l[0..8]     autocorrelation of input signal frame (lsb)
 *                scal_acf      scaling factor for the autocorrelations
 *
 *     OUTPUTS:   L_av0[0..8]   ACF averaged over last four frames
 *                L_av1[0..8]   ACF averaged over previous four frames
 *
 *     RETURN VALUE: none
 *
 ***************************************************************************/

void w_acf_averaging(Word16 r_h[],
		     Word16 r_l[],
		     Word16 scal_acf, Word32 L_av0[], Word32 L_av1[]
    )
{
	Word32 L_temp;
	Word16 scale;
	Word16 i;

	scale = w_add(9, scal_acf);

	for (i = 0; i <= 8; i++) {
		L_temp = w_L_w_shr(w_L_Comp(r_h[i], r_l[i]), scale);
		L_av0[i] = L_w_add(w_L_sacf[i], L_temp);
		L_av0[i] = L_w_add(w_L_sacf[i + 9], L_av0[i]);
		L_av0[i] = L_w_add(w_L_sacf[i + 18], L_av0[i]);
		w_L_sacf[w_pt_sacf + i] = L_temp;
		L_av1[i] = w_L_sav0[w_pt_sav0 + i];
		w_L_sav0[w_pt_sav0 + i] = L_av0[i];
	}

	/* Update the array pointers */

	if (w_sub(w_pt_sacf, 18) == 0) {
		w_pt_sacf = 0;
	} else {
		w_pt_sacf = w_add(w_pt_sacf, 9);
	}

	if (w_sub(w_pt_sav0, 27) == 0) {
		w_pt_sav0 = 0;
	} else {
		w_pt_sav0 = w_add(w_pt_sav0, 9);
	}

	return;
}

/****************************************************************************
 *
 *     FUNCTION:  w_er_w_predictor_values
 *
 *     PURPOSE:   Computes the array rav[0..8] needed for the spectral
 *                comparison and the threshold adaptation.
 *
 *     INPUTS:    L_av1[0..8]   ACF averaged over previous four frames
 *
 *     OUTPUTS:   rav1[0..8]    ACF obtained from L_av1
 *                *scal_rav1    rav1[] scaling factor
 *
 *     RETURN VALUE: none
 *
 ***************************************************************************/

void w_er_w_predictor_values(Word32 L_av1[], Word16 rav1[], Word16 * scal_rav1)
{
	Word16 vpar[8], aav1[9];

	w_er_schur_recursion(L_av1, vpar);
	w_er_step_up(8, vpar, aav1);
	w_er_compute_rav1(aav1, rav1, scal_rav1);

	return;
}

/****************************************************************************
 *
 *     FUNCTION:  w_er_schur_recursion
 *
 *     PURPOSE:   Uses the Schur recursion to compute adaptive filter
 *                reflection coefficients from an autorrelation function.
 *
 *     INPUTS:    L_av1[0..8]    autocorrelation function
 *
 *     OUTPUTS:   vpar[0..7]     reflection coefficients
 *
 *     RETURN VALUE: none
 *
 ***************************************************************************/

void w_er_schur_recursion(Word32 L_av1[], Word16 vpar[]
    )
{
	Word16 acf[9], pp[9], kk[9], temp;
	Word16 i, k, m, n;

    /*** Schur recursion with 16-bit arithmetic ***/

	if (L_av1[0] == 0) {
		for (i = 0; i < 8; i++) {
			vpar[i] = 0;
		}
		return;
	}
	temp = w_norm_l(L_av1[0]);

	for (k = 0; k <= 8; k++) {
		acf[k] = w_extract_h(w_L_w_shl(L_av1[k], temp));
	}

    /*** Initialize arrays pp[..] and kk[..] for the recursion: ***/

	for (i = 1; i <= 7; i++) {
		kk[9 - i] = acf[i];
	}

	for (i = 0; i <= 8; i++) {
		pp[i] = acf[i];
	}

    /*** Compute Parcor coefficients: ***/

	for (n = 0; n < 8; n++) {

		if ((pp[0] == 0) || (w_sub(pp[0], w_abs_s(pp[1])) < 0)) {
			for (i = n; i < 8; i++) {
				vpar[i] = 0;
			}
			return;
		}
		vpar[n] = w_div_s(w_abs_s(pp[1]), pp[0]);

		if (pp[1] > 0) {
			vpar[n] = w_negate(vpar[n]);
		}

		if (w_sub(n, 7) == 0) {
			return;
		}
	/*** Schur recursion: ***/

		pp[0] = w_add(pp[0], w_w_mult_r(pp[1], vpar[n]));

		for (m = 1; m <= 7 - n; m++) {
			pp[m] =
			    w_add(pp[1 + m], w_w_mult_r(kk[9 - m], vpar[n]));

			kk[9 - m] =
			    w_add(kk[9 - m], w_w_mult_r(pp[1 + m], vpar[n]));

		}
	}

	return;
}

/****************************************************************************
 *
 *     FUNCTION:  w_er_step_up
 *
 *     PURPOSE:   Computes the transversal filter coefficients from the
 *                reflection coefficients.
 *
 *     INPUTS:    np               filter order (2..8)
 *                vpar[0..np-1]    reflection coefficients
 *
 *     OUTPUTS:   aav1[0..np]      transversal filter coefficients
 *
 *     RETURN VALUE: none
 *
 ***************************************************************************/

void w_er_step_up(Word16 np, Word16 vpar[], Word16 aav1[]
    )
{
	Word32 L_coef[9], L_work[9];
	Word16 temp;
	Word16 i, m;

    /*** Initialization of the step-up recursion ***/

	L_coef[0] = 0x20000000L;
	L_coef[1] = w_L_w_shl(w_L_deposit_l(vpar[0]), 14);

    /*** Loop on the LPC analysis order: ***/

	for (m = 2; m <= np; m++) {
		for (i = 1; i < m; i++) {
			temp = w_extract_h(L_coef[m - i]);
			L_work[i] = w_L_mac(L_coef[i], vpar[m - 1], temp);
		}

		for (i = 1; i < m; i++) {
			L_coef[i] = L_work[i];
		}

		L_coef[m] = w_L_w_shl(w_L_deposit_l(vpar[m - 1]), 14);
	}

    /*** Keep the aav1[0..np] in 15 bits ***/

	for (i = 0; i <= np; i++) {
		aav1[i] = w_extract_h(w_L_w_shr(L_coef[i], 3));
	}

	return;
}

/****************************************************************************
 *
 *     FUNCTION:  w_er_compute_rav1
 *
 *     PURPOSE:   Computes the autocorrelation function of the adaptive
 *                filter coefficients.
 *
 *     INPUTS:    aav1[0..8]     adaptive filter coefficients
 *
 *     OUTPUTS:   rav1[0..8]     ACF of aav1
 *                *scal_rav1     rav1[] scaling factor
 *
 *     RETURN VALUE: none
 *
 ***************************************************************************/

void w_er_compute_rav1(Word16 aav1[], Word16 rav1[], Word16 * scal_rav1)
{
	Word32 L_work[9];
	Word16 i, k;

    /*** Computation of the rav1[0..8] ***/

	for (i = 0; i <= 8; i++) {
		L_work[i] = 0L;

		for (k = 0; k <= 8 - i; k++) {
			L_work[i] = w_L_mac(L_work[i], aav1[k], aav1[k + i]);
		}
	}

	if (L_work[0] == 0L) {
		*scal_rav1 = 0;
	} else {
		*scal_rav1 = w_norm_l(L_work[0]);
	}

	for (i = 0; i <= 8; i++) {
		rav1[i] = w_extract_h(w_L_w_shl(L_work[i], *scal_rav1));
	}

	return;
}

/****************************************************************************
 *
 *     FUNCTION:  w_er_spectral_comparison
 *
 *     PURPOSE:   Computes the stat flag needed for the threshold
 *                adaptation decision.
 *
 *     INPUTS:    rav1[0..8]      ACF obtained from L_av1
 *                *scal_rav1      rav1[] scaling factor
 *                L_av0[0..8]     ACF averaged over last four frames
 *
 *     OUTPUTS:   none
 *
 *     RETURN VALUE: flag to indicate spectral stationarity
 *
 ***************************************************************************/

Word16 w_er_spectral_comparison(Word16 rav1[], Word16 scal_rav1, Word32 L_av0[]
    )
{
	Word32 L_dm, L_sump, L_temp;
	Word16 stat, sav0[9], shift, divshift, temp;
	Word16 i;

    /*** Re-normalize L_av0[0..8] ***/

	if (L_av0[0] == 0L) {
		for (i = 0; i <= 8; i++) {
			sav0[i] = 0x0fff;	/* 4095 */
		}
	} else {
		shift = w_sub(w_norm_l(L_av0[0]), 3);
		for (i = 0; i <= 8; i++) {
			sav0[i] = w_extract_h(w_L_w_shl(L_av0[i], shift));
		}
	}

    /*** Compute partial sum of dm ***/

	L_sump = 0L;
	for (i = 1; i <= 8; i++) {
		L_sump = w_L_mac(L_sump, rav1[i], sav0[i]);
	}

    /*** Compute the division of the partial sum by sav0[0] ***/

	if (L_sump < 0L) {
		L_temp = w_L_w_negate(L_sump);
	} else {
		L_temp = L_sump;
	}

	if (L_temp == 0L) {
		L_dm = 0L;
		shift = 0;
	} else {
		sav0[0] = w_shl(sav0[0], 3);
		shift = w_norm_l(L_temp);
		temp = w_extract_h(w_L_w_shl(L_temp, shift));

		if (w_sub(sav0[0], temp) >= 0) {
			divshift = 0;
			temp = w_div_s(temp, sav0[0]);
		} else {
			divshift = 1;
			temp = w_sub(temp, sav0[0]);
			temp = w_div_s(temp, sav0[0]);
		}

		if (w_sub(divshift, 1) == 0) {
			L_dm = 0x8000L;
		} else {
			L_dm = 0L;
		}

		L_dm = w_L_w_shl(L_w_add(L_dm, w_L_deposit_l(temp)), 1);

		if (L_sump < 0L) {
			L_dm = w_L_w_negate(L_dm);
		}
	}

    /*** Re-normalization and final computation of L_dm ***/

	L_dm = w_L_w_shl(L_dm, 14);
	L_dm = w_L_w_shr(L_dm, shift);
	L_dm = L_w_add(L_dm, w_L_w_shl(w_L_deposit_l(rav1[0]), 11));
	L_dm = w_L_w_shr(L_dm, scal_rav1);

    /*** Compute the difference and save L_dm ***/

	L_temp = w_L_w_sub(L_dm, w_L_lastdm);
	w_L_lastdm = L_dm;

	if (L_temp < 0L) {
		L_temp = w_L_w_negate(L_temp);
	}
    /*** Evaluation of the stat flag ***/

	L_temp = w_L_w_sub(L_temp, STAT_THRESH);

	if (L_temp < 0L) {
		stat = 1;
	} else {
		stat = 0;
	}

	return stat;
}

/****************************************************************************
 *
 *     FUNCTION:  w_er_threshold_adaptation
 *
 *     PURPOSE:   Evaluates the secondary VAD decision.  If w_speech is not
 *                present then the noise model w_rvad and adaptive threshold
 *                w_thvad are updated.
 *
 *     INPUTS:    stat          flag to indicate spectral stationarity
 *                w_ptch          flag to indicate a periodic signal component
 *                tone          flag to indicate a tone signal component
 *                rav1[0..8]    ACF obtained from L_av1
 *                scal_rav1     rav1[] scaling factor
 *                pvad          filtered signal energy (mantissa+exponent)
 *                acf0          signal frame energy (mantissa+exponent)
 *
 *     OUTPUTS:   w_rvad[0..8]    autocorrelated adaptive filter coefficients
 *                *scal_w_rvad    w_rvad[] scaling factor
 *                *w_thvad        decision threshold (mantissa+exponent)
 *
 *     RETURN VALUE: none
 *
 ***************************************************************************/

void w_er_threshold_adaptation(Word16 stat,
			       Word16 w_ptch,
			       Word16 tone,
			       Word16 rav1[],
			       Word16 scal_rav1,
			       Pfloat pvad,
			       Pfloat acf0,
			       Word16 w_rvad[],
			       Word16 * scal_w_rvad, Pfloat * w_thvad)
{
	Word16 comp, comp2;
	Word32 L_temp;
	Word16 temp;
	Pfloat p_temp;
	Word16 i;

	comp = 0;

    /*** Test if acf0 < pth; if yes set w_thvad to plev ***/

	if (w_sub(acf0.e, E_PTH) < 0) {
		comp = 1;
	}

	if ((w_sub(acf0.e, E_PTH) == 0) && (w_sub(acf0.m, M_PTH) < 0)) {
		comp = 1;
	}

	if (w_sub(comp, 1) == 0) {
		w_thvad->e = E_PLEV;
		w_thvad->m = M_PLEV;

		return;
	}
    /*** Test if an adaption is required ***/

	if (w_sub(w_ptch, 1) == 0) {
		comp = 1;
	}

	if (stat == 0) {
		comp = 1;
	}

	if (w_sub(tone, 1) == 0) {
		comp = 1;
	}

	if (w_sub(comp, 1) == 0) {
		w_adaptcount = 0;
		return;
	}
    /*** Increment w_adaptcount ***/

	w_adaptcount = w_add(w_adaptcount, 1);

	if (w_sub(w_adaptcount, 8) <= 0) {
		return;
	}
    /*** computation of w_thvad-(w_thvad/dec) ***/

	w_thvad->m = w_sub(w_thvad->m, w_shr(w_thvad->m, 5));

	if (w_sub(w_thvad->m, 0x4000) < 0) {
		w_thvad->m = w_shl(w_thvad->m, 1);
		w_thvad->e = w_sub(w_thvad->e, 1);
	}
    /*** computation of pvad*fac ***/

	L_temp = w_L_w_mult(pvad.m, FAC);
	L_temp = w_L_w_shr(L_temp, 15);
	p_temp.e = w_add(pvad.e, 1);

	if (L_temp > 0x7fffL) {
		L_temp = w_L_w_shr(L_temp, 1);
		p_temp.e = w_add(p_temp.e, 1);
	}
	p_temp.m = w_extract_l(L_temp);

    /*** w_test if w_thvad < pvad*fac ***/

	if (w_sub(w_thvad->e, p_temp.e) < 0) {
		comp = 1;
	}

	if ((w_sub(w_thvad->e, p_temp.e) == 0) &&
	    (w_sub(w_thvad->m, p_temp.m) < 0)) {
		comp = 1;
	}
    /*** compute minimum(w_thvad+(w_thvad/inc), pvad*fac) when comp = 1 ***/

	if (w_sub(comp, 1) == 0) {
	/*** compute w_thvad + (w_thvad/inc) ***/

		L_temp = L_w_add(w_L_deposit_l(w_thvad->m),
				 w_L_deposit_l(w_shr(w_thvad->m, 4)));

		if (w_L_w_sub(L_temp, 0x7fffL) > 0) {
			w_thvad->m = w_extract_l(w_L_w_shr(L_temp, 1));
			w_thvad->e = w_add(w_thvad->e, 1);
		} else {
			w_thvad->m = w_extract_l(L_temp);
		}

		comp2 = 0;

		if (w_sub(p_temp.e, w_thvad->e) < 0) {
			comp2 = 1;
		}

		if ((w_sub(p_temp.e, w_thvad->e) == 0) &&
		    (w_sub(p_temp.m, w_thvad->m) < 0)) {
			comp2 = 1;
		}

		if (w_sub(comp2, 1) == 0) {
			w_thvad->e = p_temp.e;
			w_thvad->m = p_temp.m;
		}
	}
    /*** compute pvad + margin ***/

	if (w_sub(pvad.e, E_MARGIN) == 0) {
		L_temp =
		    L_w_add(w_L_deposit_l(pvad.m), w_L_deposit_l(M_MARGIN));
		p_temp.m = w_extract_l(w_L_w_shr(L_temp, 1));
		p_temp.e = w_add(pvad.e, 1);
	} else {

		if (w_sub(pvad.e, E_MARGIN) > 0) {
			temp = w_sub(pvad.e, E_MARGIN);
			temp = w_shr(M_MARGIN, temp);
			L_temp =
			    L_w_add(w_L_deposit_l(pvad.m), w_L_deposit_l(temp));

			if (w_L_w_sub(L_temp, 0x7fffL) > 0) {
				p_temp.e = w_add(pvad.e, 1);
				p_temp.m = w_extract_l(w_L_w_shr(L_temp, 1));

			} else {
				p_temp.e = pvad.e;
				p_temp.m = w_extract_l(L_temp);
			}
		} else {
			temp = w_sub(E_MARGIN, pvad.e);
			temp = w_shr(pvad.m, temp);
			L_temp =
			    L_w_add(w_L_deposit_l(M_MARGIN),
				    w_L_deposit_l(temp));

			if (w_L_w_sub(L_temp, 0x7fffL) > 0) {
				p_temp.e = w_add(E_MARGIN, 1);
				p_temp.m = w_extract_l(w_L_w_shr(L_temp, 1));

			} else {
				p_temp.e = E_MARGIN;
				p_temp.m = w_extract_l(L_temp);
			}
		}
	}

    /*** Test if w_thvad > pvad + margin ***/

	comp = 0;

	if (w_sub(w_thvad->e, p_temp.e) > 0) {
		comp = 1;
	}

	if ((w_sub(w_thvad->e, p_temp.e) == 0) &&
	    (w_sub(w_thvad->m, p_temp.m) > 0)) {
		comp = 1;
	}

	if (w_sub(comp, 1) == 0) {
		w_thvad->e = p_temp.e;
		w_thvad->m = p_temp.m;
	}
    /*** Normalise and retain w_rvad[0..8] in memory ***/

	*scal_w_rvad = scal_rav1;

	for (i = 0; i <= 8; i++) {
		w_rvad[i] = rav1[i];
	}

    /*** Set w_adaptcount to adp + 1 ***/

	w_adaptcount = 9;

	return;
}

/****************************************************************************
 *
 *     FUNCTION:  w_er_tone_detection
 *
 *     PURPOSE:   Computes the tone flag needed for the threshold
 *                adaptation decision.
 *
 *     INPUTS:    rc[0..3]    reflection coefficients calculated in the
 *                            w_speech encoder short term w_predictor
 *
 *     OUTPUTS:   *tone       flag to indicate a periodic signal component
 *
 *     RETURN VALUE: none
 *
 ***************************************************************************/

void w_er_tone_detection(Word16 rc[], Word16 * tone)
{
	Word32 L_num, L_den, L_temp;
	Word16 temp, w_prederr, a[3];
	Word16 i;

	*tone = 0;

    /*** Calculate filter coefficients ***/

	w_er_step_up(2, rc, a);

    /*** Calculate ( a[1] * a[1] ) ***/

	temp = w_shl(a[1], 3);
	L_den = w_L_w_mult(temp, temp);

    /*** Calculate ( 4*a[2] - a[1]*a[1] ) ***/

	L_temp = w_L_w_shl(w_L_deposit_h(a[2]), 3);
	L_num = w_L_w_sub(L_temp, L_den);

    /*** Check if pole frequency is less than 385 Hz ***/

	if (L_num <= 0) {
		return;
	}

	if (a[1] < 0) {
		temp = w_extract_h(L_den);
		L_den = w_L_w_mult(temp, FREQTH);

		L_temp = w_L_w_sub(L_num, L_den);

		if (L_temp < 0) {
			return;
		}
	}
    /*** Calculate normalised w_prediction w_error ***/

	w_prederr = 0x7fff;

	for (i = 0; i < 4; i++) {
		temp = w_mult(rc[i], rc[i]);
		temp = w_sub(0x7fff, temp);
		w_prederr = w_mult(w_prederr, temp);
	}

    /*** Test if w_prediction w_error is smaller than threshold ***/

	temp = w_sub(w_prederr, PREDTH);

	if (temp < 0) {
		*tone = 1;
	}
	return;
}

/****************************************************************************
 *
 *     FUNCTION:  w_er_vad_decision
 *
 *     PURPOSE:   Computes the VAD decision based on the comparison of the
 *                floating point representations of pvad and w_thvad.
 *
 *     INPUTS:    pvad          filtered signal energy (mantissa+exponent)
 *                w_thvad         decision threshold (mantissa+exponent)
 *
 *     OUTPUTS:   none
 *
 *     RETURN VALUE: vad decision before hangover is w_added
 *
 ***************************************************************************/

Word16 w_er_vad_decision(Pfloat pvad, Pfloat w_thvad)
{
	Word16 vvad;

	if (w_sub(pvad.e, w_thvad.e) > 0) {
		vvad = 1;
	} else if ((w_sub(pvad.e, w_thvad.e) == 0) &&
		   (w_sub(pvad.m, w_thvad.m) > 0)) {
		vvad = 1;
	} else {
		vvad = 0;
	}

	return vvad;
}

/****************************************************************************
 *
 *     FUNCTION:  w_er_vad_hangover
 *
 *     PURPOSE:   Computes the final VAD decision for the current frame
 *                being processed.
 *
 *     INPUTS:    vvad           vad decision before hangover is w_added
 *
 *     OUTPUTS:   none
 *
 *     RETURN VALUE: vad decision after hangover is w_added
 *
 ***************************************************************************/

Word16 w_er_vad_hangover(Word16 vvad)
{

	if (w_sub(vvad, 1) == 0) {
		w_burstcount = w_add(w_burstcount, 1);
	} else {
		w_burstcount = 0;
	}

	if (w_sub(w_burstcount, BURSTCONST) >= 0) {
		w_hangcount = HANGCONST;
		w_burstcount = BURSTCONST;
	}

	if (w_hangcount >= 0) {
		w_hangcount = w_sub(w_hangcount, 1);
		return 1;	/* vad = 1 */
	}
	return vvad;		/* vad = vvad */
}

/****************************************************************************
 *
 *     FUNCTION:  w_er_periodicity_update
 *
 *     PURPOSE:   Computes the w_ptch flag needed for the threshold
 *                adaptation decision for the next frame.
 *
 *     INPUTS:    lags[0..1]       w_speech encoder long term w_predictor lags
 *
 *     OUTPUTS:   *w_ptch            Boolean voiced / unvoiced decision
 *
 *     RETURN VALUE: none
 *
 ***************************************************************************/

void w_er_periodicity_update(Word16 lags[], Word16 * w_ptch)
{
	Word16 minlag, maxlag, lw_agcount, temp;
	Word16 i;

    /*** Run loop for the two halves in the frame ***/

	lw_agcount = 0;

	for (i = 0; i <= 1; i++) {
	/*** Search the maximum and minimum of consecutive lags ***/

		if (w_sub(w_oldlag, lags[i]) > 0) {
			minlag = lags[i];
			maxlag = w_oldlag;
		} else {
			minlag = w_oldlag;
			maxlag = lags[i];
		}

		temp = w_sub(maxlag, minlag);

		if (w_sub(temp, LTHRESH) < 0) {
			lw_agcount = w_add(lw_agcount, 1);
		}
	/*** Save the current LTP lag ***/

		w_oldlag = lags[i];
	}

    /*** Update the veryoldlw_agcount and oldlw_agcount ***/

	veryoldlw_agcount = oldlw_agcount;

	oldlw_agcount = lw_agcount;

    /*** Make w_ptch decision ready for next frame ***/

	temp = w_add(oldlw_agcount, veryoldlw_agcount);

	if (w_sub(temp, NTHRESH) >= 0) {
		*w_ptch = 1;
	} else {
		*w_ptch = 0;
	}

	return;
}

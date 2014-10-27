/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/

/*
 File : VAD.C
*/

/* Voice Activity Detector functions */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ld8k.h"
#include "tab_ld8k.h"
#include "ld8cp.h"
#include "vad.h"
#include "dtx.h"
#include "tab_dtx.h"

/* Local functions */
static int MakeDec(float dSLE,	/* (i)  : differential low band energy */
		   float dSE,	/* (i)  : differential full band energy */
		   float SD,	/* (i)  : differential spectral distortion  */
		   float dSZC	/* (i)  : differential zero crossing rate */
    );

/* static variables */
static float MeanLSF[M];
static float Min_buffer[16];
static float Prev_Min, Next_Min, Min;
static float MeanE, MeanSE, MeanSLE, MeanSZC;
static float prev_energy;
static int count_sil, count_update, count_ext;
static int flag, v_flag, less_count;

/*---------------------------------------------------------------------------*
 * Function  vad_init                                                                                                            *
 * ~~~~~~~~~~~~~~~~~~                                                                                                            *
 *                                                                                                                                                       *
 * -> Initialization of variables for voice activity detection                           *
 *                                                                                                                                                       *
*---------------------------------------------------------------------------*/
void vad_init(void)
{
	/* Static vectors to zero */
	set_zero(MeanLSF, M);

	/* Initialize VAD parameters */
	MeanSE = (float) 0.0;
	MeanSLE = (float) 0.0;
	MeanE = (float) 0.0;
	MeanSZC = (float) 0.0;
	count_sil = 0;
	count_update = 0;
	count_ext = 0;
	less_count = 0;
	flag = 1;
	Min = FLT_MAX_G729;
	return;
}

/*-----------------------------------------------------------------*
* Functions vad                                                   *
*           ~~~                                                   *
* Input:                                                          *
*   rc            : reflection coefficient                        *
*   lsf[]         : unquantized lsf vector                        *
*   rxx[]         : autocorrelation vector                        *
*   sigpp[]       : preprocessed input signal                     *
*   frm_count     : frame counter                                 *
*   prev_marker   : VAD decision of the last frame                *
*   pprev_marker  : VAD decision of the frame before last frame   *
*                                                                 *
* Output:                                                         *
*                                                                 *
*   marker        : VAD decision of the current frame             *
*                                                                 *
*-----------------------------------------------------------------*/

void vad(float rc,
	 float * lsf,
	 float * rxx,
	 float * sigpp,
	 int frm_count,
	 int prev_marker, int pprev_marker, int *marker, float * Energy_db)
{
	float tmp[M];
	float SD;
	float E_low;
	float dtemp;
	float dSE;
	float dSLE;
	float ZC;
	float COEF;
	float COEFZC;
	float COEFSD;
	float dSZC;
	float norm_energy;
	int i;

	/* compute the frame energy */
	norm_energy =
	    (float) 10.0 *(float) log10((float) (rxx[0] / (float) 240.0 + EPSI));
	*Energy_db = norm_energy;

	/* compute the low band energy */
	E_low = (float) 0.0;
	for (i = 1; i <= NP; i++)
		E_low = E_low + rxx[i] * lbf_corr[i];

	E_low = rxx[0] * lbf_corr[0] + (float) 2.0 *E_low;
	if (E_low < (float) 0.0)
		E_low = (float) 0.0;
	E_low = (float) 10.0 *(float) log10((float) (E_low / (float) 240.0 + EPSI));

	/* compute SD */
	/* Normalize lsfs */
	for (i = 0; i < M; i++)
		lsf[i] /= (float) 2. *PI;
	dvsub(lsf, MeanLSF, tmp, M);
	SD = dvdot(tmp, tmp, M);

	/* compute # zero crossing */
	ZC = (float) 0.0f;
	dtemp = sigpp[ZC_START];
	for (i = ZC_START + 1; i <= ZC_END; i++) {
		if (dtemp * sigpp[i] < (float) 0.0) {
			ZC = ZC + (float) 1.0;
		}
		dtemp = sigpp[i];
	}
	ZC = ZC / (float) 80.0;

	/* Initialize and update Mins */
	if (frm_count < 129) {
		if (norm_energy < Min) {
			Min = norm_energy;
			Prev_Min = norm_energy;
		}
		if ((frm_count % 8) == 0) {
			Min_buffer[(int)frm_count / 8 - 1] = Min;
			Min = FLT_MAX_G729;
		}
	}
	if ((frm_count % 8) == 0) {
		Prev_Min = Min_buffer[0];
		for (i = 1; i < 15; i++) {
			if (Min_buffer[i] < Prev_Min)
				Prev_Min = Min_buffer[i];
		}
	}

	if (frm_count >= 129) {
		if ((frm_count % 8) == 1) {
			Min = Prev_Min;
			Next_Min = FLT_MAX_G729;
		}
		if (norm_energy < Min)
			Min = norm_energy;
		if (norm_energy < Next_Min)
			Next_Min = norm_energy;
		if ((frm_count % 8) == 0) {
			for (i = 0; i < 15; i++)
				Min_buffer[i] = Min_buffer[i + 1];
			Min_buffer[15] = Next_Min;
			Prev_Min = Min_buffer[0];
			for (i = 1; i < 16; i++) {
				if (Min_buffer[i] < Prev_Min)
					Prev_Min = Min_buffer[i];
			}

		}
	}

	if (frm_count <= INIT_FRAME) {
		if (norm_energy < (float) 21.0) {
			less_count++;
			*marker = NOISE;
		} else {
			*marker = VOICE;
			MeanE =
			    (MeanE * ((float) (frm_count - less_count - 1)) +
			     norm_energy) / (float) (frm_count - less_count);
			MeanSZC =
			    (MeanSZC * ((float) (frm_count - less_count - 1)) +
			     ZC) / (float) (frm_count - less_count);
			dvwadd(MeanLSF, (float) (frm_count - less_count - 1),
			       lsf, (float) 1.0, MeanLSF, M);
			dvsmul(MeanLSF,
			       (float) 1.0 / (float) (frm_count - less_count),
			       MeanLSF, M);
		}
	}

	if (frm_count >= INIT_FRAME) {
		if (frm_count == INIT_FRAME) {
			MeanSE = MeanE - (float) 10.0;
			MeanSLE = MeanE - (float) 12.0;
		}

		dSE = MeanSE - norm_energy;
		dSLE = MeanSLE - E_low;
		dSZC = MeanSZC - ZC;

		if (norm_energy < (float) 21.0) {
			*marker = NOISE;
		} else {
			*marker = MakeDec(dSLE, dSE, SD, dSZC);
		}

		v_flag = 0;
		if ((prev_marker == VOICE) && (*marker == NOISE) &&
		    (norm_energy > MeanSE + (float) 2.0)
		    && (norm_energy > (float) 21.0)) {
			*marker = VOICE;
			v_flag = 1;
		}

		if (flag == 1) {
			if ((pprev_marker == VOICE) && (prev_marker == VOICE) &&
			    (*marker == NOISE)
			    && (fabs(prev_energy - norm_energy) <= (float) 3.0)) {
				count_ext++;
				*marker = VOICE;
				v_flag = 1;
				if (count_ext <= 4)
					flag = 1;
				else {
					flag = 0;
					count_ext = 0;
				}
			}
		} else
			flag = 1;

		if (*marker == NOISE)
			count_sil++;

		if ((*marker == VOICE) && (count_sil > 10) &&
		    ((norm_energy - prev_energy) <= (float) 3.0)) {
			*marker = NOISE;
			count_sil = 0;
		}

		if (*marker == VOICE)
			count_sil = 0;

		if ((norm_energy < MeanSE + (float) 3.0) && (frm_count > 128)
		    && (!v_flag) && (rc < (float) 0.6))
			*marker = NOISE;

		if ((norm_energy < MeanSE + (float) 3.0) && (rc < (float) 0.75)
		    && (SD < (float) 0.002532959)) {
			count_update++;
			if (count_update < INIT_COUNT) {
				COEF = (float) 0.75;
				COEFZC = (float) 0.8;
				COEFSD = (float) 0.6;
			} else if (count_update < INIT_COUNT + 10) {
				COEF = (float) 0.95;
				COEFZC = (float) 0.92;
				COEFSD = (float) 0.65;
			} else if (count_update < INIT_COUNT + 20) {
				COEF = (float) 0.97;
				COEFZC = (float) 0.94;
				COEFSD = (float) 0.70;
			} else if (count_update < INIT_COUNT + 30) {
				COEF = (float) 0.99;
				COEFZC = (float) 0.96;
				COEFSD = (float) 0.75;
			} else if (count_update < INIT_COUNT + 40) {
				COEF = (float) 0.995;
				COEFZC = (float) 0.99;
				COEFSD = (float) 0.75;
			} else {
				COEF = (float) 0.995;
				COEFZC = (float) 0.998;
				COEFSD = (float) 0.75;
			}
			dvwadd(MeanLSF, COEFSD, lsf, (float) 1.0 - COEFSD, MeanLSF,
			       M);
			MeanSE = COEF * MeanSE + ((float) 1.0 - COEF) * norm_energy;
			MeanSLE = COEF * MeanSLE + ((float) 1.0 - COEF) * E_low;
			MeanSZC = COEFZC * MeanSZC + ((float) 1.0 - COEFZC) * ZC;
		}

		if (((frm_count > 128)
		     && ((MeanSE < Min) && (SD < (float) 0.002532959)))
		    || (MeanSE > Min + (float) 10.0)) {
			MeanSE = Min;
			count_update = 0;
		}
	}

	prev_energy = norm_energy;
	return;
}

static float a[14] = {
	(float) 1.750000e-03, (float) - 4.545455e-03, (float) - 2.500000e+01,
	(float) 2.000000e+01,
	(float) 0.000000e+00, (float) 8.800000e+03, (float) 0.000000e+00, (float) 2.5e+01,
	(float) - 2.909091e+01, (float) 0.000000e+00, (float) 1.400000e+04, (float) 0.928571,
	(float) - 1.500000e+00, (float) 0.714285
};

static float b[14] = {
	(float) 0.00085, (float) 0.001159091, (float) - 5.0, (float) - 6.0, (float) - 4.7,
	(float) - 12.2, (float) 0.0009,
	(float) - 7.0, (float) - 4.8182, (float) - 5.3, (float) - 15.5, (float) 1.14285, (float) - 9.0,
	(float) - 2.1428571
};

static int MakeDec(float dSLE, float dSE, float SD, float dSZC)
{

	float pars[4];

	pars[0] = dSLE;
	pars[1] = dSE;
	pars[2] = SD;
	pars[3] = dSZC;

	/* SD vs dSZC */
	if (pars[2] > a[0] * pars[3] + b[0]) {
		return (VOICE);
	}
	if (pars[2] > a[1] * pars[3] + b[1]) {
		return (VOICE);
	}

	/*   dE vs dSZC */

	if (pars[1] < a[2] * pars[3] + b[2]) {
		return (VOICE);
	}
	if (pars[1] < a[3] * pars[3] + b[3]) {
		return (VOICE);
	}
	if (pars[1] < b[4]) {
		return (VOICE);
	}

	/*   dE vs SD */
	if (pars[1] < a[5] * pars[2] + b[5]) {
		return (VOICE);
	}
	if (pars[2] > b[6]) {
		return (VOICE);
	}

	/* dEL vs dSZC */
	if (pars[1] < a[7] * pars[3] + b[7]) {
		return (VOICE);
	}
	if (pars[1] < a[8] * pars[3] + b[8]) {
		return (VOICE);
	}
	if (pars[1] < b[9]) {
		return (VOICE);
	}

	/* dEL vs SD */
	if (pars[0] < a[10] * pars[2] + b[10]) {
		return (VOICE);
	}

	/* dEL vs dE */
	if (pars[0] > a[11] * pars[1] + b[11]) {
		return (VOICE);
	}

	if (pars[0] < a[12] * pars[1] + b[12]) {
		return (VOICE);
	}
	if (pars[0] < a[13] * pars[1] + b[13]) {
		return (VOICE);
	}

	return (NOISE);
}

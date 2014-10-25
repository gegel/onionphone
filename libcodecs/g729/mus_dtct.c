/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/

/* ----------------------------------------------------------------- */
/*                        MUSIC DETECTION MODULE                     */
/*                                                                   */
/*                                                                   */
/*                 (C) Copyright 1999 : Conexant Systems             */
/*                                                                   */
/* ----------------------------------------------------------------- */

/*
 File : MUS_DTCT.C
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ld8k.h"
#include "tab_ld8k.h"
#include "ld8cp.h"
#include "vad.h"

#define         sqr(a)          ((a)*(a))

void musdetect(int rate,
	       float Energy,
	       float * rc,
	       int *lags,
	       float * pgains,
	       int stat_flg,
	       int frm_count, int prev_vad, int *Vad, float LLenergy)
{

	int i;
	static int count_music = 0;
	static float Mcount_music = (float) 0.0;
	static int count_consc = 0;
	float sum1, sum2, std;
	static float MeanPgain = (float) 0.5;
	short PFLAG1, PFLAG2, PFLAG;

	static int count_pflag = 0;
	static float Mcount_pflag = (float) 0.0;
	static int count_consc_pflag = 0;
	static int count_consc_rflag = 0;
	static float mrc[10] = { (float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0,
		(float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0
	};
	static float MeanSE = (float) 0.0;
	float pderr, Lenergy, SD, tmp_vec[10];
	float Thres;

	pderr = (float) 1.0;
	for (i = 0; i < 4; i++)
		pderr *= ((float) 1.0 - rc[i] * rc[i]);
	dvsub(mrc, rc, tmp_vec, 10);
	SD = dvdot(tmp_vec, tmp_vec, 10);

	Lenergy = (float) 10.0 *(float) log10(pderr * Energy / (float) 240.0 + EPSI);

	if (*Vad == NOISE) {
		dvwadd(mrc, (float) 0.9, rc, (float) 0.1, mrc, 10);
		MeanSE = (float) 0.9 *MeanSE + (float) 0.1 *Lenergy;
	}

	sum1 = (float) 0.0;
	sum2 = (float) 0.0;
	for (i = 0; i < 5; i++) {
		sum1 += (float) lags[i];
		sum2 += pgains[i];
	}

	sum1 = sum1 / (float) 5.0;
	sum2 = sum2 / (float) 5.0;
	std = (float) 0.0;
	for (i = 0; i < 5; i++)
		std += sqr(((float) lags[i] - sum1));
	std = (float) sqrt(std / (float) 4.0);

	MeanPgain = (float) 0.8 *MeanPgain + (float) 0.2 *sum2;

	if (rate == G729D)
		Thres = (float) 0.73;
	else
		Thres = (float) 0.63;

	if (MeanPgain > Thres)
		PFLAG2 = 1;
	else
		PFLAG2 = 0;

	if (std < (float) 1.30 && MeanPgain > (float) 0.45)
		PFLAG1 = 1;
	else
		PFLAG1 = 0;

	PFLAG =
	    (int16_t) (((int16_t) prev_vad & (int16_t) (PFLAG1 | PFLAG2)) |
		     (int16_t) (PFLAG2));

	if (rc[1] <= (float) 0.45 && rc[1] >= (float) 0.0 && MeanPgain < (float) 0.5)
		count_consc_rflag++;
	else
		count_consc_rflag = 0;

	if (stat_flg == 1 && (*Vad == VOICE))
		count_music++;

	if ((frm_count % 64) == 0) {
		if (frm_count == 64)
			Mcount_music = (float) count_music;
		else
			Mcount_music =
			    (float) 0.9 *Mcount_music +
			    (float) 0.1 *(float) count_music;
	}

	if (count_music == 0)
		count_consc++;
	else
		count_consc = 0;

	if (count_consc > 500 || count_consc_rflag > 150)
		Mcount_music = (float) 0.0;

	if ((frm_count % 64) == 0)
		count_music = 0;

	if (PFLAG == 1)
		count_pflag++;

	if ((frm_count % 64) == 0) {
		if (frm_count == 64)
			Mcount_pflag = (float) count_pflag;
		else {
			if (count_pflag > 25) {
				Mcount_pflag =
				    (float) 0.98 *Mcount_pflag +
				    (float) 0.02 *(float) count_pflag;
			} else if (count_pflag > 20) {
				Mcount_pflag =
				    (float) 0.95 *Mcount_pflag +
				    (float) 0.05 *(float) count_pflag;
			} else
				Mcount_pflag =
				    (float) 0.90 *Mcount_pflag +
				    (float) 0.10 *(float) count_pflag;
		}
	}

	if (count_pflag == 0)
		count_consc_pflag++;
	else
		count_consc_pflag = 0;

	if (count_consc_pflag > 100 || count_consc_rflag > 150)
		Mcount_pflag = (float) 0.0;

	if ((frm_count % 64) == 0)
		count_pflag = 0;

	if (rate == G729E) {
		if (SD > (float) 0.15 && (Lenergy - MeanSE) > (float) 4.0
		    && (LLenergy > 50.0))
			*Vad = VOICE;
		else if ((SD > (float) 0.38 || (Lenergy - MeanSE) > (float) 4.0)
			 && (LLenergy > 50.0))
			*Vad = VOICE;
		else if ((Mcount_pflag >= (float) 10.0 || Mcount_music >= (float) 5.0
			  || frm_count < 64)
			 && (LLenergy > 7.0))
			*Vad = VOICE;
	}
	return;
}

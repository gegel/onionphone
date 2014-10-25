/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C - Reference C code for floating point
                         implementation of G.729
                         Version 1.01 of 15.September.98
*/

/*
----------------------------------------------------------------------
                    COPYRIGHT NOTICE
----------------------------------------------------------------------
   ITU-T G.729 Annex C ANSI C source code
   Copyright (C) 1998, AT&T, France Telecom, NTT, University of
   Sherbrooke.  All rights reserved.

----------------------------------------------------------------------
*/

/*
File : GAINPRED.C
*/

#include <math.h>
#include "ld8k.h"
#include "tab_ld8k.h"

/*---------------------------------------------------------------------------*
* Function  Gain_predict                                                    *
* ~~~~~~~~~~~~~~~~~~~~~~                                                    *
* MA prediction is performed on the innovation energy (in dB with mean      *
* removed).                                                                 *
*---------------------------------------------------------------------------*/
void gain_predict(float past_qua_en[],	/* (i)     :Past quantized energies        */
		  float code[],	/* (i)     :Innovative vector.             */
		  int l_subfr,	/* (i)     :Subframe length.               */
		  float * gcode0	/* (o)     :Predicted codebook gain        */
    )
{
	float ener_code, pred_code;
	int i;

	pred_code = MEAN_ENER;

	/* innovation energy */
	ener_code = (float) 0.01;
	for (i = 0; i < l_subfr; i++)
		ener_code += code[i] * code[i];
	ener_code = (float) 10.0 *(float) log10(ener_code / (float) l_subfr);

	pred_code -= ener_code;

	/* predicted energy */
	for (i = 0; i < 4; i++)
		pred_code += pred[i] * past_qua_en[i];

	/* predicted codebook gain */
	*gcode0 = pred_code;
	*gcode0 = (float) pow((double)10.0, (double)(*gcode0 / 20.0));	/* predicted gain */

	return;
}

/*---------------------------------------------------------------------------*
* Function  gain_update                                                     *
* ~~~~~~~~~~~~~~~~~~~~~~                                                    *
* update table of past quantized energies                                   *
*---------------------------------------------------------------------------*/
void gain_update(float past_qua_en[],	/* input/output :Past quantized energies     */
		 float g_code	/*  input: gbk1[indice1][1]+gbk2[indice2][1] */
    )
{
	int i;

	/* update table of past quantized energies */
	for (i = 3; i > 0; i--)
		past_qua_en[i] = past_qua_en[i - 1];
	past_qua_en[0] = (float) 20.0 *(float) log10((double)g_code);

	return;
}

/*---------------------------------------------------------------------------*
* Function  gain_update_erasure                                             *
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                                             *
* update table of past quantized energies (frame erasure)                   *
*---------------------------------------------------------------------------*
*     av_pred_en = 0.0;                                                     *
*     for (i = 0; i < 4; i++)                                               *
*        av_pred_en += past_qua_en[i];                                      *
*     av_pred_en = av_pred_en*0.25 - 4.0;                                   *
*     if (av_pred_en < -14.0) av_pred_en = -14.0;                           *
*---------------------------------------------------------------------------*/
void gain_update_erasure(float past_qua_en[]	/* input/output:Past quantized energies        */
    )
{
	int i;
	float av_pred_en;

	av_pred_en = (float) 0.0;
	for (i = 0; i < 4; i++)
		av_pred_en += past_qua_en[i];
	av_pred_en = av_pred_en * (float) 0.25 - (float) 4.0;
	if (av_pred_en < (float) - 14.0)
		av_pred_en = (float) - 14.0;

	for (i = 3; i > 0; i--)
		past_qua_en[i] = past_qua_en[i - 1];
	past_qua_en[0] = av_pred_en;

	return;
}

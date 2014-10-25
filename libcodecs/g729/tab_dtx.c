/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/

/*
 File : TAB_DTX.C
*/

/*********************************************************************/
/******             Tables used for VAD/DTX/CNG                 ******/
/*********************************************************************/
#include <stdio.h>
#include "ld8k.h"
#include "vad.h"
#include "dtx.h"
#include "tab_dtx.h"

/* VAD constants */
float lbf_corr[NP + 1] = {
	(float) 0.24017939691329, (float) 0.21398822343783, (float) 0.14767692339633,
	(float) 0.07018811903116, (float) 0.00980856433051, (float) - 0.02015934721195,
	(float) - 0.02388269958005, (float) - 0.01480076155002, (float) - 0.00503292155509,
	(float) 0.00012141366508, (float) 0.00119354245231, (float) 0.00065908718613,
	(float) 0.00015015782285
};

/* Quantization of SID gain */
float fact[NB_GAIN + 1] = { (float) 0.003125, (float) 0.00078125, (float) 0.000390625 };

float tab_Sidgain[32] = {
	(float) 0.502, (float) 1.262, (float) 2.000, (float) 3.170, (float) 5.024, (float) 7.962,
	(float) 12.619, (float) 15.887, (float) 20.000, (float) 25.179, (float) 31.698, (float) 39.905,
	(float) 50.238, (float) 63.246, (float) 79.621, (float) 100.237, (float) 126.191,
	(float) 158.866,
	(float) 200.000, (float) 251.785, (float) 316.979, (float) 399.052, (float) 502.377,
	(float) 632.456,
	(float) 796.214, (float) 1002.374, (float) 1261.915, (float) 1588.656, (float) 2000.000,
	(float) 2517.851,
	(float) 3169.786, (float) 3990.525
};

/* Quantization of LSF vector */
int PtrTab_1[32] = { 96, 52, 20, 54, 86, 114, 82, 68, 36, 121, 48, 92, 18, 120,
	94, 124, 50, 125, 4, 100, 28, 76, 12, 117, 81, 22, 90, 116,
	127, 21, 108, 66
};
int PtrTab_2[2][16] =
    { {31, 21, 9, 3, 10, 2, 19, 26, 4, 3, 11, 29, 15, 27, 21, 12},
{16, 1, 0, 0, 8, 25, 22, 20, 19, 23, 20, 31, 4, 31, 20, 31}
};

float noise_fg[MODE][MA_NP][M];

float noise_fg_sum[MODE][M] = {
	{(float) 2.379833e-01f, (float) 2.577898e-01f, (float) 2.504044e-01f,
	 (float) 2.530900e-01f,
	 (float) 2.479934e-01f,
	 (float) 2.587054e-01f, (float) 2.577898e-01f, (float) 2.656026e-01f,
	 (float) 2.759789e-01f,
	 (float) 2.625813e-01f}
	,
	{(float) 0.320883796f, (float) 0.378502704f, (float) 0.391650136f, (float) 0.363609794f,
	 (float) 0.349357626f,
	 (float) 0.356157116f, (float) 0.339738164f, (float) 0.345200944f, (float) 0.361461228f,
	 (float) 0.349363712f},
};

float noise_fg_sum_inv[MODE][M] = {
	{(float) 4.201788e+00f, (float) 3.879025e+00f, (float) 3.993530e+00f,
	 (float) 3.951048e+00f,
	 (float) 4.032350e+00f,
	 (float) 3.865596e+00f, (float) 3.879025e+00f, (float) 3.765007e+00f,
	 (float) 3.623157e+00f,
	 (float) 3.807978e+00f}
	,
	{(float) 3.11639295117289f, (float) 2.64198905168191f, (float) 2.55329925380136f,
	 (float) 2.75020094755754f, (float) 2.86239636858535f, (float) 2.80774960003888f,
	 (float) 2.94344323353675f, (float) 2.89686345701303f, (float) 2.76654844983817f,
	 (float) 2.86234650495126f}
};

float Mp[MODE] = { (float) 0.065942075f, (float) 0.12644604f };

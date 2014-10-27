/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729 Annex C+ - Reference C code for floating point
                         implementation of G.729 Annex C+
                         (integration of Annexes B, D and E)
                          Version 2.1 of October 1999
*/

/*
 File : TABLD8CP.C
*/

#include <stdio.h>
#include <stdlib.h>
#include "ld8k.h"
#include "ld8cp.h"
#include "tabld8cp.h"

/*-----------------------------------------------------*
 | Tables for routine bits().                          |
 -----------------------------------------------------*/
int bitsno_B[PRM_SIZE_SID] = { 1,	/* SID Lsp : MA  */
	5,			/* SID Lsp : 1st stage */
	4,			/* SID Lsp : 2nd stage */
	5
};				/* SID gain */

int bitsno_D[PRM_SIZE_D] = { 1 + NC0_B,	/* MA + 1st stage */
	NC1_B * 2,		/* 2nd stage */
	8, 9, 2, 6,		/* first subframe  */
	4, 9, 2, 6
};				/* second subframe */

int bitsno_E_fwd[PRM_SIZE_E_fwd - 1] = { 1 + NC0_B,	/* MA + 1st stage */
	NC1_B * 2,		/* 2nd stage */
	8, 1, 7, 7, 7, 7, 7, 7,	/* first subframe  */
	5, 7, 7, 7, 7, 7, 7
};				/* second subframe */

int bitsno_E_bwd[PRM_SIZE_E_bwd - 1] = {
	8, 1, 13, 10, 7, 7, 7, 7,	/* first subframe  */
	5, 13, 10, 7, 7, 7, 7
};				/* second subframe */

/*--------------------------------------------------------------------------*
 * tables specific to G729E                                                 *
 *--------------------------------------------------------------------------*/
float lag_bwd[M_BWD] = {
	(float) 0.999892, (float) 0.999869, (float) 0.999831, (float) 0.999777, (float) 0.999707,
	(float) 0.999622, (float) 0.999522, (float) 0.999407, (float) 0.999276, (float) 0.999129,
	(float) 0.998968, (float) 0.998790, (float) 0.998598, (float) 0.998390, (float) 0.998167,
	(float) 0.997928, (float) 0.997674, (float) 0.997405, (float) 0.997121, (float) 0.996821,
	(float) 0.996506, (float) 0.996175, (float) 0.995830, (float) 0.995469, (float) 0.995093,
	(float) 0.994702, (float) 0.994295, (float) 0.993874, (float) 0.993437, (float) 0.992985,
};

float hw[NRP + L_FRAME + M_BWD] = {
	(float) 0.047765, (float) 0.095421, (float) 0.142859, (float) 0.189970, (float) 0.236649,
	(float) 0.282787, (float) 0.328279, (float) 0.373022, (float) 0.416914, (float) 0.459853,
	(float) 0.501743, (float) 0.542488, (float) 0.581994, (float) 0.620172, (float) 0.656934,
	(float) 0.692196, (float) 0.725879, (float) 0.757904, (float) 0.788199, (float) 0.816695,
	(float) 0.843326, (float) 0.868033, (float) 0.890757, (float) 0.911449, (float) 0.930060,
	(float) 0.946547, (float) 0.960874, (float) 0.973008, (float) 0.982920, (float) 0.990588,
	(float) 0.995995, (float) 0.999129, (float) 0.999982, (float) 0.998552, (float) 0.994842,
	(float) 0.988862, (float) 0.981775, (float) 0.974740, (float) 0.967754, (float) 0.960819,
	(float) 0.953934, (float) 0.947098, (float) 0.940311, (float) 0.933572, (float) 0.926882,
	(float) 0.920240, (float) 0.913645, (float) 0.907098, (float) 0.900597, (float) 0.894143,
	(float) 0.887735, (float) 0.881374, (float) 0.875058, (float) 0.868787, (float) 0.862561,
	(float) 0.856379, (float) 0.850242, (float) 0.844149, (float) 0.838100, (float) 0.832094,
	(float) 0.826131, (float) 0.820211, (float) 0.814333, (float) 0.808497, (float) 0.802703,
	(float) 0.796951, (float) 0.791240, (float) 0.785569, (float) 0.779940, (float) 0.774351,
	(float) 0.768801, (float) 0.763292, (float) 0.757822, (float) 0.752391, (float) 0.747000,
	(float) 0.741646, (float) 0.736332, (float) 0.731055, (float) 0.725816, (float) 0.720614,
	(float) 0.715450, (float) 0.710323, (float) 0.705233, (float) 0.700179, (float) 0.695161,
	(float) 0.690180, (float) 0.685234, (float) 0.680323, (float) 0.675448, (float) 0.670607,
	(float) 0.665802, (float) 0.661030, (float) 0.656293, (float) 0.651590, (float) 0.646921,
	(float) 0.642285, (float) 0.637682, (float) 0.633112, (float) 0.628575, (float) 0.624070,
	(float) 0.619598, (float) 0.615158, (float) 0.610750, (float) 0.606373, (float) 0.602027,
	(float) 0.597713, (float) 0.593430, (float) 0.589177, (float) 0.584955, (float) 0.580763,
	(float) 0.576601, (float) 0.572469, (float) 0.568367, (float) 0.564294, (float) 0.560250,
	(float) 0.556235, (float) 0.552249, (float) 0.548291, (float) 0.544362, (float) 0.540461,
	(float) 0.536588, (float) 0.532742, (float) 0.528925, (float) 0.525134, (float) 0.521371,
	(float) 0.517635, (float) 0.513925, (float) 0.510242, (float) 0.506586, (float) 0.502956,
	(float) 0.499351, (float) 0.495773, (float) 0.492220, (float) 0.488693, (float) 0.485190,
	(float) 0.481713, (float) 0.478261, (float) 0.474834, (float) 0.471431, (float) 0.468053,
	(float) 0.464699, (float) 0.461369, (float) 0.458062, (float) 0.454780, (float) 0.451521
};

int ipos[16] = { 0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 0 };

/*--------------------------------------------------------------------------*
 * tables specific to G729D                                                *
 *--------------------------------------------------------------------------*/
/* tables of positions for each track */
int trackTable0[16] =
    { 1, 3, 6, 8, 11, 13, 16, 18, 21, 23, 26, 28, 31, 33, 36, 38 };
int trackTable1[32] =
    { 0, 2, 4, 5, 7, 9, 10, 12, 14, 15, 17, 19, 20, 22, 24, 25, 27, 29, 30, 32,
	34, 35, 37, 39, 1, 6, 11, 16, 21, 26, 31, 36
};
int posSearched[2] = { 16, 32 };	/* number of positions search for each track */

int grayEncode[32] = {
	0, 1, 3, 2, 6, 7, 5, 4, 12, 13, 15, 14, 10, 11, 9, 8,
	24, 25, 27, 26, 30, 31, 29, 28, 20, 21, 23, 22, 18, 19, 17, 16
};

int grayDecode[32] = {
	0, 1, 3, 2, 7, 6, 4, 5, 15, 14, 12, 13, 8, 9, 11, 10,
	31, 30, 28, 29, 24, 25, 27, 26, 16, 17, 19, 18, 23, 22, 20, 21
};

/* anti-sparseness post-processing */
float ph_imp_low[L_SUBFR] = {
	(float) 0.4483, (float) 0.3515, (float) 0.0387, -(float) 0.0843, -(float) 0.1731,
	(float) 0.2293, -(float) 0.0011,
	(float) - 0.0857, (float) - 0.0928, (float) 0.1472, (float) 0.0901, (float) - 0.2571,
	(float) 0.1155, (float) 0.0444,
	(float) 0.0665, (float) - 0.2636, (float) 0.2457, (float) - 0.0642, (float) - 0.0444,
	(float) 0.0237, (float) 0.0338,
	(float) - 0.0728, (float) 0.0688, (float) - 0.0111, (float) - 0.0206, (float) - 0.0642,
	(float) 0.1845, (float) - 0.1734,
	(float) 0.0327, (float) 0.0953, (float) - 0.1544, (float) 0.1621, (float) - 0.0711,
	(float) - 0.1138, (float) 0.2113,
	(float) - 0.1187, (float) 0.0206, (float) - 0.0542, (float) 0.0009, (float) 0.3096
};

float ph_imp_mid[L_SUBFR] = {
	(float) 0.9239, (float) 0.1169, (float) - 0.1232, (float) 0.0907, (float) - 0.0320,
	(float) - 0.0306, (float) 0.0756,
	(float) - 0.0929, (float) 0.0859, (float) - 0.0681, (float) 0.0535, (float) - 0.0492,
	(float) 0.0523, (float) - 0.0542,
	(float) 0.0471, (float) - 0.0308, (float) 0.0131, (float) - 0.0052, (float) 0.0144,
	(float) - 0.0386, (float) 0.0664,
	(float) - 0.0826, (float) 0.0770, (float) - 0.0495, (float) 0.0105, (float) 0.0252,
	(float) - 0.0467, (float) 0.0526,
	(float) - 0.0506, (float) 0.0519, (float) - 0.0630, (float) 0.0807, (float) - 0.0934,
	(float) 0.0884, (float) - 0.0604,
	(float) 0.0170, (float) 0.0238, (float) - 0.0418, (float) 0.0257, (float) 0.0200
};

float ph_imp_high[L_SUBFR] = {
	(float) 1.0, (float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0,
	(float) 0.0, (float) 0.0,
	(float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0,
	(float) 0.0, (float) 0.0,
	(float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0,
	(float) 0.0, (float) 0.0,
	(float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0, (float) 0.0,
	(float) 0.0, (float) 0.0
};

/* 6.4k (for NTT CS-VQ)*/
float gbk1_6k[NCODE1_6K][2] = {
	{(float) 0.357003, (float) 0.00000},
/*{ (float)0.357003, (float)0.000010},*/
	{(float) 0.178752, (float) 0.065771},
	{(float) 0.575276, (float) 0.166704},
	{(float) 0.370335, (float) 0.371903},
	{(float) 0.220734, (float) 0.411803},
	{(float) 0.193548, (float) 0.566385},
	{(float) 0.238962, (float) 0.785625},
	{(float) 0.304379, (float) 1.360714}
};

float gbk2_6k[NCODE2_6K][2] = {
/*{ (float)0.000010, (float)0.254841},*/
	{(float) 0.00000, (float) 0.254841},
	{(float) 0.243384, (float) 0.00000},
/*{ (float)0.243384, (float)0.000010},*/
	{(float) 0.273293, (float) 0.447009},
	{(float) 0.480707, (float) 0.477384},
	{(float) 0.628117, (float) 0.694884},
	{(float) 0.660905, (float) 1.684719},
	{(float) 0.729735, (float) 0.655223},
	{(float) 1.002375, (float) 0.959743}
};

float coef_6k[2][2] = {
	{(float) 36.632507, (float) 2.514171},
	{(float) 0.399259, (float) 0.073709}
};

float thr1_6k[NCODE1_6K - NCAN1_6K] = {
	(float) 1.210869,
	(float) 2.401702
};

float thr2_6k[NCODE2_6K - NCAN2_6K] = {
	(float) 0.525915,
	(float) 0.767320
};

int map1_6k[NCODE1_6K] = { 0, 4, 6, 5, 2, 1, 7, 3 };

int imap1_6k[NCODE1_6K] = { 0, 5, 4, 7, 1, 3, 2, 6 };

int map2_6k[NCODE2_6K] = { 0, 4, 3, 7, 5, 1, 6, 2 };

int imap2_6k[NCODE2_6K] = { 0, 5, 7, 2, 1, 4, 6, 3 };

float freq_prev_reset[M] = {	/* previous LSP vector(init) */
	(float) 0.285599, (float) 0.571199, (float) 0.856798, (float) 1.142397, (float) 1.427997,
	(float) 1.713596, (float) 1.999195, (float) 2.284795, (float) 2.570394, (float) 2.855993
};				/* PI*(float)(j+1)/(float)(M+1) */

float lwindow[M + 2] = {
	(float) 0.99879038,
	(float) 0.99546894,
	(float) 0.98995779,
	(float) 0.98229335,
	(float) 0.97252620,
	(float) 0.96072035,
	(float) 0.94695264,
	(float) 0.93131180,
	(float) 0.91389754,
	(float) 0.89481964,
	(float) 0.87419660,
	(float) 0.85215437
};

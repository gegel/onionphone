/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*

2.4 kbps MELP Proposed Federal Standard speech coder

Fixed-point C code, version 1.0

Copyright (c) 1998, Texas Instruments, Inc.

Texas Instruments has intellectual property rights on the MELP
algorithm.	The Texas Instruments contact for licensing issues for
commercial and non-government use is William Gordon, Director,
Government Contracts, Texas Instruments Incorporated, Semiconductor
Group (phone 972 480 7442).

The fixed-point version of the voice codec Mixed Excitation Linear
Prediction (MELP) is based on specifications on the C-language software
simulation contained in GSM 06.06 which is protected by copyright and
is the property of the European Telecommunications Standards Institute
(ETSI). This standard is available from the ETSI publication office
tel. +33 (0)4 92 94 42 58. ETSI has granted a license to United States
Department of Defense to use the C-language software simulation contained
in GSM 06.06 for the purposes of the development of a fixed-point
version of the voice codec Mixed Excitation Linear Prediction (MELP).
Requests for authorization to make other use of the GSM 06.06 or
otherwise distribute or modify them need to be addressed to the ETSI
Secretariat fax: +33 493 65 47 16.

*/

/* coeff.c: filter coefficient file */
/*                                  */
/* (C) 1997  Texas Instruments      */
/*                                  */

#include "sc1200.h"

/* Lowpass filter coefficient in second-order sections */
/* Butterworth, 6th order, Cutoff at 1 kHz             */
/* matlab commands: [z, p, k] = butter(6, 1000/4000);  */
/*						sos = zp2sos(z, p, k);		   */

const int16_t lpf_num[(LPF_ORD / 2) * 3] = {	/* Q13 */
	713, 1426, 713,
	798, 1600, 801,
	1016, 2028, 1012
};

/* Signs of coefficients for lpf_den are reversed.  Note that lpf_den[] is    */
/* initialized at 0, 3, 6 with -8192, but the program does not use their      */
/* values.                                                                    */

const int16_t lpf_den[(LPF_ORD / 2) * 3] = {	/* Q13 */
	-8192, 6884, -1543,
	-8192, 7723, -2731,
	-8192, 9793, -5657
};

/* Butterworth bandpass filters in second-order sections */

const int16_t bpf_num[NUM_BANDS * ((BPF_ORD / 2) * 3)] = {	/* Q13 */
	285, 567, 283,		/* lowpass, cutoff at 500 Hz */
	245, 491, 245,
	227, 455, 228,

	5001, -10001, 5001,	/* bandpass, pass band at 500-1000 Hz */
	429, 857, 429,
	1359, 0, -1359,

	4470, -8941, 4470,	/* bandpass, pass band at 1000-2000 Hz */
	1624, 3248, 1624,
	2399, 0, -2399,

	4470, 8941, 4470,	/* bandpass, pass band at 2000-3000 Hz */
	1624, -3248, 1624,
	2399, 0, -2399,

	1020, -2028, 1008,	/* highpass, cutoff at 3000 Hz */
	795, -1599, 805,
	713, -1426, 713
};

const int16_t bpf_num_class[BPF_ORD + 1] = {	/* Q24 */
	484, 2902, 7254, 9672, 7254, 2902, 484
};

/* Signs of coefficients for bpf_den are reversed.  bpf_den[3*n] are          */
/* initialized to -8192 but their values are not used.                        */

const int16_t bpf_den[NUM_BANDS * ((BPF_ORD / 2) * 3)] = {
	-8192, 13772, -6715,	/* lowpass, cutoff at 500 Hz */
	-8192, 11913, -4703,
	-8192, 11051, -3770,

	-8192, 14036, -7108,	/* bandpass, pass band at 500-1000 Hz */
	-8192, 10624, -6408,
	-8192, 11585, -5474,

	-8192, 9687, -6002,	/* bandpass, pass band at 1000-2000 Hz */
	-8192, 645, -5339,
	-8192, 4799, -3393,

	-8192, -9687, -6002,	/* bandpass, pass band at 2000-3000 Hz */
	-8192, -645, -5339,
	-8192, -4799, -3393,

	-8192, -9793, -5657,	/* highpass, cutoff at 3000 Hz */
	-8192, -7723, -2731,
	-8192, -6883, -1543
};

const int16_t bpf_den_class[BPF_ORD + 1] = {	/* Q11 */
	2048, -9184, 17467, -17980, 10542, -3334, 444
};

/* Hamming window coefficents in Q15 */

const int16_t win_cof[LPC_FRAME] = {
	2621, 2628, 2651, 2689, 2741, 2808, 2891, 2988, 3099, 3225,
	3366, 3521, 3690, 3873, 4070, 4280, 4504, 4741, 4990, 5253,
	5528, 5815, 6113, 6424, 6745, 7078, 7421, 7774, 8138, 8510,
	8892, 9283, 9682, 10089, 10504, 10925, 11354, 11789, 12230, 12676,
	13127, 13583, 14043, 14506, 14973, 15442, 15914, 16387, 16862, 17337,
	17813, 18289, 18764, 19238, 19711, 20181, 20649, 21115, 21576, 22034,
	22488, 22936, 23380, 23818, 24250, 24675, 25093, 25504, 25907, 26302,
	26688, 27066, 27434, 27792, 28140, 28478, 28805, 29121, 29426, 29719,
	30000, 30268, 30524, 30768, 30998, 31215, 31419, 31609, 31785, 31947,
	32094, 32228, 32347, 32451, 32541, 32616, 32676, 32721, 32751, 32766,
	32766, 32751, 32721, 32676, 32616, 32541, 32451, 32347, 32228, 32094,
	31947, 31785, 31609, 31419, 31215, 30998, 30768, 30524, 30268, 30000,
	29719, 29426, 29121, 28805, 28478, 28140, 27792, 27434, 27066, 26688,
	26302, 25907, 25504, 25093, 24675, 24250, 23818, 23380, 22936, 22488,
	22034, 21576, 21115, 20649, 20181, 19711, 19238, 18764, 18289, 17813,
	17337, 16862, 16387, 15914, 15442, 14973, 14506, 14043, 13583, 13127,
	12676, 12230, 11789, 11354, 10925, 10504, 10089, 9682, 9283, 8892,
	8510, 8138, 7774, 7421, 7078, 6745, 6424, 6113, 5815, 5528,
	5253, 4990, 4741, 4504, 4280, 4070, 3873, 3690, 3521, 3366,
	3225, 3099, 2988, 2891, 2808, 2741, 2689, 2651, 2628, 2621
};

/* Bandpass filter coeffients */

const int16_t bp_cof[NUM_BANDS][MIX_ORD + 1] = {
	{			/* lowpass, cutoff at 500 Hz */
	 0, -50, -115, -185, -245, -274, -253, -165,
	 0, 242, 548, 897, 1257, 1590, 1860, 2036,
	 2097, 2036, 1860, 1590, 1257, 897, 548, 242,
	 0, -165, -253, -274, -245, -185, -115, -50,
	 0},
	{			/* bandpass, pass band at 500-1000 Hz */
	 0, -41, -46, 42, 238, 470, 593, 456,
	 0, -668, -1286, -1539, -1221, -362, 748, 1677,
	 2037, 1677, 748, -362, -1221, -1539, -1286, -668,
	 0, 456, 593, 470, 238, 42, -46, -41,
	 0},
	{			/* bandpass, pass band at 1000-2000 Hz */
	 0, -38, 162, 342, 0, -506, -357, 126,
	 0, -185, 774, 1656, 0, -2933, -2627, 1556,
	 4188, 1556, -2627, -2933, 0, 1656, 774, -185,
	 0, 126, -357, -506, 0, 342, 162, -38,
	 0},
	{			/* bandpass, pass band at 2000-3000 Hz */
	 0, 38, 162, -342, 0, 506, -357, -126,
	 0, 185, 774, -1656, 0, 2933, -2627, -1556,
	 4188, -1556, -2627, 2933, 0, -1656, 774, 185,
	 0, -126, -357, 506, 0, -342, 162, 38,
	 0},
	{			/* highpass, cutoff at 3000 Hz */
	 0, 91, -161, 140, 0, -208, 354, -302,
	 0, 442, -768, 680, 0, -1205, 2604, -3725,
	 4153, -3725, 2604, -1205, 0, 680, -768, 442,
	 0, -302, 354, -208, 0, 140, -161, 91,
	 0}
};

/* Triangle pulse dispersion filter */

const int16_t disp_cof[DISP_ORD + 1] = {
	-5670, -461, 401, 3724, 65, 0, 1484, -30,
	-34, 836, -2077, -40, 463, 7971, -579, -6,
	1923, -107, 199, 902, -1098, 197, 471, 27150,
	11, -118, 2406, -170, 425, 960, -652, 399,
	387, -12755, 236, -378, 2761, -117, 705, 973,
	-409, 608, 25, -2539, 408, -892, 2381, 155,
	1156, 876, -244, 846, 6, -926, 564, -1967,
	-2319, 300, 1993, 592, -104, 1129, 9, -345,
	710
};

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

/* ====================================== */
/* pit_lib.h: pitch analysis header files */
/* ====================================== */

#ifndef _PIT_LIB_H_
#define _PIT_LIB_H_

int16_t f_pitch_scale(int16_t sig_out[], int16_t sig_in[],
			int16_t length);

int16_t find_pitch(int16_t sig_in[], int16_t * pcorr, int16_t lower,
		     int16_t upper, int16_t length);

int16_t frac_pch(int16_t sig_in[], int16_t * pcorr, int16_t fpitch,
		   int16_t range, int16_t pmin, int16_t pmax,
		   int16_t pmin_q7, int16_t pmax_q7, int16_t lmin);

int16_t p_avg_update(int16_t pitch, int16_t pcorr, int16_t pthresh);

int16_t pitch_ana(int16_t speech[], int16_t resid[],
		    int16_t pitch_est, int16_t pitch_avg,
		    int16_t * pcorr2);

#endif

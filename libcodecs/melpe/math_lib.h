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

/*
   math_lib.h	 Math include file.
				 (Log and Divide functions.)

   Copyright (c) 1997 by Texas Instruments, Inc.  All rights reserved.
*/

#ifndef _MATH_LIB_H_
#define _MATH_LIB_H_

/* External function definitions */

int16_t L_divider2(int32_t numer, int32_t denom, int16_t numer_shift,
		     int16_t denom_shift);

int16_t log10_fxp(int16_t x, int16_t Q);

int16_t L_log10_fxp(int32_t x, int16_t Q);

int16_t pow10_fxp(int16_t x, int16_t Q);

int16_t sqrt_fxp(int16_t x, int16_t Q);

int16_t L_sqrt_fxp(int32_t x, int16_t Q);

int16_t L_pow_fxp(int32_t x, int16_t power, int16_t Q_in,
		    int16_t Q_out);

int16_t sin_fxp(int16_t x);

int16_t cos_fxp(int16_t x);

int16_t sqrt_Q15(int16_t x);

int16_t add_shr(int16_t Var1, int16_t Var2);

#endif

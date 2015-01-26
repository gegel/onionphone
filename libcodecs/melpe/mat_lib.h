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

/* =================================================================== */
/* mat.h	 Matrix include file.                                      */
/*			 (Low level matrix and vector functions.)                  */
/*                                                                     */
/* Copyright (c) 1995 by Texas Instruments, Inc.  All rights reserved. */
/* =================================================================== */

#ifndef _MAT_LIB_H_
#define _MAT_LIB_H_

int16_t *v_add(int16_t vec1[], const int16_t vec2[], int16_t n);

int16_t *v_equ(int16_t vec1[], const int16_t v2[], int16_t n);

int16_t *v_equ_shr(int16_t vec1[], int16_t vec2[], int16_t scale,
		     int16_t n);

int32_t *L_v_equ(int32_t L_vec1[], int32_t L_vec2[], int16_t n);

int32_t L_v_inner(int16_t vec1[], int16_t vec2[], int16_t n,
		   int16_t qvec1, int16_t qvec2, int16_t qout);

int32_t L_v_magsq(int16_t vec1[], int16_t n, int16_t qvec1,
		   int16_t qout);

int16_t *v_scale(int16_t vec1[], int16_t scale, int16_t n);

int16_t *v_scale_shl(int16_t vec1[], int16_t scale, int16_t n,
		       int16_t shift);

int16_t *v_sub(int16_t vec1[], const int16_t vec2[], int16_t n);

int16_t *v_zap(int16_t vec1[], int16_t n);

int16_t *v_get(int16_t n);

int32_t *L_v_get(int16_t n);

void v_free(void *v);

#endif

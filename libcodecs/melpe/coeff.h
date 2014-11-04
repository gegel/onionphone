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

/* coeff.h: filter coefficient header file */
/*                                         */
/* (C) 1997  Texas Instruments             */
/*                                         */

#ifndef _COEFF_H_
#define _COEFF_H_

#include "sc1200.h"

/* Lowpass filter coefficient in second-order sections */

extern const int16_t lpf_num[];
extern const int16_t lpf_den[];

/* Butterworth bandpass filters in second-order sections */
extern const int16_t bpf_num[];
extern const int16_t bpf_num_class[];

/* sign of coefficients for bpf_den is reversed */
extern const int16_t bpf_den[];
extern const int16_t bpf_den_class[];

/* Hamming window coefficents in Q15 */
extern const int16_t win_cof[];

/* Bandpass filter coeffients */
extern const int16_t bp_cof[][MIX_ORD + 1];

/* Triangle pulse dispersion filter */
extern const int16_t disp_cof[];

#endif

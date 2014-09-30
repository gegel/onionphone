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


Shortword	f_pitch_scale(Shortword sig_out[], Shortword sig_in[],
						  Shortword length);

Shortword	find_pitch(Shortword sig_in[], Shortword *pcorr, Shortword lower,
					   Shortword upper, Shortword length);

Shortword	frac_pch(Shortword sig_in[], Shortword *pcorr, Shortword fpitch,
					 Shortword range, Shortword pmin, Shortword pmax,
					 Shortword pmin_q7, Shortword pmax_q7, Shortword lmin);

Shortword	p_avg_update(Shortword pitch, Shortword pcorr, Shortword pthresh);

Shortword	pitch_ana(Shortword speech[], Shortword resid[],
					  Shortword pitch_est, Shortword pitch_avg,
					  Shortword *pcorr2);


#endif


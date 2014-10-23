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

/* ============================================= */
/* melp_sub.h: include file for MELP subroutines */
/* ============================================= */

#ifndef _MELP_SUB_H_
#define _MELP_SUB_H_

void bpvc_ana(Shortword speech[], Shortword fpitch[], Shortword bpvc[],
	      Shortword * pitch);

void dc_rmv(Shortword sigin[], Shortword sigout[], Shortword delin[],
	    Shortword delout_hi[], Shortword delout_lo[], Shortword frame);

void remove_dc(Shortword sigin[], Shortword sigout[], Shortword len);

Shortword gain_ana(Shortword sigin[], Shortword pitch, Shortword minlength,
		   Shortword maxlength);

Shortword lin_int_bnd(Shortword x, Shortword xmin, Shortword xmax,
		      Shortword ymin, Shortword ymax);

void noise_est(Shortword gain, Shortword * noise_gain, Shortword up,
	       Shortword down, Shortword min, Shortword max);

void noise_sup(Shortword * gain, Shortword noise_gain, Shortword max_noise,
	       Shortword max_atten, Shortword nfact);

BOOLEAN q_bpvc(Shortword * bpvc, Shortword * bpvc_index, Shortword num_bands);

void q_bpvc_dec(Shortword bpvc[], Shortword bpvc_index, BOOLEAN uv_flag,
		Shortword num_bands);

void q_gain(Shortword * gain, Shortword * gain_index, Shortword qlow,
	    Shortword qup, Shortword qlev, Shortword qlev_q,
	    Shortword double_flag, Shortword scale);

void q_gain_dec(Shortword * gain, Shortword * gain_index, Shortword qlow,
		Shortword qup, Shortword qlev_q, Shortword scale);

void scale_adj(Shortword * speech, Shortword gain, Shortword length,
	       Shortword scale_over, Shortword inv_scale_over);

#endif

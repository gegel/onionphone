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

void bpvc_ana(int16_t speech[], int16_t fpitch[], int16_t bpvc[],
	      int16_t * pitch);

void dc_rmv(int16_t sigin[], int16_t sigout[], int16_t delin[],
	    int16_t delout_hi[], int16_t delout_lo[], int16_t frame);

void remove_dc(int16_t sigin[], int16_t sigout[], int16_t len);

int16_t gain_ana(int16_t sigin[], int16_t pitch, int16_t minlength,
		   int16_t maxlength);

int16_t lin_int_bnd(int16_t x, int16_t xmin, int16_t xmax,
		      int16_t ymin, int16_t ymax);

void noise_est(int16_t gain, int16_t * noise_gain, int16_t up,
	       int16_t down, int16_t min, int16_t max);

void noise_sup(int16_t * gain, int16_t noise_gain, int16_t max_noise,
	       int16_t max_atten, int16_t nfact);

BOOLEAN q_bpvc(int16_t * bpvc, int16_t * bpvc_index, int16_t num_bands);

void q_bpvc_dec(int16_t bpvc[], int16_t bpvc_index, BOOLEAN uv_flag,
		int16_t num_bands);

void q_gain(int16_t * gain, int16_t * gain_index, int16_t qlow,
	    int16_t qup, int16_t qlev, int16_t qlev_q,
	    int16_t double_flag, int16_t scale);

void q_gain_dec(int16_t * gain, int16_t * gain_index, int16_t qlow,
		int16_t qup, int16_t qlev_q, int16_t scale);

void scale_adj(int16_t * speech, int16_t gain, int16_t length,
	       int16_t scale_over, int16_t inv_scale_over);

#endif

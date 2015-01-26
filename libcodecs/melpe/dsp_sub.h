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

/* ======================= */
/* dsp_sub.h: include file */
/* ======================= */

#ifndef _DSP_SUB_H_
#define _DSP_SUB_H_

#include <stdio.h>

void envelope(int16_t input[], int16_t prev_in, int16_t output[],
	      int16_t npts);

void fill(int16_t output[], int16_t fillval, int16_t npts);

void L_fill(int32_t output[], int32_t fillval, int16_t npts);

void interp_array(int16_t prev[], int16_t curr[], int16_t out[],
		  int16_t ifact, int16_t size);

int16_t median3(int16_t input[]);

void pack_code(int16_t code, unsigned char **ptr_ch_begin,
	       int16_t * ptr_ch_bit, int16_t numbits, int16_t wsize);

int16_t peakiness(int16_t input[], int16_t npts);

void quant_u(int16_t * p_data, int16_t * p_index, int16_t qmin,
	     int16_t qmax, int16_t nlev, int16_t nlev_q,
	     int16_t double_flag, int16_t scale);

void quant_u_dec(int16_t index, int16_t * p_data, int16_t qmin,
		 int16_t qmax, int16_t nlev_q, int16_t scale);

void rand_num(int16_t output[], int16_t amplitude, int16_t npts);

int16_t rand_minstdgen();

BOOLEAN unpack_code(unsigned char **ptr_ch_begin, int16_t * ptr_ch_bit,
		    int16_t * code, int16_t numbits, int16_t wsize,
		    uint16_t erase_mask);

void window(int16_t input[], const int16_t win_coeff[],
	    int16_t output[], int16_t npts);

void window_Q(int16_t input[], int16_t win_coeff[], int16_t output[],
	      int16_t npts, int16_t Qin);

void zerflt(int16_t input[], const int16_t coeff[], int16_t output[],
	    int16_t order, int16_t npts);

void zerflt_Q(int16_t input[], const int16_t coeff[],
	      int16_t output[], int16_t order, int16_t npts,
	      int16_t Q_coeff);

void iir_2nd_d(int16_t input[], const int16_t den[],
	       const int16_t num[], int16_t output[], int16_t delin[],
	       int16_t delout_hi[], int16_t delout_lo[], int16_t npts);

void iir_2nd_s(int16_t input[], const int16_t den[],
	       const int16_t num[], int16_t output[],
	       int16_t delin[], int16_t delout[], int16_t npts);

int16_t interp_scalar(int16_t prev, int16_t curr, int16_t ifact);

#endif

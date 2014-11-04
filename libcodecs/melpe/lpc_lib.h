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

/* ========================= */
/* lpc.h   LPC include file. */
/* ========================= */

#ifndef _LPC_LIB_H_
#define _LPC_LIB_H_

/* better names */
#define lpc_autocorr			   lpc_acor
#define lpc_bw_expand			   lpc_bwex
#define lpc_lag_window			   lpc_lagw
#define lpc_schur				   lpc_schr
#define lpc_clamp				   lpc_clmp
#define lpc_synthesis			   lpc_syn

/* autocorrelation routine */
void lpc_acor(int16_t input[], const int16_t win_cof[], int16_t r[],
	      int16_t hf_correction, int16_t order, int16_t npts);

int32_t lpc_aejw(int16_t lpc[], int16_t omega, int16_t order);

int16_t lpc_bwex(int16_t lpc[], int16_t aw[], int16_t gamma,
		   int16_t order);

int16_t lpc_clmp(int16_t lsp[], int16_t delta, int16_t order);

int16_t lpc_schr(int16_t autocorr[], int16_t lpc[], int16_t order);

int16_t lpc_pred2lsp(int16_t lpc[], int16_t lsf[], int16_t order);

int16_t lpc_pred2refl(int16_t lpc[], int16_t * refc, int16_t order);

int16_t lpc_lsp2pred(int16_t lsf[], int16_t lpc[], int16_t order);

int16_t lpc_syn(int16_t x[], int16_t y[], int16_t a[],
		  int16_t order, int16_t length);

#endif

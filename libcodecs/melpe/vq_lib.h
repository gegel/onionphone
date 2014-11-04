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

#ifndef _VQ_LIB_H_
#define _VQ_LIB_H_

/* External function definitions */

/* The two aliases mapping msvq_enc() and fsvq_enc() to vq_ms4() and vq_enc() */
/* are now removed because they mask off the interrelationships among the     */
/* signals.                                                                   */

int16_t *vq_lspw(int16_t weight[], int16_t lsp[], int16_t lpc[],
		   int16_t order);

int16_t vq_ms4(const int16_t * cb, int16_t * u, const int16_t * u_est,
		 const int16_t levels[], int16_t ma, int16_t stages,
		 int16_t p, int16_t * w, int16_t * u_hat,
		 int16_t * a_indices, int16_t max_inner);

int32_t vq_enc(const int16_t codebook[], int16_t u[], int16_t levels,
		int16_t order, int16_t u_hat[], int16_t * indices);

void vq_msd2(const int16_t * cb, int16_t * u_hat, const int16_t * u_est,
	     int16_t * indices, const int16_t levels[], int16_t stages,
	     int16_t p, int16_t diff_Q);

void vq_fsw(int16_t w_fs[], int16_t num_harm, int16_t pitch);

#endif

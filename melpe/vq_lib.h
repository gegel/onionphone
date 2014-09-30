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

Shortword	*vq_lspw(Shortword weight[], Shortword lsp[], Shortword lpc[],
					 Shortword order);

Shortword	vq_ms4(const Shortword *cb, Shortword *u, const Shortword *u_est,
				   const Shortword levels[], Shortword ma, Shortword stages,
				   Shortword p, Shortword *w, Shortword *u_hat,
				   Shortword *a_indices, Shortword max_inner);

Longword	vq_enc(const Shortword codebook[], Shortword u[], Shortword levels,
				   Shortword order, Shortword u_hat[], Shortword *indices);

void	vq_msd2(const Shortword *cb, Shortword *u_hat, const Shortword *u_est,
				Shortword *indices, const Shortword levels[], Shortword stages,
				Shortword p, Shortword diff_Q);

void	vq_fsw(Shortword w_fs[], Shortword num_harm, Shortword pitch);


#endif

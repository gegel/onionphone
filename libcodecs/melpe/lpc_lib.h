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
void	lpc_acor(Shortword input[], const Shortword win_cof[], Shortword r[],
				 Shortword hf_correction, Shortword order, Shortword npts);

Longword	lpc_aejw(Shortword lpc[], Shortword omega, Shortword order);

Shortword	lpc_bwex(Shortword lpc[], Shortword aw[], Shortword gamma,
					 Shortword order);

Shortword	lpc_clmp(Shortword lsp[], Shortword delta, Shortword order);

Shortword	lpc_schr(Shortword autocorr[], Shortword lpc[], Shortword order);

Shortword	lpc_pred2lsp(Shortword lpc[], Shortword lsf[], Shortword order);

Shortword	lpc_pred2refl(Shortword lpc[], Shortword *refc, Shortword order);

Shortword	lpc_lsp2pred(Shortword lsf[], Shortword lpc[], Shortword order);

Shortword	lpc_syn(Shortword x[], Shortword y[], Shortword a[],
					Shortword order, Shortword length);


#endif


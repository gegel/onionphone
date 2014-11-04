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

#ifndef _MATHHALF_H_
#define _MATHHALF_H_

/* addition */

static inline Shortword melpe_add(Shortword var1, Shortword var2) __attribute__((always_inline));	/* 1 ops */

static inline Shortword melpe_sub(Shortword var1, Shortword var2) __attribute__((always_inline));	/* 1 ops */

static inline Longword melpe_L_add(Longword L_var1, Longword L_var2) __attribute__((always_inline));	/* 2 ops */

static inline Longword melpe_L_sub(Longword L_var1, Longword L_var2) __attribute__((always_inline));	/* 2 ops */

/* multiplication */

static inline Shortword melpe_mult(Shortword var1, Shortword var2) __attribute__((always_inline));	/* 1 ops */

static inline Longword melpe_L_mult(Shortword var1, Shortword var2) __attribute__((always_inline));	/* 1 ops */

/* arithmetic shifts */

static inline Shortword melpe_shr(Shortword var1, Shortword var2);	/* 1 ops */

static inline Shortword melpe_shl(Shortword var1, Shortword var2) __attribute__((always_inline));	/* 1 ops */

static inline Longword melpe_L_shr(Longword L_var1, Shortword var2);	/* 2 ops */

static inline Longword melpe_L_shl(Longword L_var1, Shortword var2) __attribute__((always_inline));	/* 2 ops */

static inline Shortword melpe_shift_r(Shortword var, Shortword var2) __attribute__((always_inline));	/* 2 ops */

static inline Longword melpe_L_shift_r(Longword L_var, Shortword var2) __attribute__((always_inline));	/* 3 ops */

/* absolute value  */

static inline Shortword melpe_abs_s(Shortword var1) __attribute__((always_inline));	/* 1 ops */

static inline Longword melpe_L_abs(Longword var1) __attribute__((always_inline));	/* 3 ops */

/* multiply accumulate	*/

static inline Longword melpe_L_mac(Longword L_var3, Shortword var1, Shortword var2) __attribute__((always_inline));	/* 1 op */

static inline Longword melpe_L_msu(Longword L_var3, Shortword var1, Shortword var2) __attribute__((always_inline));	/* 1 op */

static inline Shortword melpe_msu_r(Longword L_var3, Shortword var1, Shortword var2) __attribute__((always_inline));	/* 2 op */

/* negation  */

static inline Shortword melpe_negate(Shortword var1) __attribute__((always_inline));	/* 1 ops */

static inline Longword melpe_L_negate(Longword L_var1) __attribute__((always_inline));	/* 2 ops */

/* Accumulator manipulation */

static inline Longword melpe_L_deposit_l(Shortword var1) __attribute__((always_inline));	/* 1 ops */

static inline Longword melpe_L_deposit_h(Shortword var1) __attribute__((always_inline));	/* 1 ops */

static inline Shortword melpe_extract_l(Longword L_var1) __attribute__((always_inline));	/* 1 ops */

static inline Shortword melpe_extract_h(Longword L_var1) __attribute__((always_inline));	/* 1 ops */

/* r_ound */

static inline Shortword melpe_r_ound(Longword L_var1) __attribute__((always_inline));	/* 1 ops */

/* Normalization */

static inline Shortword melpe_norm_l(Longword L_var1) __attribute__((always_inline));	/* 30 ops */

static inline Shortword melpe_norm_s(Shortword var1) __attribute__((always_inline));	/* 15 ops */

/* Division */

static inline Shortword melpe_divide_s(Shortword var1, Shortword var2) __attribute__((always_inline));	/* 18 ops */

/* -------------------------------------------------------------------------- */
/* 40-Bit Routines....added by Andre 11/23/99 */

/* new 40 bits basic operators */

static inline Word40 melpe_L40_add(Word40 acc, Longword L_var1) __attribute__((always_inline));

static inline Word40 melpe_L40_sub(Word40 acc, Longword L_var1) __attribute__((always_inline));

static inline Word40 melpe_L40_mac(Word40 acc, Shortword var1, Shortword var2) __attribute__((always_inline));

static inline Word40 melpe_L40_msu(Word40 acc, Shortword var1, Shortword var2) __attribute__((always_inline));

static inline Word40 melpe_L40_shl(Word40 acc, Shortword var1);

static inline Word40 melpe_L40_shr(Word40 acc, Shortword var1) __attribute__((always_inline));

static inline Word40 melpe_L40_negate(Word40 acc) __attribute__((always_inline));

static inline Shortword melpe_norm32(Word40 acc) __attribute__((always_inline));
static inline Longword melpe_L_sat32(Word40 acc) __attribute__((always_inline));

#include "mathhalf_i.h"

#endif

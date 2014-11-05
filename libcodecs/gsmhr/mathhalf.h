/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#ifndef __MATHHALF
#define __MATHHALF

#include "typedefs.h"

/*_________________________________________________________________________
 |                                                                         |
 |                            Function Prototypes                          |
 |_________________________________________________________________________|
*/

/* addition */
/************/

int16_t add(int16_t var1, int16_t var2);	/* 1 ops */
int16_t sub(int16_t var1, int16_t var2);	/* 1 ops */
int32_t L_add(int32_t L_var1, int32_t L_var2);	/* 2 ops */
int32_t L_sub(int32_t L_var1, int32_t L_var2);	/* 2 ops */

/* multiplication */
/******************/

int16_t mult(int16_t var1, int16_t var2);	/* 1 ops */
int32_t L_mult(int16_t var1, int16_t var2);	/* 1 ops */
int16_t mult_r(int16_t var1, int16_t var2);	/* 2 ops */

/* arithmetic shifts */
/*********************/

int16_t shr(int16_t var1, int16_t var2);	/* 1 ops */
int16_t shl(int16_t var1, int16_t var2);	/* 1 ops */
int32_t L_shr(int32_t L_var1, int16_t var2);	/* 2 ops */
int32_t L_shl(int32_t L_var1, int16_t var2);	/* 2 ops */
int16_t shift_r(int16_t var, int16_t var2);	/* 2 ops */
int32_t L_shift_r(int32_t L_var, int16_t var2);	/* 3 ops */

/* absolute value  */
/*******************/

int16_t abs_s(int16_t var1);	/* 1 ops */
int32_t L_abs(int32_t var1);	/* 3 ops */

/* multiply accumulate  */
/************************/

int32_t L_mac(int32_t L_var3, int16_t var1, int16_t var2);	/* 1 op */
int16_t mac_r(int32_t L_var3, int16_t var1, int16_t var2);	/* 2 op */
int32_t L_msu(int32_t L_var3, int16_t var1, int16_t var2);	/* 1 op */
int16_t msu_r(int32_t L_var3, int16_t var1, int16_t var2);	/* 2 op */

/* negation  */
/*************/

int16_t negate(int16_t var1);	/* 1 ops */
int32_t L_negate(int32_t L_var1);	/* 2 ops */

/* Accumulator manipulation */
/****************************/

int32_t L_deposit_l(int16_t var1);	/* 1 ops */
int32_t L_deposit_h(int16_t var1);	/* 1 ops */
int16_t extract_l(int32_t L_var1);	/* 1 ops */
int16_t extract_h(int32_t L_var1);	/* 1 ops */

/* Round */
/*********/

int16_t hr_round(int32_t L_var1);	/* 1 ops */

/* Normalization */
/*****************/

int16_t norm_l(int32_t L_var1);	/* 30 ops */
int16_t norm_s(int16_t var1);	/* 15 ops */

/* Division */
/************/
int16_t divide_s(int16_t var1, int16_t var2);	/* 18 ops */

/* Non-saturating instructions */
/*******************************/
int32_t L_add_c(int32_t L_Var1, int32_t L_Var2);	/* 2 ops */
int32_t L_sub_c(int32_t L_Var1, int32_t L_Var2);	/* 2 ops */
int32_t L_sat(int32_t L_var1);	/* 4 ops */
int32_t L_macNs(int32_t L_var3, int16_t var1, int16_t var2);	/* 1 ops */
int32_t L_msuNs(int32_t L_var3, int16_t var1, int16_t var2);	/* 1 ops */

#endif

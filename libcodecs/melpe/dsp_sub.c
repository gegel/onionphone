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

/* =============================== */
/* dsp_sub.c: general subroutines. */
/* =============================== */

/*	compiler include files	*/
#include "sc1200.h"
#include "macro.h"
#include "mathhalf.h"
#include "mat_lib.h"
#include "math_lib.h"
#include "dsp_sub.h"
#include "constant.h"
#include "global.h"

#define MAXSIZE			1024
#define LIMIT_PEAKI		20723	/* Upper limit of (magsq/sum_abs) */
					/* to prevent saturation of peak_fact */
#define C2_Q14			-15415	/* -0.9409 * (1 << 14) */
#define C1_Q14			31565	/* 1.9266 * (1 << 14) */
#define A				16807u	/* Multiplier for rand_minstdgen() */

/* Prototype */

static uint32_t L_mpyu(uint16_t var1, uint16_t var2);

/* Subroutine envelope: calculate time envelope of signal.                    */
/* Note: the delay history requires one previous sample	of the input signal   */
/* and two previous output samples.  Output is scaled down by 4 bits from     */
/* input signal.  input[], prev_in and output[] are of the same Q value.      */

void envelope(int16_t input[], int16_t prev_in, int16_t output[],
	      int16_t npts)
{
	register int16_t i;
	int16_t curr_abs, prev_abs;
	int32_t L_temp;

	prev_abs = melpe_abs_s(prev_in);
	for (i = 0; i < npts; i++) {
		curr_abs = melpe_abs_s(input[i]);

		/* output[i] = curr_abs - prev_abs + C2*output[i-2] + C1*output[i-1] */
		L_temp = melpe_L_shr(melpe_L_deposit_h(melpe_sub(curr_abs, prev_abs)), 5);
		L_temp = melpe_L_mac(L_temp, C1_Q14, output[i - 1]);
		L_temp = melpe_L_mac(L_temp, C2_Q14, output[i - 2]);
		L_temp = melpe_L_shl(L_temp, 1);
		output[i] = melpe_r_ound(L_temp);

		prev_abs = curr_abs;
	}
}

/* ====================================================== */
/* This function fills an input array with a given value. */
/* ====================================================== */
void fill(int16_t output[], int16_t fillval, int16_t npts)
{
	register int16_t i;

	for (i = 0; i < npts; i++) {
		output[i] = fillval;
	}
}

/* ====================================================== */
/* This function fills an input array with a given value. */
/* ====================================================== */
void L_fill(int32_t output[], int32_t fillval, int16_t npts)
{
	register int16_t i;

	for (i = 0; i < npts; i++) {
		output[i] = fillval;
	}
}

/* Subroutine interp_array: interpolate array                  */
/*                                                             */
/*	Q values:                                                  */
/*      ifact - Q15, prev[], curr[], out[] - the same Q value. */

void interp_array(int16_t prev[], int16_t curr[], int16_t out[],
		  int16_t ifact, int16_t size)
{
	register int16_t i;
	int16_t ifact2;
	int16_t temp1, temp2;

	if (ifact == 0)
		v_equ(out, prev, size);
	else if (ifact == ONE_Q15)
		v_equ(out, curr, size);
	else {
		ifact2 = melpe_sub(ONE_Q15, ifact);
		for (i = 0; i < size; i++) {
			temp1 = melpe_mult(ifact, curr[i]);
			temp2 = melpe_mult(ifact2, prev[i]);
			out[i] = melpe_add(temp1, temp2);
		}
	}
}

/* Subroutine median: calculate median value of an array with 3 entries.      */

int16_t median3(int16_t input[])
{
	int16_t min, max, temp;

	/* In this coder median() is always invoked with npts being NF (== 3).    */
	/* Therefore we can hardwire npts to NF and optimize the procedure and    */
	/* name the result median3().                                             */

	min = (int16_t) Min(input[0], input[1]);
	max = (int16_t) Max(input[0], input[1]);
	temp = input[2];
	if (temp < min)
		return (min);
	else if (temp > max)
		return (max);
	else
		return (temp);
}

/* ========================================================================== */
/* This function packs bits of "code" into channel.  "numbits" bits of "code" */
/*    is used and they are packed into the array pointed by "ptr_ch_begin".   */
/*    "ptr_ch_bit" points to the position of the next bit being copied onto.  */
/* ========================================================================== */
void pack_code(int16_t code, unsigned char **ptr_ch_begin,
	       int16_t * ptr_ch_bit, int16_t numbits, int16_t wsize)
{
	register int16_t i;
	unsigned char *ch_word;
	int16_t ch_bit;
	int16_t temp;

	ch_bit = *ptr_ch_bit;
	ch_word = *ptr_ch_begin;

	for (i = 0; i < numbits; i++) {	/* Mask in bit from code to channel word */
		/*      temp = shr(code & (shl(1, i)), i); */
		temp = (int16_t) (code & 0x0001);
		if (ch_bit == 0)
			*ch_word = (unsigned char)temp;
		else
			*ch_word |= (unsigned char)(melpe_shl(temp, ch_bit));

		/* Check for full channel word */
		ch_bit = melpe_add(ch_bit, 1);
		if (ch_bit >= wsize) {
			ch_bit = 0;
			(*ptr_ch_begin)++;
			ch_word++;
		}
		code = melpe_shr(code, 1);
	}

	/* Save updated bit counter */
	*ptr_ch_bit = ch_bit;
}

/* Subroutine peakiness: estimate peakiness of input signal using ratio of L2 */
/* to L1 norms.                                                               */
/*                                                                            */
/* Q_values                                                                   */
/* --------                                                                   */
/* peak_fact - Q12, input - Q0                                                */

int16_t peakiness(int16_t input[], int16_t npts)
{
	register int16_t i;
	int16_t peak_fact, scale = 4;
	int32_t sum_abs, L_temp;
	int16_t temp1, temp2, *temp_buf;

	temp_buf = v_get(npts);
	v_equ_shr(temp_buf, input, scale, npts);
	L_temp = L_v_magsq(temp_buf, npts, 0, 1);

	if (L_temp) {
		temp1 = melpe_norm_l(L_temp);
		scale = melpe_sub(scale, melpe_shr(temp1, 1));
		if (scale < 0)
			scale = 0;
	} else
		scale = 0;

	sum_abs = 0;
	for (i = 0; i < npts; i++) {
		L_temp = melpe_L_deposit_l(melpe_abs_s(input[i]));
		sum_abs = melpe_L_add(sum_abs, L_temp);
	}

	/* Right shift input signal and put in temp buffer.                       */
	if (scale)
		v_equ_shr(temp_buf, input, scale, npts);

	if (sum_abs > 0) {
		/*      peak_fact = sqrt(npts * v_magsq(input, npts))/sum_abs             */
		/*                        = sqrt(npts) * (sqrt(v_magsq(input, npts))/sum_abs)     */
		if (scale)
			L_temp = L_v_magsq(temp_buf, npts, 0, 0);
		else
			L_temp = L_v_magsq(input, npts, 0, 0);
		L_temp = melpe_L_deposit_l(L_sqrt_fxp(L_temp, 0));	/* L_temp in Q0 */
		peak_fact = L_divider2(L_temp, sum_abs, 0, 0);

		if (peak_fact > LIMIT_PEAKI) {
			peak_fact = SW_MAX;
		} else {	/* shl 7 is mult , other shift is Q7->Q12 */
			temp1 = melpe_add(scale, 5);
			temp2 = melpe_shl(npts, 7);
			temp2 = sqrt_fxp(temp2, 7);
			L_temp = melpe_L_mult(peak_fact, temp2);
			L_temp = melpe_L_shl(L_temp, temp1);
			peak_fact = melpe_extract_h(L_temp);
		}
	} else
		peak_fact = 0;

	v_free(temp_buf);
	return (peak_fact);
}

/* Subroutine quant_u(): quantize positive input value with	symmetrical       */
/* uniform quantizer over given positive input range.                         */

void quant_u(int16_t * p_data, int16_t * p_index, int16_t qmin,
	     int16_t qmax, int16_t nlev, int16_t nlev_q,
	     int16_t double_flag, int16_t scale)
{
	register int16_t i;
	int16_t step, half_step, qbnd, *p_in;
	int32_t L_step, L_half_step, L_qbnd, L_qmin, L_p_in;
	int16_t temp;
	int32_t L_temp;

	p_in = p_data;

	/*  Define symmetrical quantizer stepsize       */
	/* step = (qmax - qmin) / (nlev - 1); */
	temp = melpe_sub(qmax, qmin);
	step = melpe_divide_s(temp, nlev_q);

	if (double_flag) {
		/* double precision specified */
		/*      Search quantizer boundaries                                     */
		/*qbnd = qmin + (0.5 * step); */
		L_step = melpe_L_deposit_l(step);
		L_half_step = melpe_L_shr(L_step, 1);
		L_qmin = melpe_L_shl(melpe_L_deposit_l(qmin), scale);
		L_qbnd = melpe_L_add(L_qmin, L_half_step);

		L_p_in = melpe_L_shl(melpe_L_deposit_l(*p_in), scale);
		for (i = 0; i < nlev; i++) {
			if (L_p_in < L_qbnd)
				break;
			else
				L_qbnd = melpe_L_add(L_qbnd, L_step);
		}
		/* Quantize input to correct level */
		/* *p_in = qmin + (i * step); */
		L_temp = melpe_L_sub(L_qbnd, L_half_step);
		*p_in = melpe_extract_l(melpe_L_shr(L_temp, scale));
		*p_index = i;
	} else {
		/* Search quantizer boundaries */
		/* qbnd = qmin + (0.5 * step); */
		step = melpe_shr(step, scale);
		half_step = melpe_shr(step, 1);
		qbnd = melpe_add(qmin, half_step);

		for (i = 0; i < nlev; i++) {
			if (*p_in < qbnd)
				break;
			else
				qbnd = melpe_add(qbnd, step);
		}
		/*      Quantize input to correct level */
		/* *p_in = qmin + (i * step); */
		*p_in = melpe_sub(qbnd, half_step);
		*p_index = i;
	}
}

/* Subroutine quant_u_dec(): decode uniformly quantized value.                */
void quant_u_dec(int16_t index, int16_t * p_data, int16_t qmin,
		 int16_t qmax, int16_t nlev_q, int16_t scale)
{
	int16_t step, temp;
	int32_t L_qmin, L_temp;

	/* Define symmetrical quantizer stepsize.  (nlev - 1) is computed in the  */
	/* calling function.                                                      */

	/*      step = (qmax - qmin) / (nlev - 1); */
	temp = melpe_sub(qmax, qmin);
	step = melpe_divide_s(temp, nlev_q);

	/* Decode quantized level */
	/* double precision specified */

	L_temp = melpe_L_shr(melpe_L_mult(step, index), 1);
	L_qmin = melpe_L_shl(melpe_L_deposit_l(qmin), scale);
	L_temp = melpe_L_add(L_qmin, L_temp);
	*p_data = melpe_extract_l(melpe_L_shr(L_temp, scale));
}

/* Subroutine rand_num: generate random numbers to fill array using "minimal  */
/* standard" random number generator.                                         */

void rand_num(int16_t output[], int16_t amplitude, int16_t npts)
{
	register int16_t i;
	int16_t temp;

	for (i = 0; i < npts; i++) {

		/* rand_minstdgen returns 0 <= x < 1 */
		/* -0.5 <= temp < 0.5 */
		temp = melpe_sub(rand_minstdgen(), X05_Q15);
		output[i] = melpe_mult(amplitude, melpe_shl(temp, 1));
	}
}

/****************************************************************************/
/* RAND() - COMPUTE THE NEXT VALUE IN THE RANDOM NUMBER SEQUENCE.			*/
/*																			*/
/*	   The sequence used is x' = (A*x) mod M,  (A = 16807, M = 2^31 - 1).	*/
/*	   This is the "minimal standard" generator from CACM Oct 1988, p. 1192.*/
/*	   The implementation is based on an algorithm using 2 31-bit registers */
/*	   to represent the product (A*x), from CACM Jan 1990, p. 87.			*/
/*																			*/
/****************************************************************************/

int16_t rand_minstdgen()
{
	static uint32_t next = 1;	/* seed; must not be zero!!! */
	int32_t old_saturation;
	uint16_t x0 = melpe_extract_l(next);	/* 16 LSBs OF SEED */
	uint16_t x1 = melpe_extract_h(next);	/* 16 MSBs OF SEED */
	uint32_t p, q;		/* MSW, LSW OF PRODUCT */
	uint32_t L_temp1, L_temp2, L_temp3;

	/*----------------------------------------------------------------------*/
	/* COMPUTE THE PRODUCT (A * next) USING CROSS MULTIPLICATION OF         */
	/* 16-BIT HALVES OF THE INPUT VALUES.   THE RESULT IS REPRESENTED AS 2  */
	/* 31-BIT VALUES.       SINCE 'A' FITS IN 15 BITS, ITS UPPER HALF CAN BE    */
	/* DISREGARDED.  USING THE NOTATION val[m::n] TO MEAN "BITS n THROUGH   */
	/* m OF val", THE PRODUCT IS COMPUTED AS:                               */
	/*   q = (A * x)[0::30]  = ((A * x1)[0::14] << 16) + (A * x0)[0::30]    */
	/*   p = (A * x)[31::60] =  (A * x1)[15::30]            + (A * x0)[31]  + C */
	/* WHERE C = q[31] (CARRY BIT FROM q).  NOTE THAT BECAUSE A < 2^15,     */
	/* (A * x0)[31] IS ALWAYS 0.                                            */
	/*----------------------------------------------------------------------*/
	/* save and reset saturation count                                      */

	old_saturation = saturation;
	saturation = 0;

	/* q = ((A * x1) << 17 >> 1) + (A * x0); */
	/* p = ((A * x1) >> 15) + (q >> 31); */
	/* q = q << 1 >> 1; *//* CLEAR CARRY */
	L_temp1 = L_mpyu(A, x1);
	/* store bit 15 to bit 31 in p */
	p = melpe_L_shr(L_temp1, 15);
	/* mask bit 15 to bit 31 */
	L_temp1 = melpe_L_shl((L_temp1 & (int32_t) 0x00007fff), 16);
	L_temp2 = L_mpyu(A, x0);
	L_temp3 = melpe_L_sub(LW_MAX, L_temp1);
	if (L_temp2 > L_temp3) {
		/* subtract 0x80000000 from sum */
		L_temp1 = melpe_L_sub(L_temp1, (int32_t) 0x7fffffff);
		L_temp1 = melpe_L_sub(L_temp1, 1);
		q = melpe_L_add(L_temp1, L_temp2);
		p = melpe_L_add(p, 1);
	} else
		q = melpe_L_add(L_temp1, L_temp2);

	/*---------------------------------------------------------------------*/
	/* IF (p + q) < 2^31, RESULT IS (p + q).  OTHERWISE, RESULT IS         */
	/* (p + q) - 2^31 + 1.  (SEE REFERENCES).                              */
	/*---------------------------------------------------------------------*/
	/* p += q; next = ((p + (p >> 31)) << 1) >> 1; */
	/* ADD CARRY, THEN CLEAR IT */

	L_temp3 = melpe_L_sub(LW_MAX, p);
	if (q > L_temp3) {
		/* subtract 0x7fffffff from sum */
		L_temp1 = melpe_L_sub(p, (int32_t) 0x7fffffff);
		L_temp1 = melpe_L_add(L_temp1, q);
	} else
		L_temp1 = melpe_L_add(p, q);
	next = L_temp1;

	/* restore saturation count */
	saturation = old_saturation;

	x1 = melpe_extract_h(next);
	return (x1);
}

/***************************************************************************
 *
 *	 FUNCTION NAME: L_mpyu
 *
 *	 PURPOSE:
 *
 *	   Perform an unsigned fractional multipy of the two unsigned 16 bit
 *	   input numbers with saturation.  Output a 32 bit unsigned number.
 *
 *	 INPUTS:
 *
 *	   var1
 *					   16 bit short unsigned integer (int16_t) whose value
 *					   falls in the range 0xffff 0000 <= var1 <= 0x0000 ffff.
 *	   var2
 *					   16 bit short unsigned integer (int16_t) whose value
 *					   falls in the range 0xffff 0000 <= var2 <= 0x0000 ffff.
 *
 *	 OUTPUTS:
 *
 *	   none
 *
 *	 RETURN VALUE:
 *
 *	   L_Out
 *					   32 bit long unsigned integer (int32_t) whose value
 *					   falls in the range
 *					   0x0000 0000 <= L_var1 <= 0xffff ffff.
 *
 *	 IMPLEMENTATION:
 *
 *	   Multiply the two unsigned 16 bit input numbers.
 *
 *	 KEYWORDS: multiply, mult, mpy
 *
 *************************************************************************/

static uint32_t L_mpyu(uint16_t var1, uint16_t var2)
{
	uint32_t L_product;

	L_product = (uint32_t) var1 *var2;	/* integer multiply */
	return (L_product);
}

/* ========================================================================== */
/* This function unpacks bits of "code" from channel.  It returns 1 if an     */
/*    erasure is encountered, or 0 otherwise.  "numbits" bits of "code" */
/*    is used and they are packed into the array pointed by "ptr_ch_begin".   */
/*    "ptr_ch_bit" points to the position of the next bit being copied onto.  */
/* ========================================================================== */
BOOLEAN unpack_code(unsigned char **ptr_ch_begin, int16_t * ptr_ch_bit,
		    int16_t * code, int16_t numbits, int16_t wsize,
		    uint16_t erase_mask)
{
	register int16_t i;
	unsigned char *ch_word;
	int16_t ret_code;
	int16_t ch_bit;

	ch_bit = *ptr_ch_bit;
	ch_word = *ptr_ch_begin;
	*code = 0;
	ret_code = (int16_t) (*ch_word & erase_mask);

	for (i = 0; i < numbits; i++) {
		/* Mask in bit from channel word to code */
		*code |=
		    melpe_shl(melpe_shr
			((int16_t) ((int16_t) * ch_word & melpe_shl(1, ch_bit)),
			 ch_bit), i);

		/* Check for end of channel word */
		ch_bit = melpe_add(ch_bit, 1);
		if (ch_bit >= wsize) {
			ch_bit = 0;
			(*ptr_ch_begin)++;
			ch_word++;
		}
	}

	/* Save updated bit counter */
	*ptr_ch_bit = ch_bit;

	/* Catch erasure in new word if read */
	if (ch_bit != 0)
		ret_code |= *ch_word & erase_mask;

	return (ret_code);
}

/* ============================================ */
/* Subroutine window: multiply signal by window */
/*                                              */
/* Q values:                                    */
/*    input - Q0, win_coeff - Q15, output - Q0  */
/* ============================================ */

void window(int16_t input[], const int16_t win_coeff[], int16_t output[],
	    int16_t npts)
{
	register int16_t i;

	for (i = 0; i < npts; i++) {
		output[i] = melpe_mult(win_coeff[i], input[i]);
	}
}

/* ============================================== */
/* Subroutine window_Q: multiply signal by window */
/*                                                */
/* Q values:                                      */
/*    win_coeff - Qin, output = input             */
/* ============================================== */

void window_Q(int16_t input[], int16_t win_coeff[], int16_t output[],
	      int16_t npts, int16_t Qin)
{
	register int16_t i;
	int16_t shift;

	/* After computing "shift", win_coeff[]*2^(-shift) is considered Q15.     */
	shift = melpe_sub(15, Qin);
	for (i = 0; i < npts; i++) {
		output[i] =
		    melpe_extract_h(melpe_L_shl(melpe_L_mult(win_coeff[i], input[i]), shift));
	}
}

/* Subroutine zerflt(): all zero (FIR) filter.                                */
/*   Note: the output array can overlay the input.                            */
/*                                                                            */
/* Q values:                                                                  */
/*   input[], output[] - Q0, coeff[] - Q12                                    */

void zerflt(int16_t input[], const int16_t coeff[], int16_t output[],
	    int16_t order, int16_t npts)
{
	register int16_t i, j;
	int32_t accum;

	for (i = melpe_sub(npts, 1); i >= 0; i--) {
		accum = 0;
		for (j = 0; j <= order; j++)
			accum = melpe_L_mac(accum, input[i - j], coeff[j]);
		/* r_ound off output */
		accum = melpe_L_shl(accum, 3);
		output[i] = melpe_r_ound(accum);
	}
}

/* Subroutine zerflt_Q: all zero (FIR) filter.                                */
/* Note: the output array can overlay the input.                              */
/*                                                                            */
/* Q values:                                                                  */
/* coeff - specified by Q_coeff, output - same as input                       */

void zerflt_Q(int16_t input[], const int16_t coeff[], int16_t output[],
	      int16_t order, int16_t npts, int16_t Q_coeff)
{
	register int16_t i, j;
	int16_t scale;
	int32_t accum;

	scale = melpe_sub(15, Q_coeff);
	for (i = melpe_sub(npts, 1); i >= 0; i--) {
		accum = 0;
		for (j = 0; j <= order; j++)
			accum = melpe_L_mac(accum, input[i - j], coeff[j]);
		/* r_ound off output */
		accum = melpe_L_shl(accum, scale);
		output[i] = melpe_r_ound(accum);
	}
}

/* ========================================================================== */
/* Subroutine iir_2nd_d: Second order IIR filter (Double precision)           */
/*    Note: the output array can overlay the input.                           */
/*                                                                            */
/* Input scaled down by a factor of 2 to prevent overflow                     */
/* ========================================================================== */
void iir_2nd_d(int16_t input[], const int16_t den[], const int16_t num[],
	       int16_t output[], int16_t delin[], int16_t delout_hi[],
	       int16_t delout_lo[], int16_t npts)
{
	register int16_t i;
	int16_t temp;
	int32_t accum;

	for (i = 0; i < npts; i++) {
		accum = melpe_L_mult(delout_lo[0], den[1]);
		accum = melpe_L_mac(accum, delout_lo[1], den[2]);
		accum = melpe_L_shr(accum, 14);
		accum = melpe_L_mac(accum, delout_hi[0], den[1]);
		accum = melpe_L_mac(accum, delout_hi[1], den[2]);

		accum = melpe_L_mac(accum, melpe_shr(input[i], 1), num[0]);
		accum = melpe_L_mac(accum, delin[0], num[1]);
		accum = melpe_L_mac(accum, delin[1], num[2]);

		/* shift result to correct Q value */
		accum = melpe_L_shl(accum, 2);	/* assume coefficients in Q13 */

		/* update input delay buffer */
		delin[1] = delin[0];
		delin[0] = melpe_shr(input[i], 1);

		/* update output delay buffer */
		delout_hi[1] = delout_hi[0];
		delout_lo[1] = delout_lo[0];
		delout_hi[0] = melpe_extract_h(accum);
		temp = melpe_shr(melpe_extract_l(accum), 2);
		delout_lo[0] = (int16_t) (temp & (int16_t) 0x3FFF);

		/* r_ound off result */
		accum = melpe_L_shl(accum, 1);
		output[i] = melpe_r_ound(accum);
	}
}

/* ========================================================================== */
/* Subroutine iir_2nd_s: Second order IIR filter (Single precision)           */
/*    Note: the output array can overlay the input.                           */
/*                                                                            */
/* Input scaled down by a factor of 2 to prevent overflow                     */
/* ========================================================================== */
void iir_2nd_s(int16_t input[], const int16_t den[], const int16_t num[],
	       int16_t output[], int16_t delin[], int16_t delout[],
	       int16_t npts)
{
	register int16_t i;
	int32_t accum;

	for (i = 0; i < npts; i++) {
		accum = melpe_L_mult(input[i], num[0]);
		accum = melpe_L_mac(accum, delin[0], num[1]);
		accum = melpe_L_mac(accum, delin[1], num[2]);

		accum = melpe_L_mac(accum, delout[0], den[1]);
		accum = melpe_L_mac(accum, delout[1], den[2]);

		/* shift result to correct Q value */
		accum = melpe_L_shl(accum, 2);	/* assume coefficients in Q13 */

		/* update input delay buffer */
		delin[1] = delin[0];
		delin[0] = input[i];

		/* r_ound off result */
		output[i] = melpe_r_ound(accum);

		/* update output delay buffer */
		delout[1] = delout[0];
		delout[0] = output[i];
	}
}

/* ========================================================================== */
/* This function interpolates two scalars to obtain the output.  It calls     */
/* interp_array() to do the job.  "ifact" is Q15, and "prev", "curr" and the  */
/* returned value are of the same Q value.                                    */
/* ========================================================================== */

int16_t interp_scalar(int16_t prev, int16_t curr, int16_t ifact)
{
	int16_t out;

	interp_array(&prev, &curr, &out, ifact, 1);
	return (out);
}

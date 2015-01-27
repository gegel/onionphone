/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*

2.4 kbps MELP Proposed Federal Standard mf_speech coder

version 1.2

Copyright (c) 1996, Texas Instruments, Inc.  

Texas Instruments has intellectual property rights on the MELP
algorithm.  The Texas Instruments contact for licensing issues for
commercial and non-government use is William Gordon, Director,
Government Contracts, Texas Instruments Incorporated, Semiconductor
Group (phone 972 480 7442).


*/

/*  

  dsp_sub.c: general subroutines.

*/

/*  compiler include files  */
#include	<stdio.h>
#include	<stdlib.h>
#include	<math.h>
#include "dsp_sub.h"
#include "spbstd.h"
#include "mat.h"

typedef short SPEECH;
#define PRINT 1

/*								*/
/*	Subroutine mf_autocorr: calculate mf_autocorrelations         */
/*								*/
void mf_autocorr(float input[], float r[], int order, int npts)
{
	int i;

	for (i = 0; i <= order; i++)
		r[i] = mf_v_inner(&input[0], &input[i], (npts - i));
	if (r[0] < 1.0)
		r[0] = 1.0;

}

/*								*/
/*	Subroutine mf_envelope: calculate time mf_envelope of signal. */
/*      Note: the delay history requires one previous sample    */
/*      of the input signal and two previous output samples.    */
/*								*/
#define C2 (-0.9409)
#define C1 1.9266

void mf_envelope(float input[], float prev_in, float output[], int npts)
{
	int i;
	float curr_abs, prev_abs;

	prev_abs = fabs(prev_in);
	for (i = 0; i < npts; i++) {
		curr_abs = fabs(input[i]);
		output[i] =
		    curr_abs - prev_abs + C2 * output[i - 2] + C1 * output[i -
									   1];
		prev_abs = curr_abs;
	}
}

/*								*/
/*  Subroutine mf_fill: mf_fill an input array with a value.		*/
/*								*/
void mf_fill(float output[], float mf_fillval, int npts)
{
	int i;

	for (i = 0; i < npts; i++)
		output[i] = mf_fillval;

}

/*								*/
/*	Subroutine mf_interp_array: interpolate array              */
/*                                                              */
void mf_interp_array(float prev[], float curr[], float out[], float ifact,
		     int size)
{
	int i;
	float ifact2;

	ifact2 = 1.0 - ifact;
	for (i = 0; i < size; i++)
		out[i] = ifact * curr[i] + ifact2 * prev[i];

}

/*								*/
/*	Subroutine mf_median: calculate mf_median value               */
/*								*/
#define MAXSORT 5
float mf_median(float input[], int npts)
{
	int i, j, loc;
	float insert_val;
	float sorted[MAXSORT];

	/* sort data in temporary array */

#ifdef PRINT
	if (npts > MAXSORT) {
		printf("ERROR: mf_median size too large.\n");
		exit(1);
	}
#endif

	mf_v_equ(sorted, input, npts);
	for (i = 1; i < npts; i++) {

		/* for each data point */
		for (j = 0; j < i; j++) {

			/* find location in current sorted list */
			if (sorted[i] < sorted[j])
				break;
		}

		/* insert new value */
		loc = j;
		insert_val = sorted[i];
		for (j = i; j > loc; j--)
			sorted[j] = sorted[j - 1];
		sorted[loc] = insert_val;
	}

	return (sorted[npts / 2]);

}

#undef MAXSORT

/*								*/
/*	Subroutine PACK_CODE: Pack bit code into channel.	*/
/*								*/
void mf_pack_code(int code, unsigned int **p_ch_beg, int *p_ch_bit, int numbits,
		  int wsize)
{
	int i, ch_bit;
	unsigned int *ch_word;

	ch_bit = *p_ch_bit;
	ch_word = *p_ch_beg;

	for (i = 0; i < numbits; i++) {
		/* Mask in bit from code to channel word        */
		if (ch_bit == 0)
			*ch_word = ((code & (1 << i)) >> i);
		else
			*ch_word |= (((code & (1 << i)) >> i) << ch_bit);

		/* Check for full channel word                  */
		if (++ch_bit >= wsize) {
			ch_bit = 0;
			(*p_ch_beg)++;
			ch_word++;
		}
	}

	/* Save updated bit counter     */
	*p_ch_bit = ch_bit;
}

/*								*/
/*	Subroutine mf_peakiness: estimate mf_peakiness of input       */
/*      signal using ratio of L2 to L1 norms.                   */
/*								*/
float mf_peakiness(float input[], int npts)
{
	int i;
	float sum_abs, peak_fact;

	sum_abs = 0.0;
	for (i = 0; i < npts; i++)
		sum_abs += fabs(input[i]);

	if (sum_abs > 0.01)
		peak_fact = sqrt(npts * v_mf_magsq(input, npts)) / sum_abs;
	else
		peak_fact = 0.0;

	return (peak_fact);
}

/*								*/
/*	Subroutine mf_polflt: all pole (IIR) filter.		*/
/*	Note: The filter coefficients represent the		*/
/*	denominator only, and the leading coefficient		*/
/*	is assumed to be 1.					*/
/*      The output array can overlay the input.                 */
/*								*/
void mf_polflt(float input[], float coeff[], float output[], int order,
	       int npts)
{
	int i, j;
	float accum;

	for (i = 0; i < npts; i++) {
		accum = input[i];
		for (j = 1; j <= order; j++)
			accum -= output[i - j] * coeff[j];
		output[i] = accum;
	}
}

/*								*/
/*	Subroutine QUANT_U: quantize positive input value with 	*/
/*	symmetrical uniform quantizer over given positive	*/
/*	input range.						*/
/*								*/
void mf_quant_u(float *p_data, int *p_index, float qmin, float qmax, int nlev)
{
	register int i, j;
	register float step, qbnd, *p_in;

	p_in = p_data;

	/*  Define symmetrical quantizer stepsize       */
	step = (qmax - qmin) / (nlev - 1);

	/*  Search quantizer boundaries                 */
	qbnd = qmin + (0.5 * step);
	j = nlev - 1;
	for (i = 0; i < j; i++) {
		if (*p_in < qbnd)
			break;
		else
			qbnd += step;
	}

	/*  Quantize input to correct level             */
	*p_in = qmin + (i * step);
	*p_index = i;

}

/*								*/
/*	Subroutine QUANT_U_DEC: decode uniformly quantized	*/
/*	value.							*/
/*								*/
void mf_mf_quant_u_dec(int index, float *p_data, float qmin, float qmax,
		       int nlev)
{
	register float step;

	/*  Define symmetrical quantizer stepsize       */
	step = (qmax - qmin) / (nlev - 1);

	/*  Decode quantized level                      */
	*p_data = qmin + (index * step);

}

/*								*/
/*	Subroutine mf_rand_num: generate random numbers to mf_fill    */
/*      array using system random number generator.             */
/*                                                              */
void mf_rand_num(float output[], float amplitude, int npts)
{
	int i;

	for (i = 0; i < npts; i++) {

		/* use system random number generator from -1 to +1 */
#ifdef RAND_MAX
		/* ANSI C environment */
		output[i] =
		    (amplitude * 2.0) * ((float)rand() * (1.0 / RAND_MAX) -
					 0.5);
#else
		/* assume Sun OS4 */
		output[i] =
		    amplitude * (float)(((random() >> 16) / 32767. - .5) * 2);
#endif

	}
}

/*								*/
/*	Subroutine UNPACK_CODE: Unpack bit code from channel.	*/
/*      Return 1 if erasure, otherwise 0.                       */
/*								*/
int unmf_pack_code(unsigned int **p_ch_beg, int *p_ch_bit, int *p_code,
		   int numbits, int wsize, unsigned int ERASE_MASK)
{

	int ret_code;
	int i, ch_bit;
	unsigned int *ch_word;

	ch_bit = *p_ch_bit;
	ch_word = *p_ch_beg;
	*p_code = 0;
	ret_code = *ch_word & ERASE_MASK;

	for (i = 0; i < numbits; i++) {
		/* Mask in bit from channel word to code        */
		*p_code |= (((*ch_word & (1 << ch_bit)) >> ch_bit) << i);

		/* Check for end of channel word                */
		if (++ch_bit >= wsize) {
			ch_bit = 0;
			(*p_ch_beg)++;
			ch_word++;
		}
	}

	/*  Save updated bit counter    */
	*p_ch_bit = ch_bit;

	/* Catch erasure in new word if read */
	if (ch_bit != 0)
		ret_code |= *ch_word & ERASE_MASK;

	return (ret_code);
}

/*								*/
/*	Subroutine mf_window: multiply signal by mf_window            */
/*								*/
void mf_window(float input[], float mf_win_cof[], float output[], int npts)
{
	int i;

	for (i = 0; i < npts; i++)
		output[i] = mf_win_cof[i] * input[i];

}

/*								*/
/*	Subroutine mf_zerflt: all zero (FIR) filter.		*/
/*      Note: the output array can overlay the input.           */
/*								*/
void mf_zerflt(float input[], float coeff[], float output[], int order,
	       int npts)
{
	int i, j;
	float accum;

	for (i = npts - 1; i >= 0; i--) {
		accum = 0.0;
		for (j = 0; j <= order; j++)
			accum += input[i - j] * coeff[j];
		output[i] = accum;
	}
}

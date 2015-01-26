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

/* mat_lib.c: Matrix and vector manipulation library                          */

#include <stdlib.h>
#include <assert.h>

#include "sc1200.h"
#include "mathhalf.h"
#include "mat_lib.h"

/***************************************************************************
 *
 *	 FUNCTION NAME: v_add
 *
 *	 PURPOSE:
 *
 *	   Perform the addition of the two 16 bit input vector with
 *	   saturation.
 *
 *	 INPUTS:
 *
 *	   vec1 		   16 bit short signed integer (int16_t) vector whose
 *					   values fall in the range
 *					   0xffff 8000 <= vec1 <= 0x0000 7fff.
 *
 *	   vec2 		   16 bit short signed integer (int16_t) vector whose
 *					   values falls in the range
 *					   0xffff 8000 <= vec2 <= 0x0000 7fff.
 *
 *	   n			   size of input vectors.
 *
 *	 OUTPUTS:
 *
 *	   none
 *
 *	 RETURN VALUE:
 *
 *	   vec1 		   16 bit short signed integer (int16_t) vector whose
 *					   values fall in the range
 *					   0xffff 8000 <= vec1[] <= 0x0000 7fff.
 *
 *	 IMPLEMENTATION:
 *
 *	   Perform the addition of the two 16 bit input vectors with
 *	   saturation.
 *
 *	   vec1 = vec1 + vec2
 *
 *	   vec1[] is set to 0x7fff if the operation results in an
 *	   overflow.  vec1[] is set to 0x8000 if the operation results
 *	   in an underflow.
 *
 *	 KEYWORDS: add, addition
 *
 *************************************************************************/

int16_t *v_add(int16_t vec1[], const int16_t vec2[], int16_t n)
{
	register int16_t i;

	for (i = 0; i < n; i++) {
		*vec1 = melpe_add(*vec1, *vec2);
		vec1++;
		vec2++;
	}
	return (vec1 - n);
}

/***************************************************************************
 *
 *	 FUNCTION NAME: v_equ
 *
 *	 PURPOSE:
 *
 *	   Copy the contents of one 16 bit input vector to another
 *
 *	 INPUTS:
 *
 *	   vec2 		   16 bit short signed integer (int16_t) vector whose
 *					   values falls in the range
 *					   0xffff 8000 <= vec2 <= 0x0000 7fff.
 *
 *	   n			   size of input vector
 *
 *	 OUTPUTS:
 *
 *	   vec1 		   16 bit short signed integer (int16_t) vector whose
 *					   values fall in the range
 *					   0xffff 8000 <= vec1 <= 0x0000 7fff.
 *
 *	 RETURN VALUE:
 *
 *	   vec1 		   16 bit short signed integer (int16_t) vector whose
 *					   values fall in the range
 *					   0xffff 8000 <= vec1[] <= 0x0000 7fff.
 *
 *	 IMPLEMENTATION:
 *
 *	   Copy the contents of one 16 bit input vector to another
 *
 *	   vec1 = vec2
 *
 *	 KEYWORDS: equate, copy
 *
 *************************************************************************/
int16_t *v_equ(int16_t vec1[], const int16_t vec2[], int16_t n)
{
	register int16_t i;

	for (i = 0; i < n; i++) {
		*vec1 = *vec2;
		vec1++;
		vec2++;
	}
	return (vec1 - n);
}

/***************************************************************************
 *
 *	 FUNCTION NAME: v_equ_shr
 *
 *	 PURPOSE:
 *
 *	   Copy the contents of one 16 bit input vector to another with shift
 *
 *	 INPUTS:
 *
 *	   vec2 		   16 bit short signed integer (int16_t) vector whose
 *					   values falls in the range
 *					   0xffff 8000 <= vec2 <= 0x0000 7fff.
 *	   scale		   right shift factor
 *	   n			   size of input vector
 *
 *	 OUTPUTS:
 *
 *	   vec1 		   16 bit short signed integer (int16_t) vector whose
 *					   values fall in the range
 *					   0xffff 8000 <= vec1 <= 0x0000 7fff.
 *
 *	 RETURN VALUE:
 *
 *	   vec1 		   16 bit short signed integer (int16_t) vector whose
 *					   values fall in the range
 *					   0xffff 8000 <= vec1[] <= 0x0000 7fff.
 *
 *	 IMPLEMENTATION:
 *
 *	   Copy the contents of one 16 bit input vector to another with shift
 *
 *	   vec1 = vec2>>scale
 *
 *	 KEYWORDS: equate, copy
 *
 *************************************************************************/

int16_t *v_equ_shr(int16_t vec1[], int16_t vec2[], int16_t scale,
		     int16_t n)
{
	register int16_t i;

	for (i = 0; i < n; i++) {
		*vec1 = melpe_shr(*vec2, scale);
		vec1++;
		vec2++;
	}
	return (vec1 - n);
}

/***************************************************************************
 *
 *	 FUNCTION NAME: L_v_equ
 *
 *	 PURPOSE:
 *
 *	   Copy the contents of one 32 bit input vector to another
 *
 *	 INPUTS:
 *
 *	   L_vec2		   32 bit long signed integer (int32_t) vector whose
 *					   values falls in the range
 *					   0x8000 0000 <= L_vec2 <= 0x7fff ffff.
 *
 *	   n			   size of input vector
 *
 *	 OUTPUTS:
 *
 *	   L_vec1		   32 bit long signed integer (int32_t) vector whose
 *					   values fall in the range
 *					   0x8000 0000 <= L_vec1 <= 0x7fff ffff.
 *
 *	 RETURN VALUE:
 *
 *	   L_vec1		   32 bit long signed integer (int32_t) vector whose
 *					   values fall in the range
 *					   0x8000 0000 <= L_vec1[] <= 0x7fff ffff.
 *
 *	 IMPLEMENTATION:
 *
 *	   Copy the contents of one 32 bit input vector to another
 *
 *	   vec1 = vec2
 *
 *	 KEYWORDS: equate, copy
 *
 *************************************************************************/

int32_t *L_v_equ(int32_t L_vec1[], int32_t L_vec2[], int16_t n)
{
	register int16_t i;

	for (i = 0; i < n; i++) {
		*L_vec1 = *L_vec2;
		L_vec1++;
		L_vec2++;
	}
	return (L_vec1 - n);
}

/***************************************************************************
 *
 *	 FUNCTION NAME: L_v_inner
 *
 *	 PURPOSE:
 *
 *	   Compute the inner product of two 16 bit input vectors
 *	   with saturation and truncation.	Output is a 32 bit number.
 *
 *	 INPUTS:
 *
 *	   vec1 		   16 bit short signed integer (int16_t) whose value
 *					   falls in the range 0xffff 8000 <= vec1 <= 0x0000 7fff.
 *
 *	   vec2 		   16 bit short signed integer (int16_t) whose value
 *					   falls in the range 0xffff 8000 <= vec2 <= 0x0000 7fff.
 *
 *	   n			   size of input vectors
 *
 *	   qvec1		   Q value of vec1
 *
 *	   qvec2		   Q value of vec2
 *
 *	   qout 		   Q value of output
 *
 *	 OUTPUTS:
 *
 *	   none
 *
 *	 RETURN VALUE:
 *
 *	   L_innerprod	   32 bit long signed integer (int32_t) whose value
 *					   falls in the range
 *					   0x8000 0000 <= L_innerprod <= 0x7fff ffff.
 *
 *	 IMPLEMENTATION:
 *
 *	   Compute the inner product of the two 16 bit vectors
 *	   The output is a 32 bit number.
 *
 *	 KEYWORDS: inner product
 *
 *************************************************************************/

int32_t L_v_inner(int16_t vec1[], int16_t vec2[], int16_t n,
		   int16_t qvec1, int16_t qvec2, int16_t qout)
{
	register int16_t i;
	int16_t shift;
	int32_t L_innerprod, L_temp;

	L_temp = 0;
	for (i = 0; i < n; i++) {
		L_temp = melpe_L_mac(L_temp, *vec1, *vec2);
		vec1++;
		vec2++;
	}

	/* L_temp is now (qvec1 + qvec2 + 1) */
	shift = melpe_sub(qout, melpe_add(melpe_add(qvec1, qvec2), 1));
	L_innerprod = melpe_L_shl(L_temp, shift);
	return (L_innerprod);
}

/***************************************************************************
 *
 *	 FUNCTION NAME: L_v_magsq
 *
 *	 PURPOSE:
 *
 *	   Compute the sum of square magnitude of a 16 bit input vector
 *	   with saturation and truncation.	Output is a 32 bit number.
 *
 *	 INPUTS:
 *
 *	   vec1 		   16 bit short signed integer (int16_t) whose value
 *					   falls in the range 0xffff 8000 <= vec1 <= 0x0000 7fff.
 *
 *	   n			   size of input vectors
 *
 *	   qvec1		   Q value of vec1
 *
 *	   qout 		   Q value of output
 *
 *	 OUTPUTS:
 *
 *	   none
 *
 *	 RETURN VALUE:
 *
 *	   L_magsq		   32 bit long signed integer (int32_t) whose value
 *					   falls in the range
 *					   0x8000 0000 <= L_magsq <= 0x7fff ffff.
 *
 *	 IMPLEMENTATION:
 *
 *	   Compute the sum of square magnitude of a 16 bit input vector.
 *	   The output is a 32 bit number.
 *
 *	 KEYWORDS: square magnitude
 *
 *************************************************************************/

int32_t L_v_magsq(int16_t vec1[], int16_t n, int16_t qvec1,
		   int16_t qout)
{
	register int16_t i;
	int16_t shift;
	int32_t L_magsq, L_temp;

	L_temp = 0;
	for (i = 0; i < n; i++) {
		L_temp = melpe_L_mac(L_temp, *vec1, *vec1);
		vec1++;
	}
	/* ((qout-16)-((2*qvec1+1)-16)) */
	shift = melpe_sub(melpe_sub(qout, melpe_shl(qvec1, 1)), 1);
	L_magsq = melpe_L_shl(L_temp, shift);
	return (L_magsq);
}

/***************************************************************************
 *
 *	 FUNCTION NAME: v_scale
 *
 *	 PURPOSE:
 *
 *	   Perform a multipy of the 16 bit input vector with a 16 bit input
 *	   scale with saturation and truncation.
 *
 *	 INPUTS:
 *
 *	   vec1 		   16 bit short signed integer (int16_t) vector whose
 *					   values fall in the range
 *					   0xffff 8000 <= vec1 <= 0x0000 7fff.
 *	   scale
 *					   16 bit short signed integer (int16_t) whose value
 *					   falls in the range 0xffff 8000 <= var1 <= 0x0000 7fff.
 *
 *	   n			   size of vec1
 *
 *	 OUTPUTS:
 *
 *	   none
 *
 *	 RETURN VALUE:
 *
 *	   vec1 		   16 bit short signed integer (int16_t) vector whose
 *					   values fall in the range
 *					   0xffff 8000 <= vec1[] <= 0x0000 7fff.
 *
 *	 IMPLEMENTATION:
 *
 *	   Perform a multipy of the 16 bit input vector with the 16 bit input
 *	   scale.  The output is a 16 bit vector.
 *
 *	 KEYWORDS: scale
 *
 *************************************************************************/

int16_t *v_scale(int16_t vec1[], int16_t scale, int16_t n)
{
	register int16_t i;

	for (i = 0; i < n; i++) {
		*vec1 = melpe_mult(*vec1, scale);
		vec1++;
	}
	return (vec1 - n);
}

/***************************************************************************
 *
 *	 FUNCTION NAME: v_scale_shl
 *
 *	 PURPOSE:
 *
 *	   Perform a multipy of the 16 bit input vector with a 16 bit input
 *	   scale and shift to left with saturation and truncation.
 *
 *	 INPUTS:
 *
 *	   vec1 		   16 bit short signed integer (int16_t) vector whose
 *					   values fall in the range
 *					   0xffff 8000 <= vec1 <= 0x0000 7fff.
 *
 *	   scale		   16 bit short signed integer (int16_t) whose value
 *					   falls in the range 0xffff 8000 <= var1 <= 0x0000 7fff.
 *
 *	   n			   size of vec1
 *
 *	   shift		   16 bit short signed integer (int16_t) whose value
 *					   falls in the range 0x0000 0000 <= var1 <= 0x0000 1f.
 *
 *	 OUTPUTS:
 *
 *	   none
 *
 *	 RETURN VALUE:
 *
 *	   vec1 		   16 bit short signed integer (int16_t) vector whose
 *					   values fall in the range
 *					   0xffff 8000 <= vec1[] <= 0x0000 7fff.
 *
 *	 IMPLEMENTATION:
 *
 *	   Perform a multipy of the 16 bit input vector with the 16 bit input
 *	   scale.  The output is a 16 bit vector.
 *
 *	 KEYWORDS: scale
 *
 *************************************************************************/

int16_t *v_scale_shl(int16_t vec1[], int16_t scale, int16_t n,
		       int16_t shift)
{
	register int16_t i;

	for (i = 0; i < n; i++) {
		*vec1 = melpe_extract_h(melpe_L_shl(melpe_L_mult(*vec1, scale), shift));
		vec1++;
	}
	return (vec1 - n);
}

/***************************************************************************
 *
 *	 FUNCTION NAME: v_sub
 *
 *	 PURPOSE:
 *
 *	   Perform the subtraction of the two 16 bit input vector with
 *	   saturation.
 *
 *	 INPUTS:
 *
 *	   vec1 		   16 bit short signed integer (int16_t) vector whose
 *					   values fall in the range
 *					   0xffff 8000 <= vec1 <= 0x0000 7fff.
 *
 *	   vec2 		   16 bit short signed integer (int16_t) vector whose
 *					   values falls in the range
 *					   0xffff 8000 <= vec2 <= 0x0000 7fff.
 *
 *	   n			   size of input vectors.
 *
 *	 OUTPUTS:
 *
 *	   none
 *
 *	 RETURN VALUE:
 *
 *	   vec1 		   16 bit short signed integer (int16_t) vector whose
 *					   values fall in the range
 *					   0xffff 8000 <= vec1[] <= 0x0000 7fff.
 *
 *	 IMPLEMENTATION:
 *
 *	   Perform the subtraction of the two 16 bit input vectors with
 *	   saturation.
 *
 *	   vec1 = vec1 - vec2
 *
 *	   vec1[] is set to 0x7fff if the operation results in an
 *	   overflow.  vec1[] is set to 0x8000 if the operation results
 *	   in an underflow.
 *
 *	 KEYWORDS: sub, subtraction
 *
 *************************************************************************/

int16_t *v_sub(int16_t vec1[], const int16_t vec2[], int16_t n)
{
	register int16_t i;

	for (i = 0; i < n; i++) {
		*vec1 = melpe_sub(*vec1, *vec2);
		vec1++;
		vec2++;
	}

	return (vec1 - n);
}

/***************************************************************************
 *
 *	 FUNCTION NAME: v_zap
 *
 *	 PURPOSE:
 *
 *	   Set the elements of a 16 bit input vector to zero.
 *
 *	 INPUTS:
 *
 *	   vec1 		   16 bit short signed integer (int16_t) vector whose
 *					   values fall in the range
 *					   0xffff 8000 <= vec1 <= 0x0000 7fff.
 *
 *	   n			   size of vec1.
 *
 *	 OUTPUTS:
 *
 *	   none
 *
 *	 RETURN VALUE:
 *
 *	   vec1 		   16 bit short signed integer (int16_t) vector whose
 *					   values are equal to 0x0000 0000.
 *
 *	 IMPLEMENTATION:
 *
 *	   Set the elements of 16 bit input vector to zero.
 *
 *	   vec1 = 0
 *
 *	 KEYWORDS: zap, clear, reset
 *
 *************************************************************************/
int16_t *v_zap(int16_t vec1[], int16_t n)
{
	register int16_t i;

	for (i = 0; i < n; i++) {
		*vec1 = 0;
		vec1++;
	}
	return (vec1 - n);
}

int16_t *v_get(int16_t n)
{
	int16_t *ptr;
	int32_t size;

	size = sizeof(int16_t) * n;
	ptr = malloc(size);
	assert(ptr != NULL);
	return (ptr);
}

int32_t *L_v_get(int16_t n)
{
	int32_t *ptr;
	int32_t size;

	size = sizeof(int32_t) * n;
	ptr = malloc(size);
	assert(ptr != NULL);
	return (ptr);
}

void v_free(void *v)
{
	if (v)
		free(v);
}

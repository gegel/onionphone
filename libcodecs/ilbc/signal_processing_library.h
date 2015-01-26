/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

/*
 * This header file includes all of the fix point signal processing library (SPL) function
 * descriptions and declarations.
 * For specific function calls, see bottom of file.
 */

#ifndef WEBRTC_SPL_SIGNAL_PROCESSING_LIBRARY_H_
#define WEBRTC_SPL_SIGNAL_PROCESSING_LIBRARY_H_

#include <string.h>
#include "typedefs.h"

#ifdef ARM_WINM
#include <Armintr.h>		// intrinsic file for windows mobile
#endif

// Macros specific for the fixed point implementation
#define WEBRTC_SPL_WORD16_MAX       32767
#define WEBRTC_SPL_WORD16_MIN       -32768
#define WEBRTC_SPL_WORD32_MAX       (int32_t)0x7fffffff
#define WEBRTC_SPL_WORD32_MIN       (int32_t)0x80000000
#define WEBRTC_SPL_MAX_LPC_ORDER    14
#define WEBRTC_SPL_MAX_SEED_USED    0x80000000L
#define WEBRTC_SPL_MIN(A, B)        (A < B ? A : B)	// Get min value
#define WEBRTC_SPL_MAX(A, B)        (A > B ? A : B)	// Get max value
// TODO(kma/bjorn): For the next two macros, investigate how to correct the code
// for inputs of a = WEBRTC_SPL_WORD16_MIN or WEBRTC_SPL_WORD32_MIN.
#define WEBRTC_SPL_ABS_W16(a) \
    (((int16_t)a >= 0) ? ((int16_t)a) : -((int16_t)a))
#define WEBRTC_SPL_ABS_W32(a) \
    (((int32_t)a >= 0) ? ((int32_t)a) : -((int32_t)a))

#ifdef WEBRTC_ARCH_LITTLE_ENDIAN
#define WEBRTC_SPL_GET_BYTE(a, nr)  (((int8_t *)a)[nr])
#define WEBRTC_SPL_SET_BYTE(d_ptr, val, index) \
    (((int8_t *)d_ptr)[index] = (val))
#else
#define WEBRTC_SPL_GET_BYTE(a, nr) \
    ((((int16_t *)a)[nr >> 1]) >> (((nr + 1) & 0x1) * 8) & 0x00ff)
#define WEBRTC_SPL_SET_BYTE(d_ptr, val, index) \
    ((int16_t *)d_ptr)[index >> 1] = \
    ((((int16_t *)d_ptr)[index >> 1]) \
    & (0x00ff << (8 * ((index) & 0x1)))) | (val << (8 * ((index + 1) & 0x1)))
#endif

#define WEBRTC_SPL_MUL(a, b) \
    ((int32_t) ((int32_t)(a) * (int32_t)(b)))
#define WEBRTC_SPL_UMUL(a, b) \
    ((uint32_t) ((uint32_t)(a) * (uint32_t)(b)))
#define WEBRTC_SPL_UMUL_RSFT16(a, b) \
    ((uint32_t) ((uint32_t)(a) * (uint32_t)(b)) >> 16)
#define WEBRTC_SPL_UMUL_16_16(a, b) \
    ((uint32_t) (uint16_t)(a) * (uint16_t)(b))
#define WEBRTC_SPL_UMUL_16_16_RSFT16(a, b) \
    (((uint32_t) (uint16_t)(a) * (uint16_t)(b)) >> 16)
#define WEBRTC_SPL_UMUL_32_16(a, b) \
    ((uint32_t) ((uint32_t)(a) * (uint16_t)(b)))
#define WEBRTC_SPL_UMUL_32_16_RSFT16(a, b) \
    ((uint32_t) ((uint32_t)(a) * (uint16_t)(b)) >> 16)
#define WEBRTC_SPL_MUL_16_U16(a, b) \
    ((int32_t)(int16_t)(a) * (uint16_t)(b))
#define WEBRTC_SPL_DIV(a, b) \
    ((int32_t) ((int32_t)(a) / (int32_t)(b)))
#define WEBRTC_SPL_UDIV(a, b) \
    ((uint32_t) ((uint32_t)(a) / (uint32_t)(b)))

#ifndef WEBRTC_ARCH_ARM_V7A
// For ARMv7 platforms, these are inline functions in spl_inl_armv7.h
#define WEBRTC_SPL_MUL_16_16(a, b) \
    ((int32_t) (((int16_t)(a)) * ((int16_t)(b))))
#define WEBRTC_SPL_MUL_16_32_RSFT16(a, b) \
    (WEBRTC_SPL_MUL_16_16(a, b >> 16) \
     + ((WEBRTC_SPL_MUL_16_16(a, (b & 0xffff) >> 1) + 0x4000) >> 15))
#define WEBRTC_SPL_MUL_32_32_RSFT32(a32a, a32b, b32) \
    ((int32_t)(WEBRTC_SPL_MUL_16_32_RSFT16(a32a, b32) \
    + (WEBRTC_SPL_MUL_16_32_RSFT16(a32b, b32) >> 16)))
#define WEBRTC_SPL_MUL_32_32_RSFT32BI(a32, b32) \
    ((int32_t)(WEBRTC_SPL_MUL_16_32_RSFT16(( \
    (int16_t)(a32 >> 16)), b32) + \
    (WEBRTC_SPL_MUL_16_32_RSFT16(( \
    (int16_t)((a32 & 0x0000FFFF) >> 1)), b32) >> 15)))
#endif

#define WEBRTC_SPL_MUL_16_32_RSFT11(a, b) \
    ((WEBRTC_SPL_MUL_16_16(a, (b) >> 16) << 5) \
    + (((WEBRTC_SPL_MUL_16_U16(a, (uint16_t)(b)) >> 1) + 0x0200) >> 10))
#define WEBRTC_SPL_MUL_16_32_RSFT14(a, b) \
    ((WEBRTC_SPL_MUL_16_16(a, (b) >> 16) << 2) \
    + (((WEBRTC_SPL_MUL_16_U16(a, (uint16_t)(b)) >> 1) + 0x1000) >> 13))
#define WEBRTC_SPL_MUL_16_32_RSFT15(a, b) \
    ((WEBRTC_SPL_MUL_16_16(a, (b) >> 16) << 1) \
    + (((WEBRTC_SPL_MUL_16_U16(a, (uint16_t)(b)) >> 1) + 0x2000) >> 14))

#ifdef ARM_WINM
#define WEBRTC_SPL_MUL_16_16(a, b) \
    _SmulLo_SW_SL((int16_t)(a), (int16_t)(b))
#endif

#define WEBRTC_SPL_MUL_16_16_RSFT(a, b, c) \
    (WEBRTC_SPL_MUL_16_16(a, b) >> (c))

#define WEBRTC_SPL_MUL_16_16_RSFT_WITH_ROUND(a, b, c) \
    ((WEBRTC_SPL_MUL_16_16(a, b) + ((int32_t) \
                                  (((int32_t)1) << ((c) - 1)))) >> (c))
#define WEBRTC_SPL_MUL_16_16_RSFT_WITH_FIXROUND(a, b) \
    ((WEBRTC_SPL_MUL_16_16(a, b) + ((int32_t) (1 << 14))) >> 15)

// C + the 32 most significant bits of A * B
#define WEBRTC_SPL_SCALEDIFF32(A, B, C) \
    (C + (B >> 16) * A + (((uint32_t)(0x0000FFFF & B) * A) >> 16))

#define WEBRTC_SPL_ADD_SAT_W32(a, b)    WebRtcSpl_AddSatW32(a, b)
#define WEBRTC_SPL_SAT(a, b, c)         (b > a ? a : b < c ? c : b)
#define WEBRTC_SPL_MUL_32_16(a, b)      ((a) * (b))

#define WEBRTC_SPL_SUB_SAT_W32(a, b)    WebRtcSpl_SubSatW32(a, b)
#define WEBRTC_SPL_ADD_SAT_W16(a, b)    WebRtcSpl_AddSatW16(a, b)
#define WEBRTC_SPL_SUB_SAT_W16(a, b)    WebRtcSpl_SubSatW16(a, b)

// We cannot do casting here due to signed/unsigned problem
#define WEBRTC_SPL_IS_NEG(a)            ((a) & 0x80000000)
// Shifting with negative numbers allowed
// Positive means left shift
#define WEBRTC_SPL_SHIFT_W16(x, c) \
    (((c) >= 0) ? ((x) << (c)) : ((x) >> (-(c))))
#define WEBRTC_SPL_SHIFT_W32(x, c) \
    (((c) >= 0) ? ((x) << (c)) : ((x) >> (-(c))))

// Shifting with negative numbers not allowed
// We cannot do casting here due to signed/unsigned problem
#define WEBRTC_SPL_RSHIFT_W16(x, c)     ((x) >> (c))
#define WEBRTC_SPL_LSHIFT_W16(x, c)     ((x) << (c))
#define WEBRTC_SPL_RSHIFT_W32(x, c)     ((x) >> (c))
#define WEBRTC_SPL_LSHIFT_W32(x, c)     ((x) << (c))

#define WEBRTC_SPL_RSHIFT_U16(x, c)     ((uint16_t)(x) >> (c))
#define WEBRTC_SPL_LSHIFT_U16(x, c)     ((uint16_t)(x) << (c))
#define WEBRTC_SPL_RSHIFT_U32(x, c)     ((uint32_t)(x) >> (c))
#define WEBRTC_SPL_LSHIFT_U32(x, c)     ((uint32_t)(x) << (c))

#define WEBRTC_SPL_VNEW(t, n)           (t *) malloc (sizeof (t) * (n))
#define WEBRTC_SPL_FREE                 free

#define WEBRTC_SPL_RAND(a) \
    ((int16_t)(WEBRTC_SPL_MUL_16_16_RSFT((a), 18816, 7) & 0x00007fff))

#ifdef __cplusplus
extern "C" {
#endif

#define WEBRTC_SPL_MEMCPY_W8(v1, v2, length) \
   memcpy(v1, v2, (length) * sizeof(char))
#define WEBRTC_SPL_MEMCPY_W16(v1, v2, length) \
   memcpy(v1, v2, (length) * sizeof(int16_t))

#define WEBRTC_SPL_MEMMOVE_W16(v1, v2, length) \
   memmove(v1, v2, (length) * sizeof(int16_t))

// inline functions:
#include "spl_inl.h"

// Copy and set operations. Implementation in copy_set_operations.c.
// Descriptions at bottom of file.
	void WebRtcSpl_MemSetW16(int16_t * vector,
				 int16_t set_value, int vector_length);
	void WebRtcSpl_MemCpyReversedOrder(int16_t * out_vector,
					   int16_t * in_vector,
					   int vector_length);
// End: Copy and set operations.

// Minimum and maximum operations. Implementation in min_max_operations.c.

// Returns the largest absolute value in a signed 16-bit vector.
//
// Input:
//      - vector : 16-bit input vector.
//      - length : Number of samples in vector.
//
// Return value  : Maximum absolute value in vector;
//                 or -1, if (vector == NULL || length <= 0).
	int16_t WebRtcSpl_MaxAbsValueW16(const int16_t * vector, int length);

// Returns the largest absolute value in a signed 32-bit vector.
//
// Input:
//      - vector : 32-bit input vector.
//      - length : Number of samples in vector.
//
// Return value  : Maximum absolute value in vector;
//                 or -1, if (vector == NULL || length <= 0).
	int32_t WebRtcSpl_MaxAbsValueW32(const int32_t * vector, int length);

// Returns the maximum value of a 32-bit vector.
//
// Input:
//      - vector : 32-bit input vector.
//      - length : Number of samples in vector.
//
// Return value  : Maximum sample value in |vector|.
//                 If (vector == NULL || length <= 0) WEBRTC_SPL_WORD32_MIN
//                 is returned. Note that WEBRTC_SPL_WORD32_MIN is a feasible
//                 value and we can't catch errors purely based on it.
	int32_t WebRtcSpl_MaxValueW32(const int32_t * vector, int length);

// Returns the vector index to the maximum sample value of a 32-bit vector.
//
// Input:
//      - vector : 32-bit input vector.
//      - length : Number of samples in vector.
//
// Return value  : Index to the maximum value in vector;
//                 or -1, if (vector == NULL || length <= 0).
	int WebRtcSpl_MaxIndexW32(const int32_t * vector, int length);

// Returns the vector index to the minimum sample value of a 32-bit vector.
//
// Input:
//      - vector : 32-bit input vector.
//      - length : Number of samples in vector.
//
// Return value  : Index to the mimimum value in vector;
//                 or -1, if (vector == NULL || length <= 0).
	int WebRtcSpl_MinIndexW32(const int32_t * vector, int length);

// End: Minimum and maximum operations.

// Vector scaling operations. Implementation in vector_scaling_operations.c.
// Description at bottom of file.
	void WebRtcSpl_VectorBitShiftW32(int32_t * out_vector,
					 int16_t vector_length,
					 const int32_t * in_vector,
					 int16_t right_shifts);

	void WebRtcSpl_ScaleVector(const int16_t * in_vector,
				   int16_t * out_vector,
				   int16_t gain,
				   int16_t vector_length,
				   int16_t right_shifts);
	void WebRtcSpl_ScaleVectorWithSat(const int16_t * in_vector,
					  int16_t * out_vector,
					  int16_t gain,
					  int16_t vector_length,
					  int16_t right_shifts);
	void WebRtcSpl_ScaleAndAddVectors(const int16_t * in_vector1,
					  int16_t gain1,
					  int right_shifts1,
					  const int16_t * in_vector2,
					  int16_t gain2,
					  int right_shifts2,
					  int16_t * out_vector,
					  int vector_length);

// iLBC specific functions. Implementations in ilbc_specific_functions.c.
// Description at bottom of file.
	void WebRtcSpl_ReverseOrderMultArrayElements(int16_t * out_vector,
						     const int16_t *
						     in_vector,
						     const int16_t *
						     window,
						     int16_t
						     vector_length,
						     int16_t
						     right_shifts);
	void WebRtcSpl_ElementwiseVectorMult(int16_t * out_vector,
					     const int16_t * in_vector,
					     const int16_t * window,
					     int16_t vector_length,
					     int16_t right_shifts);
	void WebRtcSpl_AddVectorsAndShift(int16_t * out_vector,
					  const int16_t * in_vector1,
					  const int16_t * in_vector2,
					  int16_t vector_length,
					  int16_t right_shifts);
	void WebRtcSpl_AddAffineVectorToVector(int16_t * out_vector,
					       int16_t * in_vector,
					       int16_t gain,
					       int32_t add_constant,
					       int16_t right_shifts,
					       int vector_length);
// End: iLBC specific functions.

// Signal processing operations. Descriptions at bottom of this file.
	int WebRtcSpl_AutoCorrelation(const int16_t * vector,
				      int vector_length, int order,
				      int32_t * result_vector,
				      int *scale);
	int16_t WebRtcSpl_LevinsonDurbin(int32_t * auto_corr,
					       int16_t * lpc_coef,
					       int16_t * refl_coef,
					       int16_t order);
	void WebRtcSpl_CrossCorrelation(int32_t * cross_corr,
					int16_t * vector1,
					int16_t * vector2,
					int16_t dim_vector,
					int16_t dim_cross_corr,
					int16_t right_shifts,
					int16_t step_vector2);
// End: Signal processing operations.

// Math functions
	int32_t WebRtcSpl_SqrtFloor(int32_t value);

// Divisions. Implementations collected in division_operations.c and
// descriptions at bottom of this file.
	int32_t WebRtcSpl_DivW32W16(int32_t num, int16_t den);
	int32_t WebRtcSpl_DivW32HiLow(int32_t num,
					    int16_t den_hi,
					    int16_t den_low);
// End: Divisions.

	int32_t WebRtcSpl_DotProductWithScale(int16_t * vector1,
						    int16_t * vector2,
						    int vector_length,
						    int scaling);

// Filter operations.
	void WebRtcSpl_FilterMAFastQ12(int16_t * in_vector,
				       int16_t * out_vector,
				       int16_t * ma_coef,
				       int16_t ma_coef_length,
				       int16_t vector_length);

// Performs a AR filtering on a vector in Q12
// Input:
//      - data_in            : Input samples
//      - data_out           : State information in positions
//                               data_out[-order] .. data_out[-1]
//      - coefficients       : Filter coefficients (in Q12)
//      - coefficients_length: Number of coefficients (order+1)
//      - data_length        : Number of samples to be filtered
// Output:
//      - data_out           : Filtered samples
	void WebRtcSpl_FilterARFastQ12(const int16_t * data_in,
				       int16_t * data_out,
				       const int16_t * __restrict coefficients,
				       int coefficients_length,
				       int data_length);

// Performs a MA down sampling filter on a vector
// Input:
//      - data_in            : Input samples (state in positions
//                               data_in[-order] .. data_in[-1])
//      - data_in_length     : Number of samples in |data_in| to be filtered.
//                               This must be at least
//                               |delay| + |factor|*(|out_vector_length|-1) + 1)
//      - data_out_length    : Number of down sampled samples desired
//      - coefficients       : Filter coefficients (in Q12)
//      - coefficients_length: Number of coefficients (order+1)
//      - factor             : Decimation factor
//      - delay              : Delay of filter (compensated for in out_vector)
// Output:
//      - data_out           : Filtered samples
// Return value              : 0 if OK, -1 if |in_vector| is too short
	int WebRtcSpl_DownsampleFast(const int16_t * data_in,
				     int data_in_length,
				     int16_t * data_out,
				     int data_out_length,
				     const int16_t * __restrict coefficients,
				     int coefficients_length,
				     int factor, int delay);

// End: Filter operations.

/************************************************************
 *
 * RESAMPLING FUNCTIONS AND THEIR STRUCTS ARE DEFINED BELOW
 *
 ************************************************************/

/*******************************************************************
 * resample.c
 *
 * Includes the following resampling combinations
 * 22 kHz -> 16 kHz
 * 16 kHz -> 22 kHz
 * 22 kHz ->  8 kHz
 *  8 kHz -> 22 kHz
 *
 ******************************************************************/

// state structure for 22 -> 16 resampler
	typedef struct {
		int32_t S_22_44[8];
		int32_t S_44_32[8];
		int32_t S_32_16[8];
	} WebRtcSpl_State22khzTo16khz;

// state structure for 16 -> 22 resampler
	typedef struct {
		int32_t S_16_32[8];
		int32_t S_32_22[8];
	} WebRtcSpl_State16khzTo22khz;

// state structure for 22 -> 8 resampler
	typedef struct {
		int32_t S_22_22[16];
		int32_t S_22_16[8];
		int32_t S_16_8[8];
	} WebRtcSpl_State22khzTo8khz;

// state structure for 8 -> 22 resampler
	typedef struct {
		int32_t S_8_16[8];
		int32_t S_16_11[8];
		int32_t S_11_22[8];
	} WebRtcSpl_State8khzTo22khz;

/*******************************************************************
 * resample_48khz.c
 *
 * Includes the following resampling combinations
 * 48 kHz -> 16 kHz
 * 16 kHz -> 48 kHz
 * 48 kHz ->  8 kHz
 *  8 kHz -> 48 kHz
 *
 ******************************************************************/

	typedef struct {
		int32_t S_48_48[16];
		int32_t S_48_32[8];
		int32_t S_32_16[8];
	} WebRtcSpl_State48khzTo16khz;

	typedef struct {
		int32_t S_16_32[8];
		int32_t S_32_24[8];
		int32_t S_24_48[8];
	} WebRtcSpl_State16khzTo48khz;

	typedef struct {
		int32_t S_48_24[8];
		int32_t S_24_24[16];
		int32_t S_24_16[8];
		int32_t S_16_8[8];
	} WebRtcSpl_State48khzTo8khz;

	typedef struct {
		int32_t S_8_16[8];
		int32_t S_16_12[8];
		int32_t S_12_24[8];
		int32_t S_24_48[8];
	} WebRtcSpl_State8khzTo48khz;

/************************************************************
 * END OF RESAMPLING FUNCTIONS
 ************************************************************/
#ifdef __cplusplus
}
#endif				// __cplusplus
#endif				// WEBRTC_SPL_SIGNAL_PROCESSING_LIBRARY_H_


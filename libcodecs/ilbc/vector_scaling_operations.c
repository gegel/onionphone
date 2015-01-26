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
 * This file contains implementations of the functions
 * WebRtcSpl_VectorBitShiftW32()
 * WebRtcSpl_ScaleVector()
 * WebRtcSpl_ScaleVectorWithSat()
 * WebRtcSpl_ScaleAndAddVectors()
 * WebRtcSpl_ScaleAndAddVectorsWithRound()
 */

#include "signal_processing_library.h"

void WebRtcSpl_VectorBitShiftW32(int32_t * out_vector,
				 int16_t vector_length,
				 const int32_t * in_vector,
				 int16_t right_shifts)
{
	int i;

	if (right_shifts > 0) {
		for (i = vector_length; i > 0; i--) {
			(*out_vector++) = ((*in_vector++) >> right_shifts);
		}
	} else {
		for (i = vector_length; i > 0; i--) {
			(*out_vector++) = ((*in_vector++) << (-right_shifts));
		}
	}
}

void WebRtcSpl_ScaleVector(const int16_t * in_vector,
			   int16_t * out_vector, int16_t gain,
			   int16_t in_vector_length,
			   int16_t right_shifts)
{
	// Performs vector operation: out_vector = (gain*in_vector)>>right_shifts
	int i;
	const int16_t *inptr;
	int16_t *outptr;

	inptr = in_vector;
	outptr = out_vector;

	for (i = 0; i < in_vector_length; i++) {
		(*outptr++) =
		    (int16_t) WEBRTC_SPL_MUL_16_16_RSFT(*inptr++, gain,
							      right_shifts);
	}
}

void WebRtcSpl_ScaleVectorWithSat(const int16_t * in_vector,
				  int16_t * out_vector,
				  int16_t gain,
				  int16_t in_vector_length,
				  int16_t right_shifts)
{
	// Performs vector operation: out_vector = (gain*in_vector)>>right_shifts
	int i;
	int32_t tmpW32;
	const int16_t *inptr;
	int16_t *outptr;

	inptr = in_vector;
	outptr = out_vector;

	for (i = 0; i < in_vector_length; i++) {
		tmpW32 =
		    WEBRTC_SPL_MUL_16_16_RSFT(*inptr++, gain, right_shifts);
		(*outptr++) = WebRtcSpl_SatW32ToW16(tmpW32);
	}
}

void WebRtcSpl_ScaleAndAddVectors(const int16_t * in1,
				  int16_t gain1, int shift1,
				  const int16_t * in2,
				  int16_t gain2, int shift2,
				  int16_t * out, int vector_length)
{
	// Performs vector operation: out = (gain1*in1)>>shift1 + (gain2*in2)>>shift2
	int i;
	const int16_t *in1ptr;
	const int16_t *in2ptr;
	int16_t *outptr;

	in1ptr = in1;
	in2ptr = in2;
	outptr = out;

	for (i = 0; i < vector_length; i++) {
		(*outptr++) =
		    (int16_t) WEBRTC_SPL_MUL_16_16_RSFT(gain1, *in1ptr++,
							      shift1)
		    + (int16_t) WEBRTC_SPL_MUL_16_16_RSFT(gain2,
								*in2ptr++,
								shift2);
	}
}


/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

/*
 * This file contains implementations of the divisions
 * WebRtcSpl_DivW32W16()
 * WebRtcSpl_DivResultInQ31()
 * WebRtcSpl_DivW32HiLow()
 *
 * The description header can be found in signal_processing_library.h
 *
 */

#include "signal_processing_library.h"

int32_t WebRtcSpl_DivW32W16(int32_t num, int16_t den)
{
	// Guard against division with 0
	if (den != 0) {
		return (int32_t) (num / den);
	} else {
		return (int32_t) 0x7FFFFFFF;
	}
}

int32_t WebRtcSpl_DivW32HiLow(int32_t num, int16_t den_hi,
				    int16_t den_low)
{
	int16_t approx, tmp_hi, tmp_low, num_hi, num_low;
	int32_t tmpW32;

	approx =
	    (int16_t) WebRtcSpl_DivW32W16((int32_t) 0x1FFFFFFF,
						den_hi);
	// result in Q14 (Note: 3FFFFFFF = 0.5 in Q30)

	// tmpW32 = 1/den = approx * (2.0 - den * approx) (in Q30)
	tmpW32 = (WEBRTC_SPL_MUL_16_16(den_hi, approx) << 1)
	    + ((WEBRTC_SPL_MUL_16_16(den_low, approx) >> 15) << 1);
	// tmpW32 = den * approx

	tmpW32 = (int32_t) 0x7fffffffL - tmpW32;	// result in Q30 (tmpW32 = 2.0-(den*approx))

	// Store tmpW32 in hi and low format
	tmp_hi = (int16_t) WEBRTC_SPL_RSHIFT_W32(tmpW32, 16);
	tmp_low = (int16_t) WEBRTC_SPL_RSHIFT_W32((tmpW32
							 -
							 WEBRTC_SPL_LSHIFT_W32((int32_t) tmp_hi, 16)),
						  1);

	// tmpW32 = 1/den in Q29
	tmpW32 =
	    ((WEBRTC_SPL_MUL_16_16(tmp_hi, approx) +
	      (WEBRTC_SPL_MUL_16_16(tmp_low, approx)
	       >> 15)) << 1);

	// 1/den in hi and low format
	tmp_hi = (int16_t) WEBRTC_SPL_RSHIFT_W32(tmpW32, 16);
	tmp_low = (int16_t) WEBRTC_SPL_RSHIFT_W32((tmpW32
							 -
							 WEBRTC_SPL_LSHIFT_W32((int32_t) tmp_hi, 16)),
						  1);

	// Store num in hi and low format
	num_hi = (int16_t) WEBRTC_SPL_RSHIFT_W32(num, 16);
	num_low = (int16_t) WEBRTC_SPL_RSHIFT_W32((num
							 -
							 WEBRTC_SPL_LSHIFT_W32((int32_t) num_hi, 16)),
						  1);

	// num * (1/den) by 32 bit multiplication (result in Q28)

	tmpW32 =
	    (WEBRTC_SPL_MUL_16_16(num_hi, tmp_hi) +
	     (WEBRTC_SPL_MUL_16_16(num_hi, tmp_low)
	      >> 15) + (WEBRTC_SPL_MUL_16_16(num_low, tmp_hi) >> 15));

	// Put result in Q31 (convert from Q28)
	tmpW32 = WEBRTC_SPL_LSHIFT_W32(tmpW32, 3);

	return tmpW32;
}

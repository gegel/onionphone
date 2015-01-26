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
 * This file contains the implementation of functions
 * WebRtcSpl_MemSetW16()
 * WebRtcSpl_MemCpyReversedOrder()
 *
 * The description header can be found in signal_processing_library.h
 *
 */

#include <string.h>
#include "signal_processing_library.h"

void WebRtcSpl_MemSetW16(int16_t * ptr, int16_t set_value,
			 int length)
{
	int j;
	int16_t *arrptr = ptr;

	for (j = length; j > 0; j--) {
		*arrptr++ = set_value;
	}
}

void WebRtcSpl_MemCpyReversedOrder(int16_t * dest, int16_t * source,
				   int length)
{
	int j;
	int16_t *destPtr = dest;
	int16_t *sourcePtr = source;

	for (j = 0; j < length; j++) {
		*destPtr-- = *sourcePtr++;
	}
}


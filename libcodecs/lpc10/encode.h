/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _ENCODE_H_
#define _ENCODE_H_

#include <stdint.h>

int lpc10_internal_encode(int32_t * voice, int32_t * pitch, float *rms,
			  float *rc, int32_t * ipitch, int32_t * irms,
			  int32_t * irc);

#endif				/* _ENCODE_H_ */

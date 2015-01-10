/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _DECODE_H_
#define _DECODE_H_

#include <stdint.h>

#include "lpc10.h"

int lpc10_internal_decode(int32_t * ipitv, int32_t * irms,
			  int32_t * irc, int32_t * voice, int32_t * pitch,
			  float *rms, float *rc,
			  struct lpc10_decoder_state *st);

#endif				/* _DECODE_H_ */

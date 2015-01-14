/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _SYNTHS_H_
#define _SYNTHS_H_

#include <stdint.h>

#include "lpc10.h"

int lpc10_synths(int32_t * voice, int32_t * pitch, float *rms, float *rc,
		 float *speech, int32_t * k, struct lpc10_decoder_state *st);

#endif				/* _SYNTHS_H_ */

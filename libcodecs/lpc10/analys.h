/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _ANALYS_H_
#define _ANALYS_H_

#include <stdint.h>

#include "lpc10.h"

int lpc10_analys(float *speech, int32_t * voice, int32_t
		 * pitch, float *rms, float *rc,
		 struct lpc10_encoder_state *st);

#endif				/* _ANALYS_H_ */

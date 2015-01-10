/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _PITSYN_H_
#define _PITSYN_H_

#include <stdint.h>

#include "lpc10.h"

int lpc10_pitsyn(int32_t * order, int32_t * voice, int32_t * pitch,
		 float *rms, float *rc, int32_t * lframe, int32_t * ivuv,
		 int32_t * ipiti, float *rmsi, float *rci, int32_t * nout,
		 float *ratio, struct lpc10_decoder_state *st);

#endif				/* _PITSYN_H_ */

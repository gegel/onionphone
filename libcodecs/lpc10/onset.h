/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _ONSET_H_
#define _ONSET_H_

#include <stdint.h>

#include "lpc10.h"

int lpc10_onset(float *pebuf, int32_t * osbuf, int32_t * osptr,
		int32_t * oslen, int32_t * sbufl, int32_t * sbufh,
		int32_t * lframe, struct lpc10_encoder_state *st);

#endif				/* _ONSET_H_ */

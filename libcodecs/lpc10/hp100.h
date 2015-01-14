/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _HP100_H_
#define _HP100_H_

#include <stdint.h>

#include "lpc10.h"

int lpc10_hp100(float *speech, int32_t * start, int32_t * end,
		struct lpc10_encoder_state *st);

#endif				/* _HP100_H_ */

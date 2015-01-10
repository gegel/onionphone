/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _PREPRO_H_
#define _PREPRO_H_

#include <stdint.h>

#include "lpc10.h"

int lpc10_prepro(float *speech, int32_t * length,
		 struct lpc10_encoder_state *st);

#endif				/* _PREPRO_H_ */

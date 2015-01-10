/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _BSYNZ_H_
#define _BSYNZ_H_

#include <stdint.h>

#include "lpc10.h"

int lpc10_bsynz(float *coef, int32_t * ip, int32_t * iv,
		float *sout, float *rms, float *ratio,
		float *g2pass, struct lpc10_decoder_state *st);

#endif				/* _BSYNZ_H_ */

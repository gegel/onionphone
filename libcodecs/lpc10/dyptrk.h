/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _DYPTRK_H_
#define _DYPTRK_H_

#include <stdint.h>

#include "lpc10.h"

int lpc10_dyptrk(float *amdf, int32_t * ltau, int32_t * minptr,
		 int32_t * voice, int32_t * pitch, int32_t * midx,
		 struct lpc10_encoder_state *st);

#endif				/* _DYPTRK_H_ */

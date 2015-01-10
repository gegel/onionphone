/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _CHANWR_H_
#define _CHANWR_H_

#include <stdint.h>

#include "lpc10.h"

int lpc10_chanwr(int32_t * order, int32_t * ipitv, int32_t * irms,
		 int32_t * irc, int32_t * ibits,
		 struct lpc10_encoder_state *st);
int lpc10_chanrd(int32_t * order, int32_t * ipitv, int32_t * irms,
		 int32_t * irc, int32_t * ibits);

#endif				/* _CHANWR_H_ */

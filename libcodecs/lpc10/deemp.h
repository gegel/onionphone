/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _DEEMP_H_
#define _DEEMP_H_

#include <stdint.h>

#include "lpc10.h"

int lpc10_deemp(float *x, int32_t * n, struct lpc10_decoder_state *st);

#endif				/* _DEEMP_H_ */

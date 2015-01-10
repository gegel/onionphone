/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _PREEMP_H_
#define _PREEMP_H_

#include <stdint.h>

int lpc10_preemp(float *inbuf, float *pebuf, int32_t * nsamp, float *coef,
		 float *z__);

#endif				/* _PREEMP_H_ */

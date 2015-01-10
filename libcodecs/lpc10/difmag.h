/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _DIFMAG_H_
#define _DIFMAG_H_

#include <stdint.h>

int lpc10_difmag(float *speech, int32_t * lpita, int32_t * tau,
		 int32_t * ltau, int32_t * maxlag, float *amdf,
		 int32_t * minptr, int32_t * maxptr);

#endif				/* _DIFMAG_H_ */

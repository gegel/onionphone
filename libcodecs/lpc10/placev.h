/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _PLACEV_H_
#define _PLACEV_H_

#include <stdint.h>

int lpc10_placev(int32_t * osbuf, int32_t * osptr, int32_t * oslen,
		 int32_t * obound, int32_t * vwin, int32_t * af,
		 int32_t * lframe, int32_t * minwin, int32_t * maxwin,
		 int32_t * dvwinl, int32_t * dvwinh);

#endif				/* _PLACEV_H_ */

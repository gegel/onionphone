/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _IVFILT_H_
#define _IVFILT_H_

#include <stdint.h>

int lpc10_ivfilt(float *lpbuf, float *ivbuf, int32_t * len, int32_t * nsamp,
		 float *ivrc);

#endif				/* _IVFILT_H_ */

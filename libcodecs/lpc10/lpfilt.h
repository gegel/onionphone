/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _LPFILT_H_
#define _LPFILT_H_

#include <stdint.h>

int lpc10_lpfilt(float *inbuf, float *lpbuf, int32_t * len, int32_t * nsamp);

#endif				/* _LPFILT_H_ */

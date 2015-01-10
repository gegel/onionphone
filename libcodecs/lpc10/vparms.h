/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _VPARMS_H_
#define _VPARMS_H_

#include <stdint.h>

int lpc10_vparms(int32_t * vwin, float *inbuf, float *lpbuf, int32_t * buflim,
		 int32_t * half, float *dither, int32_t * mintau,
		 int32_t * zc, int32_t * lbe, int32_t * fbe, float *qs,
		 float *rc1, float *ar_b__, float *ar_f__);

#endif				/* _VPARMS_H_ */

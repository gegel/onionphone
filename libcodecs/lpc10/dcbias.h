/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _DCBIAS_H_
#define _DCBIAS_H_

#include <stdint.h>

int lpc10_dcbias(int32_t * len, float *speech, float *sigout);

#endif				/* _DCBIAS_H_ */

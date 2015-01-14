/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _MLOAD_H_
#define _MLOAD_H_

#include <stdint.h>

int lpc10_mload(int32_t * order, int32_t * awins, int32_t * awinf,
		float *speech, float *phi, float *psi);

#endif				/* _MLOAD_H_ */

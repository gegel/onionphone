/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _TBDM_H_
#define _TBDM_H_

#include <stdint.h>

int lpc10_tbdm(float *speech, int32_t * lpita, int32_t * tau, int32_t * ltau,
	       float *amdf, int32_t * minptr, int32_t * maxptr,
	       int32_t * mintau);

#endif				/* _TBDM_H_ */

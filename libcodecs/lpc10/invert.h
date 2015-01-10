/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _INVERT_H_
#define _INVERT_H_

#include <stdint.h>

int lpc10_invert(int32_t * order, float *phi, float *psi, float *rc);

#endif				/* _INVERT_H_ */

/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _ENERGY_H_
#define _ENERGY_H_

#include <stdint.h>

int lpc10_energy(int32_t * len, float *speech, float *rms);

#endif				/* _ENERGY_H_ */

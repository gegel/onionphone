/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _HAM84_H_
#define _HAM84_H_

#include <stdint.h>

int lpc10_ham84(int32_t * input, int32_t * output, int32_t * errcnt);

#endif				/* _HAM84_H_ */

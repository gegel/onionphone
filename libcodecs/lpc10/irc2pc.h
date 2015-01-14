/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _IRC2PC_H_
#define _IRC2PC_H_

#include <stdint.h>

int lpc10_irc2pc(float *rc, float *pc, int32_t * order, float *gprime,
		 float *g2pass);

#endif				/* _IRC2PC_H_ */

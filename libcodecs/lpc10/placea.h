/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _PLACEA_H_
#define _PLACEA_H_

#include <stdint.h>

int lpc10_placea(int32_t * ipitch, int32_t * voibuf, int32_t * obound,
		 int32_t * af, int32_t * vwin, int32_t * awin, int32_t * ewin,
		 int32_t * lframe, int32_t * maxwin);

#endif				/* _PLACEA_H_ */

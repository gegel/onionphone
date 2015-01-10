/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#pragma once

#ifndef _VOICIN_H_
#define _VOICIN_H_

#include <stdint.h>

#include "lpc10.h"

int lpc10_voicin(int32_t * vwin, float *inbuf, float *lpbuf, int32_t * buflim,
		 int32_t * half, float *minamd, float *maxamd,
		 int32_t * mintau, float *ivrc, int32_t * obound,
		 int32_t * voibuf, int32_t * af,
		 struct lpc10_encoder_state *st);

#endif				/* _VOICIN_H_ */

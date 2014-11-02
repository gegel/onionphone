/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* Copyright (c) 2007-2008 CSIRO
   Copyright (c) 2007-2009 Xiph.Org Foundation
   Copyright (c) 2008 Gregory Maxwell
   Written by Jean-Marc Valin and Gregory Maxwell */
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MODES_H
#define MODES_H

#include <stdint.h>
#include "celt.h"
#include "arch.h"
#include "mdct.h"
#include "entenc.h"
#include "entdec.h"

#define MAX_PERIOD 1024

#ifndef OVERLAP
#define OVERLAP(mode) ((mode)->overlap)
#endif

#ifndef FRAMESIZE
#define FRAMESIZE(mode) ((mode)->mdctSize)
#endif

typedef struct {
	int size;
	const int16_t *index;
	const unsigned char *bits;
	const unsigned char *caps;
} PulseCache;

/** Mode definition (opaque)
 @brief Mode definition
 */
struct OpusCustomMode {
	int32_t Fs;
	int overlap;

	int nbEBands;
	int effEBands;
	opus_val16 preemph[4];
	const int16_t *eBands;/**< Definition for each "pseudo-critical band" */

	int maxLM;
	int nbShortMdcts;
	int shortMdctSize;

	int nbAllocVectors;	/**< Number of lines in the matrix below */
	const unsigned char *allocVectors;/**< Number of bits in each band for several rates */
	const int16_t *logN;

	const opus_val16 *window;
	mdct_lookup mdct;
	PulseCache cache;
};

#endif

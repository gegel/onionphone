/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* Copyright (C) 2003-2006 Jean-Marc Valin

   File: mdf.c
   Echo canceller based on the MDF algorithm (see below)

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

   1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

/*
   The echo canceller is based on the MDF algorithm described in:

   J. S. Soo, K. K. Pang Multidelay block frequency adaptive filter, 
   IEEE Trans. Acoust. Speech Signal Process., Vol. ASSP-38, No. 2, 
   February 1990.
   
   We use the Alternatively Updated MDF (AUMDF) variant. Robustness to 
   double-talk is achieved using a variable learning rate as described in:
   
   Valin, J.-M., On Adjusting the Learning Rate in Frequency Domain Echo 
   Cancellation With Double-Talk. IEEE Transactions on Audio,
   Speech and Language Processing, Vol. 15, No. 3, pp. 1030-1034, 2007.
   http://people.xiph.org/~jm/papers/valin_taslp2006.pdf
   
   There is no explicit double-talk detection, but a continuous variation
   in the learning rate based on residual echo, double-talk and background
   noise.
   
   About the fixed-point version:
   All the signals are represented with 16-bit words. The filter weights 
   are represented with 32-bit words, but only the top 16 bits are used
   in most cases. The lower 16 bits are completely unreliable (due to the
   fact that the update is done only on the top bits), but help in the
   adaptation -- probably by removing a "threshold effect" due to
   quantization (rounding going to zero) when the gradient is small.
   
   Another kludge that seems to work good: when performing the weight
   update, we only move half the way toward the "goal" this seems to
   reduce the effect of quantization noise in the update phase. This
   can be seen as applying a gradient descent on a "soft constraint"
   instead of having a hard constraint.
   
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "arch.h"
#include "speex/speex_echo.h"
#include "fftwrap.h"
#include "pseudofloat.h"
#include "math_approx.h"
#include "os_support.h"
#include <ophmconsts.h>

#ifdef FIXED_POINT
#define WEIGHT_SHIFT 11
#define NORMALIZE_SCALEDOWN 5
#define NORMALIZE_SCALEUP 3
#else
#define WEIGHT_SHIFT 0
#endif

#ifdef FIXED_POINT
#define WORD2INT(x) ((x) < -32767 ? -32768 : ((x) > 32766 ? 32767 : (x)))
#else
#define WORD2INT(x) ((x) < -32767.5f ? -32768 : ((x) > 32766.5f ? 32767 : floor(.5+(x))))
#endif

/* If enabled, the AEC will use a foreground filter and a background filter to be more robust to double-talk
   and difficult signals in general. The cost is an extra FFT and a matrix-vector multiply */
#define TWO_PATH

#ifdef FIXED_POINT
static const spx_float_t MIN_LEAK = { 20972, -22 };

/* Constants for the two-path filter */
static const spx_float_t VAR1_SMOOTH = { 23593, -16 };
static const spx_float_t VAR2_SMOOTH = { 23675, -15 };
static const spx_float_t VAR1_UPDATE = { 16384, -15 };
static const spx_float_t VAR2_UPDATE = { 16384, -16 };
static const spx_float_t VAR_BACKTRACK = { 16384, -12 };

#define TOP16(x) ((x)>>16)

#else

static const spx_float_t MIN_LEAK = .005f;

/* Constants for the two-path filter */
static const spx_float_t VAR1_SMOOTH = .36f;
static const spx_float_t VAR2_SMOOTH = .7225f;
static const spx_float_t VAR1_UPDATE = .5f;
static const spx_float_t VAR2_UPDATE = .25f;
static const spx_float_t VAR_BACKTRACK = 4.f;
#define TOP16(x) (x)
#endif

#define PLAYBACK_DELAY 2

void speex_echo_get_residual(SpeexEchoState * st, spx_word32_t * Yout, int len);

/** Speex echo cancellation state. */
struct SpeexEchoState_ {
	int frame_size;	     /**< Number of samples processed each time */
	int window_size;
	spx_word16_t leak_estimate;

	spx_word16_t *y;	/* scratch */
	spx_word16_t *last_y;
	spx_word16_t *Y;	/* scratch */
	spx_word16_t *window;
	void *fft_table;
};

/** Compute power spectrum of a half-complex (packed) vector */
static inline void power_spectrum(const spx_word16_t * X, spx_word32_t * ps,
				  int N)
{
	int i, j;
	ps[0] = MULT16_16(X[0], X[0]);
	for (i = 1, j = 1; i < N - 1; i += 2, j++) {
		ps[j] = MULT16_16(X[i], X[i]) + MULT16_16(X[i + 1], X[i + 1]);
	}
	ps[j] = MULT16_16(X[i], X[i]);
}

#ifdef DUMP_ECHO_CANCEL_DATA
#include <stdio.h>
static FILE *rFile = NULL, *pFile = NULL, *oFile = NULL;

static void dump_audio(const int16_t * rec, const int16_t * play,
		       const int16_t * out, int len)
{
	if (!(rFile && pFile && oFile)) {
		speex_fatal("Dump files not open");
	}
	fwrite(rec, sizeof(int16_t), len, rFile);
	fwrite(play, sizeof(int16_t), len, pFile);
	fwrite(out, sizeof(int16_t), len, oFile);
}
#endif

/* Compute spectrum of estimated echo for use in an echo post-filter */
void speex_echo_get_residual(SpeexEchoState * st, spx_word32_t * residual_echo,
			     int len)
{
	(void)len;

	int i;
	spx_word16_t leak2;
	int N;

	N = st->window_size;

	/* Apply hanning window (should pre-compute it) */
	for (i = 0; i < N; i++)
		st->y[i] = MULT16_16_Q15(st->window[i], st->last_y[i]);

	/* Compute power spectrum of the echo */
	spx_fft(st->fft_table, st->y, st->Y);
	power_spectrum(st->Y, residual_echo, N);

#ifdef FIXED_POINT
	if (st->leak_estimate > 16383)
		leak2 = 32767;
	else
		leak2 = SHL16(st->leak_estimate, 1);
#else
	if (st->leak_estimate > .5)
		leak2 = 1;
	else
		leak2 = 2 * st->leak_estimate;
#endif
	/* Estimate residual echo */
	for (i = 0; i <= st->frame_size; i++)
		residual_echo[i] =
		    (int32_t) MULT16_32_Q15(leak2, residual_echo[i]);

}


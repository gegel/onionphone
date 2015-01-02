/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#include <stdio.h>
#ifndef NO_LPC_FIX
#include <stdlib.h>
#endif
#include <string.h>
#include <math.h>
#include <ophmconsts.h>
#include <sys/types.h>
#ifdef __linux__
#include <arpa/inet.h>
#endif
//#include <netinet/in.h>
#ifdef _WIN32
#include <htons.h>
#endif
#include "lpc.h"

#define MAXWINDOW	1000	/* Max analysis window length */
#define FS		8000.0	/* Sampling rate */

#define DOWN		5	/* Decimation for pitch analyzer */
#define PITCHORDER	4	/* Model order for pitch analyzer */
#define FC		600.0	/* Pitch analyzer filter cutoff */
#define MINPIT		50.0	/* Minimum pitch */
#define MAXPIT		300.0	/* Maximum pitch */

#define MINPER		(int)(FS/(DOWN*MAXPIT)+.5)	/* Minimum period */
#define MAXPER		(int)(FS/(DOWN*MINPIT)+.5)	/* Maximum period */

#define WSCALE		1.5863	/* Energy loss due to windowing */

#define BUFLEN		((FRAMESIZE * 3) / 2)

#define SILENCEFIX		/* Enable absolute silence fix */

/*  The LPC coder does truly awful things when driven into clipping.
    If you set GAIN_ADJUST to a number less than 1.0, samples will
    be scaled by that factor to avoid overdriving the coder.  */

#define GAIN_ADJUST 0.9

static struct lpcwork {
	float w_s[MAXWINDOW], w_y[MAXWINDOW], w_h[MAXWINDOW], w_w[MAXWINDOW];
} *work = NULL;

static float fa[6], u, u1, yp1, yp2;

static void auto_correl(w, n, p, r)
float *w;
int n;
int p;
float *r;
{
	int i, k, nk;

	for (k = 0; k <= p; k++) {
		nk = n - k;
		r[k] = 0.0;
		for (i = 0; i < nk; i++)
			r[k] += w[i] * w[i + k];
	}
}

static void durbin(r, p, k, g)
float *r;
int p;
float *k;
float *g;
{
	int i, j;
	float a[LPC_FILTORDER + 1], at[LPC_FILTORDER + 1], e;

	for (i = 0; i <= p; i++)
		a[i] = at[i] = 0.0;

	e = r[0];
	for (i = 1; i <= p; i++) {
		k[i] = -r[i];
		for (j = 1; j < i; j++) {
			at[j] = a[j];
			k[i] -= a[j] * r[i - j];
		}
#ifdef SILENCEFIX
		if (e == 0) {
			*g = 0;
			return;
		}
#endif
		k[i] /= e;
		a[i] = k[i];
		for (j = 1; j < i; j++)
			a[j] = at[j] + k[i] * at[i - j];
		e *= 1.0 - k[i] * k[i];
	}

#ifdef SILENCEFIX
	if (e < 0) {
		e = 0;
	}
#endif
	*g = sqrt(e);
}

static void inverse_filter(w, k)
float *w;
float *k;
{
	int i, j;
	float b[PITCHORDER + 1], bp[PITCHORDER + 1], f[PITCHORDER + 1];

	for (i = 0; i <= PITCHORDER; i++)
		b[i] = f[i] = bp[i] = 0.0;

	for (i = 0; i < BUFLEN / DOWN; i++) {
		f[0] = b[0] = w[i];
		for (j = 1; j <= PITCHORDER; j++) {
			f[j] = f[j - 1] + k[j] * bp[j - 1];
			b[j] = k[j] * f[j - 1] + bp[j - 1];
			bp[j - 1] = b[j - 1];
		}
		w[i] = f[PITCHORDER];
	}
}

static void calc_pitch(w, per)
float *w;
float *per;
{
	int i, j, rpos;
	float d[MAXWINDOW / DOWN], k[PITCHORDER + 1], r[MAXPER + 1], g, rmax;
	float rval, rm, rp;
	float a, b, c, x, y;
	static int vuv = 0;

#ifdef NO_LPC_FIX
	/* Old decimation sometimes fails to recognise voiced. */
	for (i = 0, j = 0; i < BUFLEN; i += DOWN)
		d[j++] = w[i];
#else
	/* New: average rather than decimating. */
	for (i = 0, j = 0;; j++) {
		d[j] = 0;
		for (rpos = 0; rpos < DOWN; rpos++) {
			d[j] += w[i++];
		}
		if (i >= BUFLEN)
			break;
		/* d[j] /- DOWN;       Not actually necessary. */
	}
#endif
	auto_correl(d, BUFLEN / DOWN, PITCHORDER, r);
	durbin(r, PITCHORDER, k, &g);
	inverse_filter(d, k);
#ifdef NO_LPC_FIX
	auto_correl(d, BUFLEN / DOWN, MAXPER + 1, r);
#else
	auto_correl(d, BUFLEN / DOWN, MAXPER, r);
#endif
	rpos = 0;
	rmax = 0.0;
	for (i = MINPER; i <= MAXPER; i++) {
		if (r[i] > rmax) {
			rmax = r[i];
			rpos = i;
		}
	}

	rm = r[rpos - 1];
	rp = r[rpos + 1];

	a = 0.5 * rm - rmax + 0.5 * rp;
	b = -0.5 * rm * (2.0 * rpos + 1.0) +
	    2.0 * rpos * rmax + 0.5 * rp * (1.0 - 2.0 * rpos);
	c = 0.5 * rm * (rpos * rpos + rpos) +
	    rmax * (1.0 - rpos * rpos) + 0.5 * rp * (rpos * rpos - rpos);

	x = -b / (2.0 * a);
	y = a * x * x + b * x + c;
	x *= DOWN;

	rmax = y;
#ifdef OLDWAY
	rval = rmax / r[0];
#else
	if (r[0] == 0.0) {
		rval = 1.0;
	} else {
		rval = rmax / r[0];
	}
#endif
#ifdef NO_LPC_FIX
	if (rval >= 0.4 || (vuv == 3 && rval >= 0.3)) {
#else
	if ((rval >= 0.4 || (vuv == 3 && rval >= 0.3)) && (x > 0)) {
#endif
		*per = x;
		vuv = (vuv & 1) * 2 + 1;
	} else {
		*per = 0.0;
		vuv = (vuv & 1) * 2;
	}
}

#define s   work->w_s
#define y   work->w_y
#define h   work->w_h

void lpc_init(state)
lpcstate_t *state;
{
	int i;
	float r, v, w, wcT;

#ifdef HEXDUMP
	/*  Let's make sure the compiler hasn't inserted
	   padding in the lpcparams_t structure which will
	   make it incompatible with other machines.  */

	{
		lpcparams_t *p = 0;

		if ((&p->gain != ((unsigned char *)2)) ||
		    (p->k != ((signed char *)4))) {
			fprintf(stderr,
				"Alignment problem in lpcparams.h structure.\n");
			fprintf(stderr,
				"Add definitions or compiler options in lpc directory\n");
			fprintf(stderr,
				"to guarantee this structure is packed.\n");
		}
	}
#endif

	for (i = 0; i < BUFLEN; i++) {
		s[i] = 0.0;
		h[i] = WSCALE * (0.54 - 0.46 *
				 cos(2 * M_PI * i / (BUFLEN - 1.0)));
	}
	wcT = 2 * M_PI * FC / FS;
	r = 0.36891079 * wcT;
	v = 0.18445539 * wcT;
	w = 0.92307712 * wcT;
	fa[1] = -exp(-r);
	fa[2] = 1.0 + fa[1];
	fa[3] = -2.0 * exp(-v) * cos(w);
	fa[4] = exp(-2.0 * v);
	fa[5] = 1.0 + fa[3] + fa[4];

	u1 = 0.0;
	yp1 = 0.0;
	yp2 = 0.0;

	state->Oldper = 0.0;
	state->OldG = 0.0;
	for (i = 0; i <= LPC_FILTORDER; i++) {
		state->Oldk[i] = 0.0;
		state->bp[i] = 0.0;
	}
	state->pitchctr = 0;
}

void lpc_analyze(buf, params)
short *buf;
lpcparams_t *params;
{
	int i, j;
#define w   work->w_w
	float r[LPC_FILTORDER + 1];
	float per, G, k[LPC_FILTORDER + 1];

	for (i = 0, j = BUFLEN - FRAMESIZE; i < FRAMESIZE; i++, j++) {
		s[j] = (float)(GAIN_ADJUST * ((*buf++) / 32768.));
		u = fa[2] * s[j] - fa[1] * u1;
		y[j] = fa[5] * u1 - fa[3] * yp1 - fa[4] * yp2;
		u1 = u;
		yp2 = yp1;
		yp1 = y[j];
	}

	calc_pitch(y, &per);

	for (i = 0; i < BUFLEN; i++)
		w[i] = s[i] * h[i];
	auto_correl(w, BUFLEN, LPC_FILTORDER, r);
	durbin(r, LPC_FILTORDER, k, &G);

	params->period =
	    (unsigned short)htons((unsigned short)(per * (1 << 8)));
#ifdef NO_LPC_FIX
	params->gain = G * (1 << 8);
#else
	i = (int)(G * (1 << 8));
	if (i > 255) {
		i = 255;
	}
	params->gain = i;
#endif
	for (i = 0; i < LPC_FILTORDER; i++) {
#ifdef NO_LPC_FIX
		params->k[i] = k[i + 1] * (1 << 7);
#else
		params->k[i] = (signed char)(0.5 + (127.0 * k[i + 1]));
#endif
	}

	memmove(s, s + FRAMESIZE, (BUFLEN - FRAMESIZE) * sizeof(s[0]));
	memmove(y, y + FRAMESIZE, (BUFLEN - FRAMESIZE) * sizeof(y[0]));
#undef w
}

void lpc_synthesize(buf, params, state)
short *buf;
lpcparams_t *params;
lpcstate_t *state;
{
	int i, j;
	register double u, f, per, G, NewG, Ginc, Newper, perinc;
	double k[LPC_FILTORDER + 1], Newk[LPC_FILTORDER + 1],
	    kinc[LPC_FILTORDER + 1];

	per =
	    (double)((unsigned short)(((params->period) >> 8) +
				      ((params->period) << 8))) / 256.;
	//per = (double) ((unsigned short) ntohs(params->period)) / 256.;
	G = (double)params->gain / 256.;
	k[0] = 0.0;
	for (i = 0; i < LPC_FILTORDER; i++)
		k[i + 1] = (double)(params->k[i]) / 128.;

	G /= sqrt(BUFLEN / (per == 0.0 ? 3.0 : per));
	Newper = state->Oldper;
	NewG = state->OldG;
	for (i = 1; i <= LPC_FILTORDER; i++)
		Newk[i] = state->Oldk[i];

	if (state->Oldper != 0 && per != 0) {
		perinc = (per - state->Oldper) / (double)FRAMESIZE;
		Ginc = (G - state->OldG) / (double)FRAMESIZE;
		for (i = 1; i <= LPC_FILTORDER; i++)
			kinc[i] = (k[i] - state->Oldk[i]) / (double)FRAMESIZE;
	} else {
		perinc = 0.0;
		Ginc = 0.0;
		for (i = 1; i <= LPC_FILTORDER; i++)
			kinc[i] = 0.0;
	}

	if (Newper == 0)
		state->pitchctr = 0;

	for (i = 0; i < FRAMESIZE; i++) {
		if (Newper == 0) {
#ifdef NO_LPC_FIX
			u = ((double)random() / 2147483648.0) * NewG;
#else
			u = ((rand() / (1.0 + RAND_MAX)) - 0.5) * 1.5874 * NewG;
#endif
		} else {
			if (state->pitchctr == 0) {
				u = NewG;
				state->pitchctr = (int)Newper;
			} else {
				u = 0.0;
				state->pitchctr--;
			}
		}

		f = u;
		for (j = LPC_FILTORDER; j >= 1; j--) {
			register double b = state->bp[j - 1];
			register double kj = Newk[j];
			Newk[j] = kj + kinc[j];
			f -= b * kj;
			b += f * kj;
			state->bp[j] = b;
		}
		state->bp[0] = f;

#ifdef NO_LPC_FIX
		*buf++ = (short)(f * 32768.0) & 0xffff;
#else
		u = f;
		if (u < -0.9999) {
			u = -0.9999;
		} else if (u > 0.9999) {
			u = 0.9999;
		}
		*buf++ = (short)(u * 32767.0);
#endif

		Newper += perinc;
		NewG += Ginc;
	}

	state->Oldper = per;
	state->OldG = G;
	for (i = 1; i <= LPC_FILTORDER; i++)
		state->Oldk[i] = k[i];
}

/*  LPC_START  --  Allocate working storage for LPC coder.  */

int lpc_start()
{
	if (work == NULL) {
		work = (struct lpcwork *)malloc(sizeof(struct lpcwork));
	}
	return work != NULL;
}

/*  LPC_END  --  Release working storage for LPC coder.  */

void lpc_end()
{
	if (work != NULL) {
		free(work);
		work = NULL;
	}
}

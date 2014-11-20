/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*
Copyright (c) 2003-2004, Mark Borgerding

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the author nor the names of any contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os_support.h"
#include "kiss_fftr.h"
#include "_kiss_fft_guts.h"

struct kiss_fftr_state {
	kiss_fft_cfg substate;
	kiss_fft_cpx *tmpbuf;
	kiss_fft_cpx *super_twiddles;
#ifdef USE_SIMD
	long pad;
#endif
};

/*  facbuf is populated by p1,m1,p2,m2, ...
    where 
    p[i] * m[i] = m[i-1]
    m0 = n                  */
static
void kf_factor(int n, int *facbuf)
{
	int p = 4;

	/*factor out powers of 4, powers of 2, then any remaining primes */
	do {
		while (n % p) {
			switch (p) {
			case 4:
				p = 2;
				break;
			case 2:
				p = 3;
				break;
			default:
				p += 2;
				break;
			}
			if (p > 32000 || (int32_t) p * (int32_t) p > n)
				p = n;	/* no more factors, skip to end */
		}
		n /= p;
		*facbuf++ = p;
		*facbuf++ = n;
	} while (n > 1);
}

/*
 *
 * User-callable function to allocate all necessary storage space for the fft.
 *
 * The return value is a contiguous block of memory, allocated with malloc.  As such,
 * It can be freed with free(), rather than a kiss_fft-specific function.
 * */
static kiss_fft_cfg kiss_fft_alloc(int nfft, int inverse_fft, void *mem,
				   size_t * lenmem)
{
	kiss_fft_cfg st = NULL;
	size_t memneeded = sizeof(struct kiss_fft_state)
	    + sizeof(kiss_fft_cpx) * (nfft - 1);	/* twiddle factors */

	if (lenmem == NULL) {
		st = (kiss_fft_cfg) calloc(1, memneeded);
	} else {
		if (mem != NULL && *lenmem >= memneeded)
			st = (kiss_fft_cfg) mem;
		*lenmem = memneeded;
	}
	if (st) {
		int i;
		st->nfft = nfft;
		st->inverse = inverse_fft;
#ifdef FIXED_POINT
		for (i = 0; i < nfft; ++i) {
			spx_word32_t phase = i;
			if (!st->inverse)
				phase = -phase;
			kf_cexp2(st->twiddles + i,
				 DIV32(SHL32(phase, 17), nfft));
		}
#else
		for (i = 0; i < nfft; ++i) {
			const double pi = 3.14159265358979323846264338327;
			double phase = (-2 * pi / nfft) * i;
			if (st->inverse)
				phase *= -1;
			kf_cexp(st->twiddles + i, phase);
		}
#endif
		kf_factor(nfft, st->factors);
	}
	return st;
}

static void kf_bfly2(kiss_fft_cpx * Fout,
		     const size_t fstride,
		     const kiss_fft_cfg st, int m, int N, int mm)
{
	kiss_fft_cpx *Fout2;
	kiss_fft_cpx *tw1;
	kiss_fft_cpx t;
	if (!st->inverse) {
		int i, j;
		kiss_fft_cpx *Fout_beg = Fout;
		for (i = 0; i < N; i++) {
			Fout = Fout_beg + i * mm;
			Fout2 = Fout + m;
			tw1 = st->twiddles;
			for (j = 0; j < m; j++) {
				/* Almost the same as the code path below, except that we divide the input by two
				   (while keeping the best accuracy possible) */
				spx_word32_t tr, ti;
				tr = SHR32(SUB32
					   (MULT16_16(Fout2->r, tw1->r),
					    MULT16_16(Fout2->i, tw1->i)), 1);
				ti = SHR32(ADD32
					   (MULT16_16(Fout2->i, tw1->r),
					    MULT16_16(Fout2->r, tw1->i)), 1);
				tw1 += fstride;
				Fout2->r =
				    PSHR32(SUB32
					   (SHL32(EXTEND32(Fout->r), 14), tr),
					   15);
				Fout2->i =
				    PSHR32(SUB32
					   (SHL32(EXTEND32(Fout->i), 14), ti),
					   15);
				Fout->r =
				    PSHR32(ADD32
					   (SHL32(EXTEND32(Fout->r), 14), tr),
					   15);
				Fout->i =
				    PSHR32(ADD32
					   (SHL32(EXTEND32(Fout->i), 14), ti),
					   15);
				++Fout2;
				++Fout;
			}
		}
	} else {
		int i, j;
		kiss_fft_cpx *Fout_beg = Fout;
		for (i = 0; i < N; i++) {
			Fout = Fout_beg + i * mm;
			Fout2 = Fout + m;
			tw1 = st->twiddles;
			for (j = 0; j < m; j++) {
				C_MUL(t, *Fout2, *tw1);
				tw1 += fstride;
				C_SUB(*Fout2, *Fout, t);
				C_ADDTO(*Fout, t);
				++Fout2;
				++Fout;
			}
		}
	}
}

static void kf_bfly4(kiss_fft_cpx * Fout,
		     const size_t fstride,
		     const kiss_fft_cfg st, int m, int N, int mm)
{
	kiss_fft_cpx *tw1, *tw2, *tw3;
	kiss_fft_cpx scratch[6];
	const size_t m2 = 2 * m;
	const size_t m3 = 3 * m;
	int i, j;

	if (st->inverse) {
		kiss_fft_cpx *Fout_beg = Fout;
		for (i = 0; i < N; i++) {
			Fout = Fout_beg + i * mm;
			tw3 = tw2 = tw1 = st->twiddles;
			for (j = 0; j < m; j++) {
				C_MUL(scratch[0], Fout[m], *tw1);
				C_MUL(scratch[1], Fout[m2], *tw2);
				C_MUL(scratch[2], Fout[m3], *tw3);

				C_SUB(scratch[5], *Fout, scratch[1]);
				C_ADDTO(*Fout, scratch[1]);
				C_ADD(scratch[3], scratch[0], scratch[2]);
				C_SUB(scratch[4], scratch[0], scratch[2]);
				C_SUB(Fout[m2], *Fout, scratch[3]);
				tw1 += fstride;
				tw2 += fstride * 2;
				tw3 += fstride * 3;
				C_ADDTO(*Fout, scratch[3]);

				Fout[m].r = scratch[5].r - scratch[4].i;
				Fout[m].i = scratch[5].i + scratch[4].r;
				Fout[m3].r = scratch[5].r + scratch[4].i;
				Fout[m3].i = scratch[5].i - scratch[4].r;
				++Fout;
			}
		}
	} else {
		kiss_fft_cpx *Fout_beg = Fout;
		for (i = 0; i < N; i++) {
			Fout = Fout_beg + i * mm;
			tw3 = tw2 = tw1 = st->twiddles;
			for (j = 0; j < m; j++) {
				C_MUL4(scratch[0], Fout[m], *tw1);
				C_MUL4(scratch[1], Fout[m2], *tw2);
				C_MUL4(scratch[2], Fout[m3], *tw3);

				Fout->r = PSHR16(Fout->r, 2);
				Fout->i = PSHR16(Fout->i, 2);
				C_SUB(scratch[5], *Fout, scratch[1]);
				C_ADDTO(*Fout, scratch[1]);
				C_ADD(scratch[3], scratch[0], scratch[2]);
				C_SUB(scratch[4], scratch[0], scratch[2]);
				Fout[m2].r = PSHR16(Fout[m2].r, 2);
				Fout[m2].i = PSHR16(Fout[m2].i, 2);
				C_SUB(Fout[m2], *Fout, scratch[3]);
				tw1 += fstride;
				tw2 += fstride * 2;
				tw3 += fstride * 3;
				C_ADDTO(*Fout, scratch[3]);

				Fout[m].r = scratch[5].r + scratch[4].i;
				Fout[m].i = scratch[5].i - scratch[4].r;
				Fout[m3].r = scratch[5].r - scratch[4].i;
				Fout[m3].i = scratch[5].i + scratch[4].r;
				++Fout;
			}
		}
	}
}

static void kf_bfly3(kiss_fft_cpx * Fout,
		     const size_t fstride, const kiss_fft_cfg st, size_t m)
{
	size_t k = m;
	const size_t m2 = 2 * m;
	kiss_fft_cpx *tw1, *tw2;
	kiss_fft_cpx scratch[5];
	kiss_fft_cpx epi3;
	epi3 = st->twiddles[fstride * m];

	tw1 = tw2 = st->twiddles;

	do {
		if (!st->inverse) {
			C_FIXDIV(*Fout, 3);
			C_FIXDIV(Fout[m], 3);
			C_FIXDIV(Fout[m2], 3);
		}

		C_MUL(scratch[1], Fout[m], *tw1);
		C_MUL(scratch[2], Fout[m2], *tw2);

		C_ADD(scratch[3], scratch[1], scratch[2]);
		C_SUB(scratch[0], scratch[1], scratch[2]);
		tw1 += fstride;
		tw2 += fstride * 2;

		Fout[m].r = Fout->r - HALF_OF(scratch[3].r);
		Fout[m].i = Fout->i - HALF_OF(scratch[3].i);

		C_MULBYSCALAR(scratch[0], epi3.i);

		C_ADDTO(*Fout, scratch[3]);

		Fout[m2].r = Fout[m].r + scratch[0].i;
		Fout[m2].i = Fout[m].i - scratch[0].r;

		Fout[m].r -= scratch[0].i;
		Fout[m].i += scratch[0].r;

		++Fout;
	} while (--k);
}

static void kf_bfly5(kiss_fft_cpx * Fout,
		     const size_t fstride, const kiss_fft_cfg st, int m)
{
	kiss_fft_cpx *Fout0, *Fout1, *Fout2, *Fout3, *Fout4;
	int u;
	kiss_fft_cpx scratch[13];
	kiss_fft_cpx *twiddles = st->twiddles;
	kiss_fft_cpx *tw;
	kiss_fft_cpx ya, yb;
	ya = twiddles[fstride * m];
	yb = twiddles[fstride * 2 * m];

	Fout0 = Fout;
	Fout1 = Fout0 + m;
	Fout2 = Fout0 + 2 * m;
	Fout3 = Fout0 + 3 * m;
	Fout4 = Fout0 + 4 * m;

	tw = st->twiddles;
	for (u = 0; u < m; ++u) {
		if (!st->inverse) {
			C_FIXDIV(*Fout0, 5);
			C_FIXDIV(*Fout1, 5);
			C_FIXDIV(*Fout2, 5);
			C_FIXDIV(*Fout3, 5);
			C_FIXDIV(*Fout4, 5);
		}
		scratch[0] = *Fout0;

		C_MUL(scratch[1], *Fout1, tw[u * fstride]);
		C_MUL(scratch[2], *Fout2, tw[2 * u * fstride]);
		C_MUL(scratch[3], *Fout3, tw[3 * u * fstride]);
		C_MUL(scratch[4], *Fout4, tw[4 * u * fstride]);

		C_ADD(scratch[7], scratch[1], scratch[4]);
		C_SUB(scratch[10], scratch[1], scratch[4]);
		C_ADD(scratch[8], scratch[2], scratch[3]);
		C_SUB(scratch[9], scratch[2], scratch[3]);

		Fout0->r += scratch[7].r + scratch[8].r;
		Fout0->i += scratch[7].i + scratch[8].i;

		scratch[5].r =
		    scratch[0].r + S_MUL(scratch[7].r,
					 ya.r) + S_MUL(scratch[8].r, yb.r);
		scratch[5].i =
		    scratch[0].i + S_MUL(scratch[7].i,
					 ya.r) + S_MUL(scratch[8].i, yb.r);

		scratch[6].r =
		    S_MUL(scratch[10].i, ya.i) + S_MUL(scratch[9].i, yb.i);
		scratch[6].i =
		    -S_MUL(scratch[10].r, ya.i) - S_MUL(scratch[9].r, yb.i);

		C_SUB(*Fout1, scratch[5], scratch[6]);
		C_ADD(*Fout4, scratch[5], scratch[6]);

		scratch[11].r =
		    scratch[0].r + S_MUL(scratch[7].r,
					 yb.r) + S_MUL(scratch[8].r, ya.r);
		scratch[11].i =
		    scratch[0].i + S_MUL(scratch[7].i,
					 yb.r) + S_MUL(scratch[8].i, ya.r);
		scratch[12].r =
		    -S_MUL(scratch[10].i, yb.i) + S_MUL(scratch[9].i, ya.i);
		scratch[12].i =
		    S_MUL(scratch[10].r, yb.i) - S_MUL(scratch[9].r, ya.i);

		C_ADD(*Fout2, scratch[11], scratch[12]);
		C_SUB(*Fout3, scratch[11], scratch[12]);

		++Fout0;
		++Fout1;
		++Fout2;
		++Fout3;
		++Fout4;
	}
}

/* perform the butterfly for one stage of a mixed radix FFT */
static void kf_bfly_generic(kiss_fft_cpx * Fout,
			    const size_t fstride,
			    const kiss_fft_cfg st, int m, int p)
{
	int u, k, q1, q;
	kiss_fft_cpx *twiddles = st->twiddles;
	kiss_fft_cpx t;
	kiss_fft_cpx scratchbuf[17];
	int Norig = st->nfft;

	/*CHECKBUF(scratchbuf,nscratchbuf,p); */
	if (p > 17)
		speex_fatal("KissFFT: max radix supported is 17");

	for (u = 0; u < m; ++u) {
		k = u;
		for (q1 = 0; q1 < p; ++q1) {
			scratchbuf[q1] = Fout[k];
			if (!st->inverse) {
				C_FIXDIV(scratchbuf[q1], p);
			}
			k += m;
		}

		k = u;
		for (q1 = 0; q1 < p; ++q1) {
			int twidx = 0;
			Fout[k] = scratchbuf[0];
			for (q = 1; q < p; ++q) {
				twidx += fstride * k;
				if (twidx >= Norig)
					twidx -= Norig;
				C_MUL(t, scratchbuf[q], twiddles[twidx]);
				C_ADDTO(Fout[k], t);
			}
			k += m;
		}
	}
}

static
void kf_shuffle(kiss_fft_cpx * Fout,
		const kiss_fft_cpx * f,
		const size_t fstride,
		int in_stride, int *factors, const kiss_fft_cfg st)
{
	const int p = *factors++;	/* the radix  */
	const int m = *factors++;	/* stage's fft length/p */

	/*printf ("fft %d %d %d %d %d %d\n", p*m, m, p, s2, fstride*in_stride, N); */
	if (m == 1) {
		int j;
		for (j = 0; j < p; j++) {
			Fout[j] = *f;
			f += fstride * in_stride;
		}
	} else {
		int j;
		for (j = 0; j < p; j++) {
			kf_shuffle(Fout, f, fstride * p, in_stride, factors,
				   st);
			f += fstride * in_stride;
			Fout += m;
		}
	}
}

static
void kf_work(kiss_fft_cpx * Fout,
	     const kiss_fft_cpx * f,
	     const size_t fstride,
	     int in_stride,
	     int *factors, const kiss_fft_cfg st, int N, int s2, int m2)
{
	(void)s2;		/* void due to if 0 and commented code below */

	int i;
	kiss_fft_cpx *Fout_beg = Fout;
	const int p = *factors++;	/* the radix  */
	const int m = *factors++;	/* stage's fft length/p */
#if 0
	/*printf ("fft %d %d %d %d %d %d\n", p*m, m, p, s2, fstride*in_stride, N); */
	if (m == 1) {
		/*   int j;
		   for (j=0;j<p;j++)
		   {
		   Fout[j] = *f;
		   f += fstride*in_stride;
		   } */
	} else {
		int j;
		for (j = 0; j < p; j++) {
			kf_work(Fout, f, fstride * p, in_stride, factors, st,
				N * p, fstride * in_stride, m);
			f += fstride * in_stride;
			Fout += m;
		}
	}

	Fout = Fout_beg;

	switch (p) {
	case 2:
		kf_bfly2(Fout, fstride, st, m);
		break;
	case 3:
		kf_bfly3(Fout, fstride, st, m);
		break;
	case 4:
		kf_bfly4(Fout, fstride, st, m);
		break;
	case 5:
		kf_bfly5(Fout, fstride, st, m);
		break;
	default:
		kf_bfly_generic(Fout, fstride, st, m, p);
		break;
	}
#else
	/*printf ("fft %d %d %d %d %d %d %d\n", p*m, m, p, s2, fstride*in_stride, N, m2); */
	if (m == 1) {
		/*for (i=0;i<N;i++)
		   {
		   int j;
		   Fout = Fout_beg+i*m2;
		   const kiss_fft_cpx * f2 = f+i*s2;
		   for (j=0;j<p;j++)
		   {
		   *Fout++ = *f2;
		   f2 += fstride*in_stride;
		   }
		   } */
	} else {
		kf_work(Fout, f, fstride * p, in_stride, factors, st, N * p,
			fstride * in_stride, m);
	}

	switch (p) {
	case 2:
		kf_bfly2(Fout, fstride, st, m, N, m2);
		break;
	case 3:
		for (i = 0; i < N; i++) {
			Fout = Fout_beg + i * m2;
			kf_bfly3(Fout, fstride, st, m);
		}
		break;
	case 4:
		kf_bfly4(Fout, fstride, st, m, N, m2);
		break;
	case 5:
		for (i = 0; i < N; i++) {
			Fout = Fout_beg + i * m2;
			kf_bfly5(Fout, fstride, st, m);
		}
		break;
	default:
		for (i = 0; i < N; i++) {
			Fout = Fout_beg + i * m2;
			kf_bfly_generic(Fout, fstride, st, m, p);
		}
		break;
	}
#endif
}

static void kiss_fft_stride(kiss_fft_cfg st, const kiss_fft_cpx * fin,
			    kiss_fft_cpx * fout, int in_stride)
{
	if (fin == fout) {
		speex_fatal("In-place FFT not supported");
		/*CHECKBUF(tmpbuf,ntmpbuf,st->nfft);
		   kf_work(tmpbuf,fin,1,in_stride, st->factors,st);
		   SPEEX_MOVE(fout,tmpbuf,st->nfft); */
	} else {
		kf_shuffle(fout, fin, 1, in_stride, st->factors, st);
		kf_work(fout, fin, 1, in_stride, st->factors, st, 1, in_stride,
			1);
	}
}

static void kiss_fft(kiss_fft_cfg cfg, const kiss_fft_cpx * fin,
		     kiss_fft_cpx * fout)
{
	kiss_fft_stride(cfg, fin, fout, 1);
}

kiss_fftr_cfg kiss_fftr_alloc(int nfft, int inverse_fft, void *mem,
			      size_t * lenmem)
{
	int i;
	kiss_fftr_cfg st = NULL;
	size_t subsize, memneeded;

	if (nfft & 1) {
		speex_warning("Real FFT optimization must be even.\n");
		return NULL;
	}
	nfft >>= 1;

	kiss_fft_alloc(nfft, inverse_fft, NULL, &subsize);
	memneeded =
	    sizeof(struct kiss_fftr_state) + subsize +
	    sizeof(kiss_fft_cpx) * (nfft * 2);

	if (lenmem == NULL) {
		st = (kiss_fftr_cfg) calloc(1, memneeded);
	} else {
		if (*lenmem >= memneeded)
			st = (kiss_fftr_cfg) mem;
		*lenmem = memneeded;
	}
	if (!st)
		return NULL;

	st->substate = (kiss_fft_cfg) (st + 1);	/*just beyond kiss_fftr_state struct */
	st->tmpbuf = (kiss_fft_cpx *) (((char *)st->substate) + subsize);
	st->super_twiddles = st->tmpbuf + nfft;
	kiss_fft_alloc(nfft, inverse_fft, st->substate, &subsize);

#ifdef FIXED_POINT
	for (i = 0; i < nfft; ++i) {
		spx_word32_t phase = i + (nfft >> 1);
		if (!inverse_fft)
			phase = -phase;
		kf_cexp2(st->super_twiddles + i, DIV32(SHL32(phase, 16), nfft));
	}
#else
	for (i = 0; i < nfft; ++i) {
		const double pi = 3.14159265358979323846264338327;
		double phase = pi * (((double)i) / nfft + .5);
		if (!inverse_fft)
			phase = -phase;
		kf_cexp(st->super_twiddles + i, phase);
	}
#endif
	return st;
}

void kiss_fftr(kiss_fftr_cfg st, const kiss_fft_scalar * timedata,
	       kiss_fft_cpx * freqdata)
{
	/* input buffer timedata is stored row-wise */
	int k, ncfft;
	kiss_fft_cpx fpnk, fpk, f1k, f2k, tw, tdc;

	if (st->substate->inverse) {
		speex_fatal("kiss fft usage error: improper alloc\n");
	}

	ncfft = st->substate->nfft;

	/*perform the parallel fft of two real signals packed in real,imag */
	kiss_fft(st->substate, (const kiss_fft_cpx *)timedata, st->tmpbuf);
	/* The real part of the DC element of the frequency spectrum in st->tmpbuf
	 * contains the sum of the even-numbered elements of the input time sequence
	 * The imag part is the sum of the odd-numbered elements
	 *
	 * The sum of tdc.r and tdc.i is the sum of the input time sequence. 
	 *      yielding DC of input time sequence
	 * The difference of tdc.r - tdc.i is the sum of the input (dot product) [1,-1,1,-1... 
	 *      yielding Nyquist bin of input time sequence
	 */

	tdc.r = st->tmpbuf[0].r;
	tdc.i = st->tmpbuf[0].i;
	C_FIXDIV(tdc, 2);
	CHECK_OVERFLOW_OP(tdc.r, +, tdc.i);
	CHECK_OVERFLOW_OP(tdc.r, -, tdc.i);
	freqdata[0].r = tdc.r + tdc.i;
	freqdata[ncfft].r = tdc.r - tdc.i;
#ifdef USE_SIMD
	freqdata[ncfft].i = freqdata[0].i = _mm_set1_ps(0);
#else
	freqdata[ncfft].i = freqdata[0].i = 0;
#endif

	for (k = 1; k <= ncfft / 2; ++k) {
		fpk = st->tmpbuf[k];
		fpnk.r = st->tmpbuf[ncfft - k].r;
		fpnk.i = -st->tmpbuf[ncfft - k].i;
		C_FIXDIV(fpk, 2);
		C_FIXDIV(fpnk, 2);

		C_ADD(f1k, fpk, fpnk);
		C_SUB(f2k, fpk, fpnk);
		C_MUL(tw, f2k, st->super_twiddles[k]);

		freqdata[k].r = HALF_OF(f1k.r + tw.r);
		freqdata[k].i = HALF_OF(f1k.i + tw.i);
		freqdata[ncfft - k].r = HALF_OF(f1k.r - tw.r);
		freqdata[ncfft - k].i = HALF_OF(tw.i - f1k.i);
	}
}

void kiss_fftri(kiss_fftr_cfg st, const kiss_fft_cpx * freqdata,
		kiss_fft_scalar * timedata)
{
	/* input buffer timedata is stored row-wise */
	int k, ncfft;

	if (st->substate->inverse == 0) {
		speex_fatal("kiss fft usage error: improper alloc\n");
	}

	ncfft = st->substate->nfft;

	st->tmpbuf[0].r = freqdata[0].r + freqdata[ncfft].r;
	st->tmpbuf[0].i = freqdata[0].r - freqdata[ncfft].r;
	/*C_FIXDIV(st->tmpbuf[0],2); */

	for (k = 1; k <= ncfft / 2; ++k) {
		kiss_fft_cpx fk, fnkc, fek, fok, tmp;
		fk = freqdata[k];
		fnkc.r = freqdata[ncfft - k].r;
		fnkc.i = -freqdata[ncfft - k].i;
		/*C_FIXDIV( fk , 2 );
		   C_FIXDIV( fnkc , 2 ); */

		C_ADD(fek, fk, fnkc);
		C_SUB(tmp, fk, fnkc);
		C_MUL(fok, tmp, st->super_twiddles[k]);
		C_ADD(st->tmpbuf[k], fek, fok);
		C_SUB(st->tmpbuf[ncfft - k], fek, fok);
#ifdef USE_SIMD
		st->tmpbuf[ncfft - k].i *= _mm_set1_ps(-1.0);
#else
		st->tmpbuf[ncfft - k].i *= -1;
#endif
	}
	kiss_fft(st->substate, st->tmpbuf, (kiss_fft_cpx *) timedata);
}

void kiss_fftr2(kiss_fftr_cfg st, const kiss_fft_scalar * timedata,
		kiss_fft_scalar * freqdata)
{
	/* input buffer timedata is stored row-wise */
	int k, ncfft;
	kiss_fft_cpx f2k, tdc;
	spx_word32_t f1kr, f1ki, twr, twi;

	if (st->substate->inverse) {
		speex_fatal("kiss fft usage error: improper alloc\n");
	}

	ncfft = st->substate->nfft;

	/*perform the parallel fft of two real signals packed in real,imag */
	kiss_fft(st->substate, (const kiss_fft_cpx *)timedata, st->tmpbuf);
	/* The real part of the DC element of the frequency spectrum in st->tmpbuf
	 * contains the sum of the even-numbered elements of the input time sequence
	 * The imag part is the sum of the odd-numbered elements
	 *
	 * The sum of tdc.r and tdc.i is the sum of the input time sequence. 
	 *      yielding DC of input time sequence
	 * The difference of tdc.r - tdc.i is the sum of the input (dot product) [1,-1,1,-1... 
	 *      yielding Nyquist bin of input time sequence
	 */

	tdc.r = st->tmpbuf[0].r;
	tdc.i = st->tmpbuf[0].i;
	C_FIXDIV(tdc, 2);
	CHECK_OVERFLOW_OP(tdc.r, +, tdc.i);
	CHECK_OVERFLOW_OP(tdc.r, -, tdc.i);
	freqdata[0] = tdc.r + tdc.i;
	freqdata[2 * ncfft - 1] = tdc.r - tdc.i;

	for (k = 1; k <= ncfft / 2; ++k) {
		/*fpk    = st->tmpbuf[k]; 
		   fpnk.r =   st->tmpbuf[ncfft-k].r;
		   fpnk.i = - st->tmpbuf[ncfft-k].i;
		   C_FIXDIV(fpk,2);
		   C_FIXDIV(fpnk,2);

		   C_ADD( f1k, fpk , fpnk );
		   C_SUB( f2k, fpk , fpnk );

		   C_MUL( tw , f2k , st->super_twiddles[k]);

		   freqdata[2*k-1] = HALF_OF(f1k.r + tw.r);
		   freqdata[2*k] = HALF_OF(f1k.i + tw.i);
		   freqdata[2*(ncfft-k)-1] = HALF_OF(f1k.r - tw.r);
		   freqdata[2*(ncfft-k)] = HALF_OF(tw.i - f1k.i);
		 */

		/*f1k.r = PSHR32(ADD32(EXTEND32(st->tmpbuf[k].r), EXTEND32(st->tmpbuf[ncfft-k].r)),1);
		   f1k.i = PSHR32(SUB32(EXTEND32(st->tmpbuf[k].i), EXTEND32(st->tmpbuf[ncfft-k].i)),1);
		   f2k.r = PSHR32(SUB32(EXTEND32(st->tmpbuf[k].r), EXTEND32(st->tmpbuf[ncfft-k].r)),1);
		   f2k.i = SHR32(ADD32(EXTEND32(st->tmpbuf[k].i), EXTEND32(st->tmpbuf[ncfft-k].i)),1);

		   C_MUL( tw , f2k , st->super_twiddles[k]);

		   freqdata[2*k-1] = HALF_OF(f1k.r + tw.r);
		   freqdata[2*k] = HALF_OF(f1k.i + tw.i);
		   freqdata[2*(ncfft-k)-1] = HALF_OF(f1k.r - tw.r);
		   freqdata[2*(ncfft-k)] = HALF_OF(tw.i - f1k.i);
		 */
		f2k.r =
		    SHR32(SUB32
			  (EXTEND32(st->tmpbuf[k].r),
			   EXTEND32(st->tmpbuf[ncfft - k].r)), 1);
		f2k.i =
		    PSHR32(ADD32
			   (EXTEND32(st->tmpbuf[k].i),
			    EXTEND32(st->tmpbuf[ncfft - k].i)), 1);

		f1kr =
		    SHL32(ADD32
			  (EXTEND32(st->tmpbuf[k].r),
			   EXTEND32(st->tmpbuf[ncfft - k].r)), 13);
		f1ki =
		    SHL32(SUB32
			  (EXTEND32(st->tmpbuf[k].i),
			   EXTEND32(st->tmpbuf[ncfft - k].i)), 13);

		twr =
		    SHR32(SUB32
			  (MULT16_16(f2k.r, st->super_twiddles[k].r),
			   MULT16_16(f2k.i, st->super_twiddles[k].i)), 1);
		twi =
		    SHR32(ADD32
			  (MULT16_16(f2k.i, st->super_twiddles[k].r),
			   MULT16_16(f2k.r, st->super_twiddles[k].i)), 1);

#ifdef FIXED_POINT
		freqdata[2 * k - 1] = PSHR32(f1kr + twr, 15);
		freqdata[2 * k] = PSHR32(f1ki + twi, 15);
		freqdata[2 * (ncfft - k) - 1] = PSHR32(f1kr - twr, 15);
		freqdata[2 * (ncfft - k)] = PSHR32(twi - f1ki, 15);
#else
		freqdata[2 * k - 1] = .5f * (f1kr + twr);
		freqdata[2 * k] = .5f * (f1ki + twi);
		freqdata[2 * (ncfft - k) - 1] = .5f * (f1kr - twr);
		freqdata[2 * (ncfft - k)] = .5f * (twi - f1ki);

#endif
	}
}

void kiss_fftri2(kiss_fftr_cfg st, const kiss_fft_scalar * freqdata,
		 kiss_fft_scalar * timedata)
{
	/* input buffer timedata is stored row-wise */
	int k, ncfft;

	if (st->substate->inverse == 0) {
		speex_fatal("kiss fft usage error: improper alloc\n");
	}

	ncfft = st->substate->nfft;

	st->tmpbuf[0].r = freqdata[0] + freqdata[2 * ncfft - 1];
	st->tmpbuf[0].i = freqdata[0] - freqdata[2 * ncfft - 1];
	/*C_FIXDIV(st->tmpbuf[0],2); */

	for (k = 1; k <= ncfft / 2; ++k) {
		kiss_fft_cpx fk, fnkc, fek, fok, tmp;
		fk.r = freqdata[2 * k - 1];
		fk.i = freqdata[2 * k];
		fnkc.r = freqdata[2 * (ncfft - k) - 1];
		fnkc.i = -freqdata[2 * (ncfft - k)];
		/*C_FIXDIV( fk , 2 );
		   C_FIXDIV( fnkc , 2 ); */

		C_ADD(fek, fk, fnkc);
		C_SUB(tmp, fk, fnkc);
		C_MUL(fok, tmp, st->super_twiddles[k]);
		C_ADD(st->tmpbuf[k], fek, fok);
		C_SUB(st->tmpbuf[ncfft - k], fek, fok);
#ifdef USE_SIMD
		st->tmpbuf[ncfft - k].i *= _mm_set1_ps(-1.0);
#else
		st->tmpbuf[ncfft - k].i *= -1;
#endif
	}
	kiss_fft(st->substate, st->tmpbuf, (kiss_fft_cpx *) timedata);
}

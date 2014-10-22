/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*                                                                         *
*	CELP Voice Coder                                                  *
*	Version 3.2c	                                                  *
*                                                                         *
***************************************************************************/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "ccsub.h"
#include <ophtools.h>

#define PROTECT

#define TRUE		1
#define FALSE		0

#include "celplib.h"
#include "celp.h"

#define STREAMBITS	144
#define CODELENGTH1	15
#define CODELENGTH2	11
#define PARITYLENGTH	(CODELENGTH1 - CODELENGTH2)
#define SYNDRUN		100
#define OMEGA		0.994127	/* Bandwidth expansion for LPC analysis (15 Hz) */
#define ALPHA		0.8	/* Bandwidth expansion for postfilter */
#define BETA		0.5	/* Bandwidth expansion for postfilter */

#define mmax(A,B)        ((A)>(B)?(A):(B))
#define mmin(A,B)        ((A)<(B)?(A):(B))

static const int cbgbits = 5, ncsize = 512, no = 10;
static int cbindex = 0, gindex = 0, idb = 0;
static int nseg = 0, pindex = 0, frame = 0, tauptr = 0, minptr = 0, plevel1 = 0;
static int plevel2 = 0;
static const int pbits[MAXNP + 2] = { 8, 6, 5, 0, 0 };

static const int mxsw = 1;

static float bb[MAXNP + 1], e0[MAXLP];
static float fc[MAXNO + 1], fcn[MAXNO + 1], fci[MAXNO + 1];
static float h[MAXLP];
static const float gamma2 = 0.8f, prewt = 0.0f;

/*  	Adaptive code book index (pitch delay) file */
static const float pdelay[MAXPD] = {
#include "pdelay.h"
};

/*  	Stochastic code book vector file */
static const float x[MAXCODE] = {
#include "codebook.h"
};

/*  	Pitch delay coding tables for bit assignment:
    	    pdencode.h for encoding, pddecode.h for decoding */
static const int pdencode[MAXPD] = {
#include "pdencode.h"
};

static const float pddecode[MAXPD] = {
#include "pddecode.h"
};

static const char ptype[10] = "max2", cbgtype[10] = "log";
static const char pstype[10] = "hier";

static const int ll = 240, lp = 60, l = 60;
static int cbi[MAXLL / MAXL];
static int i, j, k;
static int nn;
static int findex[MAXNO];
static int lspflag;
static int pdtabi[MAXPD];

static float cbg[MAXLL / MAXL], pgs[MAXLL / MAXL];
static float sold[MAXLL], snew[MAXLL], ssub[MAXLL], v[MAXLL];
static float vdecoded[MAXLL], rcn[MAXNO], hamw[MAXLL];
static float dps[MAXPA], newfreq[MAXNO], unqfreq[MAXNO],
    lsp[MAXLL / MAXL][MAXNO];
static float dpps[MAXPA];
static float decodedgain, taus[4];
static const float scale = 1.0;

/*  	Filter coefficients for 2nd order Butterworth 100 Hz HPF */

static const float ahpf[3] = { 0.946, -1.892, 0.946 };
static const float bhpf[3] = { 1.0, -1.889033, 0.8948743 };

/*  	Bit stream  */

static const int cbbits = 9;
static int pointer, bitpointer, bitsum1, bitsum2;
static const int sbits[MAXNO] = { 3, 4, 4, 4, 4, 3, 3, 3, 3, 3 };

static short stream[STREAMBITS], savestream[STREAMBITS];

/*  	Filter memories (should be maxno+1) */

static float dhpf1[3], dhpf2[3], dss[MAXNO + 1];
static float dp1[MAXNO + 1], dp2[MAXNO + 1], dp3[2];
static float ip, op;

/*  	Error control coding parameters */

static float syndavg = 0.0;
static int twoerror;

#ifdef PROTECT
static int snrflag;
static int syncBit = 1;
static int codeword[CODELENGTH1], hmatrix[CODELENGTH1];
static int syndrometable[CODELENGTH1], paritybit, protect;
static int syndrome;

/*  	Bit protection vector */
static const int bitprotect[CODELENGTH2] = {
#include "bitprot.h"
};
#endif

static int celp_error;		/* Error in encoding or decoding */

#include "celpfiles.h"

/*  	CELP_INIT  --  Initialise CELP Codec.  Set "prot" nonzero
    	    	       if you wish to include bit error protection
		       in the encoded stream.  If you're using a
		       transmission layer which performs its own
		       error detection and correction (such as Internet
		       TCP or UDP transmission), bit error recover in
		       the CELP stream is a waste of time.  */

void celp_init(int prot)
{
#ifdef PROTECT
	protect = prot;		/* Set bit error recovery mode */
#endif

	/* Number of codewords/LPC frame */

	nn = ll / l;

	/* Dimension of d1a and d1b */

	idb = MMAX + MAXNP - 1 + l;
	plevel1 = 1 << pbits[0];

	/* Levels of delta tau */

	plevel2 = 1 << pbits[1];

	/* number of bits per subframe */

	bitsum1 = cbbits + cbgbits + pbits[0] + pbits[2];
	bitsum2 = cbbits + cbgbits + pbits[1] + pbits[2];

	/* For double error detecting FEC codes (NOT USED) */

#ifdef PROTECT
	twoerror = FALSE;
	snrflag = FALSE;
	lspflag = TRUE;
#endif

	/* Initialise arrays */

	for (i = 0; i < MAXLP; i++) {
		h[i] = e0[i] = 0.0;
	}
	for (i = 0; i < MAXLL; i++) {
		sold[i] = 0.0;
	}
	for (i = 0; i < STREAMBITS; i++) {
		stream[i] = savestream[i] = 0;
	}

	/* Start nseg at 0 to do pitch on odd segments.
	   (nseg is incremented before csub). */

	nseg = 0;

	/* Generate matrix for error control coding */

#ifdef PROTECT
	matrixgen(CODELENGTH1, CODELENGTH2, hmatrix, syndrometable);
#endif

	/* Generate Hamming windows */

	ham(hamw, ll);

	/* Generate pdtabi for delta delay coding */
	for (i = 0; i < MAXPD; i++) {
		pdtabi[pdencode[i]] = i;
	}
}

/*  CELP_ENCODE  --  Encode a 240 sample frame of audio in CELP.  */

int celp_encode(short iarf[MAXLL], char packedbits[STREAMBITS / 8])
{
	int i, pointer;
	int i1, i2, i3;

	frame++;
	pointer = 0;
	celp_error = CELP_OK;

	/*  Scale and convert to real speech.
	   The ssub buffer used for subframe CELP analysis is 1/2 a
	   frame behind the snew buffer and 1/2 a frame ahead of the 
	   sold buffer.  */

	for (i = 0; i < ll; i++) {
		snew[i] = mmax(-32768., mmin(scale * iarf[i], 32767.));
	}

	/* High pass filter snew. */

	zerofilt(ahpf, 2, dhpf1, snew, ll);
	polefilt(bhpf, 2, dhpf2, snew, ll);
	if (celp_error) {
		return celp_error;
	}

	/* Make ssub vector from snew and sold. */

	for (i = 0; i < ll / 2; i++) {
		ssub[i] = sold[i + ll / 2];
		ssub[i + ll / 2] = snew[i];
	}

	autohf(snew, hamw, ll, no, OMEGA, fcn, rcn);

	/* Pc -> lsp (new). */

	pctolsp2(fcn, no, newfreq, &lspflag);
	if (lspflag) {
#ifdef CELPDIAG
		fprintf(stderr, "celp: Bad \"new\" lsp at frame: %d\n", frame);
		fprintf(stderr, "lsp: ");
		for (i = 0; i < no; i++) {
			fprintf(stderr, "%9.5f", newfreq[i]);
		}
		fprintf(stderr, "\npc: ");
		for (i = 0; i < no + 1; i++) {
			fprintf(stderr, "%9.5f", fcn[i]);
		}
		fprintf(stderr, "\nrc: ");
		for (i = 0; i < no; i++) {
			fprintf(stderr, "%9.5f", rcn[i]);
		}
		fprintf(stderr, "\n");
#endif
		return CELP_LSPERR;
	}

	/* Save unquantized lsp. */

	for (i = 0; i < no; i++) {
		unqfreq[i] = newfreq[i];
	}

	/* Quantize lsp's. */

	lsp34(newfreq, no, sbits, findex);

	/* Pack lsp indices in bit stream array. */

	for (i = 0; i < no; i++) {
		pack(findex[i], sbits[i], stream, &pointer);
	}

	/* Linearly interpolate LSP's for each subframe. */

	intanaly(newfreq, nn, lsp);

	/* For each subframe, search stochastic & adaptive code books. */

	k = 0;
	for (i = 0; i < nn; i++) {
		lsptopc(&lsp[i][0], fci);
		for (j = 0; j < no + 1; j++) {
			fc[j] = fci[j];
		}
		nseg++;

		/* Code book & pitch searches. */

		csub(&ssub[k], &v[k], l, lp);

		/* Pitch quantization tau. */

		/* Pack parameter indices in bit stream array. */

		if (((i + 1) % 2) != 0) {
			packtau(tauptr - minptr, pbits[0], pdencode, stream,
				&pointer);
		} else {
			pack(tauptr - minptr, pbits[1], stream, &pointer);
		}

		pack(pindex, pbits[2], stream, &pointer);
		cbindex--;
		pack(cbindex, cbbits, stream, &pointer);
		pack(gindex, cbgbits, stream, &pointer);

		/* Decode parameters for analysis by synthesis. */

		cbindex++;

		k += l;
	}

	/* Bit error protection
	   Extract bits to protect from stream array. */

#ifdef PROTECT
	if (protect) {
		for (i = 0; i < CODELENGTH2; i++) {
			codeword[i] = stream[bitprotect[i] - 1];
		}

		/* Hamming encode. */

		encodeham(CODELENGTH1, CODELENGTH2, hmatrix, &paritybit,
			  codeword);

		/* Pack future bit. */

		pack(0, 1, stream, &pointer);

		/* Pack parity bits. */

		for (i = 0; i < PARITYLENGTH; i++) {
			pack(codeword[CODELENGTH2 + i], 1, stream, &pointer);
		}

		/* Toggle and pack the sync bit. */

		syncBit = syncBit ^ 1;
		pack(syncBit, 1, stream, &pointer);
	}
#endif

	/*  At this time "stream" contains the CELP encoded bitstream.  The
	   stream array consists of one bit per int element.  */

	i2 = 0x80;
	i3 = 0;
	memzero(packedbits, STREAMBITS / 8);
	for (i1 = 0; i1 < STREAMBITS; i1++) {
		packedbits[i3] |= (stream[i1] ? i2 : 0);
		i2 >>= 1;
		if (i2 == 0) {
			i2 = 0x80;
			i3++;
		}
	}

	/* Shift new speech buffer into old speech buffer

	   sold                snew
	   |-------------------|-------------------| snew
	   |-------------------|
	   ssub                                      */

	for (i = 0; i < ll; i++) {
		sold[i] = snew[i];
	}
	return celp_error;
}

/*  CELP_DECODE  --  Decode a CELP-encoded frame into PCM audio.  */

int celp_decode(char packedbits[STREAMBITS / 8], short pf[MAXLL])
{
	int i1, i2, i3;

	i2 = 0x80;
	i3 = 0;
	for (i1 = 0; i1 < STREAMBITS; i1++) {
		stream[i1] = (packedbits[i3] & i2) ? 1 : 0;
		i2 >>= 1;
		if (i2 == 0) {
			i2 = 0x80;
			i3++;
		}
	}

	frame++;
	pointer = -1;
	celp_error = CELP_OK;

	/* Unpack parity bits. */

#ifdef PROTECT
	if (protect) {
		pointer = pointer - PARITYLENGTH - 2;

		pointer = 138;

		for (i = 0; i < PARITYLENGTH; i++) {
			unpack(stream, 1, &codeword[CODELENGTH2 + i], &pointer);
		}

		/* Extract code word from stream array. */

		for (i = 0; i < CODELENGTH2; i++) {
			codeword[i] = stream[bitprotect[i] - 1];
		}

		/* Repack Bisnu bit (remains constant for now). */

		codeword[10] = 0;

		/* Hamming decode. */

		decodeham(CODELENGTH1, hmatrix, syndrometable, paritybit,
			  codeword, &twoerror, &syndrome);

		/* Disable parity check (if parity not used). */
		twoerror = FALSE;

		/* Bit error rate estimator (running avg of bad syndromes). */

		if (syndrome != 0) {
			syndrome = 1;
		}
		syndavg =
		    (1.0 - (1.0 / SYNDRUN)) * syndavg +
		    (1.0 / SYNDRUN) * (float)syndrome;

		/* Repack protected bits. */

		for (i = 0; i < CODELENGTH2; i++) {
			stream[bitprotect[i] - 1] = codeword[i];
		}

		/* Frame repeat if two errors detected in code word. */

		if (twoerror) {
#ifdef CELPDIAG
			fprintf(stderr,
				"celp: two errors have occured in frame %d\n",
				frame);
#endif
			return CELP_TWOERR;
		}
	}
#endif				/* PROTECT */

	pointer = -1;

	/* Unpack data stream. */

	for (i = 0; i < no; i++) {
		unpack(stream, sbits[i], &findex[i], &pointer);
	}

	/* Decode lsp's. */

	lspdecode34(findex, no, newfreq);

	/* Interpolate spectrum lsp's for nn subframes. */

	intsynth(newfreq, nn, lsp, twoerror, syndavg);

	/* Decode all code book and pitch parameters. */

	bitpointer = pointer;
	dcodtau(pbits[0], pbits[1], bitsum1, bitsum2, &bitpointer, nn, stream,
		pddecode, pdtabi, taus);
	dcodpg(pbits[2], bitsum1, bitsum2, &bitpointer, nn, stream, pgs);
	if (celp_error) {
		return celp_error;
	}
	dcodcbi(cbbits, bitsum1, bitsum2, &bitpointer, nn, stream, cbi);
	if (celp_error) {
		return celp_error;
	}
	dcodcbg(cbgbits, bitsum1, bitsum2, &bitpointer, nn, stream, cbg);
	if (celp_error) {
		return celp_error;
	}

	/* *** synthesize each subframe                                      */

	nseg -= nn;

	k = 0;
	for (i = 0; i < nn; i++) {
		nseg++;

		/* Decode values for subframe. */

		cbindex = cbi[i];
		decodedgain = cbg[i];
#ifdef PROTECT
		if (protect) {
			smoothcbgain(&decodedgain, twoerror, syndavg, cbg,
				     i + 1);
		}
#endif

		/* Code book synthesis. */

		vdecode(decodedgain, l, &vdecoded[k]);

#ifdef PROTECT
		if (protect) {
			smoothtau(&taus[i], twoerror, syndavg, taus[2], i + 1);
		}
#endif
		bb[0] = taus[i];
		bb[2] = pgs[i];
#ifdef PROTECT
		if (protect) {
			smoothpgain(&bb[2], twoerror, syndavg, pgs, i + 1);
		}
#endif

		/* Pitch synthesis. */

		pitchvq(&vdecoded[k], l, dps, idb, bb, "long");

		/* Pitch pre-filter (synthesis). */

		if (prewt != 0.0) {
			prefilt(&vdecoded[k], l, dpps);
		}

		/* Convert lsp's to pc's. */

		lsptopc(&lsp[i][0], fci);

		/* LPC synthesis. */

		polefilt(fci, no, dss, &vdecoded[k], l);
		if (celp_error) {
			return celp_error;
		}

		/* Post-filtering */

		postfilt(&vdecoded[k], l, ALPHA, BETA, &ip, &op, dp1, dp2, dp3);

		/* Test for output speech clipping. */

		while (clip(&vdecoded[k], l)) {

			/* Frame repeat & recall synthesizer?
			   or scale vdecoded? */

#ifdef CELPDIAG
			fprintf(stderr, "celp: Clipping detected at frame %d\n",
				frame);
#endif
			for (j = 0; j < l; j++) {
				vdecoded[k + j] = 0.05 * vdecoded[k + j];
			}
		}

		/* Write postfiltered output speech disk files. */

		for (j = 0; j < l; j++) {
			pf[k + j] =
			    round(mmax
				  (-32768.0, mmin(32767.0, vdecoded[k + j])));
		}

		k += l;
	}
	return celp_error;
}

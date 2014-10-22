/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/**************************************************************************
*
* NAME	
*	pctolsp2
*
* FUNCTION
*
*	Compute LSP from predictor polynomial.
*
*	NOTE:  Much faster conversion can be performed
*	       if the LSP quantization is incorporated.
*
* SYNOPSIS
*
*	subroutine pctolsp2(a,m,freq,lspflag)
*
*   formal 
*			data	I/O
*	name		type	type	function
*	-------------------------------------------------------------------
*	a		float	i	a-polynomial a(0)=1
*	m		int	i	order of a
*	freq		float	o	lsp frequencies
*	lspflag 	int	o	ill-conditioned lsp test
*	N		int	na	grid points in search of zeros
*					of p-polynomials
*	EPS		float	na	precision for computing zeros
*	NB		int	na	iteration limit?
*
***************************************************************************
*	
* DESCRIPTION
*
*	Compute lsp frequencies by disection method as described in:
*	
*	Line Spectrum Pair (LSP) and Speech Data Compression,
*	F.K. Soong and B-H Juang,
*	Proc. ICASSP 84, pp. 1.10.1-1.10.4
*
*       CELP's LPC predictor coefficient convention is:
*	       p+1	   -(i-1)
*	A(z) = SUM   a	 z	    where a  = +1.0
*	       i=1    i 		   1
*
*	Peter uses n=128, eps=1.e-06, nb=15 (this appears to be overkill!)
*
***************************************************************************
*
* CALLED BY
*
*	celp
*
* CALLS
*
*
**************************************************************************/
#define MAXORD	24
#define N	128
#define NB	15
#define EPS	1.e-6

static void pctolsp2(float a[], int m, float freq[], int *lspflag)
{
	static float lastfreq[MAXORD];
	float p[MAXORD], q[MAXORD], ang, fm, tempfreq;
	float fr, pxr, tpxr, tfr, pxm, pxl, fl, qxl, tqxr;
	float qxm, qxr, tqxl;
	int mp, mh, nf, mb, jc, i, j;

	mp = m + 1;
	mh = m / 2;

	/* *generate p and q polynomials                                      */

	for (i = 0; i < mh; i++) {
		p[i] = a[i + 1] + a[m - i];
		q[i] = a[i + 1] - a[m - i];
	}

	/* *compute p at f=0.                                                 */

	fl = 0.;
	for (pxl = 1.0, j = 0; j < mh; j++)
		pxl += p[j];

	/* *search for zeros of p                                             */

	nf = 0;
	for (i = 1; i <= N; pxl = tpxr, fl = tfr, i++) {
		mb = 0;
		fr = i * (0.5 / N);
		pxr = cos(mp * CELP_PI * fr);
		for (j = 0; j < mh; j++) {
			jc = mp - (j + 1) * 2;
			ang = jc * CELP_PI * fr;
			pxr += cos(ang) * p[j];
		}
		tpxr = pxr;
		tfr = fr;
		if (pxl * pxr > 0.0)
			continue;

		do {
			mb++;
			fm = fl + (fr - fl) / (pxl - pxr) * pxl;
			pxm = cos(mp * CELP_PI * fm);

			for (j = 0; j < mh; j++) {
				jc = mp - (j + 1) * 2;
				ang = jc * CELP_PI * fm;
				pxm += cos(ang) * p[j];
			}
			(pxm * pxl > 0.0) ? (pxl = pxm, fl = fm) : (pxr =
								    pxm, fr =
								    fm);

		} while ((fabs(pxm) > EPS) && (mb < 4));

		if ((pxl - pxr) * pxl == 0) {
			for (j = 0; j < m; j++)
				freq[j] = (j + 1) * 0.04545;
#ifdef CELPDIAG
			fprintf(stderr,
				"pctolsp2: default lsps used, avoiding /0\n");
#endif
			return;
		}
		freq[nf] = fl + (fr - fl) / (pxl - pxr) * pxl;
		nf += 2;
		if (nf > m - 2)
			break;
	}

	/* *search for the zeros of q(z)                                      */

	freq[m] = 0.5;
	fl = freq[0];
	qxl = sin(mp * CELP_PI * fl);
	for (j = 0; j < mh; j++) {
		jc = mp - (j + 1) * 2;
		ang = jc * CELP_PI * fl;
		qxl += sin(ang) * q[j];
	}

	for (i = 2; i < mp; qxl = tqxr, fl = tfr, i += 2) {
		mb = 0;
		fr = freq[i];
		qxr = sin(mp * CELP_PI * fr);
		for (j = 0; j < mh; j++) {
			jc = mp - (j + 1) * 2;
			ang = jc * CELP_PI * fr;
			qxr += sin(ang) * q[j];
		}
		tqxl = qxl;
		tfr = fr;
		tqxr = qxr;

		do {
			mb++;
			fm = (fl + fr) * 0.5;
			qxm = sin(mp * CELP_PI * fm);

			for (j = 0; j < mh; j++) {
				jc = mp - (j + 1) * 2;
				ang = jc * CELP_PI * fm;
				qxm += sin(ang) * q[j];
			}
			(qxm * qxl > 0.0) ? (qxl = qxm, fl = fm) : (qxr =
								    qxm, fr =
								    fm);

		} while ((fabs(qxm) > EPS * tqxl) && (mb < NB));

		if ((qxl - qxr) * qxl == 0) {
			for (j = 0; j < m; j++)
				freq[j] = lastfreq[j];
#ifdef CELPDIAG
			fprintf(stderr,
				"pctolsp2: last lsps used, avoiding /0\n");
#endif
			return;
		}
		freq[i - 1] = fl + (fr - fl) / (qxl - qxr) * qxl;
	}

	/* *** ill-conditioned cases                                          */

	*lspflag = FALSE;
	if (freq[0] == 0.0 || freq[0] == 0.5)
		*lspflag = TRUE;
	for (i = 1; i < m; i++) {
		if (freq[i] == 0.0 || freq[i] == 0.5)
			*lspflag = TRUE;

		/* *reorder lsps if non-monotonic                                   */

		if (freq[i] < freq[i - 1]) {
			*lspflag = TRUE;
#ifdef CELPDIAG
			fprintf(stderr, "pctolsp2: non-monotonic lsps\n");
#endif
			tempfreq = freq[i];
			freq[i] = freq[i - 1];
			freq[i - 1] = tempfreq;
		}
	}

	/* *if non-monotonic after 1st pass, reset to last values             */

	for (i = 1; i < m; i++) {
		if (freq[i] < freq[i - 1]) {
#ifdef CELPDIAG
			fprintf(stderr,
				"pctolsp2: Reset to previous lsp values\n");
#endif
			for (j = 0; j < m; j++)
				freq[j] = lastfreq[j];
			break;
		}
	}
	for (i = 0; i < m; i++)
		lastfreq[i] = freq[i];
}

#undef MAXORD
#undef N
#undef NB
#undef EPS

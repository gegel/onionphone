/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/***********************************************************************
 *
 *  FUNCTION:  w_Az_lsp
 *
 *  PURPOSE:   Compute the LSPs from  the LP coefficients  (order=10)
 *
 *  DESCRIPTION:
 *    - The sum and difference filters are computed and divided by
 *      1+z^{-1}   and   1-z^{-1}, respectively.
 *
 *         f1[i] = a[i] + a[11-i] - f1[i-1] ;   i=1,...,5
 *         f2[i] = a[i] - a[11-i] + f2[i-1] ;   i=1,...,5
 *
 *    - The roots of F1(z) and F2(z) are found using Chebyshev polynomial
 *      evaluation. The polynomials are evaluated at 60 points regularly
 *      spaced in the frequency domain. The sign change interval is
 *      w_subdivided 4 times to better track the root.
 *      The LSPs are found in the cosine domain [1,-1].
 *
 *    - If less than 10 roots are found, the LSPs from the past frame are
 *      used.
 *
 ***********************************************************************/

#include <stdint.h>
#include "basic_op.h"
#include "oper_32b.h"
#include "cnst.h"

#include "grid.tab"

/* M = LPC order, NC = M/2 */

#define NC   M/2

/* local function */

static int16_t Chebps(int16_t x, int16_t f[], int16_t n);

void w_Az_lsp(int16_t a[],	/* (i)     : w_predictor coefficients                 */
	      int16_t lsp[],	/* (o)     : line spectral pairs                    */
	      int16_t old_lsp[]	/* (i)     : old lsp[] (in case not found 10 roots) */
    )
{
	int16_t i, j, nf, ip;
	int16_t xlow, ylow, xhigh, yhigh, xmid, ymid, xint;
	int16_t x, y, sign, exp;
	int16_t *coef;
	int16_t f1[M / 2 + 1], f2[M / 2 + 1];
	int32_t t0;

    /*-------------------------------------------------------------*
     *  find the sum and diff. pol. F1(z) and F2(z)                *
     *    F1(z) <--- F1(z)/(1+z**-1) & F2(z) <--- F2(z)/(1-z**-1)  *
     *                                                             *
     * f1[0] = 1.0;                                                *
     * f2[0] = 1.0;                                                *
     *                                                             *
     * for (i = 0; i< NC; i++)                                     *
     * {                                                           *
     *   f1[i+1] = a[i+1] + a[M-i] - f1[i] ;                       *
     *   f2[i+1] = a[i+1] - a[M-i] + f2[i] ;                       *
     * }                                                           *
     *-------------------------------------------------------------*/

	f1[0] = 1024;		/* f1[0] = 1.0 */
	f2[0] = 1024;		/* f2[0] = 1.0 */

	for (i = 0; i < NC; i++) {
		t0 = w_L_w_mult(a[i + 1], 8192);	/* x = (a[i+1] + a[M-i]) >> 2  */
		t0 = w_L_mac(t0, a[M - i], 8192);
		x = w_extract_h(t0);
		/* f1[i+1] = a[i+1] + a[M-i] - f1[i] */
		f1[i + 1] = w_sub(x, f1[i]);

		t0 = w_L_w_mult(a[i + 1], 8192);	/* x = (a[i+1] - a[M-i]) >> 2 */
		t0 = w_L_msu(t0, a[M - i], 8192);
		x = w_extract_h(t0);
		/* f2[i+1] = a[i+1] - a[M-i] + f2[i] */
		f2[i + 1] = w_add(x, f2[i]);
	}

    /*-------------------------------------------------------------*
     * find the LSPs using the Chebychev pol. evaluation           *
     *-------------------------------------------------------------*/

	nf = 0;			/* number of found frequencies */
	ip = 0;			/* indicator for f1 or f2      */

	coef = f1;

	xlow = w_grid[0];
	ylow = Chebps(xlow, coef, NC);

	j = 0;

	/* while ( (nf < M) && (j < w_grid_points) ) */
	while ((w_sub(nf, M) < 0) && (w_sub(j, w_grid_points) < 0)) {
		j++;
		xhigh = xlow;
		yhigh = ylow;
		xlow = w_grid[j];
		ylow = Chebps(xlow, coef, NC);

		if (w_L_w_mult(ylow, yhigh) <= (int32_t) 0L) {

			/* divide 4 times the interval */

			for (i = 0; i < 4; i++) {
				/* xmid = (xlow + xhigh)/2 */
				xmid = w_add(w_shr(xlow, 1), w_shr(xhigh, 1));
				ymid = Chebps(xmid, coef, NC);

				if (w_L_w_mult(ylow, ymid) <= (int32_t) 0L) {
					yhigh = ymid;
					xhigh = xmid;
				} else {
					ylow = ymid;
					xlow = xmid;
				}
			}

	    /*-------------------------------------------------------------*
             * Linear interpolation                                        *
             *    xint = xlow - ylow*(xhigh-xlow)/(yhigh-ylow);            *
             *-------------------------------------------------------------*/

			x = w_sub(xhigh, xlow);
			y = w_sub(yhigh, ylow);

			if (y == 0) {
				xint = xlow;
			} else {
				sign = y;
				y = w_abs_s(y);
				exp = w_norm_s(y);
				y = w_shl(y, exp);
				y = w_div_s((int16_t) 16383, y);
				t0 = w_L_w_mult(x, y);
				t0 = w_L_w_shr(t0, w_sub(20, exp));
				y = w_extract_l(t0);	/* y= (xhigh-xlow)/(yhigh-ylow) */

				if (sign < 0)
					y = w_negate(y);

				t0 = w_L_w_mult(ylow, y);
				t0 = w_L_w_shr(t0, 11);
				xint = w_sub(xlow, w_extract_l(t0));	/* xint = xlow - ylow*y */
			}

			lsp[nf] = xint;
			xlow = xint;
			nf++;

			if (ip == 0) {
				ip = 1;
				coef = f2;
			} else {
				ip = 0;
				coef = f1;
			}
			ylow = Chebps(xlow, coef, NC);

		}

	}

	/* Check if M roots found */

	if (w_sub(nf, M) < 0) {
		for (i = 0; i < M; i++) {
			lsp[i] = old_lsp[i];
		}

	}
	return;
}

/************************************************************************
 *
 *  FUNCTION:  Chebps
 *
 *  PURPOSE:   Evaluates the Chebyshev polynomial series
 *
 *  DESCRIPTION:
 *  - The polynomial order is   n = m/2 = 5
 *  - The polynomial F(z) (F1(z) or F2(z)) is given by
 *     F(w) = 2 exp(-j5w) C(x)
 *    where
 *      C(x) = T_n(x) + f(1)T_n-1(x) + ... +f(n-1)T_1(x) + f(n)/2
 *    and T_m(x) = cos(mw) is the mth order Chebyshev polynomial ( x=cos(w) )
 *  - The function returns the value of C(x) for the input x.
 *
 ***********************************************************************/

static int16_t Chebps(int16_t x, int16_t f[], int16_t n)
{
	int16_t i, cheb;
	int16_t b0_h, b0_l, b1_h, b1_l, b2_h, b2_l;
	int32_t t0;

	b2_h = 256;		/* b2 = 1.0 */
	b2_l = 0;

	t0 = w_L_w_mult(x, 512);	/* 2*x                 */
	t0 = w_L_mac(t0, f[1], 8192);	/* + f[1]              */
	w_L_Extract(t0, &b1_h, &b1_l);	/* b1 = 2*x + f[1]     */

	for (i = 2; i < n; i++) {
		t0 = w_w_Mpy_32_16(b1_h, b1_l, x);	/* t0 = 2.0*x*b1        */
		t0 = w_L_w_shl(t0, 1);
		t0 = w_L_mac(t0, b2_h, (int16_t) 0x8000);	/* t0 = 2.0*x*b1 - b2   */
		t0 = w_L_msu(t0, b2_l, 1);
		t0 = w_L_mac(t0, f[i], 8192);	/* t0 = 2.0*x*b1 - b2 + f[i] */

		w_L_Extract(t0, &b0_h, &b0_l);	/* b0 = 2.0*x*b1 - b2 + f[i] */

		b2_l = b1_l;	/* b2 = b1; */
		b2_h = b1_h;
		b1_l = b0_l;	/* b1 = b0; */
		b1_h = b0_h;
	}

	t0 = w_w_Mpy_32_16(b1_h, b1_l, x);	/* t0 = x*b1; */
	t0 = w_L_mac(t0, b2_h, (int16_t) 0x8000);	/* t0 = x*b1 - b2   */
	t0 = w_L_msu(t0, b2_l, 1);
	t0 = w_L_mac(t0, f[i], 4096);	/* t0 = x*b1 - b2 + f[i]/2 */

	t0 = w_L_w_shl(t0, 6);

	cheb = w_extract_h(t0);

	return (cheb);
}

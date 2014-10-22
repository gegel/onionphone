/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*************************************************************************
 *
 *  FUNCTION:   w_Lsp_Az
 *
 *  PURPOSE:  Converts from the line spectral pairs (LSP) to
 *            LP coefficients, for a 10th order filter.
 *
 *  DESCRIPTION:
 *     - Find the coefficients of F1(z) and F2(z) (see w_Get_lsp_pol)
 *     - Multiply F1(z) by 1+z^{-1} and F2(z) by 1-z^{-1}
 *     - A(z) = ( F1(z) + F2(z) ) / 2
 *
 *************************************************************************/

#include "ophint.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "count.h"
#include "sig_proc.h"

/* local function */

static void w_Get_lsp_pol(Word16 * lsp, Word32 * f);

void w_Lsp_Az(Word16 lsp[],	/* (i)  : line spectral frequencies            */
	      Word16 a[]	/* (o)  : w_predictor coefficients (order = 10)  */
    )
{
	Word16 i, j;
	Word32 f1[6], f2[6];
	Word32 t0;

	w_Get_lsp_pol(&lsp[0], f1);
	w_Get_lsp_pol(&lsp[1], f2);

	for (i = 5; i > 0; i--) {
		f1[i] = L_w_add(f1[i], f1[i - 1]);	/* f1[i] += f1[i-1]; */
		f2[i] = w_L_w_sub(f2[i], f2[i - 1]);	/* f2[i] -= f2[i-1]; */
	}

	a[0] = 4096;
	for (i = 1, j = 10; i <= 5; i++, j--) {
		t0 = L_w_add(f1[i], f2[i]);	/* f1[i] + f2[i] */
		a[i] = w_extract_l(w_w_L_w_w_shr_r(t0, 13));
		t0 = w_L_w_sub(f1[i], f2[i]);	/* f1[i] - f2[i] */
		a[j] = w_extract_l(w_w_L_w_w_shr_r(t0, 13));
	}

	return;
}

/*************************************************************************
 *
 *  FUNCTION:  w_Get_lsp_pol
 *
 *  PURPOSE:  Find the polynomial F1(z) or F2(z) from the LSPs.
 *            If the LSP vector is passed at w_address 0  F1(z) is computed
 *            and if it is passed at w_address 1  F2(z) is computed.
 *
 *  DESCRIPTION:
 *       This is performed by expanding the product polynomials:
 *
 *           F1(z) =   product   ( 1 - 2 lsp[i] z^-1 + z^-2 )
 *                   i=0,2,4,6,8
 *           F2(z) =   product   ( 1 - 2 lsp[i] z^-1 + z^-2 )
 *                   i=1,3,5,7,9
 *
 *       where lsp[] is the LSP vector in the cosine domain.
 *
 *       The expansion is performed using the following recursion:
 *
 *            f[0] = 1
 *            b = -2.0 * lsp[0]
 *            f[1] = b
 *            for i=2 to 5 do
 *               b = -2.0 * lsp[2*i-2];
 *               f[i] = b*f[i-1] + 2.0*f[i-2];
 *               for j=i-1 down to 2 do
 *                   f[j] = f[j] + b*f[j-1] + f[j-2];
 *               f[1] = f[1] + b;
 *
 *************************************************************************/

static void w_Get_lsp_pol(Word16 * lsp, Word32 * f)
{
	Word16 i, j, hi, lo;
	Word32 t0;

	/* f[0] = 1.0;             */
	*f = w_L_w_mult(4096, 2048);
	f++;
	*f = w_L_msu((Word32) 0, *lsp, 512);	/* f[1] =  -2.0 * lsp[0];  */
	f++;
	lsp += 2;		/* Advance lsp pointer     */

	for (i = 2; i <= 5; i++) {
		*f = f[-2];

		for (j = 1; j < i; j++, f--) {
			w_L_Extract(f[-1], &hi, &lo);
			t0 = w_w_Mpy_32_16(hi, lo, *lsp);	/* t0 = f[-1] * lsp    */
			t0 = w_L_w_shl(t0, 1);
			*f = L_w_add(*f, f[-2]);	/* *f += f[-2]      */
			*f = w_L_w_sub(*f, t0);	/* *f -= t0            */
		}
		*f = w_L_msu(*f, *lsp, 512);	/* *f -= lsp<<9     */
		f += i;		/* Advance f pointer   */
		lsp += 2;	/* Advance lsp pointer */
	}

	return;
}

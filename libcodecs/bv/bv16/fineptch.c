/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*****************************************************************************/
/* BroadVoice(R)16 (BV16) Fixed-Point ANSI-C Source Code                     */
/* Revision Date: November 13, 2009                                          */
/* Version 1.1                                                               */
/*****************************************************************************/

/*****************************************************************************/
/* Copyright 2000-2009 Broadcom Corporation                                  */
/*                                                                           */
/* This software is provided under the GNU Lesser General Public License,    */
/* version 2.1, as published by the Free Software Foundation ("LGPL").       */
/* This program is distributed in the hope that it will be useful, but       */
/* WITHOUT ANY SUPPORT OR WARRANTY; without even the implied warranty of     */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LGPL for     */
/* more details.  A copy of the LGPL is available at                         */
/* http://www.broadcom.com/licenses/LGPLv2.1.php,                            */
/* or by writing to the Free Software Foundation, Inc.,                      */
/* 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                 */
/*****************************************************************************/

/*****************************************************************************
  fineptch.c : Fine pitch search functions.

  $Log$
******************************************************************************/

#include <stdint.h>
#include "bvcommon.h"
#include "bv16cnst.h"
#include "bv16strct.h"
#include "basop32.h"

#define  DEV   (DECF-1)

int16_t refinepitch(int16_t * x,	/* (i) Q1 */
		   int16_t cpp, int16_t * ppt)
{				/* (o) Q9 */
	int32_t a0, a1;
	int32_t cor, energy, cormax, enermax32;	/* Q3 */
	int16_t energymax, energymax_exp, ener, ener_exp;
	int16_t cor2, cor2_exp, cor2max, cor2max_exp;
	int16_t *sp0, *sp1, *sp2, *sp3;
	int16_t *xt;
	int16_t s, t;
	int16_t lb, ub;
	int pp, i, j;

	if (cpp >= MAXPP)
		cpp = MAXPP - 1;
	if (cpp < MINPP)
		cpp = MINPP;
	lb = bv_sub((int16_t) cpp, DEV);
	if (lb < MINPP)
		lb = MINPP;	/* lower bound of pitch period search range */
	ub = bv_add((int16_t) cpp, DEV);
	/* to avoid selecting HMAXPP as the refined pitch period */
	if (ub >= MAXPP)
		ub = MAXPP - 1;	/* lower bound of pitch period search range */

	i = lb;			/* start the search from lower bound       */
	xt = x + XOFF;
	sp0 = xt;
	sp1 = xt - i;
	cor = energy = 0;
	for (j = 0; j < FRSZ; j++) {
		s = *sp1++;
		t = *sp0++;
		energy = bv_L_mac0(energy, s, s);
		cor = bv_L_mac0(cor, s, t);
	}

	pp = i;
	cormax = cor;
	enermax32 = energy;
	energymax_exp = bv_norm_l(enermax32);
	energymax = bv_extract_h(L_bv_shl(enermax32, energymax_exp));
	a0 = cor;
	cor2max_exp = bv_norm_l(a0);
	s = bv_extract_h(L_bv_shl(a0, cor2max_exp));
	cor2max_exp = bv_shl(cor2max_exp, 1);
	cor2max = bv_extract_h(L_bv_mult0(s, s));
	sp0 = xt + FRSZ - lb - 1;
	sp1 = xt - lb - 1;
	for (i = lb + 1; i <= ub; i++) {
		sp2 = xt;
		sp3 = xt - i;
		cor = 0;
		for (j = 0; j < FRSZ; j++)
			cor = bv_L_mac0(cor, *sp2++, *sp3++);

		a0 = cor;
		cor2_exp = bv_norm_l(a0);
		s = bv_extract_h(L_bv_shl(a0, cor2_exp));
		cor2_exp = bv_shl(cor2_exp, 1);
		cor2 = bv_extract_h(L_bv_mult0(s, s));

		s = *sp0--;
		t = *sp1--;
		energy = bv_L_msu0(energy, s, s);
		energy = bv_L_mac0(energy, t, t);
		a0 = energy;
		ener_exp = bv_norm_l(a0);
		ener = bv_extract_h(L_bv_shl(a0, ener_exp));

		if (ener > 0) {
			a0 = L_bv_mult0(cor2, energymax);
			a1 = L_bv_mult0(cor2max, ener);
			s = bv_add(cor2_exp, energymax_exp);
			t = bv_add(cor2max_exp, ener_exp);
			if (s >= t)
				a0 = L_bv_shr(a0, bv_sub(s, t));
			else
				a1 = L_bv_shr(a1, bv_sub(t, s));
			if (a0 > a1) {
				pp = i;
				cormax = cor;
				enermax32 = energy;
				cor2max = cor2;
				cor2max_exp = cor2_exp;
				energymax = ener;
				energymax_exp = ener_exp;
			}
		}
	}

	if ((enermax32 == 0) || (cormax <= 0))
		*ppt = 0;
	else {
		ub = bv_sub(bv_norm_l(cormax), 1);
		lb = bv_norm_l(enermax32);
		s = bv_extract_h(L_bv_shl(cormax, ub));
		t = bv_extract_h(L_bv_shl(enermax32, lb));
		s = bv_div_s(s, t);
		lb = bv_sub(bv_sub(lb, ub), 6);
		*ppt = bv_shl(s, lb);
	}
	return pp;
}

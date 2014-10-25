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
  allzero.c : Common Fixed-Point Library: all-zero filter

  $Log$
******************************************************************************/

#include <stdint.h>
#include "basop32.h"

void azfilter(int16_t a[],	/* (i) Q12 : prediction coefficients          */
	      int16_t m,		/* (i)     : LPC order                        */
	      int16_t x[],	/* (i) Q0  : input signal samples, incl. past */
	      int16_t y[],	/* (o) Q0  : filtered output signal           */
	      int16_t lg		/* (i)     : size of filtering                */
    )
{
	int16_t i, n;
	int32_t a0;
	int16_t *fp1;

	/* loop through every element of the current vector */
	for (n = 0; n < lg; n++) {

		/* perform bv_multiply-bv_adds along the delay line of filter */
		fp1 = x + n;
		a0 = L_bv_mult0(a[0], *fp1--);	// Q12
		for (i = 1; i <= m; i++)
			a0 = bv_L_mac0(a0, a[i], *fp1--);	// Q12

		/* get the output with rounding */
		y[n] = intround(L_bv_shl(a0, 4));	// Q0
	}

	return;
}

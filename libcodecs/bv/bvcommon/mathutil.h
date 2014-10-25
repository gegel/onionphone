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
  fixmath.h : Common Fixed-Point Library: 

  $Log$
******************************************************************************/

int32_t Pow2(int16_t int_comp,	/* Q0 Integer part      */
		   int16_t frac_comp	/* Q15 frac_compal part  */
    );

void Log2(int32_t x,		/* (i) input           */
	  int16_t * int_comp,	/* Q0 integer part     */
	  int16_t * frac_comp	/* Q15 fractional part */
    );

void sqrt_i(int16_t x_man, int16_t x_exp, int16_t * y_man, int16_t * y_exp);
int16_t sqrts(int16_t x);

extern int16_t tabsqrt[];
extern int16_t tablog[];
extern int16_t tabpow[];
extern int16_t costable[];
extern int16_t acosslope[];

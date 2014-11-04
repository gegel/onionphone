/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*******************************************************************
 *
 * typedef statements of types used in all half-rate GSM routines
 *
 ******************************************************************/

#ifndef __TYPEDEFS
#define __TYPEDEFS

#include <stdint.h>

#define DATE    "August 8, 1996    "
#define VERSION "Version 4.2       "

#define LW_SIGN (int32_t)0x80000000	/* sign bit */
#define LW_MIN (int32_t)0x80000000
#define LW_MAX (int32_t)0x7fffffff

#define SW_SIGN (int16_t)0x8000	/* sign bit for int16_t type */
#define SW_MIN (int16_t)0x8000	/* smallest Ram */
#define SW_MAX (int16_t)0x7fff	/* largest Ram */

/* Definition of Types *
 ***********************/

struct NormSw {			/* normalized int16_t fractional
				 * number snr.man precedes snr.sh (the
				 * shift count)i */
	int16_t man;		/* "mantissa" stored in 16 bit
				 * location */
	int16_t sh;		/* the shift count, stored in 16 bit
				 * location */
};

/* Global constants *
 ********************/

#define NP 10			/* order of the lpc filter */
#define N_SUB 4			/* number of subframes */
#define F_LEN 160		/* number of samples in a frame */
#define S_LEN 40		/* number of samples in a subframe */
#define A_LEN 170		/* LPC analysis length */
#define OS_FCTR 6		/* maximum LTP lag oversampling
				 * factor */

#define OVERHANG 8		/* vad parameter */
#define strStr strStr16

/* global variables */
/********************/

extern int giFrmCnt;		/* 0,1,2,3,4..... */
extern int giSfrmCnt;		/* 0,1,2,3 */

extern int giDTXon;		/* DTX Mode on/off */

#endif

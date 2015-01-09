/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*

$Log$
Revision 1.15  2004/06/26 03:50:14  markster
Merge source cleanups (bug #1911)

Revision 1.14  2003/02/12 13:59:15  matteo
mer feb 12 14:56:57 CET 2003

Revision 1.1.1.1  2003/02/12 13:59:15  matteo
mer feb 12 14:56:57 CET 2003

Revision 1.2  2000/01/05 08:20:39  markster
Some OSS fixes and a few lpc changes to make it actually work

 * Revision 1.2  1996/08/20  20:31:21  jaf
 * Removed all static local variables that were SAVE'd in the Fortran
 * code, and put them in struct lpc10_encoder_state that is passed as an
 * argument.
 *
 * Removed init function, since all initialization is now done in
 * init_lpc10_encoder_state().
 *
 * Changed name of function from lpcenc_ to lpc10_encode, simply to make
 * all lpc10 functions have more consistent naming with each other.
 *
 * Revision 1.1  1996/08/19  22:31:44  jaf
 * Initial revision
 *

*/

/*  -- translated by f2c (version 19951025).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "lpc10.h"

#ifdef P_R_O_T_O_T_Y_P_E_S
extern int lpcenc_(float *speech, int32_t * bits);
extern int initlpcenc_(void);
/*:ref: prepro_ 14 2 6 4 */
/*:ref: analys_ 14 5 6 4 4 6 6 */
/*:ref: encode_ 14 7 4 4 6 6 4 4 4 */
/*:ref: chanwr_ 14 5 4 4 4 4 4 */
/*:ref: initprepro_ 14 0 */
/*:ref: initanalys_ 14 0 */
#endif

/* Table of constant values */

static int32_t c__180 = 180;
static int32_t c__10 = 10;

/* ***************************************************************** */

/* $Log$
 * Revision 1.15  2004/06/26 03:50:14  markster
 * Merge source cleanups (bug #1911)
 *
 * Revision 1.14  2003/02/12 13:59:15  matteo
 * mer feb 12 14:56:57 CET 2003
 *
 * Revision 1.1.1.1  2003/02/12 13:59:15  matteo
 * mer feb 12 14:56:57 CET 2003
 *
 * Revision 1.2  2000/01/05 08:20:39  markster
 * Some OSS fixes and a few lpc changes to make it actually work
 *
 * Revision 1.2  1996/08/20  20:31:21  jaf
 * Removed all static local variables that were SAVE'd in the Fortran
 * code, and put them in struct lpc10_encoder_state that is passed as an
 * argument.
 *
 * Removed init function, since all initialization is now done in
 * init_lpc10_encoder_state().
 *
 * Changed name of function from lpcenc_ to lpc10_encode, simply to make
 * all lpc10 functions have more consistent naming with each other.
 *
 * Revision 1.1  1996/08/19  22:31:44  jaf
 * Initial revision
 * */
/* Revision 1.2  1996/03/28  00:01:22  jaf */
/* Commented out some trace statements. */

/* Revision 1.1  1996/03/28  00:00:27  jaf */
/* Initial revision */

/* ***************************************************************** */

/* Encode one frame of 180 speech samples to 54 bits. */

/* Input: */
/*  SPEECH - Speech encoded as float values in the range [-1,+1]. */
/*           Indices 1 through 180 read, and modified (by PREPRO). */
/* Output: */
/*  BITS   - 54 encoded bits, stored 1 per array element. */
/*           Indices 1 through 54 written. */

/* This subroutine maintains local state from one call to the next.  If */
/* you want to switch to using a new audio stream for this filter, or */
/* reinitialize its state for any other reason, call the ENTRY */
/* INITLPCENC. */

/* Subroutine */ int lpc10_encode(float *speech, int32_t * bits,
				  struct lpc10_encoder_state *st)
{
	int32_t irms, voice[2], pitch, ipitv;
	float rc[10];
	extern /* Subroutine */ int encode_(int32_t *, int32_t *, float *,
					    float *,
					    int32_t *, int32_t *, int32_t *),
	    chanwr_(int32_t *, int32_t *, int32_t *, int32_t *, int32_t *,
		    struct lpc10_encoder_state *), analys_(float *, int32_t *,
							   int32_t *, float *,
							   float *, struct
							   lpc10_encoder_state
							   *), prepro_(float *,
								       int32_t
								       *, struct
								       lpc10_encoder_state
								       *);
	int32_t irc[10];
	float rms;

/*       Arguments */
/* $Log$
 * Revision 1.15  2004/06/26 03:50:14  markster
 * Merge source cleanups (bug #1911)
 *
 * Revision 1.14  2003/02/12 13:59:15  matteo
 * mer feb 12 14:56:57 CET 2003
 * 
 * Revision 1.1.1.1  2003/02/12 13:59:15  matteo
 * mer feb 12 14:56:57 CET 2003
 *
 * Revision 1.2  2000/01/05 08:20:39  markster
 * Some OSS fixes and a few lpc changes to make it actually work
 *
 * Revision 1.2  1996/08/20  20:31:21  jaf
 * Removed all static local variables that were SAVE'd in the Fortran
 * code, and put them in struct lpc10_encoder_state that is passed as an
 * argument.
 *
 * Removed init function, since all initialization is now done in
 * init_lpc10_encoder_state().
 *
 * Changed name of function from lpcenc_ to lpc10_encode, simply to make
 * all lpc10 functions have more consistent naming with each other.
 *
 * Revision 1.1  1996/08/19  22:31:44  jaf
 * Initial revision
 * */
/* Revision 1.3  1996/03/29  22:03:47  jaf */
/* Removed definitions for any constants that were no longer used. */

/* Revision 1.2  1996/03/26  19:34:33  jaf */
/* Added comments indicating which constants are not needed in an */
/* application that uses the LPC-10 coder. */

/* Revision 1.1  1996/02/07  14:43:51  jaf */
/* Initial revision */

/*   LPC Configuration parameters: */
/* Frame size, Prediction order, Pitch period */
/*       Local variables that need not be saved */
/*       Uncoded speech parameters */
/*       Coded speech parameters */
/*       Local state */
/*       None */
	/* Parameter adjustments */
	if (speech) {
		--speech;
	}
	if (bits) {
		--bits;
	}

	/* Function Body */
	prepro_(&speech[1], &c__180, st);
	analys_(&speech[1], voice, &pitch, &rms, rc, st);
	encode_(voice, &pitch, &rms, rc, &ipitv, &irms, irc);
	chanwr_(&c__10, &ipitv, &irms, irc, &bits[1], st);
	return 0;
}				/* lpcenc_ */

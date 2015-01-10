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

 * Revision 1.1  1996/08/19  22:32:17  jaf
 * Initial revision
 *

*/

#include "energy.h"
#include "lpc10.h"

/* ********************************************************************* */

/* 	ENERGY Version 50 */

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
 * Revision 1.1  1996/08/19  22:32:17  jaf
 * Initial revision
 * */
/* Revision 1.3  1996/03/18  21:17:41  jaf */
/* Just added a few comments about which array indices of the arguments */
/* are used, and mentioning that this subroutine has no local state. */

/* Revision 1.2  1996/03/13  16:46:02  jaf */
/* Comments added explaining that none of the local variables of this */
/* subroutine need to be saved from one invocation to the next. */

/* Revision 1.1  1996/02/07 14:45:40  jaf */
/* Initial revision */

/* ********************************************************************* */

/* Compute RMS energy. */

/* Input: */
/*  LEN    - Length of speech buffer */
/*  SPEECH - Speech buffer */
/*           Indices 1 through LEN read. */
/* Output: */
/*  RMS    - Root Mean Square energy */

/* This subroutine has no local state. */

int lpc10_energy(int32_t * len, float *speech, float *rms)
{
	/* System generated locals */
	int32_t i__1;

	/* Builtin functions */
	double sqrt(double);

	/* Local variables */
	int32_t i__;

/*       Arguments */
/*       Local variables that need not be saved */
	/* Parameter adjustments */
	--speech;

	/* Function Body */
	*rms = 0.f;
	i__1 = *len;
	for (i__ = 1; i__ <= i__1; ++i__) {
		*rms += speech[i__] * speech[i__];
	}
	*rms = (float)sqrt(*rms / *len);
	return 0;
}				/* lpc10_energy */

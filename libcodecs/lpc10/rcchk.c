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

 * Revision 1.1  1996/08/19  22:30:41  jaf
 * Initial revision
 *

*/

#include <stdlib.h>

#include "lpc10.h"
#include "lpc10tools.h"
#include "rcchk.h"

/* ********************************************************************* */

/* 	RCCHK Version 45G */

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
 * Revision 1.1  1996/08/19  22:30:41  jaf
 * Initial revision
 * */
/* Revision 1.4  1996/03/27  18:13:47  jaf */
/* Commented out a call to subroutine ERROR. */

/* Revision 1.3  1996/03/18  15:48:53  jaf */
/* Just added a few comments about which array indices of the arguments */
/* are used, and mentioning that this subroutine has no local state. */

/* Revision 1.2  1996/03/13  16:55:22  jaf */
/* Comments added explaining that none of the local variables of this */
/* subroutine need to be saved from one invocation to the next. */

/* Revision 1.1  1996/02/07 14:49:08  jaf */
/* Initial revision */

/* ********************************************************************* */

/*  Check RC's, repeat previous frame's RC's if unstable */

/* Input: */
/*  ORDER - Number of RC's */
/*  RC1F  - Previous frame's RC's */
/*          Indices 1 through ORDER may be read. */
/* Input/Output: */
/*  RC2F  - Present frame's RC's */
/*          Indices 1 through ORDER may be read, and written. */

/* This subroutine has no local state. */

int lpc10_rcchk(int32_t * order, float *rc1f, float *rc2f)
{
	/* System generated locals */
	int32_t i__1;
	float r__1;

	/* Local variables */
	int32_t i__;

/*       Arguments */
/*       Local variables that need not be saved */
	/* Parameter adjustments */
	--rc2f;
	--rc1f;

	/* Function Body */
	i__1 = *order;
	for (i__ = 1; i__ <= i__1; ++i__) {
		if ((r__1 = rc2f[i__], abs(r__1)) > .99f) {
			goto L10;
		}
	}
	return 0;
/*       Note: In version embedded in other software, all calls to ERROR 
*/
/*       should probably be removed. */
 L10:

/*       This call to ERROR is only needed for debugging purposes. */

/*       CALL ERROR('RCCHK',2,I) */
	i__1 = *order;
	for (i__ = 1; i__ <= i__1; ++i__) {
		rc2f[i__] = rc1f[i__];
	}
	return 0;
}				/* lpc10_rcchk */

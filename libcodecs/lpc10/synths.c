/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/******************************************************************
*
*	SYNTHS Version 48
*
******************************************************************
*
*  NOTE: There is excessive buffering here, BSYNZ and DEEMP should be
*	 changed to operate on variable positions within SOUT.	Also,
*	 the output length parameter is bogus, and PITSYN should be
*	 rewritten to allow a constant frame length output
*/

#include "lpcdefs.h"
#include "contrl.ch"

extern int ipiti[11], ivuv[11];
extern float rci[MAXORD][11], rmsi[11], pc[MAXORD];
extern float exc[MAXPIT + MAXORD], exc2[MAXPIT + MAXORD],
    noise[MAXPIT + MAXORD];

void synths(int voice[], int *pitch, float *rms,
	    float rc[], float speech[], int *k)
{
	int i, j, nout;
	float ratio, g2pass;
	static float sout[MAXFRM];
	float gprime = 0.7f;

	*pitch = mmax(mmin(*pitch, 156), 20);
	for (i = 1; i <= ORDER; i++)
		rc[i - 1] = mmax(mmin(rc[i - 1], .99f), -.99f);

	*k = 0;
	pitsyn(voice, pitch, rms, rc, ivuv, ipiti, rmsi, rci, &nout, &ratio);

	if (nout > 0) {
		for (j = 0; j < nout; j++) {
			irc2pc(rci, pc, gprime, &g2pass, j);

			bsynz(pc, ipiti[j], ivuv[j], sout, rmsi[j], ratio,
			      g2pass);

			deemp0(sout, ipiti[j]);
			for (i = 1; i <= ipiti[j]; i++) {
				(*k)++;
				speech[*k - 1] = sout[i - 1] * 0.000244140625f;
			}
		}
	}

}

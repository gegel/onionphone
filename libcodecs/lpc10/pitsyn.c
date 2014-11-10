/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*******************************************************************
*
*	PITSYN Version 53
*
*******************************************************************
*
*   Synthesize a single pitch epoch
*
* Inputs:
*  ORDER  - Synthesis order (number of RC's)
*  VOICE  - Half frame voicing decisions
*  PITCH  - Pitch
*  RMS	  - Energy
*  RC	  - Reflection coefficients
*  LFRAME - Length of speech buffer
* Outputs:
*  IVUV   - Pitch epoch voicing decisions
*  IPITI  - Pitch epoch length
*  RMSI   - Pitch epoch energy
*  RCI    - Pitch epoch RC's
*  NOUT   - Number of pitch periods in this frame
*  RATIO  - Previous to present energy ratio
*/

#include "lpcdefs.h"
#include <stdlib.h>

void pitsyn(int voice[], int *pitch, float *rms, float rc[],
	    int ivuv[], int ipiti[], float rmsi[],
	    float rci[ORDER][11], int *nout, float *ratio)
{
	static float rco[MAXORD], yarc[MAXORD];
	int i, j, lsamp, ip, istart, ivoice;
	static int ivoico = 0, ipito = 0;
	int jused, nl;
	static int jsamp;
	float alrn, alro, prop;
	static float rmso = 0;
	float slope, uvpit, vflag, xxy;
	static short first = 1;

	if (*rms < 1)
		*rms = 1;
	if (rmso < 1)
		rmso = 1;
	uvpit = 0.0;
	*ratio = (float)(*rms / (rmso + 8.));

	if (first) {		/*1 */
		ivoice = voice[1];
		if (ivoice == 0) {
			*pitch = (int)(LFRAME * 0.25);
		}
		*nout = LFRAME / *pitch;
		jsamp = LFRAME - *nout * *pitch;
		for (i = 1; i <= *nout; i++) {
			for (j = 1; j <= ORDER; j++)
				rci[j - 1][i - 1] = rc[j - 1];
			ivuv[i - 1] = ivoice;
			ipiti[i - 1] = *pitch;
			rmsi[i - 1] = *rms;
		}
		first = 0;
	} /*1 */
	else {			/*1 */
		vflag = 0;
		lsamp = LFRAME + jsamp;
		*nout = 0;
		jused = 0;
		istart = 1;
		if ((voice[0] == ivoico) && (voice[1] == voice[0])) {	/*2 */
			if (voice[1] == 0) {
/* SSUV - -   0  ,  0  ,  0	*/
				*pitch = (int)(LFRAME * 0.25);
				ipito = *pitch;
				if (*ratio > 8)
					rmso = *rms;
			}
/* SSVC - -   1  ,  1  ,  1	*/
			slope = (*pitch - ipito) / (float)lsamp;
			ivoice = voice[1];
		} /*2 */
		else {		/*2 */
			if (ivoico != 1) {
				if (ivoico == voice[0])
/* UV2VC2 - -  0  ,  0	,  1	*/
					nl = (int)(lsamp - LFRAME * 0.25);
				else
/* UV2VC1 - -  0  ,  1	,  1	*/
					nl = (int)(lsamp - 3 * LFRAME * 0.25);

				ipiti[0] = (int)(nl * 0.5);
				ipiti[1] = nl - ipiti[0];
				ivuv[0] = 0;
				ivuv[1] = 0;
				rmsi[0] = rmso;
				rmsi[1] = rmso;
				for (i = 1; i <= ORDER; i++) {
					rci[i - 1][0] = rco[i - 1];
					rci[i - 1][1] = rco[i - 1];
					rco[i - 1] = rc[i - 1];
				}
				slope = 0;
				*nout = 2;
				ipito = *pitch;
				jused = nl;
				istart = nl + 1;
				ivoice = 1;
			} else {
				if (ivoico != voice[0])
/* VC2UV1 - -	1  ,  0  ,  0	*/
					lsamp = (int)(LFRAME * 0.25 + jsamp);
				else
/* VC2UV2 - -	1  ,  1  ,  0	*/
					lsamp =
					    (int)(3 * LFRAME * 0.25 + jsamp);

				for (i = 1; i <= ORDER; i++) {
					yarc[i - 1] = rc[i - 1];
					rc[i - 1] = rco[i - 1];
				}
				ivoice = 1;
				slope = 0.;
				vflag = 1;
			}
		}		/*2 */

		while (1) {	/*3 */
			for (i = istart; i <= lsamp; i++) {	/*4 */
				ip = (int)(ipito + slope * i + .5);
				if (uvpit != 0.0)
					ip = (int)uvpit;
				if (ip <= i - jused) {	/*5 */
					(*nout)++;
					if (*nout > 11) {
						exit(1);
					}
					ipiti[*nout - 1] = ip;
					*pitch = ip;
					ivuv[*nout - 1] = ivoice;
					jused += ip;
					prop =
					    (float)((jused -
						     ip * 0.5) / (float)lsamp);
					for (j = 1; j <= ORDER; j++) {
						alro =
						    (float)log((1 + rco[j - 1])
							       / (1 -
								  rco[j - 1]));
						alrn =
						    (float)log((1 + rc[j - 1]) /
							       (1 - rc[j - 1]));
						xxy =
						    alro + prop * (alrn - alro);
						xxy = (float)exp(xxy);
						rci[j - 1][*nout - 1] =
						    (xxy - 1) / (xxy + 1);
					}
					rmsi[*nout - 1] =
					    (float)(log(rmso) +
						    prop * (log(*rms) -
							    log(rmso)));
					rmsi[*nout - 1] = (float)exp(rmsi[*nout - 1]);
				}	/*5 */
			}	/*4 */
			if (vflag != 1)
				break;
			vflag = 0;
			istart = jused + 1;
			lsamp = LFRAME + jsamp;
			slope = 0;
			ivoice = 0;
			uvpit = (float)((lsamp - istart) * 0.5);
			if (uvpit > 90)
				uvpit *= 0.5;
			rmso = *rms;
			for (i = 1; i <= ORDER; i++) {
				rc[i - 1] = yarc[i - 1];
				rco[i - 1] = yarc[i - 1];
			}
		}		/*3 */
		jsamp = lsamp - jused;
	}			/*1 */
	if (*nout != 0) {
		ivoico = voice[1];
		ipito = *pitch;
		rmso = *rms;
		/*DO I = 1,ORDER */
		for (i = 1; i <= ORDER; i++)
			rco[i - 1] = rc[i - 1];
	}

}

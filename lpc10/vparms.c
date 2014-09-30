/**********************************************************************
*
*	VPARMS Version 50
*
**********************************************************************
*
*  Calculate voicing parameters:
*
* Inputs:
*  VWIN   - Voicing window limits
*  INBUF  - Input speech buffer
*  LPBUF  - Low pass filtered speech
*  BUFLIM - Array bounds for INBUF and LPBUF
*  HALF   - Half frame (1 or 2)
*  DITHER - Zero crossing threshold
*  MINTAU - Lag corresponding to minimum AMDF value (pitch estimate)
* Outputs:
*  ZC	  - Zero crossing rate
*  LBE	  - Low band energy (sum of magnitudes - SM)
*  FBE	  - Full band energy (SM)
*  QS	  - Ratio of 6 dB/oct preemphasized energy to full band energy
*  RC1	  - First reflection coefficient
*  AR_B   - Product of the causal forward and reverse pitch
*	    prediction gains
*  AR_F   - Product of the noncausal forward and reverse pitch
*	    prediction gains
* Internal:
*  OLDSGN - Previous sign of dithered signal
*  VLEN   - Length of voicing window
*  START  - Lower address of current half of voicing window
*  STOP   - Upper address of current half of voicing window
*  E_0	  - Energy of LPF speech (sum of squares - SS)
*  E_B	  - Energy of LPF speech backward one pitch period (SS)
*  E_F	  - Energy of LPF speech forward one pitch period (SS)
*  R_B	  - Autocovariance of LPF speech backward one pitch period
*  R_F	  - Autocovariance of LPF speech forward one pitch period
*  LP_RMS - Energy of LPF speech (sum of magnitudes - SM)
*  AP_RMS - Energy of all-pass speech (SM)
*  E_PRE  - Energy of 6dB preemphasized speech (SM)
*  E0AP   - Energy of all-pass speech (SS)
*/

#include "lpcdefs.h"

void vparms(int *vwin, float *inbuf, float *lpbuf, int half, float *dither,
    	    int mintau, int *zc, int *lbe, int *fbe, float *qs,
	    float *rc1, float *ar_b, float *ar_f )
{
int i, vlen, start, stop;
float oldsgn, e_0, e_b, r_b, lp_rms, ap_rms, e_pre, e0ap;
float e_f, r_f;
float sign;
float *ptr1, *ptr2;

			/******** VWIN(1) => vwin[0][2] ********
			 ******** VWIN(2) => vwin[1][2] ********/

/*   Calculate zero crossings (ZC) and several energy and correlation
*   measures on low band and full band speech.	Each measure is taken
*   over either the first or the second half of the voicing window,
*   depending on the variable HALF.	*/

lp_rms = 0.;
ap_rms = 0.;
e_pre = 0.;
e0ap = 0.;
*rc1 = 0.;
e_0 = 0.;
e_b = 0.;
e_f = 0.;
r_f = 0.;
r_b = 0.;
*zc = 0;

vlen = *(vwin+AF+2) - *(vwin+2) + 1;
start = *(vwin+2) + (half-1)*(vlen>>1) + 1;
stop = start + (vlen>>1) - 1;
oldsgn = (float) ((*(inbuf+start-1)-*dither < 0)?-1.:1.);
ptr1 = lpbuf+start;
ptr2 = inbuf+start;
for(i=start; i<= stop; i++)	{
	lp_rms += (float)fabs((double)(*ptr1));
	ap_rms += (float)fabs((double)*ptr2);
	e_pre += (float)fabs((double)(*ptr2-*(ptr2-1)));
	e0ap += *ptr2**ptr2;
	*rc1 += *ptr2**(ptr2-1);
	e_0 += *ptr1**ptr1;
	e_b += *(ptr1-mintau)**(ptr1-mintau);
	e_f += *(ptr1+mintau)**(ptr1+mintau);
	r_f += *ptr1**(ptr1+mintau);
	r_b += *ptr1**(ptr1-mintau);
	sign = (float) ((*ptr2+ *dither < 0)?-1.:1.);
	if( sign != oldsgn ) {
		*zc += 1;
		oldsgn = -oldsgn;
	}
	*dither = -*dither;
	ptr1++; ptr2++;
}

/*   Normalized short-term autocovariance coefficient at unit sample delay	*/

*rc1 /= (float) mmax(e0ap,1.);

/*   Ratio of the energy of the first difference signal (6 dB/oct preemphasis)
*   to the energy of the full band signal	*/

*qs = (float) (e_pre / mmax(2.*ap_rms,1.));

/*   aR_b is the product of the forward and reverse prediction gains,
*   looking backward in time (the causal case). */

/***** *ar_b = (r_b / mmax(e_b,1.)) * (r_b / mmax(e_0,1.)); *****/
*ar_b = (float) ((r_b * r_b) / (mmax(e_b,1.) * mmax(e_0,1.)));

/*   aR_f is the same as aR_b, but looking forward in time (non causal case).  */

*ar_f = (float) ((r_f / mmax(e_f,1.)) * (r_f / mmax(e_0,1.)));

/*   Normalize ZC, LBE, and FBE to old fixed window length of 180.
*   (The fraction 90/VLEN has a range of .58 to 1)		*/

/*****
*zc =	     L_nint( *zc*2     * (90./vlen) );
*lbe = mmin( L_nint( lp_rms*0.25 * (90./vlen) ), 32767 );
*fbe = mmin( L_nint( ap_rms*0.25 * (90./vlen) ), 32767 );
*****/
*zc = (int) (( *zc*2	 * (90./vlen) ) + .5);
*lbe = (int) mmin( ( lp_rms*0.25 * (90./vlen) )+.5, 32767 );
*fbe = (int) mmin( ( ap_rms*0.25 * (90./vlen) )+.5, 32767 );


}

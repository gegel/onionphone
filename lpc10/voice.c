/***************************************************************************   
*
*	VOICIN Version 52
*
***************************************************************************
*
*	 Voicing Detection (VOICIN) makes voicing decisions for each half
*   frame of input speech.  Tentative voicing decisions are made two frames
*   in the future (2F) for each half frame.  These decisions are carried
*   through one frame in the future (1F) to the present (P) frame where
*   they are examined and smoothed, resulting in the final voicing
*   decisions for each half frame. 
*	 The voicing parameter (signal measurement) column vector (VALUE)
*   is based on a rectangular window of speech samples determined by the
*   window placement algorithm.  The voicing parameter vector contains the
*   AMDF windowed maximum-to-minimum ratio, the zero crossing rate, energy
*   measures, reflection coefficients, and prediction gains.  The voicing
*   window is placed to avoid contamination of the voicing parameter vector
*   with speech onsets. 
*	 The input signal is then classified as unvoiced (including
*   silence) or voiced.  This decision is made by a linear discriminant
*   function consisting of a dot product of the voicing decision
*   coefficient (VDC) row vector with the measurement column vector
*   (VALUE).  The VDC vector is 2-dimensional, each row vector is optimized
*   for a particular signal-to-noise ratio (SNR).  So, before the dot
*   product is performed, the SNR is estimated to select the appropriate
*   VDC vector. 
*	 The smoothing algorithm is a modified median smoother.  The
*   voicing discriminant function is used by the smoother to determine how
*   strongly voiced or unvoiced a signal is.  The smoothing is further
*   modified if a speech onset and a voicing decision transition occur
*   within one half frame.  In this case, the voicing decision transition
*   is extended to the speech onset.  For transmission purposes, there are
*   constraints on the duration and transition of voicing decisions.  The
*   smoother takes these constraints into account. 
*	 Finally, the energy estimates are updated along with the dither
*   threshold used to calculate the zero crossing rate (ZC).
*
*  Inputs:
*   VWIN      - Voicing window limits
*   INBUF     - Input speech buffer
*   LPBUF     - Low-pass filtered speech buffer
*   BUFLIM    - INBUF and LPBUF limits
*   HALF      - Present analysis half frame number
*   MINAMD    - Minimum value of the AMDF
*   MAXAMD    - Maximum value of the AMDF
*   MINTAU    - Pointer to the lag of the minimum AMDF value
*   IVRC(2)   - Inverse filter's RC's
*   OBOUND    - Onset boundary descriptions
*   AF	      - The analysis frame number
*  Output:
*   VOIBUF(2,0:AF) - Buffer of voicing decisions
*  Internal:
*   QS	      - Ratio of preemphasized to full-band energies
*   RC1       - First reflection coefficient
*   AR_B      - Product of the causal forward and reverse pitch prediction gains
*   AR_F      - Product of the noncausal forward and rev. pitch prediction gains
*   ZC	      - Zero crossing rate
*   DITHER    - Zero crossing threshold level
*   MAXMIN    - AMDF's 1 octave windowed maximum-to-minimum ratio
*   MINPTR    - Location  of minimum AMDF value
*   NVDC      - Number of elements in each VDC vector
*   NVDCL     - Number of VDC vectors
*   VDCL      - SNR values corresponding to the set of VDC's
*   VDC       - 2-D voicing decision coefficient vector
*   VALUE(9)  - Voicing Parameters
*   VOICE(2,3)- History of LDA results
*   LBE       - Ratio of low-band instantaneous to average energies
*   FBE       - Ratio of full-band instantaneous to average energies
*   LBVE      - Low band voiced energy
*   LBUE      - Low band unvoiced energy
*   FBVE      - Full band voiced energy
*   FBUE      - Full band unvoiced energy
*   OFBUE     - Previous full-band unvoiced energy
*   OLBUE     - Previous low-band unvoiced energy
*   REF       - Reference energy for initialization and DITHER threshold
*   SNR       - Estimate of signal-to-noise ratio
*   SNR2      - Estimate of low-band signal-to-noise ratio
*   SNRL      - SNR level number
*   OT	      - Onset transition present
*   VSTATE    - Decimal interpretation of binary voicing classifications
*   FIRST     - First call flag
*/

#include <stdio.h>
#include "vcomm.ch"
#include "contrl.ch"
#include "lpcdefs.h"
#include <math.h>

void voicin(int vwin[2][AF], float *inbuf, float *lpbuf, int half, float minamd,
    	    float maxamd, int mintau, float ivrc[2], int *obound, int voibuf[2][AF+1])
{
int zc, lbe, fbe;
int i, snrl;
static int vstate=0;
static float dither=20;
static float snr;
float snr2;
static float maxmin;
float qs, rc1, ar_b;
float ar_f;
static float voice[2][3];
float value[9];
short ot=0;

/*   Declare and initialize filters:	*/

static int lbve, lbue, fbve, fbue, ofbue, olbue;
static int sfbue, slbue=0;
int ref= 3000;
static short first=1;


if (first) {
	lbve = ref;
	fbve = ref;
	fbue = ref/16;
	ofbue = ref/16;
	lbue = ref/32;
	olbue = ref/32;
	snr = (float) (64*(fbve/fbue));
	first = 0;
	vdcl[0] = 600;
	vdcl[1] = 450;
	vdcl[2] = 300;
	vdcl[3] = 200;
	vdcl[4] = 6*0;
	
	for(i=0;i<3;i++)	{
		voice[1][i] = 0.0;
		voice[0][i] = 0.0;
	}
	
}

/*   The VOICE array contains the result of the linear discriminant function 
*   (analog values).  The VOIBUF array contains the hard-limited binary 
*   voicing decisions.	The VOICE and VOIBUF arrays, according to FORTRAN 
*   memory allocation, are addressed as:
*
*	   (half-frame number, future-frame number)
*
*	   |   Past    |  Present  |  Future1  |  Future2  |
*	   | 1,0 | 2,0 | 1,1 | 2,1 | 1,2 | 2,2 | 1,3 | 2,3 |  --->  time
*
*   Update linear discriminant function history each frame:		*/

if (half == 1) {
	voice[0][0]=voice[0][1];
	voice[1][0]=voice[1][1];
	voice[0][1]=voice[0][2];
	voice[1][1]=voice[1][2];
	maxmin = (float) (maxamd/mmax(minamd,1.));
}

/*   Calculate voicing parameters twice per frame:	*/

vparms( (int *) vwin, inbuf, lpbuf, half, &dither, mintau, &zc, &lbe, &fbe, &qs, &rc1, &ar_b, &ar_f );

/*   Estimate signal-to-noise ratio to select the appropriate VDC vector.
*   The SNR is estimated as the running average of the ratio of the
*   running average full-band voiced energy to the running average
*   full-band unvoiced energy. SNR filter has gain of 63.	*/

snr = (float) L_nint( 63*( snr + fbve/(float)(mmax(fbue,1)) )/64.);
snr2 = (snr*fbue)/mmax(lbue,1);

/*   Quantize SNR to SNRL according to VDCL thresholds.*/

/*DO SNRL = 1, NVDCL-1 */
for (snrl=1;snrl<nvdcl;snrl++)	{
	if (snr2 > vdcl[snrl-1]) break;
}
/*	(Note:	SNRL = NVDCL Here)	*/

/*   Linear discriminant voicing parameters:	*/
value[0] = maxmin;
value[1] = (float)(lbe)/mmax(lbve,1);
value[2] = (float) zc;
value[3] = rc1;
value[4] = qs;
value[5] = ivrc[2];
value[6] = ar_b;
value[7] = ar_f;
value[8] = 0.0;

/*   Evaluation of linear discriminant function:	*/

voice[half-1][2] = vdc[9][snrl-1];

for(i=1;i<10;i++)	{
	voice[half-1][2] += vdc[i-1][snrl-1]*value[i-1];
}

/*   Classify as voiced if discriminant > 0, otherwise unvoiced
*   Voicing decision for current half-frame:  1 = Voiced; 0 = Unvoiced	*/

if (voice[half-1][2] > 0.0) 
	voibuf[half-1][3]=1;
else
	voibuf[half-1][3]=0;

/*   Skip voicing decision smoothing in first half-frame:	*/

if (half != 1) {
/*   Voicing decision smoothing rules (override of linear combination):
*
*	Unvoiced half-frames:  At least two in a row.
*	--------------------
*
*	Voiced half-frames:    At least two in a row in one frame.
*	-------------------    Otherwise at least three in a row.
*			       (Due to the way transition frames are encoded)
*
*	In many cases, the discriminant function determines how to smooth.
*	In the following chart, the decisions marked with a * may be overridden.
*
*   Voicing override of transitions at onsets:
*	If a V/UV or UV/V voicing decision transition occurs within one-half
*	frame of an onset bounding a voicing window, then the transition is
*	moved to occur at the onset.
*
*	P	1F
*	-----	-----
*	0   0	0   0
*	0   0	0*  1	(If there is an onset there)
*	0   0	1*  0*	(Based on 2F and discriminant distance)
*	0   0	1   1
*	0   1*	0   0	(Always)
*	0   1*	0*  1	(Based on discriminant distance)
*	0*  1	1   0*	(Based on past, 2F, and discriminant distance)
*	0   1*	1   1	(If there is an onset there)
*	1   0*	0   0	(If there is an onset there)
*	1   0	0   1
*	1   0*	1*  0	(Based on discriminant distance)
*	1   0*	1   1	(Always)
*	1   1	0   0
*	1   1	0*  1*	(Based on 2F and discriminant distance)
*	1   1	1*  0	(If there is an onset there)
*	1   1	1   1
*
*   Determine if there is an onset transition between P and 1F.
*   OT (Onset Transition) is true if there is an onset between 
*   P and 1F but not after 1F.
*/

/*OT = (AND(OBOUND(1), 2) .NE. 0 .OR. OBOUND(2) .EQ. 1) .AND. AND(OBOUND(3), 1) .EQ. 0 */
ot = ((obound[1] & 2) != 0 || obound[2] == 1) && (obound[3] & 1) == 0;

/*   Multi-way dispatch on voicing decision history:	*/

vstate = voibuf[0][1]*8 + voibuf[1][1]*4 + voibuf[0][2]*2 + voibuf[1][2];
/*	GOTO (99,1,2,99,4,5,6,7,8,99,10,11,99,13,14,99) VSTATE+1	*/

/*if(count==9) printf("vstate = %d\n",vstate);*/

switch(vstate+1)	{
	case 1:
		break;
	case 2:
		if (ot && voibuf[0][3] == 1) voibuf[0][2] = 1;
		break;
	case 3:
		if (voibuf[0][3] == 0 || voice[0][1] < -voice[1][1]) 
			voibuf[0][2] = 0;
		else
			voibuf[1][2] = 1;
		break;
	case 4:
		break;
	case 5:
		voibuf[1][1] = 0;
		break;
	case 6:
		if (voice[1][0] < -voice[0][1]) 
			voibuf[1][1] = 0;
		else
			voibuf[0][2] = 1;
		break;
	case 7:
	/*   VOIBUF(2,0) must be 0	*/
		if (voibuf[0][0] == 1 || voibuf[0][3] == 1 || voice[1][1] > voice[0][0]) 
			voibuf[1][2] = 1;
		else
			voibuf[0][1] = 1;
		break;
	case 8:
		if (ot) voibuf[1][1] = 0;
		break;
	case 9:
		if (ot) voibuf[1][1] = 1;
		break;
	case 10:
		break;
	case 11:
		if (voice[0][1] <  -voice[1][0]) 
			voibuf[0][2] = 0;
		else
			voibuf[1][1] = 1;
		break;
	case 12:
		voibuf[1][1] = 1;
		break;
	case 13:
		break;
	case 14:
		if ((voibuf[0][3] == 0) && (voice[1][1] < -voice[0][1]) )
			voibuf[1][2] = 0;
		else
			voibuf[0][2] = 1;
		break;
	case 15:
		if (ot && voibuf[0][3] == 0) voibuf[0][2] = 0;
		break;
	default:
		break;
}
} /* (99)*/

/*   Now update parameters:
*   ----------------------
*
*   During unvoiced half-frames, update the low band and full band unvoiced
*   energy estimates (LBUE and FBUE) and also the zero crossing
*   threshold (DITHER).  (The input to the unvoiced energy filters is
*   restricted to be less than 10dB above the previous inputs of the
*   filters.)
*   During voiced half-frames, update the low-pass (LBVE) and all-pass 
*   (FBVE) voiced energy estimates.					*/

if (voibuf[half-1][3] == 0) {
	sfbue = L_nint(( 63*sfbue + 8*mmin(fbe,3*ofbue) )/64.);
	fbue = sfbue/8;
	ofbue = fbe;
	slbue = L_nint(( 63*slbue + 8*mmin(lbe,3*olbue) )/64.);
	lbue = slbue/8;
		
	olbue = lbe;
}
else{
	lbve = L_nint(( 63*lbve + lbe )/64.);
	fbve = L_nint(( 63*fbve + fbe )/64.);
}

/*   Set dither threshold to yield proper zero crossing rates in the
*   presence of low frequency noise and low level signal input.
*   NOTE: The divisor is a function of REF, the expected energies.	*/

dither = (float) mmin(mmax( 64*sqrt((float)(lbue*lbve)) / ref,1.),20.);

/*   Voicing decisions are returned in VOIBUF.	*/

}


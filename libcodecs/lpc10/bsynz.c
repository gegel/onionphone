/******************************************************************
*
*	BSYNZ Version 49
*
******************************************************************
*
*   Synthesize One Pitch Epoch
*
* Inputs:
*  COEF  - Predictor coefficients
*  IP	 - Pitch period (number of samples to synthesize)
*  IV	 - Voicing for the current epoch
*  RMS	 - Energy for the current epoch
*  ORDER - Synthesizer filter order (number of PC's)
*  RATIO - Energy slope for plosives
*  G2PASS- Sharpening factor for 2 pass synthesis
* Outputs:
*  SOUT  - Synthesized speech
*/

#include "lpcdefs.h"
#include <math.h>

/*
#define MESCL 1.0
#define PESCL 1.0
*/
float kexc[25]={
8,-16,26,-48,86,-162,294,-502,718,-728,
	      184,672,-610,-672,184,728,718,502,294,162,
	      86,48,26,16,8
};

extern float exc[MAXPIT+MAXORD], exc2[MAXPIT+MAXORD];

void bsynz(float coef[], int ip, int iv, float sout[],
    	   float rms, float ratio, float g2pass )
{
int px;
static int ipo=0;
int i, j, k;
float noise[MAXPIT+MAXORD];
float lpi0, hpi0;
float a0=.125, a1=.75, a2=.125/*, a3=0*/, b0=-.125, b1=.25, b2=-.125/*, b3=0*/;
float pulse, sscale, xssq, sum, ssq, gain;
float xy;
static float rmso=0.0, lpi1=0.0, lpi2=0.0, /*lpi3,*/ hpi1=0.0, hpi2=0.0/*, hpi3*/;


/*  Calculate history scale factor XY and scale filter state	*/

xy = (float) mmin((rmso/(rms+.000001)), 8.0);
rmso = rms;
for(i=0;i<ORDER;i++)
	   exc2[i] = exc2[ipo+i]*xy;

ipo = ip;

if(iv==0) {

/*  Generate white noise for unvoiced	*/

	for(i=0;i<ip;i++)
		/*exc[ORDER+i] = (int)(Rrandom() * 0.015625);*/
		exc[ORDER+i] = (float) (Rrandom() >>6);
	
/*  Impulse doublet excitation for plosives	*/

	px = ((Rrandom()+32768)*(ip-1)>>16) + ORDER + 1;
	/*pulse = PESCL*(ratio*.25)*342;*/
	pulse = (float) (ratio*85.5);
	if(pulse>2000) pulse = 2000;
	exc[px-1]   += pulse;
	exc[px] -= pulse;

/*  Load voiced excitation	*/
}
else	{
	/*sscale = sqrt((float)ip)/6.928;*/
	sscale = (float) (sqrt((float)ip)*0.144341801);
	for(i=0;i<ip;i++)	{
		exc[ORDER+i] = 0.;
		if(i<=25) exc[ORDER+i] = sscale*kexc[i];
		lpi0 = exc[ORDER+i];
		exc[ORDER+i] = a0*exc[ORDER+i] + a1*lpi1 + a2*lpi2 /*+ a3*lpi3*/;
		/*lpi3 = lpi2;*/
		lpi2 = lpi1;
		lpi1 = lpi0;
	}
	for(i=0;i<ip;i++)	{
		/*noise[ORDER+i] = MESCL * (int)(Rrandom() * 0.015625);*/
		noise[ORDER+i] = (float) (Rrandom() >>6);
		hpi0 = noise[ORDER+i];
		noise[ORDER+i] = b0*noise[ORDER+i] + b1*hpi1 + b2*hpi2 /*+ b3*hpi3*/;
		/*hpi3 = hpi2; */
		hpi2 = hpi1;
		hpi1 = hpi0;
	}
	for(i=0;i<ip;i++)
		exc[ORDER+i] += noise[ORDER+i];
	
}

/*   Synthesis filters:
*    Modify the excitation with all-zero filter  1 + G*SUM	*/

xssq = 0;
for(i=0;i<ip;i++)	{
	k = ORDER + i;
	sum = 0.;
	for(j=0;j<ORDER;j++)
		sum += coef[j]*exc[k-j-1];
	sum *= g2pass;
	exc2[k] = sum + exc[k];
}

/*   Synthesize using the all pole filter  1 / (1 - SUM)	*/

for(i=0;i<ip;i++)	{
	k = ORDER + i;
	sum = 0.;
	for(j=0;j<ORDER;j++)
		sum += coef[j]*exc2[k-j-1];
	exc2[k] += sum;
	xssq = xssq + exc2[k]*exc2[k];
}

/*  Save filter history for next epoch	*/

for(i=0;i<ORDER;i++)	{
		exc[i] = exc[ip+i];
		exc2[i] = exc2[ip+i];
}

/*  Apply gain to match RMS	*/

ssq = rms*rms*ip;
gain = (float) sqrt(ssq/xssq);
for(i=0;i<ip;i++)
		sout[i] = gain*exc2[ORDER+i];


}

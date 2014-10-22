/******************************************************************
*
*	ENCODE Version 48
*
******************************************************************
*
*  Quantize LPC parameters for transmission
*
* INPUTS:
*  ORDER  - Number of RC's
*  VOICE  - Half frame voicing decisions
*  PITCH  - Pitch
*  RMS	  - Energy
*  RC	  - Reflection coefficients
*  CORRP  - Error Correction: TRUE = yes, FALSE = none
* OUTPUTS:
*  IPITCH - Coded pitch and voicing
*  IRMS   - Quantized energy
*  IRC	  - Quantized reflection coefficients
*
*/

#include "lpcdefs.h"
#include <math.h>

int enctab[16]={0,7,11,12,13,10,6,1,14,9,5,2,3,4,8,15};
int entau[60]={19,11,27,25,29,21,23,22,30,14,15,7,39,
	38,46,42,43,41,45,37,53,49,51,50,54,52,
	60,56,58,26,90,88,92,84,86,82,83,81,85,
	69,77,73,75,74,78,70,71,67,99,97,113,112,
	114,98,106,104,108,100,101,76};
int enadd[8]={1920,-768,2432,1280,3584,1536,2816,-1152};
float enscl[8]={.0204f,.0167f,.0145f,.0147f,.0143f,.0135f,.0125f,.0112f};
int enbits[8]={6,5,4,4,4,4,3,3};
int entab6[64]={0,0,0,0,0,0,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,3,3,3,3,3,3,3,4,4,4,4,4,4,4,
	5,5,5,5,5,6,6,6,6,6,7,7,7,7,7,8,8,8,8,
	9,9,9,10,10,11,11,12,13,14,15};
int rmst[64]={1024,936,856,784,718,656,600,550,
	502,460,420,384,352,328,294,270,
	246,226,206,188,172,158,144,132,
	120,110,102,92,84,78,70,64,
	60,54,50,46,42,38,34,32,
	30,26,24,22,20,18,17,16,
	15,14,13,12,11,10,9,8,
	7,6,5,4,3,2,1,0};


void encode(int voice[2], int *pitch, float *rms, float rc[ORDER],
    	    int *ipitch, int *irms, int irc[ORDER])
{
int i, j, i2, i3, mrk, nbit, idel;
float ftemp;
int itemp;

/*  Scale RMS and RC's to integers */

*irms = (int) (*rms);

for(i=1;i<=ORDER;i++)
	irc[i] = (int) (rc[i] * 32768);

	
/*  Encode pitch and voicing	*/

if(voice[1]!=0&&voice[2]!=0) 
	*ipitch = entau[*pitch-1];
else
	*ipitch = 0;
	if(voice[1]!=voice[2]) *ipitch = 127;

/*  Encode RMS by binary table search	*/

j = 32;
idel = 16;
*irms = mmin(*irms,1023);
while(idel>0)	{
	if (*irms>rmst[j-1]) j -= idel;
	if (*irms<rmst[j-1]) j += idel;
	idel = (int) (idel * 0.5);
}
if (*irms>rmst[j-1]) j--;
*irms = (int) (31 - j*0.5);

/*  Encode RC(1) and (2) as log-area-ratios	*/

for(i=1;i<=2;i++)	{
	i2 = irc[i];
	mrk = 0;
	if(i2<0) {
		i2 = -i2 ;
		mrk = 1;
	}
	i2 = i2>>9;
	i2 = mmin(i2,63);
	i2 = entab6[i2];
	if(mrk!=0) i2 = -i2;
	irc[i] = i2;
}

/*  Encode RC(3) - (10) linearly, remove bias then scale	*/

for(i=3;i<=ORDER;i++)	{
	i2 = irc[i]>>1;
/*	i2 = (i2+enadd[ORDER-i])*enscl[ORDER-i]; */
	/* problem with truncating negative numbers */
	if(enadd[ORDER-i] < 0) { 
	  ftemp = -(i2+enadd[ORDER-i])*enscl[ORDER-i];
	  itemp = (int) ftemp;
	  i2 = -itemp;
	}
	else
	  i2 = (int) ((i2+enadd[ORDER-i])*enscl[ORDER-i]);

/*****	i2 = mmin(mmax(i2,-127),127);	*****/

	if( (i2 < -127) || (i2 > 127)) {
	  if(i2 < -127) {
	    i2 = -127;
	  } else {
	    if (i2 > 127) {
	      i2 = 127;
	    }
	  }
	}

	nbit = enbits[ORDER-i];
	i3 = 0;
	if(i2<0) i3 = -1;
	/*i2 = i2/pow(2.,(float)nbit);*/
	i2 = i2 / (2 << (nbit-1));
	if(i3==-1) i2--;
	irc[i] = i2;
}

/*	    Protect the most significant bits of the most
*     important parameters during non-voiced frames.
*     RC(1) - RC(4) are protected using 20 parity bits
*     replacing RC(5) - RC(10). */

if(*ipitch==0||*ipitch==127) {
	irc[5] = enctab[(irc[1]&30)>>1];
	irc[6] = enctab[(irc[2]&30)>>1];
	irc[7] = enctab[(irc[3]&30)>>1];
	irc[8] = enctab[(*irms&30)>>1];
	irc[9] = enctab[(irc[4]&30)>>1]>>1;
	irc[10]= enctab[(irc[4]&30)>>1]&1;
}

}

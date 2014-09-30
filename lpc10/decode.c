/******************************************************************
*
*	DECODE Version 52
*
******************************************************************
*
*   This subroutine provides error correction and decoding
*   for all LPC parameters
*
* INPUTS:
*  ORDER  - Number of RC's
*  IPITV  - Index value of pitch
*  IRMS   - Coded Energy
*  IRC	  - Coded Reflection Coefficients
*  CORRP  - Error correction:
*    If FALSE, parameters are decoded directly with no delay.  If TRUE,
*    most important parameter bits are protected by Hamming code and
*    median smoothed.  This requires an additional frame of delay.
* OUTPUTS:
*  VOICE  - Half frame voicing decisions
*  PITCH  - Decoded pitch
*  RMS	  - Energy
*  RC	  - Reflection coefficients
*
*  NOTE: Zero RC's should be done more directly, but this would affect
*   coded parameter printout.
*/


#include "lpcdefs.h"
#include <math.h>
#include <stdlib.h>

/*int rmst[64]={1024,936,856,784,718,656,600,550,
	502,460,420,384,352,328,294,270,
	246,226,206,188,172,158,144,132,
	120,110,102,92,84,78,70,64,
	60,54,50,46,42,38,34,32,
	30,26,24,22,20,18,17,16,
	15,14,13,12,11,10,9,8,
	7,6,5,4,3,2,1,0};
**** -- this is defined in encode.c also  */
extern int rmst[64];

int ivtab[32] ={
 24960,24960,24960,24960, 25480,25480, 25483, 25480,
       16640, 1560,1560,1560, 16640, 1816, 1563,
       1560, 24960,24960, 24859, 24856, 26001,
       25881, 25915, 25913, 1560,1560, 7800,
       3640, 1561,1561, 3643, 3641 
};
float corth[4][8]={
  { 32767.0f, 32767.0f, 32.0f, 32.0f, 32.0f, 32.0f, 16.0f, 16.0f },
  { 10.0f, 8.0f, 6.4f, 6.4f, 11.2f, 11.2f, 5.6f, 5.6f },
  { 5.0f, 4.0f, 3.2f, 3.2f, 6.4f, 6.4f, 3.2f, 3.2f },
  { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }
};
int detau[128]={
 0,0,0,3,0,3,3,31, 0,3,3,21,3,3,29,30,
	   0,3,3,20,3,25,27,26, 3,23,58,22,3,24,28,3,
	0,3,3,3,3,39,33,32, 3,37,35,36,3,38,34,3,
	   3,42,46,44,50,40,48,3, 54,3,56,3,52,3,3,1,
	0,3,3,108,3,78,100,104, 3,84,92,88,156,80,96,3,
	3,74,70,72,66,76,68,3, 62,3,60,3,64,3,3,1,
	3,116,132,112,148,152,3,3, 140,3,136,3,144,3,3,1,
	   124,120,128,3,3,3,3,1, 3,3,3,1,3,1,1,1
};
int detab7[32]={
4,11,18,25,32,39,46,53,60,66,72,77,82,87,92,96,101,
	104,108,111,114,115,117,119,121,122,123,124,125,126,
	127,127
};
float descl[8]={.6953f,.6250f,.5781f,.5469f,.5312f,.5391f,.4688f,.3828f};
int deadd[8]={1152,-2816,-1536,-3584,-1280,-2432,768,-1920};
int qb[8]={511,511,1023,1023,1023,1023,2047,4095};
int nbit[10]={8,8,5,5,4,4,4,4,3,2};
int zrc[MAXORD]={0,0,0,0,0,3,0,2,0,0};
int abit[5]={2,4,8,16,32};

extern int drc[3][MAXORD], dpit[3], drms[3];

void decode(int ipitv, int *irms, int irc[MAXORD],
    	    int voice[2], int *pitch, float *rms, float rc[ORDER])
{
int ivoic;
static int ivp2h=0, erate=0, iovoic=0;
int i, i1, i2, i4, iavgp=60, icorf,index, iout;
int ipit, ishift, ixcor, lsb;
int errcnt;
int ethrs=2048, ethrs1=128, ethrs2=1024, ethrs3=2048;
int fut=0, pres=1, past=2;
float ftemp;
int itemp;
static short first=1;

/*  If no error correction, do pitch and voicing then jump to decode	*/

i4 = detau[ipitv];

/*  Do error correction pitch and voicing	*/

	if(i4>4) {
		dpit[fut] = i4;
		ivoic = 2;
		iavgp = (int) ((15*iavgp+i4+8)*0.0625);
	}
	else	{
		ivoic = i4;
		dpit[fut] = iavgp;
	}
	drms[fut] = *irms;

	for(i=1;i<=ORDER;i++)
		drc[fut][i-1] = irc[i];

/*  Determine index to IVTAB from V/UV decision
*  If error rate is high then use alternate table	*/

	index = 16*ivp2h + 4*iovoic + ivoic + 1;
	i1 = ivtab[index-1];
	ipit = i1&3;
	icorf = (int) (i1*0.125);
	if(erate<ethrs) icorf = (int) (icorf * 0.015625);

/*  Determine error rate:  4=high    1=low	*/

	ixcor = 4;
	if(erate<ethrs3) ixcor = 3;
	if(erate<ethrs2) ixcor = 2;
	if(erate<ethrs1) ixcor = 1;

/*  Voice/unvoice decision determined from bits 0 and 1 of IVTAB	*/

	voice[1] = (int)(icorf*0.5)&1;
	voice[2] = icorf&1;

/*  Skip decoding on first frame because present data not yet available */

	if(first) 
	   first = 0;
	else {

/*  If bit 4 of ICORF is set then correct RMS and RC(1) - RC(4).
*    Determine error rate and correct errors using a Hamming 8,4 code
*    during transition or unvoiced frame.  If IOUT is negative,
*    more than 1 error occurred, use previous frame's parameters.       */

		if((icorf&abit[3])!=0) {
			errcnt = 0;
			lsb = drms[pres]&1;
			index = (int) (drc[pres][7]*16 + drms[pres] * 0.5);
			ham84(index,&iout,&errcnt);
			drms[pres] = drms[past];
			if(iout>=0) drms[pres] = iout*2 + lsb;

			for(i=1;i<=4;i++)	{
				if(i==1) 
					i1= (drc[pres][8]&7)*2+(drc[pres][9] &1);
				else
					i1  = drc[pres][8-i]&15;
				
				i2 = drc[pres][4-i]&31;
				lsb = i2&1;
				index = (int) (16*i1 + i2*0.5);
				ham84(index,&iout,&errcnt);
				if(iout>=0) {
					iout = iout*2+lsb;
					if((iout&16)==16) 
						iout = iout-32;
				}
				else
					iout = drc[past][4-i];
				drc[pres][4-i] = iout;
			}

/*  Determine error rate	*/
			erate = (int) (erate*.96875 + errcnt*102);
		}

/*  Get unsmoothed RMS, RC's, and PITCH */

	*irms = drms[pres];

	for(i=1;i<=ORDER;i++)
	   irc[i]= drc[pres][i-1];
	if(ipit==1) dpit[pres] = dpit[past];
	if(ipit==3) dpit[pres] = dpit[fut];
	*pitch = dpit[pres];

/*  If bit 2 of ICORF is set then smooth RMS and RC's,  */

	if ((icorf & abit[1]) != 0) {
	   if(	abs(drms[pres]-drms[fut]) >= corth[ixcor-1][1]
	      && abs(drms[pres]-drms[past]) >= corth[ixcor-1][1]  )
		*irms = median( drms[past], drms[pres], drms[fut] );

	   for(i=1;i<=6;i++)	
		if(  abs(drc[pres][i-1]-drc[fut][i-1]) >= corth[ixcor-1][i+1]
	   &&abs(drc[pres][i-1]-drc[past][i-1])>= corth[ixcor-1][i+1])
			irc[i]= median( drc[past][i-1], drc[pres][i-1],drc[fut][i-1]);
	}

/*  If bit 3 of ICORF is set then smooth pitch	*/

	if ((icorf & abit[2]) != 0) {
	   if(abs(dpit[pres]-dpit[fut]) >= corth[ixcor-1][0]
	     &&abs(dpit[pres]-dpit[past])>= corth[ixcor-1][0])
		*pitch = median( dpit[past], dpit[pres], dpit[fut] );
	}

/*  If bit 5 of ICORF is set then RC(5) - RC(10) are loaded with
*  values so that after quantization bias is removed in decode
*  the values will be zero.	*/
} /* end if first */
	if((icorf&abit[4])!=0) 
		for(i=5;i<=ORDER;i++)
			irc[i]= zrc[i-1];
	   
	

/*  House keeping  - one frame delay	*/

	iovoic = ivoic;
	ivp2h = voice[2];
	dpit[past] = dpit[pres];
	dpit[pres] = dpit[fut];
	drms[past] = drms[pres];
	drms[pres] = drms[fut];

	for(i=1;i<=ORDER;i++)	{
	   drc[past][i-1] = drc[pres][i-1];
	   drc[pres][i-1] = drc[fut][i-1];
	}

/*   Decode RMS */

	*irms = rmst[(31-*irms)*2];	

/*  Decode RC(1) and RC(2) from log-area-ratios
*  Protect from illegal coded value (-16) caused by bit errors	*/

	for(i=1;i<=2;i++)	{
	   i2 = irc[i];
	   i1 = 0;
	   if(i2<0) {
	      i1 = 1;
	      i2 = -i2;
	      if(i2>15) i2 = 0;
	   }
	   i2 = detab7[2*i2];
	   if(i1==1) i2 = -i2;
	   ishift = 15 - nbit[i-1];
/*	   irc[i]= i2*pow(2.0,(float)ishift); */
	   irc[i] = i2*(2<<(ishift-1));
	}

/*  Decode RC(3)-RC(10) to sign plus 14 bits	*/

	for(i=3;i<=ORDER;i++)	{
	   i2 = irc[i];
	   ishift = 15 - nbit[i-1];
/*	   i2 = i2*pow(2.0,(float)ishift);  */
	   i2 = i2*(2<<(ishift-1));
	   i2 += qb[i-3];
	   ftemp = i2*descl[i-3] + deadd[i-3];	/* rounding problem */
	   if(ftemp < 0)	{
		ftemp = - ftemp;
		itemp = (int) ftemp;
		irc[i] = -itemp;
	   }
	   else
		irc[i] = (int) ftemp;
	}

/*  Scale RMS and RC's to reals */

	*rms = (float) (*irms);
	for(i=1;i<=ORDER;i++)
	   rc[i]= (float)(irc[i]* 6.103515625e-05);  /* 16,384 = 2**14	*/

}

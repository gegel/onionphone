/************************************************************************
*
*	CHANL Version 49
*
************************************************************************
*
* CHANWR:
*   Place quantized parameters into bitstream
*
*   Inputs:
*    ORDER  - Number of reflection coefficients (not really variable)
*    IPITV  - Quantized pitch/voicing parameter
*    IRMS   - Quantized energy parameter
*    IRC    - Quantized reflection coefficients
*   Output:
*    IBITS  - Serial bitstream
*
* CHANRD:
*   Reconstruct parameters from bitstream
*
*   Inputs:
*    ORDER  - Number of reflection coefficients (not really variable)
*    IBITS  - Serial bitstream
*   Outputs:
*    IPITV  - Quantized pitch/voicing parameter
*    IRMS   - Quantized energy parameter
*    IRC    - Quantized reflection coefficients
*
*   IBITS is 54 bits of LPC data ordered as follows:
*	R1-0, R2-0, R3-0,  P-0,  A-0,
*	R1-1, R2-1, R3-1,  P-1,  A-1,
*	R1-2, R4-0, R3-2,  A-2,  P-2, R4-1,
*	R1-3, R2-2, R3-3, R4-2,  A-3,
*	R1-4, R2-3, R3-4, R4-3,  A-4,
*	 P-3, R2-4, R7-0, R8-0,  P-4, R4-4,
*	R5-0, R6-0, R7-1,R10-0, R8-1,
*	R5-1, R6-1, R7-2, R9-0,  P-5,
*	R5-2, R6-2,R10-1, R8-2,  P-6, R9-1,
*	R5-3, R6-3, R7-3, R9-2, R8-3, SYNC
*/

#include "lpcdefs.h"

int bit[10] = {
 2, 4, 8, 8, 8, 8, 16, 16, 16, 16 
};

int iblist[53] = {
13, 12, 11, 1, 2, 13, 12, 11, 1, 2, 13, 10,
11, 2, 1, 10, 13, 12, 11, 10, 2, 13, 12, 11,
10, 2, 1, 12, 7, 6, 1, 10, 9, 8, 7, 4,
6, 9, 8, 7, 5, 1, 9, 8, 4, 6, 1, 5,
9, 8, 7, 5, 6 
};

void channel(int which, int *ipitv, int *irms, int irc[ORDER], int ibits[54])
{
int i;
static int isync;
int itab[13];

switch(which) {
case 0: /*chanwr*/
/************************************************************************
*	Place quantized parameters into bitstream
************************************************************************

*   Place parameters into ITAB	*/

itab[0] = *ipitv;
itab[1] = *irms;
itab[2] = 0;
for(i=1;i<=ORDER;i++)
	itab[i+2] = irc[ORDER+1-i] & 32767 ;

/*   Put 54 bits into IBITS array	*/

for(i=1;i<=53;i++)	{
	ibits[i] = itab[iblist[i-1]-1] & 1;
	itab[iblist[i-1]-1] = itab[iblist[i-1]-1] >> 1;
}
ibits[54] = isync&1;
isync = 1 - isync;

break;

/************************************************************************
*	Reconstruct parameters from bitstream
*************************************************************************/
case 1: /*chanwr*/

/*   Reconstruct ITAB	*/

for(i=0;i<13;i++)
	itab[i] = 0;

for(i=1;i<=53;i++)
	itab[iblist[53-i]-1] = itab[iblist[53-i]-1]*2 + ibits[54-i];


/*   Sign extend RC's   */

for(i=1;i<=ORDER;i++)
	if( (itab[i+2] & bit[i-1])  != 0 ) itab[i+2] -= 2*bit[i-1];

/*   Restore variables	*/

*ipitv = itab[0];
*irms = itab[1];
for(i=1;i<=ORDER;i++)
	irc[i] = itab[ORDER+3-i];

break;
}

}

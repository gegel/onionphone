/* SpeedEdit 32,40,25,0,0,10,16,10 Updated 02/04/93 15:07:36 */
/************************************************************************
*
*	BITIO Version 55
*
************************************************************************/
#include <stdio.h>
#include "lpcdefs.h"

#define STRLEN 80

static char hex[23]={"0123456789ABCDEFabcdef "};

int bitio(which, fd, ibits, n)
int which, n, ibits[];
FILE *fd;
{
char str[STRLEN];
int bitsrd, bitswr, return_value, i;

switch(which)	{
/************************************************************************
*   Read a frame from bitstream file
************************************************************************/
  case 1:
	if(fscanf(fd, "%s",str) != EOF )	
	{
	  bitsrd = gethx(str, ibits, n);
	  return_value = bitsrd;
	}
	else
	  return_value = 0;
        break;

/************************************************************************
*   Write a frame to bitstream file
************************************************************************/
  case 2:
	bitswr = puthx(str, ibits, n);
	for(i=0;i<(n+3)/4;i++)
	  fprintf(fd, "%c",str[i]);
	fprintf(fd, "\n");
  break;
}
return return_value;
}


/************************************************************************
*   Read bits from hex digit stream
************************************************************************
*
*   Skip leading blanks, split hex digits into individual bits,
*  terminate after getting n bits or finding non-hex character.
*  Return value = number of bits in input record (which could be
*  more or less than n).
*/

int gethx(str, ibits, n)
char *str;
int ibits[], n;
{
int gethx, puthx;
int ib, ic, i, ii, j, k, nc;
static int first=1;

	/*if(first)	{
	  first=0;
	  strcpy(hex, "0123456789ABCDEFabcdef ");
	}*/

	/* count the number of valid characters in str */

	ic = 0;
	for( j = 0;j< strlen(str);j++)	{
	    k = the_index(hex, str[j]);
	    if ((k < 0) || ((k > 21) && (ic > 0))) break;
	    if (k<=21) ic++;
	}


	ib = -1;
	j = j - ic;
	nc = mmin((n+3)/4, ic);
	for( i = 0;i<= nc-1;i++)	{
	    k = the_index(hex, str[i+j]);
	    if ((k < 0) || (k > 21)) 
		printf("stop 'gethx: internal error'\n");
	    if (k > 15) k -= 6;
	    for( ii = 1 + mmax(0, 4*(nc-i)-n);ii<= 4;ii++)	{
	        ib++;
	        /*ibits(ib) = and(ishft(k, ii-4), 1)*/
	        ibits[ib] = (k >> (4-ii)) & 1;
	    }
	}

	gethx = ib;
	if (ic > nc) gethx = 4*ic;
	
	return gethx;
}

/************************************************************************
*   Write bits to hex digit stream
************************************************************************/

int puthx(str, ibits, n)
char str[STRLEN];
int ibits[], n;
{
int ib, nc, ic, k, j;

	ib = -1;
	nc = (n+3) / 4;
	for (ic = 1;ic<= mmin(STRLEN, nc);ic++)	{
	    k = 0;
	    for (j = 1; j<= mmin(n-4*(nc-ic), 4); j++)	{
	        ib++;
	        /*k = or(ishft(k,1), and(ibits(ib),1))*/
	        k = (k<<1) | (ibits[ib] & 1);
	    }
	    str[ic-1] = hex[k];
	}

	return ib;
}





int the_index(string,character)
char string[],character;
{
int i;

for(i=0;i<strlen(string);i++)
	if(string[i] == character)
		break;

return(i);

}

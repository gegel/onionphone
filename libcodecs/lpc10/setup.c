/***********************************************************************
*
*	SETUP Version 55 for Real Time Operation at multiple bit rates
*
*************************************************************************
*
*	Set processing options
*/

#include "lpcdefs.h"
#include "common.h"
#include "contrl.ch"
#include <stdlib.h>

void setup(int nargs, char *args[])
{
/* 
	Call to getcl() gets name of input and output file and message level.
Real time doesn't need messages about performance, etc, so those will be 
eliminated.
*/
if (nargs < 3) {
  printf("Usage: %s inputfile outputfile\n", args[0]);
  exit(1);
}
else {
  fdi = fopen(args[1], "rb");
  if(fdi==NULL) {
    printf("Problem opening [3] %s ... program exiting\n",args[1]);
    exit(1);
  }


  fdo = fopen(args[2], "w");
  if(fdo == NULL)	{
    printf("Problem opening [4] ... program exiting %s\n",args[2]);
    exit(1);
  }
}
}

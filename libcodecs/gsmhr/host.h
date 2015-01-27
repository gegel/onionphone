/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#ifndef __HOSTGSM
#define __HOSTGSM

#include <stdio.h>
#include "typedefs.h"

/*_________________________________________________________________________
 |                                                                         |
 |                           Function Prototypes                           |
 |_________________________________________________________________________|
*/

void fillBitAlloc(int iVoicing, int iR0, int *piVqIndeces,
		  int iSoftInterp, int *piLags,
		  int *piCodeWrdsA, int *piCodeWrdsB,
		  int *piGsp0s, int16_t swVadFlag,
		  int16_t swSP, int16_t * pswBAlloc);

#endif

/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/***************************************************************************
 *
 *   File Name:  host.c
 *
 *   Purpose:  Contains functions for file I/O and formatting, no signal
 *      processing.
 *
 *      The functions in this file are listed below.  All are level 2
 *      fuctions, where level 0 is main(), except for fillBitAlloc() which
 *      is level 3.  The two "Interface" routines perform truncation of the
 *      three least significant bits of the 16 bit linear input.  The others
 *      are simply file I/O functions and data reformatters.
 *
 *      fillBitAlloc()
 *      readDecfile()
 *      speechDecoderHostInterface()
 *      writeEncfile()
 *
 **************************************************************************/

/*_________________________________________________________________________
 |                                                                         |
 |                            Include Files                                |
 |_________________________________________________________________________|
*/

#include <stdio.h>
#include "typedefs.h"

/***************************************************************************
 *
 *   FUNCTION NAME: fillBitAlloc
 *
 *   PURPOSE:
 *
 *     Arrange speech parameters for encoder output
 *
 *   INPUTS:
 *
 *     The speechcoders codewords:
 *     iR0 - Frame energy
 *     piVqIndeces[0:2] - LPC vector quantizer codewords
 *     iSoftInterp - Soft interpolation bit 1 or 0
 *     iVoicing - voicing mode 0,1,2,3
 *     piLags[0:3] - Frame and delta lag codewords
 *     piCodeWrdsA[0:3] - VSELP codevector 1
 *     piCodeWrdsB[0:3] - VSELP codevector 2 (n/a for voiced modes)
 *     piGsp0s[0:3] - GSP0 codewords
 *     swVadFlag - voice activity detection flag
 *     swSP - Speech flag
 *
 *   OUTPUTS:
 *
 *     pswBAlloc[0:20] - an array into which the coded data is moved
 *
 *   RETURN VALUE:
 *
 *     none
 *
 *   REFERENCES: Sub-clause 2.1 and 4.1.12 of GSM Recomendation 06.20
 *
 **************************************************************************/

void fillBitAlloc(int iVoicing, int iR0, int *piVqIndeces,
		  int iSoftInterp, int *piLags,
		  int *piCodeWrdsA, int *piCodeWrdsB,
		  int *piGsp0s, int16_t swVadFlag,
		  int16_t swSP, int16_t * pswBAlloc)
{

/*_________________________________________________________________________
 |                                                                         |
 |                            Automatic Variables                          |
 |_________________________________________________________________________|
*/

	int i;
	int16_t *pswNxt;

/*_________________________________________________________________________
 |                                                                         |
 |                            Executable Code                              |
 |_________________________________________________________________________|
*/

	pswNxt = pswBAlloc;
	*pswNxt++ = iR0;
	for (i = 0; i < 3; i++)
		*pswNxt++ = *piVqIndeces++;
	*pswNxt++ = iSoftInterp;
	*pswNxt++ = iVoicing;

	/* check voicing mode */
	if (iVoicing) {
		/* voiced mode */
		for (i = 0; i < N_SUB; i++) {
			*pswNxt++ = *piLags++;
			*pswNxt++ = *piCodeWrdsA++;
			*pswNxt++ = *piGsp0s++;
		}
	} else {		/* unvoiced frame */
		for (i = 0; i < N_SUB; i++) {
			*pswNxt++ = *piCodeWrdsA++;
			*pswNxt++ = *piCodeWrdsB++;
			*pswNxt++ = *piGsp0s++;
		}
	}
	*pswNxt++ = swVadFlag;
	*pswNxt++ = swSP;
}


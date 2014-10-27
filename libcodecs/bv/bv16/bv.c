/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*****************************************************************************/
/* BroadVoice(R)16 (BV16) Fixed-Point ANSI-C Source Code                     */
/* Revision Date: November 13, 2009                                          */
/* Version 1.1                                                               */
/*****************************************************************************/

/*****************************************************************************/
/* Copyright 2000-2009 Broadcom Corporation                                  */
/*                                                                           */
/* This software is provided under the GNU Lesser General Public License,    */
/* version 2.1, as published by the Free Software Foundation ("LGPL").       */
/* This program is distributed in the hope that it will be useful, but       */
/* WITHOUT ANY SUPPORT OR WARRANTY; without even the implied warranty of     */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LGPL for     */
/* more details.  A copy of the LGPL is available at                         */
/* http://www.broadcom.com/licenses/LGPLv2.1.php,                            */
/* or by writing to the Free Software Foundation, Inc.,                      */
/* 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                 */
/*****************************************************************************/

/*****************************************************************************
  bv.c : BroadVoice16 Codec Entry Interface Program

  $Log$
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "bvcommon.h"
#include "bv16cnst.h"
#include "bv16strct.h"
#include "bv16.h"
#include "utility.h"
#if G192BITSTREAM
#include "g192.h"
#else
#include "bitpack.h"
#endif

int frame;
int16_t bfi = 0;

void usage(char *name)
{
	fprintf(stderr, "usage: %s enc|dec input output\n", name);
	fprintf(stderr,
		"\nFormat for speech_file:\n    Binary file of 8 kHz sampled 16-bit PCM data.\n");
#if G192BITSTREAM
	fprintf(stderr,
		"\nFormat for bitstream_file per frame: ITU-T G.192 format\n\
                                                                                One (2-byte) synchronization word [0x6B21],\n\
                                                                                One (2-byte) size word,\n\
                                                                                160 words (2-byte) containing 160 bits.\n\n");
#else
	fprintf(stderr, "\nFormat for bitstream_file per frame: Packed Bits\n");
#endif
	exit(1);
}

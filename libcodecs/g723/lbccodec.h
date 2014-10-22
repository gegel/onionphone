/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*
**
** File:        "lbccodec.h"
**
** Description:     Function prototypes and external declarations
**          for "lbccodec.c"
**
*/

/*
	ITU-T G.723.1 Software Package Release 2 (June 2006)

    ITU-T G.723.1 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

//init codec: rate: 0-Rate63, 1-Rate53
//dtx 1-enable (produce 4 bytes frames of silency)
void g723_i(int rate, int dtx);

//encode 240 short linear samples (30 mS) 
//to 20/24 bytes or 4 bytes silency
//return 0(Rate63), 1(Rate53), 2(Silency), 3(Error)
int g723_e(short *sp, unsigned char *buf);

//decode 20/24 bytes or 4 bytes silency to 240 short linear samples (30 mS)
void g723_d(unsigned char *buf, short *sp);

/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*
**
** File:        "lbccodec.c"
**
** Description:     Top-level source code for G.723.1 dual-rate codec
**
** Functions:       
**                 
**
**
*/

/*
  ITU-T G.723.1 Software Package Release 2 (June 2006)
    
  ITU-T G.723.1 Speech Coder   ANSI-C Source Code     Version 5.2
  copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
  Universite de Sherbrooke.  All rights reserved.
  Last modified : March 2006
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

//begin-----------------------------------add by haiping 2009-06-17
#define LIMIT 1
//end  -----------------------------------add by haiping 2009-06-17

#include "g723_const.h"
#include "lbccodec.h"

extern void Init_Vad(void);
extern void Init_Coder(void);
extern void Init_Decod(void);
extern void Init_Cod_Cng(void);
extern void Init_Dec_Cng(void);
extern int Coder(int16_t * DataBuff, char *Vout);
extern int Decod(int16_t * DataBuff, char *Vinp, int16_t Crc);
extern void reset_max_time(void);

unsigned int before = 0;
unsigned int temp;
unsigned int max_diff[10];

//end ------------------------------------add by haiping 2009-06-17
/* Global variables */
enum Wmode WrkMode = Both;
enum Crate WrkRate = Rate63;

int PackedFrameSize[2] = {
	24,
	20
};

int UseHp = True;
int UsePf = True;
int UseVx = False;
int UsePr = True;

void g723_i(int rate, int dtx)
{

	WrkMode = Both;
	if (!rate)
		WrkRate = Rate63;
	else
		WrkRate = Rate53;
	if (dtx)
		UseVx = True;
	else
		UseVx = False;

	/*
	   Init coder and the decoder
	 */
	Init_Coder();
	Init_Decod();

	/* Init Comfort Noise Functions */
	if (UseVx) {
		Init_Vad();
		Init_Cod_Cng();
	}
	Init_Dec_Cng();

}

//encode 240 short linear samples (30 mS) 
//to 20/24 bytes or 4 bytes silency descriptor
//return packet length: 24(Rate63), 20(Rate53), 4(Noise), 0(Iddle)
int g723_e(short *sp, unsigned char *buf)
{
	if (WrkRate == Rate53)
		reset_max_time();
	Coder(sp, (char *)buf);
	//return (*buf)&3;

	//type 0(Rate63), 1(Rate53), 2(Noise), 3(Iddle)
	switch ((*buf) & 3) {
	case 0:
		{
			return 24;
		}
	case 1:
		{
			return 20;
		}
	case 2:
		{
			return 4;
		}
	}
	return 0;
}

//decode 20/24 bytes or 4 bytes silency to 240 short linear samples (30 mS)
void g723_d(unsigned char *buf, short *sp)
{
	Decod(sp, (char *)buf, (int16_t) 0);
}

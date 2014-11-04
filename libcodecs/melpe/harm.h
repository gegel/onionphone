/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/* ================================================================== */
/*                                                                    */
/*    Microsoft Speech coder     ANSI-C Source Code                   */
/*    SC1200 1200 bps speech coder                                    */
/*    Fixed Point Implementation      Version 7.0                     */
/*    Copyright (C) 2000, Microsoft Corp.                             */
/*    All rights reserved.                                            */
/*                                                                    */
/* ================================================================== */

#ifndef _HARM_H_
#define _HARM_H_

void set_fc(int16_t bpvc[], int16_t * fc);

void harm_syn_pitch(int16_t amp[], int16_t signal[], int16_t fc,
		    int16_t length);

#endif

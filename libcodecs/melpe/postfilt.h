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

/*------------------------------------------------------------------*/
/*																	*/
/* File:		"postfilt.h"										*/
/*																	*/
/* Description: 	header filt for postfilter and postprocessing	*/
/*																	*/
/*------------------------------------------------------------------*/

#ifndef _POSTFILT_H_
#define _POSTFILT_H_

/* ========== Prototypes ========== */
void postfilt(int16_t syn[], int16_t prev_lsf[], int16_t cur_lsf[]);

#endif

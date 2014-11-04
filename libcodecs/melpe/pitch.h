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
/* File:		pitch.h												*/
/*																	*/
/* Description: new pitch estimation routines						*/
/*																	*/
/*------------------------------------------------------------------*/

#ifndef _PITCH_H_
#define _PITCH_H_

void pitchAuto(int16_t inbuf[], pitTrackParam * pitTrack,
	       classParam * classStat);

int16_t multiCheck(int16_t f1, int16_t f2);

int16_t trackPitch(int16_t pitch, pitTrackParam * pitTrack);

int16_t pitLookahead(pitTrackParam * pitTrack, int16_t num);

int16_t ratio(int16_t x, int16_t y);

int16_t L_ratio(int16_t x, int32_t y);

int16_t updateEn(int16_t prev_en, int16_t ifact, int16_t curr_en);

#endif

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


void	pitchAuto(Shortword inbuf[], pitTrackParam *pitTrack,
				  classParam *classStat);

Shortword	multiCheck(Shortword f1, Shortword f2);

Shortword	trackPitch(Shortword pitch, pitTrackParam *pitTrack);

Shortword	pitLookahead(pitTrackParam *pitTrack, Shortword num);

Shortword	ratio(Shortword x, Shortword y);

Shortword	L_ratio(Shortword x, Longword y);

Shortword	updateEn(Shortword prev_en, Shortword ifact, Shortword curr_en);


#endif


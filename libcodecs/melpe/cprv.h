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
/* File:		cprv.h												*/
/*																	*/
/* Description: Head file for classification, pitch and voicing		*/
/*																	*/
/*------------------------------------------------------------------*/

#ifndef _CPRV_H_
#define _CPRV_H_

/* ================================================================ *
 *					Definitions										*
 * ================================================================ */
#define EN_UP_RATE_Q15		29491	/* 0.9 * (1 << 15) */
#define	TRACK_NUM			9
#define	CUR_TRACK			2

/* ================================================================ *
 *					Structures										*
 * ================================================================ */

/* ======== Pitch estimation structures ======== */
typedef struct {
	int16_t pit[NODE];	/* integer pitch for each node, Q7 */
	int16_t weight[NODE];	/* time domain correlation, Q15 */
	int16_t cost[NODE];	/* cost function, Q0 */
} pitTrackParam;

typedef struct {
	int16_t classy;	/* the class */
	int16_t subEnergy;	/* full band energy, Q11 */
	int16_t zeroCrosRate;	/* zero crossing rate, Q15 */
	int16_t peakiness;	/* peakiness measure, Q11 */
	int16_t corx;		/* autocorrelation, Q15 */
	int16_t pitch;	/* pitch period, Q0 */
} classParam;

/* ============================ */
/* Prototypes from "classify.c"	*/
/* ============================ */
void classify(int16_t inbuf[], classParam * classStat, int16_t autocorr[]);

#endif

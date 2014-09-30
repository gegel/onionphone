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
#define EN_UP_RATE_Q15		29491                          /* 0.9 * (1 << 15) */
#define	TRACK_NUM			9
#define	CUR_TRACK			2

/* ================================================================ *
 *					Structures										*
 * ================================================================ */

/* ======== Pitch estimation structures ======== */
typedef struct{
	Shortword	pit[NODE];                 /* integer pitch for each node, Q7 */
	Shortword	weight[NODE];                 /* time domain correlation, Q15 */
	Shortword	cost[NODE];                              /* cost function, Q0 */
} pitTrackParam;

typedef struct {
	Shortword	classy;                                          /* the class */
	Shortword	subEnergy;                           /* full band energy, Q11 */
	Shortword	zeroCrosRate;                      /* zero crossing rate, Q15 */
	Shortword	peakiness;                          /* peakiness measure, Q11 */
	Shortword	corx;                                 /* autocorrelation, Q15 */
	Shortword	pitch;                                    /* pitch period, Q0 */
} classParam;

/* ============================ */
/* Prototypes from "classify.c"	*/
/* ============================ */
void	classify(Shortword inbuf[], classParam *classStat,
				 Shortword autocorr[]);


#endif


/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/*$Header: /cvsroot/speak-freely-u/speak_freely/gsm/inc/private.h,v 1.1.1.1 2002/11/09 12:41:01 johnwalker Exp $*/

#ifndef	PRIVATE_H
#define	PRIVATE_H

#include <stdint.h>

struct gsm_state {

	int16_t dp0[280];

	int16_t z1;		/* preprocessing.c, Offset_com. */
	int32_t L_z2;		/*                  Offset_com. */
	int mp;			/*                  Preemphasis */

	int16_t u[8];		/* short_term_aly_filter.c      */
	int16_t LARpp[2][8];	/*                              */
	int16_t j;			/*                              */

	int16_t nrp;		/* 40 *//* long_term.c, synthesis       */
	int16_t v[9];		/* short_term.c, synthesis      */
	int16_t msr;		/* decoder.c,   Postprocessing  */

	char verbose;		/* only used if !NDEBUG         */
	char fast;		/* only used if FAST            */

};

#define	MIN_WORD	(-32768)
#define	MAX_WORD	( 32767)

#define	MIN_LONGWORD	((-2147483647)-1)
#define	MAX_LONGWORD	( 2147483647)

#ifdef	SASR			/* >> is a signed arithmetic shift right */
#undef	SASR
#define	SASR(x, by)	((x) >> (by))
#endif				/* SASR */

#include "proto.h"

/*
 *	Prototypes from add.c
 */
extern int16_t gsm_mult P((int16_t a, int16_t b));
extern int32_t gsm_L_mult P((int16_t a, int16_t b));
extern int16_t gsm_mult_r P((int16_t a, int16_t b));

extern int16_t gsm_div P((int16_t num, int16_t denum));

extern int16_t gsm_add P((int16_t a, int16_t b));
extern int32_t gsm_L_add P((int32_t a, int32_t b));

extern int16_t gsm_sub P((int16_t a, int16_t b));
extern int32_t gsm_L_sub P((int32_t a, int32_t b));

extern int16_t gsm_abs P((int16_t a));

extern int16_t gsm_norm P((int32_t a));

extern int32_t gsm_L_asl P((int32_t a, int n));
extern int16_t gsm_asl P((int16_t a, int n));

extern int32_t gsm_L_asr P((int32_t a, int n));
extern int16_t gsm_asr P((int16_t a, int n));

/*
 *  Inlined functions from add.h 
 */

/* 
 * #define GSM_MULT_R(a, b) (* int16_t a, int16_t b, !(a == b == MIN_WORD) *)	\
 *	(0x0FFFF & SASR(((int32_t)(a) * (int32_t)(b) + 16384), 15))
 */
#define GSM_MULT_R(a, b) /* int16_t a, int16_t b, !(a == b == MIN_WORD) */	\
	(SASR( ((int32_t)(a) * (int32_t)(b) + 16384), 15 ))

#define GSM_MULT(a,b)	 /* int16_t a, int16_t b, !(a == b == MIN_WORD) */	\
	(SASR( ((int32_t)(a) * (int32_t)(b)), 15 ))

#define GSM_L_MULT(a, b) /* int16_t a, int16_t b */	\
	(((int32_t)(a) * (int32_t)(b)) << 1)

static inline int32_t GSM_L_ADD(int32_t a, int32_t b) __attribute__((always_inline));
static inline int32_t GSM_L_ADD(int32_t a, int32_t b)
{
	uint32_t A;

	if ((a < 0 && b >= 0) || (a >= 0 && b <= 0))
		return (a + b);

	if (a < 0)
	{
		A = (uint32_t)(-(a + 1)) + (uint32_t)(-(b + 1));
		return A >= MAX_LONGWORD ? MIN_LONGWORD : (-(int32_t)(A - 2));
	} else
	{
		A = (uint32_t)a + (uint32_t)b;
		return A > MAX_LONGWORD ? MAX_LONGWORD : A;
	}
}

/*
 * # define GSM_ADD(a, b)	\
 * 	((ltmp = (int32_t)(a) + (int32_t)(b)) >= MAX_WORD \
 * 	? MAX_WORD : ltmp <= MIN_WORD ? MIN_WORD : ltmp)
 */
/* Nonportable, but faster: */

#define	GSM_ADD(a, b)	\
	((unsigned)((ltmp = (int32_t)(a) + (int32_t)(b)) - MIN_WORD) > \
		MAX_WORD - MIN_WORD ? (ltmp > 0 ? MAX_WORD : MIN_WORD) : ltmp)

#define GSM_SUB(a, b)	\
	((ltmp = (int32_t)(a) - (int32_t)(b)) >= MAX_WORD \
	? MAX_WORD : ltmp <= MIN_WORD ? MIN_WORD : ltmp)

#define GSM_ABS(a)	((a) < 0 ? ((a) == MIN_WORD ? MAX_WORD : -(a)) : (a))

/* Use these if necessary:

# define GSM_MULT_R(a, b)	gsm_mult_r(a, b)
# define GSM_MULT(a, b)		gsm_mult(a, b)
# define GSM_L_MULT(a, b)	gsm_L_mult(a, b)

# define GSM_L_ADD(a, b)	gsm_L_add(a, b)
# define GSM_ADD(a, b)		gsm_add(a, b)

# define GSM_ABS(a)		gsm_abs(a)

*/

/*
 *  More prototypes from implementations..
 */
extern void Gsm_Coder P((struct gsm_state * S, int16_t * s,	/* [0..159] samples             IN      */
			 int16_t * LARc,	/* [0..7] LAR coefficients      OUT     */
			 int16_t * Nc,	/* [0..3] LTP lag               OUT     */
			 int16_t * bc,	/* [0..3] coded LTP gain        OUT     */
			 int16_t * Mc,	/* [0..3] RPE grid selection    OUT     */
			 int16_t * xmaxc,	/* [0..3] Coded maximum amplitude OUT   */
			 int16_t *
			 xMc /* [13*4] normalized RPE samples OUT    */ ));

extern void Gsm_Long_Term_Predictor P((	/* 4x for 160 samples */
					      struct gsm_state * S, int16_t * d,	/* [0..39]   residual signal    IN      */
					      int16_t * dp,	/* [-120..-1] d'                IN      */
					      int16_t * e,	/* [0..40]                      OUT     */
					      int16_t * dpp,	/* [0..40]                      OUT     */
					      int16_t * Nc,	/* correlation lag              OUT     */
					      int16_t *
					      bc
					      /* gain factor                  OUT     */
					      ));

extern void Gsm_LPC_Analysis P((struct gsm_state * S, int16_t * s,	/* 0..159 signals      IN/OUT  */
				int16_t * LARc));	/* 0..7   LARc's       OUT     */

extern void Gsm_Preprocess P((struct gsm_state * S, int16_t * s, int16_t * so));

extern void Gsm_Encoding P((struct gsm_state * S,
			    int16_t * e,
			    int16_t * ep, int16_t * xmaxc, int16_t * Mc, int16_t * xMc));

extern void Gsm_Short_Term_Analysis_Filter P((struct gsm_state * S, int16_t * LARc,	/* coded log area ratio [0..7]  IN      */
					      int16_t *
					      d
					      /* st res. signal [0..159]      IN/OUT  */
					      ));

extern void Gsm_Decoder P((struct gsm_state * S, int16_t * LARcr,	/* [0..7]               IN      */
			   int16_t * Ncr,	/* [0..3]               IN      */
			   int16_t * bcr,	/* [0..3]               IN      */
			   int16_t * Mcr,	/* [0..3]               IN      */
			   int16_t * xmaxcr,	/* [0..3]               IN      */
			   int16_t * xMcr,	/* [0..13*4]            IN      */
			   int16_t * s));	/* [0..159]             OUT     */

extern void Gsm_Decoding P((struct gsm_state * S, int16_t xmaxcr, int16_t Mcr, int16_t * xMcr,	/* [0..12]              IN      */
			    int16_t * erp));	/* [0..39]              OUT     */

extern void Gsm_Long_Term_Synthesis_Filtering P((struct gsm_state * S, int16_t Ncr, int16_t bcr, int16_t * erp,	/* [0..39]                IN    */
						 int16_t * drp));	/* [-120..-1] IN, [0..40] OUT   */

extern void Gsm_Short_Term_Synthesis_Filter P((struct gsm_state * S, int16_t * LARcr,	/* log area ratios [0..7]  IN   */
					       int16_t * drp,	/* received d [0...39]     IN   */
					       int16_t * s));	/* signal   s [0..159]    OUT   */

extern void Gsm_Update_of_reconstructed_short_time_residual_signal P((int16_t * dpp,	/* [0...39]     IN      */
								      int16_t * ep,	/* [0...39]     IN      */
								      int16_t * dp));	/* [-120...-1]  IN/OUT  */

extern void Gsm_RPE_Encoding P((struct gsm_state * S, int16_t * e,	/* -5..-1][0..39][40..44        IN/OUT  */
				int16_t * xmaxc,	/*                              OUT */
				int16_t * Mc,	/*                              OUT */
				int16_t * xMc));	/* [0..12]                      OUT */

extern void Gsm_RPE_Decoding P((struct gsm_state * S, int16_t xmaxcr, int16_t Mcr, int16_t * xMcr,	/* [0..12], 3 bits             IN      */
				int16_t * erp));	/* [0..39]                     OUT     */

/*
 *  Tables from table.c
 */
#ifndef	GSM_TABLE_C

extern int16_t gsm_A[8], gsm_B[8], gsm_MIC[8], gsm_MAC[8];
extern int16_t gsm_INVA[8];
extern int16_t gsm_DLB[4], gsm_QLB[4];
extern int16_t gsm_H[11];
extern int16_t gsm_NRFAC[8];
extern int16_t gsm_FAC[8];

#endif				/* GSM_TABLE_C */

/*
 *  Debugging
 */
#ifdef NDEBUG

#	define	gsm_debug_int16_ts(a, b, c, d)	/* nil */
#	define	gsm_debug_int32_ts(a, b, c, d)	/* nil */
#	define	gsm_debug_int16_t(a, b)	/* nil */
#	define	gsm_debug_int32_t(a, b)	/* nil */

#else				/* !NDEBUG => DEBUG */

extern void gsm_debug_int16_ts P((char *name, int, int, int16_t *));
extern void gsm_debug_int32_ts P((char *name, int, int, int32_t *));
extern void gsm_debug_int32_t P((char *name, int32_t));
extern void gsm_debug_int16_t P((char *name, int16_t));

#endif				/* !NDEBUG */

#include "unproto.h"

#endif				/* PRIVATE_H */

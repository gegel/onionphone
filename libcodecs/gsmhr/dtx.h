/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#ifndef __DTX
#define __DTX

#include "typedefs.h"

#define PN_INIT_SEED (int32_t)0x1091988L	/* initial seed for Comfort
						 * noise pn-generator */

#define CNINTPER    12		/* inperpolation period of CN
				 * parameters */

#define SPEECH      1
#define CNIFIRSTSID 2
#define CNICONT     3
#define CNIBFI      4

#define VALIDSID    11
#define INVALIDSID  22
#define GOODSPEECH  33
#define UNUSABLE    44

/*________________________________________________________________________
 |                                                                        |
 |                      Function Prototypes                               |
 |________________________________________________________________________|
*/

void avgCNHist(int32_t pL_R0History[],
	       int32_t ppL_CorrHistory[OVERHANG][NP + 1],
	       int32_t * pL_AvgdR0, int32_t pL_AvgdCorrSeq[]);

void avgGsHistQntz(int32_t pL_GsHistory[], int32_t * pL_GsAvgd);

int16_t swComfortNoise(int16_t swVadFlag,
			 int32_t L_UnqntzdR0, int32_t * pL_UnqntzdCorr);

int16_t getPnBits(int iBits, int32_t * L_PnSeed);

int16_t gsQuant(int32_t L_GsIn, int16_t swVoicingMode);

void updateCNHist(int32_t L_UnqntzdR0,
		  int32_t * pL_UnqntzdCorr,
		  int32_t pL_R0Hist[],
		  int32_t ppL_CorrHist[OVERHANG][NP + 1]);

void lpcCorrQntz(int32_t pL_CorrelSeq[],
		 int16_t pswFinalRc[], int piVQCodewds[]);

int32_t linInterpSid(int32_t L_New, int32_t L_Old, int16_t swDtxState);

int16_t linInterpSidShort(int16_t swNew,
			    int16_t swOld, int16_t swDtxState);

void rxInterpR0Lpc(int16_t * pswOldKs, int16_t * pswNewKs,
		   int16_t swRxDTXState,
		   int16_t swDecoMode, int16_t swFrameType);

#endif

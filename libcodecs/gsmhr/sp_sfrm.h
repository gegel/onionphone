/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

#ifndef __SP_SFRM
#define __SP_SFRM

#include "typedefs.h"

/*_________________________________________________________________________
 |                                                                         |
 |                           Function Prototypes                           |
 |_________________________________________________________________________|
*/

int16_t g_corr2(int16_t * pswIn, int16_t * pswIn2, int32_t * pL_out);

int closedLoopLagSearch(int16_t pswLagList[],
			int iNumLags,
			int16_t pswLtpState[],
			int16_t pswHCoefs[],
			int16_t pswPVect[],
			int16_t * pswLag, int16_t * pswLtpShift);

void decorr(int iNumVects, int16_t pswGivenVect[], int16_t pswVects[]);

int16_t g_quant_vl(int16_t swUVCode,
		     int16_t pswWInput[],
		     int16_t swWIShift,
		     int16_t pswWLTPVec[],
		     int16_t pswWVSVec1[],
		     int16_t pswWVSVec2[],
		     struct NormSw snsRs00,
		     struct NormSw snsRs11, struct NormSw snsRs22);

void gainTweak(struct NormSw *psErrorTerm);

void hnwFilt(int16_t pswInSample[],
	     int16_t pswOutSample[],
	     int16_t pswState[],
	     int16_t pswInCoef[],
	     int iStateOffset, int16_t swZeroState, int iNumSamples);

void sfrmAnalysis(int16_t * pswWSpeech,
		  int16_t swVoicingMode,
		  struct NormSw snsSqrtRs,
		  int16_t * pswHCoefs,
		  int16_t * pswLagList,
		  short siNumLags,
		  int16_t swPitch,
		  int16_t swHNWCoef,
		  short *psiLagCode,
		  short *psiVSCode1,
		  short *psiVSCode2, short *psiGsp0Code, int16_t swSP);

int16_t v_srch(int16_t pswWInput[],
		 int16_t pswWBasisVecs[], short int siNumBasis);

#endif
